#include "reg.h"

namespace step2 {

extern int param_cnt;

IR_Context::IR_Context() {
    last_pos = reg_start;
    int id = 0;
    reg_name[id++] = "x0";
    for (int i=0; i<12; i++)
        reg_name[id++] = std::string("s") + std::to_string(i);
    for (int i=0; i<7; i++)
        reg_name[id++] = std::string("t") + std::to_string(i);
    for (int i=0; i<8; i++)
        reg_name[id++] = std::string("a") + std::to_string(i);
    
    for (int i=0; i<28; i++)
        saved_var[i] = " ";

    for (int i=0; i<28; i++)
        locked[i] = 0;
}

void IR_Context::last_pos_incr() {
    last_pos++;
    if (last_pos == reg_end) last_pos = reg_start;
}

void IR_Context::save_param(std::ostream &out, int num) {
    for (int i=0; i<num; i++) {
        out << "store a" << i << " " << i << std::endl;
    }
}

void IR_Context::load_param(std::ostream &out, int num) {
    for (int i=0; i<num; i++) {
        out << "load " << i << " a" << i << std::endl;
    }
}

void IR_Context::save_var(int index, std::ostream &out) {
    std::string reg = reg_name[index];
    std::string var = saved_var[index];
    saved_var[index] = " ";

    // store global var
    if (var[0] == 'v') {
        // loadaddr var t6
        // t6[0] = reg
        out << "loadaddr " << var << " t6" << std::endl;
        out << "t6[0] = " << reg << std::endl;
    }
    // store local var
    else if (var[0] == 'w') {
        // store reg num
        out << "store " << reg << " " << atoi(var.substr(1).c_str()) + param_cnt << std::endl;
    }
    // remove addr and number
    else {}
}

void IR_Context::load_var(std::string var, std::string reg, std::ostream &out) {
    // load global var
    if (var[0] == 'v') {
        out << "load " << var << " " << reg << std::endl;
    }
    // load local var
    else if (var[0] == 'w') {
        out << "load " << atoi(var.substr(1).c_str()) + param_cnt << " " << reg << std::endl;
    }
    // load saved param
    else {
        out << "load " << var.substr(1) << " " << reg << std::endl;
    }
}

std::string IR_Context::free_reg(std::ostream &out) {
    for (int i=reg_start; i<reg_end; i++) {
        if (saved_var[i] == " ")
            return reg_name[i];
    }

    while (locked[last_pos])
        last_pos_incr();
    int index = last_pos;
    last_pos_incr();

    save_var(index, out);

    return reg_name[index];
}

void IR_Context::save_reg(std::ostream &out) {
    for (int i=reg_start; i<reg_end; i++) {
        if (saved_var[i] != " ")
            save_var(i, out);
    }
}

std::string IR_Context::num_reg(std::string var, std::ostream &out) {
    std::string reg = free_reg(out);
    saved_var[reg_lookup(reg)] = "@";
    out << reg << " = " << var << std::endl;
    return reg;
}

void IR_Context::remove_num_reg() {
    for (int i=reg_start; i<reg_end; i++)
        if (saved_var[i] == "@")
            saved_var[i] = " ";
}

std::string IR_Context::load_reg(std::string var, std::ostream &out, int keep_num, int load_param) {
    if (var[0] == 'a') {
        if (!load_param) return var;
    }
    if (is_num(var)) {
        if (keep_num) return var;
        else return num_reg(var, out);
    }

    for (int i=reg_start; i<reg_end; i++) {
        if (saved_var[i] == var)
            return reg_name[i];
    }

    std::string reg = free_reg(out);
    load_var(var, reg, out);
    saved_var[reg_lookup(reg)] = var;
    return reg;
}

std::string IR_Context::alloc_reg(std::string var, std::ostream &out, int keep_num, int load_param) {
    if (var[0] == 'a') return var;
    if (is_num(var)) {
        if (keep_num) return var;
        else return num_reg(var, out);
    }

    for (int i=reg_start; i<reg_end; i++) {
        if (saved_var[i] == var)
            return reg_name[i];
    }

    std::string reg = free_reg(out);
    saved_var[reg_lookup(reg)] = var;
    return reg;
}

std::string IR_Context::alloc_addr_reg(std::string var, std::ostream &out, int keep_num, int load_param) {
    if (var[0] == 'a') return var;
    if (is_num(var)) {
        if (keep_num) return var;
        else return num_reg(var, out);
    }

    for (int i=reg_start; i<reg_end; i++) {
        if (saved_var[i].substr(1) == var)
            return reg_name[i];
    }

    std::string reg = free_reg(out);
    saved_var[reg_lookup(reg)] = std::string("_") + var;

    if (var[0] == 'v') {
        out << "loadaddr " << var << " " << reg << std::endl;
    }
    else {
        out << "loadaddr " << atoi(var.substr(1).c_str()) + param_cnt << " " << reg << std::endl;
    }

    return reg;
}

int IR_Context::reg_lookup(std::string reg) {
    for (int i=reg_start; i<reg_end; i++) {
        if (reg_name[i] == reg)
            return i;
    }
    return -1;
}

void IR_Context::lock_reg(std::string reg) {
    for (int i=reg_start; i<reg_end; i++)
        if (reg_name[i] == reg)
            locked[i] = 1;
}

void IR_Context::release_reg() {
    for (int i=reg_start; i<reg_end; i++)
        locked[i] = 0;
}

}
