%{
#define YYSTYPE const char*
#include "lines.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace step2;

int global = 1;
extern int step2lex();
extern int step2get_lineno();
void yyerror(const char *s) { std::printf("Error(line: %d): %s\n", step2get_lineno(), s);}
%}

%token VAR CALL PARAM RETURN END IF GOTO LSQR RSQR NOT MIN ASS OP2 COL
%token VARNAME FUNCNAME LABNAME IMMNAME

%%

Goal: Goal Statement    {}
    | Statement         {}
    ;

Statement: FUNCNAME LSQR IMMNAME RSQR  {new_expr(expr_func, $1, $3, "", ""); global = 0; local_list.clear();}
        | VAR VARNAME {
            if (global) add_global($2, 0, 0);
            else add_local($2, 0, 1);
            new_expr(expr_decl, strdup(find_var($2).c_str()), "", "", "");
        }
        | VAR IMMNAME VARNAME {
            int size = atoi($2) / 4;
            if (global) add_global($3, 1, size);
            else add_local($3, 1, size);
            new_expr(expr_decl, strdup(find_var($3).c_str()), $2, "", "");
        }
        | NewVARNAME ASS RightValue OP2 RightValue          {new_expr(expr_ope2, $4, $3, $5, $1);}
        | NewVARNAME ASS RightValue MIN RightValue          {new_expr(expr_ope2, "-", $3, $5, $1);}
        | NewVARNAME ASS MIN RightValue                     {new_expr(expr_ope1, "-", $4, "", $1);}
        | NewVARNAME ASS NOT RightValue                     {new_expr(expr_ope1, "!", $4, "", $1);}
        | NewVARNAME ASS RightValue                         {new_expr(expr_assi, $3, $1, "0", "");}
        | NewVARNAME LSQR RightValue RSQR ASS RightValue    {new_expr(expr_assi, $6, $1, "1", $3);}
        | NewVARNAME ASS NewVARNAME LSQR RightValue RSQR    {new_expr(expr_assi, $3, $1, "2", $5);}
        | IF RightValue OP2 RightValue GOTO LABNAME         {new_expr(expr_ifbr, $3, $2, $4, $6);}
        | GOTO LABNAME                                      {new_expr(expr_goto, $2, "", "", "");}
        | LABNAME COL                                       {new_expr(expr_plab, $1, "", "", "");}
        | PARAM RightValue                                  {new_expr(expr_para, $2, "", "", "");}
        | CALL FUNCNAME                                     {new_expr(expr_call, "", $2, "", "");}
        | NewVARNAME ASS CALL FUNCNAME                      {new_expr(expr_call, $1, $4, "", "");}
        | RETURN RightValue                                 {new_expr(expr_retu, $2, "", "", "");}
        | RETURN                                            {new_expr(expr_retu, "", "", "", "");}
        | END FUNCNAME                                      {new_expr(expr_ends, $2, "", "", ""); global = 1;}
        ;

RightValue: NewVARNAME  {$$=$1;}
          | IMMNAME     {$$=$1;}

NewVARNAME: VARNAME     {$$=strdup(find_var($1).c_str());}
