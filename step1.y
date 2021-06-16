%{
#include "node.h"
#include <cstdio>
#include <cstdlib>

using namespace step1;

NRoot *root; /* the top level root node of our final AST */

extern int yydebug;
extern int step1lex();
extern int step1get_lineno();
extern int step1lex_destroy();
void yyerror(const char *s) { std::printf("Error(line: %d): %s\n", step1get_lineno(), s); step1lex_destroy(); if (!yydebug) std::exit(1); }
#define YYERROR_VERBOSE true
#define YYDEBUG 1
%}

%union {
    int token;
    step1::NIdentifier* ident;
    step1::NExpression* expr;
    step1::NRoot* root;
    step1::NDeclareStatement* declare_statement;
    step1::NFunctionDefine* fundef;
    step1::NDeclare* declare;
    step1::NArrayDeclareInitValue* array_init_value;
    step1::NArrayIdentifier* array_identifier;
    step1::NFunctionCallArgList* arg_list;
    step1::NFunctionDefineArgList* fundefarglist;
    step1::NFunctionDefineArg* fundefarg;
    step1::NBlock* block;
    step1::NStatement* stmt;
    step1::NAssignment* assignmentstmt;
    step1::NIfElseStatement* ifelsestmt;
    step1::NConditionExpression* condexp;
    std::string *string;
}

%token <string> INTEGER_VALUE IDENTIFIER
%token <token> IF ELSE WHILE BREAK CONTINUE RETURN
%token <token> CONST INT VOID
%token <token> ASSIGN EQ NE LT LE GT GE
%token <token> AND OR
%token <token> LPAREN RPAREN LBRACE RBRACE LSQUARE RSQUARE

%token <token> DOT COMMA SEMI
%token <token> PLUS MINUS MUL DIV MOD NOT

%type <token> AddOp MulOp RelOp UnaryOp BType
%type <ident> ident LVal
%type <expr> Number Exp InitVal LOrExp LAndExp EqExp AddExp MulExp PrimaryExp RelExp UnaryExp FunctionCall
%type <root> CompUnit
%type <declare_statement> Decl ConstDecl VarDecl ConstDeclStmt VarDeclStmt
%type <declare> Def DefOne DefArray ConstDef ConstDefOne ConstDefArray
%type <array_identifier> DefArrayName ArrayItem
%type <fundef> FuncDef
%type <fundefarglist> FuncFParams
%type <fundefarg> FuncFParam FuncFParamArray FuncFParamOne
%type <array_init_value> InitValArray InitValArrayInner
%type <arg_list> FuncRParams
%type <block> Block BlockItems
%type <stmt> BlockItem Stmt AssignStmt AssignStmtWithoutSemi IfStmt ReturnStmt WhileStmt BreakStmt ContinueStmt
%type <condexp> Cond

%start CompUnit
%%

CompUnit: CompUnit Decl { $$->body.push_back($<declare>2); }
        | CompUnit FuncDef { $$->body.push_back($<fundef>2); }
        | Decl { root = new NRoot(); $$ = root; $$->body.push_back($<declare>1); }
        | FuncDef { root = new NRoot(); $$ = root; $$->body.push_back($<fundef>1); }
        ;

Decl: ConstDeclStmt
    | VarDeclStmt
    ;

BType: INT;

ConstDeclStmt: ConstDecl SEMI { $$ = $1; };

ConstDecl: CONST BType ConstDef { $$ = new NDeclareStatement($2); $$->list.push_back($3); }
         | ConstDecl COMMA ConstDef { $$->list.push_back($3); }
         ;

VarDeclStmt: VarDecl SEMI { $$ = $1; };

VarDecl: BType Def { $$ = new NDeclareStatement($1); $$->list.push_back($2); }
       | VarDecl COMMA Def { $$->list.push_back($3); }
       ;

Def: DefOne
   | DefArray
   ;

DefOne: ident ASSIGN InitVal { $$ = new NVarDeclareWithInit(*$1, *$3); }
      | ident { $$ = new NVarDeclare(*$1); }
      ;

DefArray: DefArrayName ASSIGN InitValArray { $$ = new NArrayDeclareWithInit(*$1, *$3); }
        | DefArrayName { $$ = new NArrayDeclare(*$1); }
        ;

ConstDef: ConstDefOne
        | ConstDefArray
        ;

ConstDefOne: ident ASSIGN InitVal { $$ = new NVarDeclareWithInit(*$1, *$3, true); }
           ;

ConstDefArray: DefArrayName ASSIGN InitValArray { $$ = new NArrayDeclareWithInit(*$1, *$3, true); }
             ;

DefArrayName: DefArrayName LSQUARE Exp RSQUARE { $$ = $1; $$->shape.push_back($3); }
            | ident LSQUARE Exp RSQUARE { $$ = new NArrayIdentifier(*$1); $$->shape.push_back($3); }
            ;

InitVal: AddExp;

InitValArray: LBRACE InitValArrayInner RBRACE { $$ = $2; }
            | LBRACE RBRACE { $$ = new NArrayDeclareInitValue(false, nullptr); }
            ;

InitValArrayInner: InitValArrayInner COMMA InitValArray { $$ = $1; $$->value_list.push_back($3); }
                 | InitValArrayInner COMMA InitVal { $$ = $1; $$->value_list.push_back(new NArrayDeclareInitValue(true, $3)); }
                 | InitValArray { $$ = new NArrayDeclareInitValue(false, nullptr); $$->value_list.push_back($1); }
                 | InitVal { $$ = new NArrayDeclareInitValue(false, nullptr); $$->value_list.push_back(new NArrayDeclareInitValue(true, $1)); }
                 ;

