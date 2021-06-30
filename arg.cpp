#include <cstring>
#include "arg.h"

namespace arg {

std::ostream* out = &std::cout;
char eeyore_file[100] = "ir.eeyore";
char tigger_file[100] = "ir.tigger";

void parse_arg(int argc, char** argv) {
    char* in = NULL;
    int s = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (std::string("-o") == argv[i])
                s = 1;
        }
        else {
            if (s == 1) {
                if (std::string("-") == argv[i])
                    out = &std::cout;
                else
                    out = new std::ofstream(argv[i], std::ofstream::out);
            }
            else if (s == 0) {
                if (std::string("-") == argv[i])
                    in = NULL;
                else
                    in = argv[i];
            }
            s = 0;
        }
    }
    if (in != NULL) {
        freopen(in, "r", stdin);

        strcpy(eeyore_file, in);
        for (int i=0; i<strlen(eeyore_file); i++) {
            if (eeyore_file[i] == '.') {
                strcpy(eeyore_file + i + 1, "eeyore");
                break;
            }
        }

        strcpy(tigger_file, in);
        for (int i=0; i<strlen(tigger_file); i++) {
            if (tigger_file[i] == '.') {
                strcpy(tigger_file + i + 1, "tigger");
                break;
            }
        }
    }
}
}  // namespace config
