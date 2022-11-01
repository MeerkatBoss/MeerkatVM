#ifndef DISASM_UTILS_H
#define DISASM_UTILS_H
#include "assembler.h"
#include "disasm_config.h"

struct disasm_state
{
    size_t  program_length;
    size_t  mapping_size;
    byte_t* cmd;
    size_t  cmd_size;
    size_t  ip;
    long*   labels;
    size_t  label_cnt;
    FILE*   output;
};

disasm_state* disasm_ctor(const char* input_file, const char* output_file);
void disasm_dtor(disasm_state* state);

int disassemble(disasm_state* state);

#endif