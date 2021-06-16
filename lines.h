#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "reg.h"

namespace step2 {

enum EXPR_T {
    expr_func,  // 0
    expr_decl,  // 1
    expr_ope1,  // 2
    expr_ope2,  // 3
    expr_assi,  // 4
    expr_ifbr,  // 5
    expr_goto,  // 6
    expr_plab,  // 7
    expr_para,  // 8
    expr_call,  // 9
    expr_retu,  // 10
    expr_ends,  // 11
};

struct Expr {
    EXPR_T type;
    std::string op, src1, src2, dst;

    Expr(EXPR_T type, std::string op, std::string src1, std::string src2, std::string dst);
};

struct Var {
    int array, size, offset;
    std::string name;
    Var(std::string name, int array, int size, int offset);
};

int is_num(std::string);

void add_global(std::string name, int array, int size);
void add_local(std::string name, int array, int size);
std::string find_var(std::string);

int is_array(std::string);

void new_expr(EXPR_T, const char*, const char*, const char*, const char*);

std::string load_value_or_addr(std::string name, struct IR_Context &ctx, std::ostream &out, int keep_num = 0, int load_param = 0);

int ir_expr(int index, IR_Context &ctx, std::ostream &out);

void translate(IR_Context &ctx, std::ostream &out);

extern std::vector<Expr> expr_list;
extern std::vector<Var> global_list;
extern std::vector<Var> local_list;
extern int stack_size, is_global, param_num;

}
