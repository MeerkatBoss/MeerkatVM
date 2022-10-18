/**
 * @file asm_utils.h
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
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

/**
 * @brief 
 * Asssembly result description
 */
enum assembly_result
{
    ASM_SUCCESS     = 0,
    ASM_SYNTAX      = -1,
    ASM_LABEL_OVF   = -2,
    ASM_DEFS_OVF    = -3,
    ASM_FIXUP_OVF   = -4
};

/**
 * @brief 
 * Maximum length of label name
 */
const size_t MAX_LABEL_LEN = 32;

/**
 * @brief 
 * Assembler label description
 */
struct asm_label
{
    char name[MAX_LABEL_LEN];
    ssize_t addr;
};

/**
 * @brief 
 * Unknown label reference
 */
struct fixup
{
    size_t label_number;
    size_t addr;
};

/**
 * @brief 
 * Assembler constant definition
 */
struct asm_def
{
    char name[MAX_LABEL_LEN];
    int value;
};

/**
 * @brief 
 * Maximum allowed number of labels in a program
 */
const size_t MAX_LABELS = 64;

/**
 * @brief 
 * Maximum allowed number of unknown label references
 */
const size_t MAX_FIXUPS = MAX_LABELS * 4;

/**
 * @brief 
 * State of assembly process
 * 
 * @warning
 * Size of this struct exceeds 8kB. Avoid stack-allocating or
 * copying it
 */
struct assembly_state
{
    padded_header   header;
    size_t          ip;
    int*            cmd;
    size_t          cmd_size;
    asm_label       labels[MAX_LABELS];
    asm_def         definitions[MAX_LABELS];
    fixup           fixups[MAX_FIXUPS];
    assembly_result result;
};

/**
 * @brief 
 * Assemble program from lines
 * @param[in] text_lines Program lines
 * @param[out] state     Assembly state
 * @return 0 upon success, -1 otherwise
 */
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