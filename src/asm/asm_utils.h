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
    ASM_SUCCESS     = 0000,
    ASM_UNCMD       = 0001,
    ASM_ARGCNT      = 0002,
    ASM_INVAL       = 0004,
    ASM_TYPE        = 0010,
    ASM_REDEF       = 0020,
    ASM_SYNTAX      = ASM_UNCMD | ASM_ARGCNT | ASM_INVAL | ASM_TYPE | ASM_REDEF,
    ASM_LABELOVF    = 0040,
    ASM_DEFOVF      = 0100,
    ASM_FIXOVF      = 0200
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
    long addr;
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
 */
struct assembly_state
{
    padded_header   header;         /*<! program file header */
    TextLines       program;        /*<! program file text lines */
    size_t          line_num;       /*<! number of line being processed */
    size_t*         line_addr;      /*<! IP, at which code for given line starts */
    size_t          ip;             /*<! instruction pointer*/
    int*            cmd;            /*<! command array */
    size_t          cmd_size;       /*<! length of command array*/
    asm_label*      labels;         /*<! array of labels */
    asm_def*        definitions;    /*<! array of constant definitions */
    fixup*          fixups;         /*<! array of label fixups */
    assembly_result result;         /*<! assembly result */
};

/**
 * @brief 
 * Construct `assembly_state`
 * @param[in] filename Program file name
 * @return Allocated and constructed `assembly_state`
 */
assembly_state* assembly_state_ctor(const char* filename);

/**
 * @brief 
 * Destroy and free `assembly_state` and its associated resources
 * @param[inout] state State to be destroyed
 */
void assembly_state_dtor(assembly_state* state);

/**
 * @brief 
 * Print listing to specified file
 * @param[in] filename   Name of listing file
 * @param[in] text_lines Array of lines
 * @param[in] cmd_array  Array of parsed commands
 */
void print_listing(const char* filename, const assembly_state* state);

/**
 * @brief 
 * Create executable file from `assembly_state`
 * @param[in] filename Output file name
 * @param[in] state    Assembly state
 */
int create_executable(const char* filename, const assembly_state* state);

/**
 * @brief 
 * Check if string consists only of whitespace characters
 * @param[in] str Checked string
 * @return 0 if string is not empty, non-zero otherwise
 */
int strempty(const char* str);

#endif