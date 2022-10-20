#ifndef ASM_PARSER_H
#define ASM_PARSER_H

#include "asm_utils.h"

/**
 * @brief 
 * Assemble program from lines
 * @param[out] state Assembly state
 * @return 0 upon success, -1 otherwise
 */
int assemble(assembly_state* state);

#endif