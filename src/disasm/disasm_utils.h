#ifndef DISASM_UTILS_H
#define DISASM_UTILS_H

const int MAX_LABELS = 256;

struct disasm_state
{
    size_t  program_length;
    size_t  mapping_size;
    int*    cmd;
    size_t  cmd_size;
    size_t  ip;
    long*   labels;
    size_t  label_cnt;
};

disasm_state* disasm_ctor(const char* filename);
void disasm_dtor(disasm_state* state);

int disassemble(const char* output_file, disasm_state* state);

#endif