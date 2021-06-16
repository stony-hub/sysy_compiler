%{
#include "riscv.h"
#include "step3.tab.hpp"
%}

integer [1-9][0-9]*|\-[1-9][0-9]*|"0"
letter [a-zA-Z]
iden (_|{letter})(_|[0-9]|{letter})*
space [ \t]+
enter [\n]
comment "//"[^\n]*
function f_{iden}
register (x|s|t|a){integer}
variable v{integer}
label l{integer}

%%

{comment}   {}
{space}     {}
{enter}     {}
store       {return STORE;}
call        {return CALL;}
malloc      {return MALLOC;}
end         {return END;}
if          {return IF;}
goto        {return GOTO;}
loadaddr    {return LOADADDR;}
load        {return LOAD;}
return      {return RETURN;}
{variable}  {step3lval = strdup(yytext); return VARNAME;}
{register}  {step3lval = strdup(yytext); return REGNAME;}
{function}  {step3lval = strdup(yytext+2); return FUNCNAME;}
{label}     {step3lval = strdup(yytext); return LABNAME;}
{integer}   {step3lval = strdup(yytext); return IMMNAME;}
"["         {return LSQR;}
"]"         {return RSQR;}
"!"         {return NOT;}
"-"         {return MIN;}
"="         {return ASS;}
"!="        {step3lval = strdup(yytext); return OP2;}
"=="        {step3lval = strdup(yytext); return OP2;}
">"         {step3lval = strdup(yytext); return OP2;}
"<"         {step3lval = strdup(yytext); return OP2;}
">="        {step3lval = strdup(yytext); return OP2;}
"<="        {step3lval = strdup(yytext); return OP2;}
"&&"        {step3lval = strdup(yytext); return OP2;}
"||"        {step3lval = strdup(yytext); return OP2;}
"+"         {step3lval = strdup(yytext); return OP2;}
"*"         {step3lval = strdup(yytext); return OP2;}
"/"         {step3lval = strdup(yytext); return OP2;}
"%"         {step3lval = strdup(yytext); return OP2;}
":"         {return COL;}

%%

int yywrap() {
    return 1;
}
