/**
 * @file asm_utils.h
 * @author Solodovnikov Ivan (solodovnikov.ia@phystech.edu)
 * @brief Utility functions used in assembler program.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ASM_UTILS_H
#define ASM_UTILS_h

#include "assembler.h"
#include "text_lines.h"

enum assembly_result
{
    ASM_SUCCESS     = 0,
    ASM_SYNTAX      = -1,
    ASM_LABEL_OVF   = -2,
    ASM_DEFS_OVF    = -3,
    ASM_FIXUP_OVF   = -4
};

const size_t MAX_LABEL_LEN = 32;
struct asm_label
{
    char name[32];
    ssize_t addr;
};

struct fixup
{
    size_t label_number;
    size_t addr;
};

struct asm_def
{
    char name[MAX_LABEL_LEN];
    int value;
};

const size_t MAX_LABELS = 64;
const size_t MAX_FIXUPS = MAX_LABELS * 4;

struct assembly_state
{
    padded_header   header;
    size_t          ip;
    int*            cmd;
    asm_label       labels[MAX_LABELS];
    asm_def         definitions[MAX_LABELS];
    fixup           fixups[MAX_FIXUPS];
    assembly_result result;
};

int assemble(TextLines* text_lines, assembly_state* state);

/**
 * @brief 
 * Print listing to specified file
 * @param[in] filename   Name of listing file
 * @param[in] text_lines Array of lines
 * @param[in] cmd_array  Array of parsed commands
 */
void print_listing(const char* filename, const TextLines* text_lines, const void* cmd_array);

#endif