#pragma once
#include <cstdio>
#include <fstream>
#include <iostream>

namespace arg {
extern std::ostream* out;
extern char eeyore_file[100], tigger_file[100];

void parse_arg(int argc, char** argv);
}  // namespace config
