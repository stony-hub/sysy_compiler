#include "riscv.h"

namespace step3 {

int stk = 0;
std::vector<Expr> expr_list;

Expr::Expr(EXPR_T type, std::string op, std::string src1, std::string src2, std::string dst)
    : type(type), op(op), src1(src1), src2(src2), dst(dst) {}

int is_num(std::string str) {
    for (auto i : str) {
        if (!isdigit(i) && i!='-') return 0;
    }
    return 1;
}

int is_int12(std::string s) {
    int x = atoi(s.c_str());
    return x >= -2048 && x <2048;
}

int is_int12(int x) {
    return x >= -2048 && x <2048;
}

int is_int10(std::string s) {
    int x = atoi(s.c_str());
    return x >= -512 && x <512;
}

int is_int10(int x) {
    return x >= -512 && x <512;
}

void new_expr(EXPR_T type, const char* op, const char* src1, const char* src2, const char* dst) {
    expr_list.emplace_back(type, op, src1, src2, dst);
}

void riscv_func(Expr &expr, std::ostream &out) {
    stk = (atoi(expr.src2.c_str()) / 4 + 1) * 16;
    out << "    .text" << std::endl;
    out << "    .align  2" << std::endl;
    out << "    .global " << expr.op << std::endl;
    out << "    .type   " << expr.op << ", @function" << std::endl;
    out << expr.op << ":" << std::endl;

    // addi sp, sp, -STK
    if (is_int12(-stk)) {
        out << "    addi    sp, sp, " << -stk << std::endl;
    }
    else {
        out << "    li      s0, " << -stk << std::endl;
        out << "    add     sp, sp, s0" << std::endl;
    }

    // sw ra, STK-4(sp)
    if (is_int12(stk - 4)) {
        out << "    sw      ra, " << stk - 4 << "(sp)" << std::endl;
    }
    else {
        out << "    li      s0, " << stk - 4 << std::endl;
        out << "    add     s0, sp, s0" << std::endl;
        out << "    sw      ra, 0(s0)" << std::endl;
    }
    return;
}

void riscv_decl(Expr &expr, std::ostream &out) {
    if (expr.src2 == "array") {
        out << "    .comm   " << expr.op << ", " << expr.src1 << ", 4" << std::endl;
    }
    else {
        out << "    .global " << expr.op << std::endl;
        out << "    .section    .sdata" << std::endl;
        out << "    .align  2" << std::endl;
        out << "    .type   " << expr.op << ", @object" << std::endl;
        out << "    .size   " << expr.op << ", 4" << std::endl;
        out << expr.op << ":" << std::endl;
        out << "    .word   " << expr.src1 << std::endl;
    }
}

void riscv_ope2(Expr &expr, std::ostream &out) {
    std::string op = expr.op, src1 = expr.src1, src2 = expr.src2, dst = expr.dst;

    if (is_num(src2) && ((op == "+" || op == "<") && !is_int12(src2))) {
        out << "    li      s0, " << src2 << std::endl;
        src2 = "s0";
    }

    if (op == "+") {
        if (is_num(src2)) {
            out << "    addi    " << dst << ", " << src1 << ", " << src2 << std::endl;
        }
        else {
            out << "    add     " << dst << ", " << src1 << ", " << src2 << std::endl;
        }
    }
    else if (op == "-") {
        out << "    sub     " << dst << ", " << src1 << ", " << src2 << std::endl;
    }
    else if (op == "*") {
        out << "    mul     " << dst << ", " << src1 << ", " << src2 << std::endl;
    }
    else if (op == "/") {
        out << "    div     " << dst << ", " << src1 << ", " << src2 << std::endl;
    }
    else if (op == "%") {
        out << "    rem     " << dst << ", " << src1 << ", " << src2 << std::endl;
    }
    else if (op == "<") {
        if (is_num(src2)) {
            out << "    slti    " << dst << ", " << src1 << ", " << src2 << std::endl;
        }
        else {
            out << "    slt     " << dst << ", " << src1 << ", " << src2 << std::endl;
        }
    }
    else if (op == ">") {
        out << "    sgt     " << dst << ", " << src1 << ", " << src2 << std::endl;
    }
    else if (op == "<=") {
        out << "    sgt     " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    seqz    " << dst << ", " << dst << std::endl;
    }
    else if (op == ">=") {
        out << "    slt     " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    seqz    " << dst << ", " << dst << std::endl;
    }
    else if (op == "&&") {
        out << "    and     " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    snez    " << dst << ", " << dst << std::endl;
    }
    else if (op == "||") {
        out << "    or      " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    snez    " << dst << ", " << dst << std::endl;
    }
    else if (op == "!=") {
        out << "    xor     " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    snez    " << dst << ", " << dst << std::endl;
    }
    else if (op == "==") {
        out << "    xor     " << dst << ", " << src1 << ", " << src2 << std::endl;
        out << "    seqz    " << dst << ", " << dst << std::endl;
    }
}

void riscv_ope1(Expr &expr, std::ostream &out) {
    std::string op = expr.op, src = expr.src1, dst = expr.dst;
    if (op == "!") {
        out << "    seqz    " << dst << ", " << src << std::endl;
    }
    else if (op == "-") {

        out << "    neg     " << dst << ", " << src << std::endl;
    }
}

void riscv_assi(Expr &expr, std::ostream &out) {
    std::string op = expr.op, src1 = expr.src1, src2 = expr.src2, dst = expr.dst;
    // op = src1
    if (dst == "imm") {
        out << "    li      " << op << ", " << src1 << std::endl;
    }
    // op = src1
    else if (dst == "reg") {
        out << "    mv      " << op << ", " << src1 << std::endl;
    }
    // op[src2] = src1
    else if (dst == "larray") {
        if (is_int12(src2)) {
            out << "    sw      " << src1 << ", " << src2 << "(" << op << ")" << std::endl;
        }
        else {
            out << "    li      s0, " << src2 << std::endl;
            out << "    add     s0" << ", " << op << ", s0" << std::endl;
            out << "    sw      " << src1 << ", 0(s0)" << std::endl;
        }
    }
    // op = src1[src2]
    else if (dst == "rarray") {
        if (is_int12(src2)) {
            out << "    lw      " << op << ", " << src2 << "(" << src1 << ")" << std::endl;
        }
        else {
            out << "    li      s0, " << src2 << std::endl;
            out << "    add     s0" << ", " << src1 << ", s0" << std::endl;
            out << "    sw      " << op << ", 0(s0)" << std::endl;
        }
    }
}

void riscv_ifbr(Expr &expr, std::ostream &out) {
    std::string op = expr.op, src1 = expr.src1, src2 = expr.src2, dst = expr.dst;
    // if src1 op src2 goto dst
    if (op == "<") {
        out << "    blt     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
    else if (op == ">") {
        out << "    bgt     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
    else if (op == "<=") {
        out << "    ble     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
    else if (op == ">=") {
        out << "    bge     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
    else if (op == "!=") {
        out << "    bne     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
    else if (op == "==") {
        out << "    beq     " << src1 << ", " << src2 << ", ." << dst << std::endl;
    }
}

void riscv_goto(Expr &expr, std::ostream &out) {
    out << "    j       ." << expr.op << std::endl;
}

void riscv_plab(Expr &expr, std::ostream &out) {
    out << "." << expr.op << ":" << std::endl;
}

void riscv_call(Expr &expr, std::ostream &out) {
    out << "    call    " << expr.op << std::endl;
}

void riscv_stor(Expr &expr, std::ostream &out) {
    // store op src1
    std::string reg = expr.op, num = expr.src1;
    if (is_int10(num)) {
        // 	sw reg, int10*4(sp)
        out << "    sw      " << reg << ", " << atoi(num.c_str()) * 4 << "(sp)" << std::endl;
    }
    else {
        out << "    li      s0, " << num << std::endl;
        out << "    add     s0, s0, s0" << std::endl;
        out << "    add     s0, s0, s0" << std::endl;
        out << "    add     s0, sp, s0" << std::endl;
        out << "    sw      " << reg << ", 0(s0)" << std::endl;
    }
}

void riscv_load(Expr &expr, std::ostream &out) {
    // load op src1
    std::string num = expr.op, reg = expr.src1;
    if (expr.dst == "stack") {
        if (is_int10(num)) {
            // lw reg, int10*4(sp)
            out << "    lw      " << reg << ", " << atoi(num.c_str()) * 4 << "(sp)" << std::endl;
        }
        else {
            out << "    li      s0, " << num << std::endl;
            out << "    add     s0, s0, s0" << std::endl;
            out << "    add     s0, s0, s0" << std::endl;
            out << "    add     s0, sp, s0" << std::endl;
            out << "    lw      " << reg << ", 0(s0)" << std::endl;
        }
    }
    else if (expr.dst == "global") {
        // lui reg, %hi(global_var)
        out << "    lui     " << reg << ", %hi(" << num << ")" << std::endl;
        // lw reg, %lo(global_var)(reg)
        out << "    lw      " << reg << ", %lo(" << num << ")(" << reg << ")" << std::endl;
    }
}

void riscv_ldad(Expr &expr, std::ostream &out) {
    // loadaddr op src1
    std::string num = expr.op, reg = expr.src1;
    if (expr.dst == "stack") {
        if (is_int10(num)) {
            // addi reg, sp, int10*4
            out << "    addi    " << reg << ", sp, " << atoi(num.c_str()) * 4 << std::endl;
        }
        else {
            out << "    li      s0, " << num << std::endl;
            out << "    add     s0, s0, s0" << std::endl;
            out << "    add     s0, s0, s0" << std::endl;
            out << "    add     " << reg << ", sp, s0" << std::endl;
        }
    }
    else if (expr.dst == "global") {
        // la reg, global_var
        out << "    la      " << reg << ", " << num << std::endl;
    }
}

void riscv_retu(Expr &expr, std::ostream &out) {
    // lw ra, STK-4(sp)
    if (is_int12(stk - 4)) {
        out << "    lw      ra, " << stk - 4 << "(sp)" << std::endl;
    }
    else {
        out << "    li      s0, " << stk - 4 << std::endl;
        out << "    add     s0, sp, s0" << std::endl;
        out << "    lw      ra, 0(s0)" << std::endl;
    }

    // addi sp, sp, STK
    if (is_int12(stk)) {
        out << "    addi    sp, sp, " << stk << std::endl;
    }
    else {
        out << "    li      s0, " << stk << std::endl;
        out << "    add     sp, sp, s0" << std::endl;
    }

    // ret
    out << "    ret" << std::endl;
}

void riscv_endf(Expr &expr, std::ostream &out) {
    // .size   func, .-func
    out << "    .size   " << expr.op << ", .-" << expr.op << std::endl;
}

void translate(std::ostream &out) {
    for (auto &i : expr_list) {
        switch(i.type) {
            case expr_func: riscv_func(i, out); break;
            case expr_decl: riscv_decl(i, out); break;
            case expr_ope2: riscv_ope2(i, out); break;
            case expr_ope1: riscv_ope1(i, out); break;            
            case expr_assi: riscv_assi(i, out); break;            
            case expr_ifbr: riscv_ifbr(i, out); break;            
            case expr_goto: riscv_goto(i, out); break;            
            case expr_plab: riscv_plab(i, out); break;            
            case expr_call: riscv_call(i, out); break;            
            case expr_stor: riscv_stor(i, out); break;            
            case expr_load: riscv_load(i, out); break;            
            case expr_ldad: riscv_ldad(i, out); break;            
            case expr_retu: riscv_retu(i, out); break;            
            case expr_endf: riscv_endf(i, out); break;            
        }
    }
}

}
