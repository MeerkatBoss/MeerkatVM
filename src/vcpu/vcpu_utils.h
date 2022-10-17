#ifndef VCPU_UTILS_H
#define VCPU_UTILS_H

#include "int_stack.h"

const int REG_COUNT = 5;

struct proc_state
{
    size_t  program_length;
    size_t  mapping_size;
    int     registers[REG_COUNT];
    int*    cmd;
    Stack*  value_stack;
    Stack*  call_stack;
};

proc_state proc_ctor(const char* program_file);
void proc_dtor(proc_state* cpu);

int proc_run(proc_state *cpu);

#endif