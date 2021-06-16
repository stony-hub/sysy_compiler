#pragma once
#include <string>
#include <ostream>
#include <cstdlib>
#include "lines.h"

#define reg_start 2
#define reg_end 19

namespace step2 {

struct IR_Context {
    int last_pos;
    // var : value
    // _var : addr
    std::string saved_var[28];
    std::string reg_name[28];
    int locked[28];

    IR_Context ();

    void last_pos_incr();

    // find reg which saves var; if not, alloc a reg and load from mem and assign var
    std::string load_reg(std::string var, std::ostream &out, int keep_num = 0, int load_param = 0);
    // find reg which saves var; if not, alloc a reg and assign to var
    std::string alloc_reg(std::string var, std::ostream &out, int keep_num = 0, int load_param = 0);
    // find reg which saves var_addr; if not , alloc a reg and loadaddr var reg
    std::string alloc_addr_reg(std::string var, std::ostream &out, int keep_num = 0, int load_param = 0);
    // alloc a free reg
    std::string free_reg(std::ostream &out);
    // clear register
    void save_reg(std::ostream &out);
    // save one var
    void save_var(int index, std::ostream &out);
    // load one var
    void load_var(std::string var, std::string reg, std::ostream &out);
    // reg id
    int reg_lookup(std::string reg);
    // alloc reg for number
    std::string num_reg(std::string var, std::ostream &out);
    // remove num
    void remove_num_reg();
    // save param
    void save_param(std::ostream &out, int num);
    // load param
    void load_param(std::ostream &out, int num);
    // lock reg
    void lock_reg(std::string reg);
    // release reg
    void release_reg();
};

}
