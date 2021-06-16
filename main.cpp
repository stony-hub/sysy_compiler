#include <iostream>
#include <sstream>
#include "arg.h"
#include "node.h"
#include "lines.h"
#include "riscv.h"

extern step1::Node *root;
extern int step1parse();
extern int step1lex_destroy();
extern void step1set_lineno(int _line_number);

extern int step2parse();

extern int step3parse();

step1::Context ctx_1;
step2::IR_Context ctx_2;

int main(int argc, char** argv) {
    arg::parse_arg(argc, argv);

    // step1

    std::ofstream *step1_out = new std::ofstream(arg::eeyore_file, std::ofstream::out);

    step1set_lineno(1);
    step1parse();
    step1lex_destroy();

    root->ir(ctx_1, *step1_out);

    step1_out->close();

    // step2

    freopen(arg::eeyore_file, "r", stdin);
    std::ofstream *step2_out = new std::ofstream(arg::tigger_file, std::ofstream::out);

    step2parse();
    step2::translate(ctx_2, *step2_out);

    step2_out->close();

    // step3

    freopen(arg::tigger_file, "r", stdin);

    step3parse();
    step3::translate(*arg::out);

    return 0;
};
