#include "node.h"
#include "step1.tab.hpp"

namespace step1 {

std::vector<int> eval_shape(Context &ctx, std::vector<NExpression*> shape) {
    std::vector<int> res;
    for (auto i : shape) {
        res.push_back(i->eval(ctx));
    }
    return res;
}

int NExpression::eval(Context &ctx) {
    throw std::runtime_error("Cannot eval this expression!");
}

int NNumber::eval(Context &ctx) {
    return value;
}

int NIdentifier::eval(Context &ctx) {
    return ctx.const_eval(this->name).value[0];
}

int NArrayIdentifier::eval(Context &ctx) {
    std::vector<int> res = eval_shape(ctx, this->shape);
    return ctx.const_eval(this->name.name).eval(res);
}

int NBinaryExpression::eval(Context& ctx) {
    switch (this->op) {
        case PLUS:
            return lhs.eval(ctx) + rhs.eval(ctx);
            break;

        case MINUS:
            return lhs.eval(ctx) - rhs.eval(ctx);
            break;

        case MUL:
            return lhs.eval(ctx) * rhs.eval(ctx);
            break;

        case DIV:
            return lhs.eval(ctx) / rhs.eval(ctx);
            break;

        case MOD:
            return lhs.eval(ctx) % rhs.eval(ctx);
            break;

        case EQ:
            return lhs.eval(ctx) == rhs.eval(ctx);
            break;

        case NE:
            return lhs.eval(ctx) != rhs.eval(ctx);
            break;

        case GT:
            return lhs.eval(ctx) > rhs.eval(ctx);
            break;

        case GE:
            return lhs.eval(ctx) >= rhs.eval(ctx);
            break;

        case LT:
            return lhs.eval(ctx) < rhs.eval(ctx);
            break;

        case LE:
            return lhs.eval(ctx) <= rhs.eval(ctx);
            break;

        case AND:
            return lhs.eval(ctx) && rhs.eval(ctx);
            break;

        case OR:
            return lhs.eval(ctx) || rhs.eval(ctx);
            break;

        default:
            throw std::runtime_error("Unknow OP");
            break;
    }
}

int NUnaryExpression::eval(Context& ctx) {
    switch (this->op) {
        case PLUS:
            return rhs.eval(ctx);
            break;

        case MINUS:
            return -rhs.eval(ctx);
            break;

        case NOT:
            return !rhs.eval(ctx);
            break;

        default:
            throw std::runtime_error("Unknow OP");
            break;
    }
}

void NArrayDeclareInitValue::array_value(Context &ctx, std::ostream &out, std::string &name, const std::vector<int> &shape, int d, int offset, int is_const, std::vector<int> &const_value) {
    int range = shape[d], block = 1;
    for (int i=d+1; i<shape.size(); i++) block *= shape[i];

    if (is_number) {
        if (is_const) {
            int v = value->eval(ctx);
            const_value.push_back(v);
            for (int i=1; i<range*block; i++) const_value.push_back(0);
        }
        std::string tmp = value->getvar(ctx, out);
        out << ctx.var_find(name) << "[" << std::to_string(4*offset) << "] = "<< tmp << std::endl;
        return;
    }

    for (int i=0, p=0; i<value_list.size() && p < range * block; i++) {
        if (value_list[i]->is_number) {
            if (is_const) {
                int v = value_list[i]->value->eval(ctx);
                const_value.push_back(v);
            }
            std::string tmp = value_list[i]->value->getvar(ctx, out);
            out << ctx.var_find(name) << "[" << std::to_string(4*(offset+p)) << "] = " << tmp << std::endl;
            p++;
        }
        else {
            int pos = ((p + block - 1) / block) * block;  // ceil(p / block) * block
            if (is_const) {
                int fill_zero = pos - p;
                std::vector<int> fill(fill_zero);
                const_value.insert(const_value.end(), fill.begin(), fill.end());
            }
            value_list[i]->array_value(ctx, out, name, shape, d+1, offset+pos, is_const, const_value);
            p = pos + block;
        }
        if (i == value_list.size() - 1 && p < range * block && is_const) {
            int fill_zero = range * block - p;
            std::vector<int> fill(fill_zero);
            const_value.insert(const_value.end(), fill.begin(), fill.end());
        }
    }
}

std::string NExpression::getvar(Context &ctx, std::ostream &out, std::string ph) {
    throw std::runtime_error("Cannot generate tmp var for this expression!");
}

std::string NBinaryExpression::getvar(Context& ctx, std::ostream &out, std::string ph) {
    int result;
    try {
        result = this->eval(ctx);
        std::string res = std::to_string(result);
        if (ph != "")
            out << ph << " = " << res << std::endl;
        return ph == "" ? res : ph;
    } catch(std::runtime_error e) {}

    std::string lvar = lhs.getvar(ctx, out, ph);
    std::string rvar = rhs.getvar(ctx, out);
    std::string res;
    if (ph == "") {
        res = ctx.new_tmp();
        out << "var " << res << std::endl;
    }
    else {
        res = ph;
    }

    switch (this->op) {
        case PLUS:
            out << res << " = " << lvar << " + " << rvar << std::endl;
            break;

        case MINUS:
            out << res << " = " << lvar << " - " << rvar << std::endl;
            break;

        case MUL:
            out << res << " = " << lvar << " * " << rvar << std::endl;
            break;

        case DIV:
            out << res << " = " << lvar << " / " << rvar << std::endl;
            break;

        case MOD:
            out << res << " = " << lvar << " % " << rvar << std::endl;
            break;

        case EQ:
            out << res << " = " << lvar << " == " << rvar << std::endl;
            break;

        case NE:
            out << res << " = " << lvar << " != " << rvar << std::endl;
            break;

        case GT:
            out << res << " = " << lvar << " > " << rvar << std::endl;
            break;

        case GE:
            out << res << " = " << lvar << " >= " << rvar << std::endl;
            break;

        case LT:
            out << res << " = " << lvar << " < " << rvar << std::endl;
            break;

        case LE:
            out << res << " = " << lvar << " <= " << rvar << std::endl;
            break;

        case AND:
            out << res << " = " << lvar << " && " << rvar << std::endl;
            break;

        case OR:
            out << res << " = " << lvar << " || " << rvar << std::endl;
            break;

        default:
            throw std::runtime_error("Unknow OP");
            break;
    }
    return res;
}

std::string NUnaryExpression::getvar(Context& ctx, std::ostream &out, std::string ph) {
    int result;
    try {
        result = this->eval(ctx);
        std::string res = std::to_string(result);
        if (ph != "")
            out << ph << " = " << res << std::endl;
        return ph == "" ? res : ph;
    } catch(std::runtime_error e) {}

    std::string rvar = rhs.getvar(ctx, out, ph);
    std::string res;
    if (ph == "") {
        res = ctx.new_tmp();
        out << "var " << res << std::endl;
    }
    else {
        res = ph;
    }

    switch (this->op) {
        case PLUS:
            out << res << " = " << rvar << std::endl;
            break;

        case MINUS:
            out << res << " = -" << rvar << std::endl;
            break;

        case NOT:
            out << res << " = !" << rvar << std::endl;
            break;

        default:
            throw std::runtime_error("Unknow OP");
            break;
    }
    return res;
}

std::string NIdentifier::getvar(Context& ctx, std::ostream &out, std::string ph) {
    std::string res;
    if (ctx.is_const(this->name)) {
        res = std::to_string(ctx.const_eval(this->name).value[0]);
    }
    else {
        res = ctx.var_find(this->name);
    }
    if (ph != "")
        out << ph << " = " << res << std::endl;
    return ph == "" ? res : ph;
}

std::string NIdentifier::left_getvar(Context& ctx, std::ostream &out) {
    return getvar(ctx, out);
}

std::string NArrayIdentifier::getvar(Context& ctx, std::ostream &out, std::string ph) {
    int result;
    try {
        result = this->eval(ctx);
        std::string res = std::to_string(result);
        if (ph != "")
            out << ph << " = " << res << std::endl;
        return ph == "" ? res : ph;
    } catch(std::runtime_error e) {}
    std::string var_name = ctx.var_find(this->name.name);
    std::vector<int> &var_shape = ctx.shape_find(this->name.name);

    std::string res = ctx.new_tmp();
    out << "var " << res << std::endl;  // var res
    if (shape.size() > 0) {
        // T = shape[0];
        std::string shape_0 = shape[0]->getvar(ctx, out);
        out << res << " = " << shape_0 << std::endl;
    }
    else {
        out << res << " = 0" << std::endl;  // res = 0
    }

    std::string t1, t2;
    if (1 < shape.size()) {
        t1 = ctx.new_tmp();
        t2 = ctx.new_tmp();
        out << "var " << t1 << std::endl;
        out << "var " << t2 << std::endl;
    }
    for (int i=1; i<shape.size(); i++) {
        // T = T * var_shape[i] + shape[i];
        std::string shape_i = shape[i]->getvar(ctx, out);
        out << t1 << " = " << res << " * " << std::to_string(var_shape[i]) << std::endl;
        out << t2 << " = " << t1 << " + " << shape_i << std::endl;
        res = t2;
        if ((i == shape.size() - 1 || !shape.size()) && i < var_shape.size() - 1) {
            while (++i < var_shape.size()) {
                // res = res * var_shape[i]
                if (res != "0")
                    out << res << " = " << res << " * " << std::to_string(var_shape[i]) << std::endl;
            }
            if (res != "0")
                out << res << " = " << res << " * 4" << std::endl;   // res = res * 4
            if (res != "0")
                out << res << " = " << var_name << " + " << res << std::endl;   // var + res
            else
                out << res << " = " << var_name << std::endl;
            return res;
        }
    }

    out << res << " = " << res << " * 4" << std::endl;   // res = res * 4
    std::string tmp;        // tmp = var[res]
    if (ph == "") {
        tmp = ctx.new_tmp();    
        out << "var " << tmp << std::endl;
    }
    else {
        tmp = ph;
    }
    out << tmp << " = " << var_name + "[" + res + "]" << std::endl;
    return tmp;
}

std::string NArrayIdentifier::left_getvar(Context& ctx, std::ostream &out) {
    std::string var_name = ctx.var_find(this->name.name);
    std::vector<int> &var_shape = ctx.shape_find(this->name.name);

    if (!shape.size()) {
        throw std::runtime_error("Leftvar array not complete!");
    }

    std::string res = ctx.new_tmp();
    out << "var " << res << std::endl;  // var res
    std::string shape_0 = shape[0]->getvar(ctx, out);
    out << res << " = " << shape_0 << std::endl;

    std::string t1, t2;
    if (1 < shape.size()) {
        t1 = ctx.new_tmp();
        t2 = ctx.new_tmp();
        out << "var " << t1 << std::endl;
        out << "var " << t2 << std::endl;
    }
    for (int i=1; i<shape.size(); i++) {
        // T = T * var_shape[i] + shape[i];
        std::string shape_i = shape[i]->getvar(ctx, out);
        out << t1 << " = " << res << " * " << std::to_string(var_shape[i]) << std::endl;
        out << t2 << " = " << t1 << " + " << shape_i << std::endl;
        res = t2;
        if (i == shape.size() - 1 && i < var_shape.size() - 1) {
            throw std::runtime_error("Leftvar array not complete!");
        }
    }

    out << res << " = " << res << " * 4" << std::endl;   // res = res * 4
    return var_name + "[" + res + "]";      // var[res]
}

std::string NConditionExpression::getvar(Context &ctx, std::ostream &out, std::string ph) {
    return value.getvar(ctx, out, ph);
}

std::string NNumber::getvar(Context &ctx, std::ostream &out, std::string ph) {
    std::string res = std::to_string(value);
    if (ph != "") {
        out << ph << " = " << res << std::endl;
    }
    return ph == "" ? res : ph;
}

std::string NFunctionCall::getvar(Context &ctx, std::ostream &out, std::string ph) {
    ir_param(ctx, out);
    std::string res;
    if (ph == "") {
        res = ctx.new_tmp();
        out << "var " << res << std::endl;
    }
    else {
        res = ph;
    }
    out << res << " = call f_" << this->name.name << std::endl;
    return res;
}

std::string NExpression::getcond(Context &ctx, std::ostream &out, std::string &l_true, std::string &l_false) {
    return getvar(ctx, out);
}

std::string NConditionExpression::getcond(Context &ctx, std::ostream &out, std::string &l_true, std::string &l_false) {
    return value.getcond(ctx, out, l_true, l_false);
}

std::string NBinaryExpression::getcond(Context& ctx, std::ostream &out, std::string &l_true, std::string &l_false) {
    if (this->op != AND && this->op != OR)
        return getvar(ctx, out);

    std::string lvar = lhs.getcond(ctx, out, l_true, l_false);
    if (this->op == AND) {
        // if lvar == false goto l_false
        out << "if " << lvar << " == 0 goto " << l_false << std::endl;
    }
    if (this->op == OR) {
        // if lvar == true goto l_true
        out << "if " << lvar << " != 0 goto " << l_true << std::endl;
    }

    std::string rvar = rhs.getcond(ctx, out, l_true, l_false);
    std::string res = ctx.new_tmp();
    out << "var " << res << std::endl;
    if (this->op == AND) {
        out << res << " = " << lvar << " && " << rvar << std::endl;
    }
    if (this->op == OR) {
        out << res << " = " << lvar << " || " << rvar << std::endl;
    }
    return res;
}

int NExpression::detect(std::string obj) {
    throw std::runtime_error("can not detect this expression!");
}

int NIdentifier::detect(std::string obj) {
    return name == obj;
}

int NConditionExpression::detect(std::string obj) {
    return value.detect(obj);
}

int NBinaryExpression::detect(std::string obj) {
    return lhs.detect(obj) || rhs.detect(obj);
}

int NUnaryExpression::detect(std::string obj) {
    return rhs.detect(obj);
}

int NFunctionCall::detect(std::string obj) {
    for (auto i : args.args) {
        if (i->detect(obj))
            return 1;
    }
    return 0;
}

int NNumber::detect(std::string obj) {
    return 0;
}

int NArrayIdentifier::detect(std::string obj) {
    return name.name == obj;
}

}
// namespace step1
