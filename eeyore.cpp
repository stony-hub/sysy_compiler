#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include "node.h"

namespace step1 {

int shape_size(std::vector<int> shape) {
    int v = 4;
    for (auto i : shape) v *= i;
    return v;
}

void NRoot::ir(Context &ctx, std::ostream &out) {
    for (auto &i : body)
        i->ir(ctx, out);
}

void NExpression::ir(Context &ctx, std::ostream &out) {
}

void NVarDeclare::ir(Context &ctx, std::ostream &out) {
    std::string var_name = ctx.var_register(this->name.name);
    out << "var " << var_name << std::endl;
}

void NVarDeclareWithInit::ir(Context &ctx, std::ostream &out) {
    if (!this->is_const) {
        std::string var_name = ctx.var_register(this->name.name);
        out << "var " << var_name << std::endl;

        value.getvar(ctx, out, var_name);
        // out << var_name << " = " << init << std::endl;
    }
    else {
        int v = value.eval(ctx);
        ctx.const_register(this->name.name, v);
    }
}

void NArrayDeclare::ir(Context &ctx, std::ostream &out) {
    std::string var = ctx.var_register(this->name.name.name, 1, eval_shape(ctx, this->name.shape));
    out << "var " << std::to_string(shape_size(eval_shape(ctx, this->name.shape))) << " " << var << std::endl;
}

void NArrayDeclareWithInit::ir(Context &ctx, std::ostream &out_real) {
    std::vector<int> real_shape = eval_shape(ctx, this->name.shape);
    std::vector<int> const_value;

    std::ostream &out = ctx.is_global ? ctx.global_array_init : out_real;

    int array_size = shape_size(real_shape);
    std::string var_name;
    if (!this->is_const) {
        var_name = ctx.var_register(this->name.name.name, 1, real_shape);
    }
    else {
        var_name = ctx.const_register(this->name.name.name, real_shape);
    }
    out_real << "var " << std::to_string(array_size) << " " << var_name << std::endl;

    value.array_value(ctx, out, this->name.name.name, real_shape, 0, 0, is_const, const_value);

    if (is_const) {
        ctx.const_fill(this->name.name.name, const_value);
    }
}

void NFunctionDefine::ir(Context &ctx, std::ostream &out) {
    out << std::string("f_") + this->name.name + " [" + std::to_string(this->args.list.size()) + "]" << std::endl;

    if (this->name.name == "main")
        out << ctx.global_array_init.str();

    // set params
    ctx.push_params();
    ctx.is_global = 0;
    for (auto i : this->args.list) {
        if (typeid(i->name) == typeid(NIdentifier)) {
            ctx.param_register(i->name.name);
        }
        else {
            auto j = dynamic_cast<NArrayIdentifier*>(&(i->name));
            std::vector<int> real_shape = eval_shape(ctx, j->shape);
            ctx.param_register(j->name.name, 1, real_shape);
        }
    }

    std::ostringstream sout;
    this->body.ir(ctx, sout);

    // move all "var Ti" and "var ti" to the top of the fnuction
    std::istringstream sin(sout.str());
    std::string str, tmp;
    std::unordered_set<std::string> var_defns;
    while (getline(sin, str)) {
        if (str.length() > 3 && str.substr(0, 3) == "var") {
            if (var_defns.find(str) == var_defns.end()) {
                out << str << std::endl;
                var_defns.insert(str);
            }
        }
        else tmp += str + '\n';
    }
    out << tmp;

    // pop params
    ctx.pop_params();
    ctx.is_global = 1;

    out << "return" << std::endl;
    out << "end f_" + this->name.name << std::endl;
}

void NReturnStatement::ir(Context &ctx, std::ostream &out) {
    if (this->value == NULL) {
        out << "return" << std::endl;
    }
    else {
        // param vairable
        if (typeid(*value) == typeid(NIdentifier)) {
            auto iden = dynamic_cast<NIdentifier*>(value);
            std::string res;
            if (ctx.is_const(iden->name)) {
                res = std::to_string(iden->eval(ctx));
            }
            else {
                res = ctx.var_find(iden->name);
            }
            out << "return " << res << std::endl;
        }
        // param expression
        else {
            std::string res = value->getvar(ctx, out);
            out << "return " << res << std::endl;
        }
    }
}

void NContinueStatement::ir(Context &ctx, std::ostream &out) {
    out << "goto " << ctx.continue_lable() << std::endl;
}

void NBreakStatement::ir(Context &ctx, std::ostream &out) {
    out << "goto " << ctx.break_lable() << std::endl;
}

void NIfElseStatement::ir(Context &ctx, std::ostream &out) {
    std::string l0, l1, l2;
    l0 = ctx.new_label();
    l1 = ctx.new_label();
    l2 = ctx.new_label();

    std::string cond_var = cond.getcond(ctx, out, l0, l1);
    out << "if " << cond_var << " == 0 goto " << l1 << std::endl;   // if cond == 0 goto l1
    out << l0 << ":" << std::endl;                                  // l0:
    this->thenstmt.ir(ctx, out);                                    //      thenstmt
    out << "goto " << l2 << std::endl;                              //      goto l2
    out << l1 << ":" << std::endl;                                  // l1:
    this->elsestmt.ir(ctx, out);                                    //      elsestmt
    out << l2 << ":" << std::endl;                                  // l2:
}

void NWhileStatement::ir(Context &ctx, std::ostream &out) {
    std::string l0, l1, l2;
    l0 = ctx.new_label();
    l1 = ctx.new_label();
    l2 = ctx.new_label();

    ctx.push_label(l0, l2);

    out << l0 << ":" << std::endl;                                  // l0:
    std::string cond_var = cond.getcond(ctx, out, l1, l2);
    out << "if " << cond_var << " == 0 goto " << l2 << std::endl;   // if cond == 0 goto l2
    out << l1 << ":" << std::endl;                                  // l1:
    this->dostmt.ir(ctx, out);                                      //      dostmt
    out << "goto " << l0 << std::endl;                              //      goto l0
    out << l2 << ":" << std::endl;                                  // l2:

    ctx.pop_label();
}

void NEvalStatement::ir(Context &ctx, std::ostream &out) {
    value.ir(ctx, out);
}

void NDeclareStatement::ir(Context &ctx, std::ostream &out) {
    for (auto &i : list)
        i->ir(ctx, out);
}

void NBlock::ir(Context &ctx, std::ostream &out) {
    ctx.push_scope();
    for (auto &i : statements)
        i->ir(ctx, out);
    ctx.pop_scope();
}

void NVoidStatement::ir(Context &ctx, std::ostream &out) {
}

void NAssignment::ir(Context &ctx, std::ostream &out) {
    std::string lvar = lhs.left_getvar(ctx, out);
    std::string rvar;
    // array
    if (typeid(lhs) == typeid(NArrayIdentifier)) {
        rvar = rhs.getvar(ctx, out);
        out << lvar << " = " << rvar << std::endl;
    }
    // scalar
    else {
        auto iden = dynamic_cast<NIdentifier*>(&lhs);
        if (rhs.detect(iden->name)) {
            rvar = rhs.getvar(ctx, out);
            out << lvar << " = " << rvar << std::endl;
        }
        else {
            rhs.getvar(ctx, out, lvar);
        }
    }
}

void NFunctionCall::ir(Context &ctx, std::ostream &out) {
    ir_param(ctx, out);
    out << "call f_" << this->name.name << std::endl;
}

void NFunctionCall::ir_param(Context &ctx, std::ostream &out) {
    std::vector<std::string> params;
    for (auto i : args.args) {
        if (typeid(*i) == typeid(NIdentifier)) {
            auto iden = dynamic_cast<NIdentifier*>(i);
            std::string res;
            if (ctx.is_const(iden->name)) {
                res = std::to_string(iden->eval(ctx));
            }
            else {
                res = ctx.var_find(iden->name);
            }
            params.push_back(res);
            // out << "param " << res << std::endl;
        }
        else {
            std::string par = i->getvar(ctx, out);
            params.push_back(par);
            // out << "param " << par << std::endl;
        }
    }
    for (auto &i : params)
        out << "param " << i << std::endl;
}

}
// namespace step1
