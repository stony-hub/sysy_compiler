#pragma once
#define YYSTYPE const char*
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

namespace step3 {

enum EXPR_T {
    expr_func, // 0
    expr_decl, // 1
    expr_ope2, // 2
    expr_ope1, // 3
    expr_assi, // 4
    expr_ifbr, // 5
    expr_goto, // 6
    expr_plab, // 7
    expr_call, // 8
    expr_stor, // 9
    expr_load, // 10
    expr_ldad, // 11
    expr_retu, // 12
    expr_endf, // 13
};

struct Expr {
    EXPR_T type;
    std::string op, src1, src2, dst;

    Expr(EXPR_T type, std::string op, std::string src1, std::string src2, std::string dst);
};

void new_expr(EXPR_T, const char*, const char*, const char*, const char*);
void translate(std::ostream &out);

}