Exp: AddExp;

LOrExp: LAndExp OR LAndExp { $$ = new NBinaryExpression(*$1, $2, *$3); }
      | LOrExp OR LAndExp { $$ = new NBinaryExpression(*$1, $2, *$3); }
      | LAndExp
      ;

LAndExp: LAndExp AND EqExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
       | EqExp
       ;

EqExp: RelExp EQ RelExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
     | RelExp NE RelExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
     | RelExp
     ;

RelExp: AddExp
      | RelExp RelOp AddExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
      ;

AddExp: AddExp AddOp MulExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
      | MulExp
      ;

MulExp: MulExp MulOp UnaryExp  { $$ = new NBinaryExpression(*$1, $2, *$3); }
      | UnaryExp
      ;

UnaryExp: UnaryOp UnaryExp  { $$ = new NUnaryExpression($1, *$2); }
        | FunctionCall
        | PrimaryExp
        ;

FunctionCall: ident LPAREN FuncRParams RPAREN { $$ = new NFunctionCall(*$1, *$3); };
            | ident LPAREN RPAREN { $$ = new NFunctionCall(*$1, *(new NFunctionCallArgList())); }
            ;

PrimaryExp: LVal
          | Number
          | LPAREN Cond RPAREN { $$ = $2; }
          | AssignStmtWithoutSemi
          ;

ArrayItem: LVal LSQUARE Exp RSQUARE { $$ = new NArrayIdentifier(*$1); $$->shape.push_back($3);}
         | ArrayItem LSQUARE Exp RSQUARE { $$ = $1; $$->shape.push_back($3);}
         ;

LVal: ArrayItem
    | ident
    ;

FuncDef: BType ident LPAREN FuncFParams RPAREN Block { $$ = new NFunctionDefine($1, *$2, *$4, *$6); }
       | BType ident LPAREN RPAREN Block { $$ = new NFunctionDefine($1, *$2, *(new NFunctionDefineArgList()), *$5); }
       | VOID ident LPAREN FuncFParams RPAREN Block { $$ = new NFunctionDefine($1, *$2, *$4, *$6); }
       | VOID ident LPAREN RPAREN Block { $$ = new NFunctionDefine($1, *$2, *(new NFunctionDefineArgList()), *$5); }
       ;


FuncFParams: FuncFParams COMMA FuncFParam { $$->list.push_back($3); }
           | FuncFParam {{ $$ = new NFunctionDefineArgList(); $$->list.push_back($1); }}
           ;

FuncFParam: FuncFParamOne
          | FuncFParamArray
          ;

FuncRParams: FuncRParams COMMA AddExp { $$ = $1; $$->args.push_back($3); }
           | AddExp { $$ = new NFunctionCallArgList(); $$->args.push_back($1); }
           ;

FuncFParamOne: BType ident { $$ = new NFunctionDefineArg($1, *$2); };

FuncFParamArray: FuncFParamOne LSQUARE RSQUARE {
                        $$ = new NFunctionDefineArg(
                              $1->type,
                              *new NArrayIdentifier(*(new NArrayIdentifier($1->name))));
                        ((NArrayIdentifier*)&($$->name))->shape.push_back(new NNumber(1));
                        delete $1;
                  }
               | FuncFParamArray LSQUARE Exp RSQUARE { $$ = $1; ((NArrayIdentifier*)&($$->name))->shape.push_back($3);; }
               ;

Block: LBRACE RBRACE { $$ = new NBlock(); }
     | LBRACE BlockItems RBRACE { $$ = $2; }
     ;

BlockItems: BlockItem { $$ = new NBlock(); $$->statements.push_back($1); }
          | BlockItems BlockItem { $$ = $1; $$->statements.push_back($2); }
          ;

BlockItem: Decl
         | Stmt
         ;

Stmt: Block
    | AssignStmt
    | IfStmt
    | ReturnStmt
    | WhileStmt
    | BreakStmt
    | ContinueStmt
    | Exp SEMI { $$ = new  NEvalStatement(*$1); }
    | SEMI { $$ = new NVoidStatement(); }
    ;

AssignStmt: AssignStmtWithoutSemi SEMI { $$ = $1; }

AssignStmtWithoutSemi: LVal ASSIGN AddExp { $$ = new NAssignment(*$1, *$3); };

IfStmt: IF LPAREN Cond RPAREN Stmt { $$ = new NIfElseStatement(*$3, *$5, *new NVoidStatement()); }
      | IF LPAREN Cond RPAREN Stmt ELSE Stmt { $$ = new NIfElseStatement(*$3, *$5, *$7); }
      ;

ReturnStmt: RETURN Exp SEMI { $$ = new NReturnStatement($2); }
          | RETURN SEMI { $$ = new NReturnStatement(); }
          ;

WhileStmt: WHILE LPAREN Cond RPAREN Stmt { $$ = new NWhileStatement(*$3, *$5);};

BreakStmt: BREAK SEMI { $$ = new NBreakStatement(); };

ContinueStmt: CONTINUE SEMI { $$ = new NContinueStatement(); };

Cond: LOrExp;

Number: INTEGER_VALUE { $$ = new NNumber(*$1); };

AddOp: PLUS
     | MINUS
     ;

MulOp: MUL
     | DIV
     | MOD
     ;

UnaryOp: PLUS
       | MINUS
       | NOT
       ;

RelOp: GT
     | GE
     | LT
     | LE
     ;

ident: IDENTIFIER { $$ = new NIdentifier(*$1); }
      ;

%%
