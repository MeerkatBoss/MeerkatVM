/**
 * @file vcpu_utils.h
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
 * @brief Utility functions used in virtual processor.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef VCPU_UTILS_H
#define VCPU_UTILS_H

#include "int_stack.h"

const int REG_COUNT = 5;
const int MEM_SIZE  = 1024;

struct proc_state
{
    size_t  program_length;
    size_t  mapping_size;
    int     registers[REG_COUNT];
    int*    cmd;
    Stack*  value_stack;
    Stack*  call_stack;
    int     memory[MEM_SIZE];
};

proc_state proc_ctor(const char* program_file);
void proc_dtor(proc_state* cpu);

int proc_run(proc_state *cpu);

#endif