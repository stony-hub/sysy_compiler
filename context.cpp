#include "context.h"

namespace step1{

Variable::Variable (std::string name, int is_array, std::vector<int> shape)
    : name(name), is_array(is_array), shape(shape) {}

Const::Const (std::string name, std::vector<int> value, std::vector<int> shape = {})
    : name(name), value(value), is_array(1), shape(shape) {}

Const::Const (std::string name, int value)
    : name(name), value({value}), is_array(0), shape({}) {}

int Const::eval(std::vector<int> idx) {
    if (idx.size() != this->shape.size())
        throw std::runtime_error("shape wrong!");
    int r = 0;
    for (int i=0; i<this->shape.size(); i++)
        r = r * this->shape[i] + idx[i];
    return this->value[r];
}

Context::Context () {tmp_id = label_id = 0; is_global = 1;}

std::string Context::var_find(std::string name) {
    if (!param_list.empty()) {
        for (int i=param_list.back().size()-1; ~i; i--) {
            if (name == param_list.back()[i].name)
                return std::string("p") + std::to_string(i);
        }
    }
    for (int i=var_list.size()-1; ~i; i--) {
        if (name == var_list[i].name)
            return std::string("T") + std::to_string(i);
    }
    throw std::runtime_error("Variable not found!");
}

std::vector<int>& Context::shape_find(std::string name) {
    if (!param_list.empty()) {
        for (int i=param_list.back().size()-1; ~i; i--) {
            if (name == param_list.back()[i].name)
                return param_list.back()[i].shape;
        }
    }
    for (int i=var_list.size()-1; ~i; i--) {
        if (name == var_list[i].name)
            return var_list[i].shape;
    }
    throw std::runtime_error("Variable not found!");
}

Const &Context::const_eval(std::string name) {
    for (auto &i : const_list) {
        if (name == i.name)
            return i;
    }
    throw std::runtime_error("Const variable not declared!");
}

int Context::is_const(std::string name) {
    for (auto &i : const_list) {
        if (name == i.name)
            return 1;
    }
    return 0;
}

std::string Context::new_tmp() {
    int id = tmp_id++;
    return std::string("t") + std::to_string(id);
}

std::string Context::var_register(std::string name, int is_array, std::vector<int> shape) {
    int id = var_list.size();
    var_list.emplace_back(name, is_array, shape);
    return std::string("T") + std::to_string(id);
}

void Context::const_register(std::string name, int value) {
    const_list.emplace_back(name, value);
}

std::string Context::const_register(std::string name, std::vector<int> shape) {
    const_list.emplace_back(name, std::vector<int>({}), shape);
    return var_register(name, 1, shape);
}

void Context::const_fill(std::string name, std::vector<int> value) {
    Const &const_var = const_eval(name);
    const_var.value = value;
}

void Context::push_params() {
    param_list.push_back({});
}

void Context::pop_params() {
    param_list.pop_back();
}

void Context::push_scope() {
    scope_size.push_back(var_list.size());
}

void Context::pop_scope() {
    int size = scope_size.back();
    scope_size.pop_back();
    while (var_list.size() > size) var_list.pop_back();
}

std::string Context::param_register(std::string name, int is_array, std::vector<int> shape) {
    int id = param_list.back().size();
    param_list.back().emplace_back(name, is_array, shape);
    return std::string("p") + std::to_string(id);
}

std::string Context::new_label() {
    int id = label_id++;
    return std::string("l") + std::to_string(id);
}

void Context::push_label(std::string continue_l, std::string break_l) {
    continue_list.push_back(continue_l);
    break_list.push_back(break_l);
}

void Context::pop_label() {
    continue_list.pop_back();
    break_list.pop_back();
}

std::string Context::continue_lable() {
    return continue_list.back();
}

std::string Context::break_lable() {
    return break_list.back();
}

}
// namespace step1
