#ifndef VCPU_UTILS_H
#define VCPU_UTILS_H

#include "int_stack.h"

/**
 * @brief 
 * Allocate and read commands array from specified file
 * @param[in] filename Name of program file
 * @param[out] array_len Length of allocated array
 * @return Allocated array
 */
int* read_cmd_array(const char* filename, size_t* array_len);

/**
 * @brief 
 * Read and execute command from commands array
 * @param[in] cmd_array Commands array
 * @param[inout] stack  Program stack
 * @param[out] shift    Instruction pointer shift
 * @return 0 upon success, -1 otherwise
 */
int execute_command(const int* cmd_array, Stack* stack, size_t* shift);

#endif