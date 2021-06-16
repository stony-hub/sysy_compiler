%{
#define YYSTYPE const char*
#include "riscv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace step3;

extern int step3lex();
extern int step3get_lineno();
void step3error(const char *s) { std::printf("Error(line: %d): %s\n", step3get_lineno(), s);}
%}

%token MALLOC END IF GOTO CALL STORE LOAD LOADADDR RETURN
%token REGNAME VARNAME FUNCNAME LABNAME IMMNAME 
%token LSQR RSQR NOT MIN ASS OP2 COL

%%

Goal: 
      Goal Statement  {}
    | Statement       {}
    ;

Statement:
      VARNAME ASS IMMNAME                           {new_expr(expr_decl, $1, $3, "", "");}
    | VARNAME ASS MALLOC IMMNAME                    {new_expr(expr_decl, $1, $4, "array", "");}
    | FUNCNAME LSQR IMMNAME RSQR LSQR IMMNAME RSQR  {new_expr(expr_func, $1, $3, $6, "");}
    | REGNAME ASS REGNAME OP2 REGNAME               {new_expr(expr_ope2, $4, $3, $5, $1);}
    | REGNAME ASS REGNAME OP2 IMMNAME               {new_expr(expr_ope2, $4, $3, $5, $1);}
    | REGNAME ASS REGNAME MIN REGNAME               {new_expr(expr_ope2, "-", $3, $5, $1);}
    | REGNAME ASS MIN REGNAME                       {new_expr(expr_ope1, "-", $4, "", $1);}
    | REGNAME ASS NOT REGNAME                       {new_expr(expr_ope1, "!", $4, "", $1);}
    | REGNAME ASS REGNAME                           {new_expr(expr_assi, $1, $3, "", "reg");}
    | REGNAME ASS IMMNAME                           {new_expr(expr_assi, $1, $3, "", "imm");}
    | REGNAME LSQR IMMNAME RSQR ASS REGNAME         {new_expr(expr_assi, $1, $6, $3, "larray");}
    | REGNAME ASS REGNAME LSQR IMMNAME RSQR         {new_expr(expr_assi, $1, $3, $5, "rarray");}
    | IF REGNAME OP2 REGNAME GOTO LABNAME           {new_expr(expr_ifbr, $3, $2, $4, $6);}
    | GOTO LABNAME                                  {new_expr(expr_goto, $2, "", "", "");}
    | LABNAME COL                                   {new_expr(expr_plab, $1, "", "", "");}
    | CALL FUNCNAME                                 {new_expr(expr_call, $2, "", "", "");}
    | STORE REGNAME IMMNAME                         {new_expr(expr_stor, $2, $3, "", "");}
    | LOAD IMMNAME REGNAME                          {new_expr(expr_load, $2, $3, "", "stack");}
    | LOAD VARNAME REGNAME                          {new_expr(expr_load, $2, $3, "", "global");}
    | LOADADDR IMMNAME REGNAME                      {new_expr(expr_ldad, $2, $3, "", "stack");}
    | LOADADDR VARNAME REGNAME                      {new_expr(expr_ldad, $2, $3, "", "global");}
    | RETURN                                        {new_expr(expr_retu, "", "", "", "");}
    | END FUNCNAME                                  {new_expr(expr_endf, $2, "", "", "");}
    ;

%%
