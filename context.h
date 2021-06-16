#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

namespace step1 {

class Variable {
  public:
    int is_array;
    std::vector<int> shape;
    std::string name;
    Variable (std::string name, int is_array = 0, std::vector<int> shape = {});
};

class Const {
  public:
    int is_array;
    std::vector<int> shape;
    std::vector<int> value; // this->view(-1)
    std::string name;
    Const (std::string name, std::vector<int> value, std::vector<int> shape);
    Const (std::string name, int value);
    int eval(std::vector<int> idx);
};

class Context {
  public:
    std::vector<Variable> var_list; // var Ti
    std::vector<Const> const_list;  // const
    std::vector<std::vector<Variable> > param_list; // var pi
    std::ostringstream global_array_init;
    int tmp_id;     // var ti
    int label_id;   // label li
    int is_global;  // is_global
    std::vector<std::string> continue_list, break_list;
    std::vector<int> scope_size;

    Context ();

    std::string new_tmp();

    std::string var_register(std::string name, int is_array = 0, std::vector<int> shape = {});
    std::string var_find(std::string);
    std::vector<int>& shape_find(std::string);

    void const_register(std::string name, int value);
    std::string const_register(std::string name, std::vector<int> shape);
    void const_fill(std::string name, std::vector<int> value);
    Const &const_eval(std::string);
    int is_const(std::string);

    void push_params();
    void pop_params();
    void push_scope();
    void pop_scope();
    std::string param_register(std::string name, int is_array = 0, std::vector<int> shape = {});

    std::string new_label();
    void push_label(std::string continue_l, std::string break_l);
    void pop_label();
    std::string continue_lable();
    std::string break_lable();
};

}
// namespace step1
