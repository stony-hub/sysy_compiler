%{
#include <string>
#include "node.h"
#include "step1.tab.hpp"

#define SAVE_TOKEN  step1lval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (step1lval.token = t)
#define FIX_LINE    for (auto i : std::string(yytext_ptr)) { if (i == '\n') yylineno++; }
%}

%option noyywrap

%%

\/\/.*\n?                           FIX_LINE;
"/*"                        		{
                                        int c;
                                        while((c = yyinput()) != 0) {
                                            if(c == '\n')
                                                yylineno++;
                                            else if(c == '*') {
                                                if((c = yyinput()) == '/')
                                                    break;
                                                else
                                                    unput(c);
                                                }
                                        }
                                    }
[ ]                                 ;
\n                                  yylineno++;
[\t\r]                              ;
"if"                                return TOKEN(IF);
"else"                              return TOKEN(ELSE);
"while"                             return TOKEN(WHILE);
"break"                             return TOKEN(BREAK);
"continue"                          return TOKEN(CONTINUE);
"return"                            return TOKEN(RETURN);
"const"                             return TOKEN(CONST);
"int"                               return TOKEN(INT);
"void"                              return TOKEN(VOID);
"putf"[ \t\n]*"("                   FIX_LINE; step1lval.string = new std::string("printf"); *yy_cp = yy_hold_char; yy_hold_char='(';yy_cp--; yyleng--; yy_c_buf_p--; return IDENTIFIER;
"starttime"[ \t\n]*"("              FIX_LINE; step1lval.string = new std::string("_sysy_starttime"); *yy_cp = yy_hold_char; yy_hold_char='(';yy_cp--; yyleng--; yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='E';yy_c_buf_p--; *yy_c_buf_p='N';yy_c_buf_p--; *yy_c_buf_p='I';yy_c_buf_p--; *yy_c_buf_p='L';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; return IDENTIFIER;
"stoptime"[ \t\n]*"("               FIX_LINE; step1lval.string = new std::string("_sysy_stoptime"); *yy_cp = yy_hold_char; yy_hold_char='(';yy_cp--; yyleng--; yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='E';yy_c_buf_p--; *yy_c_buf_p='N';yy_c_buf_p--; *yy_c_buf_p='I';yy_c_buf_p--; *yy_c_buf_p='L';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; *yy_c_buf_p='_';yy_c_buf_p--; return IDENTIFIER;
"__LINE__"                          step1lval.string = new std::string(std::to_string(yyget_lineno())); return INTEGER_VALUE;
"_SYSY_N"                           step1lval.string = new std::string(std::to_string(1024)); return INTEGER_VALUE;
[a-zA-Z_][a-zA-Z0-9_]*              SAVE_TOKEN; return IDENTIFIER;
[0-9]+                              SAVE_TOKEN; return INTEGER_VALUE;
"0x"[0-9a-fA-F]+                    SAVE_TOKEN; return INTEGER_VALUE;

"="                                 return TOKEN(ASSIGN);
"=="                                return TOKEN(EQ);
"!="                                return TOKEN(NE);
"<"                                 return TOKEN(LT);
"<="                                return TOKEN(LE);
">"                                 return TOKEN(GT);
">="                                return TOKEN(GE);
"&&"                                return TOKEN(AND);
"||"                                return TOKEN(OR);

"("                                 return TOKEN(LPAREN);
")"                                 return TOKEN(RPAREN);
"["                                 return TOKEN(LSQUARE);
"]"                                 return TOKEN(RSQUARE);
"{"                                 return TOKEN(LBRACE);
"}"                                 return TOKEN(RBRACE);

"."                                 return TOKEN(DOT);
","                                 return TOKEN(COMMA);
";"                                 return TOKEN(SEMI);

"+"                                 return TOKEN(PLUS);
"-"                                 return TOKEN(MINUS);
"*"                                 return TOKEN(MUL);
"/"                                 return TOKEN(DIV);
"%"                                 return TOKEN(MOD);
"!"                                 return TOKEN(NOT);

.                                   printf("Unknown token: %s\n", yytext); yyterminate();

%%
