#include <cassert>
#include "lines.h"

namespace step2 {

std::vector<Expr> expr_list;
std::vector<Expr> global_init_list;
std::vector<Var> global_list;
std::vector<Var> local_list;
std::string param_list[8];

int stack_size, is_global, param_num, param_cnt;
int max_label = 0;

Expr::Expr(EXPR_T type, std::string op, std::string src1, std::string src2, std::string dst)
    : type(type), op(op), src1(src1), src2(src2), dst(dst) {}

Var::Var(std::string name, int array, int size, int offset) 
    : name(name), array(array), size(size), offset(offset) {}

int is_num(std::string str) {
    for (auto i : str) {
        if (!isdigit(i) && i!='-') return 0;
    }
    return 1;
}

void add_global(std::string name, int array, int size) {
    global_list.emplace_back(name, array, size, 0);
}

void add_local(std::string name, int array, int size) {
    int offset = 0;
    if (!local_list.empty()) {
        auto &var = local_list.back();
        offset = var.offset + var.size;
    }
    local_list.emplace_back(name, array, size, offset);
}

std::string find_var(std::string name) {
    if (is_num(name)) return name;
    for (int i=0; i<global_list.size(); i++) {
        if (global_list[i].name == name)
            return std::string("v") + std::to_string(i);
    }
    for (int i=0; i<local_list.size(); i++) {
        if (local_list[i].name == name)
            return std::string("w") + std::to_string(local_list[i].offset);
    }
    assert(name[0] == 'p');
    return std::string("a") + name.substr(1);
}

int is_array(std::string name) {
    if (is_num(name)) return 0;
    for (auto &i : global_list) {
        if (i.name == name)
            return i.array;
    }
    for (auto &i : local_list) {
        if (i.name == name)
            return i.array;
    }
    assert(name[0] == 'a');
    return 0;
}

void new_expr(EXPR_T type, const char *op, const char *src1, const char *src2, const char *dst) {
    expr_list.emplace_back(type, op, src1, src2, dst);
    if (type == expr_plab) {
        int num = atoi(op + 1);
        if (num > max_label) max_label = num;
    }
}

std::string load_value_or_addr(std::string name, IR_Context &ctx, std::ostream &out, int keep_num, int load_param) {
    if (name[0] == 'a' || !is_array(name)) {
        return ctx.load_reg(name, out, keep_num, load_param);
    }
    return ctx.alloc_addr_reg(name, out, keep_num, load_param);
}

int ir_expr(int index, IR_Context &ctx, std::ostream &out) {
    Expr &expr = expr_list[index];
    // printf("type:%d, op:%s, src1:%s, src2:%s, dst:%s\n", expr.type, expr.op.c_str(), expr.src1.c_str(), expr.src2.c_str(), expr.dst.c_str());
    std::string name, op, src1, src2, dst;
    int num, next_index, addr;
    std::ostringstream sout;
    switch (expr.type) {
        case expr_func:
            stack_size = 0;
            is_global = 0;
            param_cnt = atoi(expr.src1.c_str());
            local_list.clear();

            if (expr.op == "main") {
                for (auto &i : global_init_list) {
                    op = load_value_or_addr(i.op, ctx, sout, 1); ctx.lock_reg(op);
                    src1 = ctx.alloc_reg(i.src1, sout); ctx.release_reg();
                    ctx.remove_num_reg();

                    sout << src1 << " = " << op << std::endl;
                }
            }

            next_index = index + 1;
            while (expr_list[next_index].type != expr_ends)
                next_index = ir_expr(next_index, ctx, sout);
            ctx.save_reg(sout);
            
            out << "f_" << expr.op << " [" << expr.src1 << "] [" << std::to_string(param_cnt + stack_size) << "]" << std::endl;
            out << sout.str();
            out << "end f_" << expr.op << std::endl;

            is_global = 1;
            return next_index;

        case expr_decl:
            if (is_global) {
                name = expr.op;
                if (expr.src1 == "") {
                    add_global(expr.op, 0, 1);
                    out << name << " = 0" << std::endl;
                }
                else {
                    num = atoi(expr.src1.c_str()) / 4;
                    add_global(expr.op, 1, num);
                    out << name << " = malloc " << std::to_string(num * 4) << std::endl;
                }
            }
            else {
                if (expr.src1 == "") {
                    add_local(expr.op, 0, 1);
                    stack_size += 1;
                }
                else {
                    num = atoi(expr.src1.c_str()) / 4;
                    add_local(expr.op, 1, num);

                    // loadaddr expr.op t6
                    // op = t6 + num * 4
                    // l1:
                    //      if t6 == op goto l2
                    //      t6[0] = x0
                    //      t6 = t6 + 4
                    //      goto l1
                    // l2:
                    out << "loadaddr " << atoi(expr.op.substr(1).c_str()) + param_cnt << " t6" << std::endl;
                    op = ctx.free_reg(out);
                    out << op << " = t6 + " << num * 4 << std::endl;
                    out << "l" << max_label + 1 << ":" << std::endl;
                    out << "if t6 == " << op << " goto l" << max_label + 2 << std::endl;
                    out << "t6[0] = x0" << std::endl;
                    out << "t6 = t6 + 4" << std::endl;
                    out << "goto l" << max_label + 1 << std::endl;
                    out << "l" << max_label + 2 << ":" << std::endl;

                    max_label += 2;
                    stack_size += num;
                }
            }
            break;

        case expr_ope1:
            // dst = op src1
            src1 = ctx.load_reg(expr.src1, out); ctx.lock_reg(src1);
            dst = ctx.alloc_reg(expr.dst, out); ctx.release_reg();
            ctx.remove_num_reg();

            out << dst << " = " << expr.op << " " << src1 << std::endl;
            break;

        case expr_ope2:
            // dst = src1 op src2
            src1 = load_value_or_addr(expr.src1, ctx, out); ctx.lock_reg(src1);
            src2 = load_value_or_addr(expr.src2, ctx, out); ctx.lock_reg(src2);
            dst = ctx.alloc_reg(expr.dst, out); ctx.release_reg();
            ctx.remove_num_reg();

            out << dst << " = " << src1 << " " << expr.op << " " << src2 << std::endl;
            break;

        case expr_assi:
            // src1 = op
            if (expr.src2[0] == '0') {
                if (is_global) {
                    global_init_list.push_back(expr);
                }
                else {
                    op = load_value_or_addr(expr.op, ctx, out, 1); ctx.lock_reg(op);
                    src1 = ctx.alloc_reg(expr.src1, out); ctx.release_reg();
                    ctx.remove_num_reg();

                    out << src1 << " = " << op << std::endl;
                }
            }
            // src1[dst] = op
            else if (expr.src2[0] == '1') {
                op = load_value_or_addr(expr.op, ctx, out); ctx.lock_reg(op);
                src1 = load_value_or_addr(expr.src1, ctx, out); ctx.lock_reg(src1);
                dst = ctx.load_reg(expr.dst, out, 1); ctx.release_reg();
                ctx.remove_num_reg();

                if (is_num(dst)) {
                    out << src1 << "[" << dst << "] = " << op << std::endl;
                }
                else {
                    out << "t6 = " << src1 << " + " << dst << std::endl;
                    out << "t6[0] = " << op << std::endl;
                }
            }
            // src1 = op[dst]
            else {
                op = load_value_or_addr(expr.op, ctx, out); ctx.lock_reg(op);
                dst = ctx.load_reg(expr.dst, out, 1); ctx.lock_reg(dst);
                src1 = ctx.alloc_reg(expr.src1, out); ctx.lock_reg(src1);
                ctx.remove_num_reg();

                if (is_num(dst)) {
                    out << src1 << " = " << op << "[" << dst << "]" << std::endl;
                }
                else {
                    src2 = ctx.free_reg(out);
                    out << src2 << " = " << op << " + " << dst << std::endl;
                    out << src1 << " = " << src2 << "[0]" << std::endl;
                }

                ctx.release_reg();
            }
            break;

        case expr_ifbr:
            src1 = ctx.load_reg(expr.src1, out); ctx.lock_reg(src1);
            src2 = ctx.load_reg(expr.src2, out); ctx.release_reg();
            ctx.save_reg(out);
            // if src1 op src2 goto dst
            out << "if " << src1 << " "  << expr.op << " " << src2 << " goto " << expr.dst << std::endl;
            break;

        case expr_goto:
            ctx.save_reg(out);
            // goto op
            out << "goto " << expr.op << std::endl;
            break;

        case expr_plab:
            ctx.save_reg(out);
            //op:
            out << expr.op << ":" << std::endl;
            break;

        case expr_para:
            // op = load_value_or_addr(expr.op, ctx, out, 1);
            // param = op
            // out << "a" << std::to_string(param_num) << " = " << op << std::endl;
            param_list[param_num] = expr.op;
            param_num++;
            break;

        case expr_call:
            ctx.save_reg(out);
            ctx.save_param(out, param_cnt);
            for (int i=0, reg_id; i<param_num; i++) {
                name = load_value_or_addr(param_list[i], ctx, out, 1, 1);
                out << "a" << i << " = " << name << std::endl;
                reg_id = ctx.reg_lookup(name);
                if (~reg_id) ctx.saved_var[reg_id] = " ";
            }
            param_num = 0;
            // call src1
            out << "call f_" << expr.src1 << std::endl;
            if (expr.op != "") {
                // op = a0
                op = ctx.alloc_reg(expr.op, out);
                out << op << " = a0" << std::endl;
            }
            ctx.load_param(out, param_cnt);
            break;

        case expr_retu:
            ctx.save_reg(out);
            if (expr.op != "") {
                // a0 = op
                op = load_value_or_addr(expr.op, ctx, out, 1);
                out << "a0 = " << op << std::endl;
            }
            // return
            out << "return" << std::endl;
            break;

        case expr_ends:
            break;

        default:
            break;
    }
    return index + 1;
}

void translate(IR_Context &ctx, std::ostream &out) {
    global_list.clear();
    is_global = 1;
    param_num = param_cnt = 0;
    int index = 0;
    while (index < expr_list.size())
        index = ir_expr(index, ctx, out);
}

}
