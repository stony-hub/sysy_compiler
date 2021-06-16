%{
#define YYSTYPE const char*
#include "lines.h"
#include "step2.tab.hpp"

%}

integer [1-9][0-9]*|\-[1-9][0-9]*|"0"
letter [a-zA-Z]
iden (_|{letter})(_|[0-9]|{letter})*
space [ \t]+
enter [\n]
comment "//"[^\n]*
function f_{iden}
variable (T|t|p){integer}
label l{integer}

%%

{comment}   {}
{space}     {}
{enter}     {}
var         {return VAR;}
call        {return CALL;}
param       {return PARAM;}
return      {return RETURN;}
end         {return END;}
if          {return IF;}
goto        {return GOTO;}
{variable}  {step2lval = strdup(yytext); return VARNAME;}
{function}  {step2lval = strdup(yytext+2); return FUNCNAME;}
{label}     {step2lval = strdup(yytext); return LABNAME;}
{integer}   {step2lval = strdup(yytext); return IMMNAME;}
"["         {return LSQR;}
"]"         {return RSQR;}
"!"         {return NOT;}
"-"         {return MIN;}
"="         {return ASS;}
"!="        {step2lval = strdup(yytext); return OP2;}
"=="        {step2lval = strdup(yytext); return OP2;}
">"         {step2lval = strdup(yytext); return OP2;}
"<"         {step2lval = strdup(yytext); return OP2;}
">="        {step2lval = strdup(yytext); return OP2;}
"<="        {step2lval = strdup(yytext); return OP2;}
"&&"        {step2lval = strdup(yytext); return OP2;}
"||"        {step2lval = strdup(yytext); return OP2;}
"+"         {step2lval = strdup(yytext); return OP2;}
"*"         {step2lval = strdup(yytext); return OP2;}
"/"         {step2lval = strdup(yytext); return OP2;}
"%"         {step2lval = strdup(yytext); return OP2;}
":"         {return COL;}

%%


int yywrap() {
    return 1;
}
