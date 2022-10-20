/**
 * @file assembler.h
 * @author MeerkatBoss
 * @brief Type definitions for assembler and virtual CPU
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stddef.h>

#define ASM_CMD(name, num, ...) CMD_##name=num,

/**
 * @brief 
 * Byte-codes of virtual CPU instructions
 */
enum cpu_command : unsigned
{
    #include "asm_cmd.h"
};

#undef ASM_CMD

#define ASM_REG(name, num, ...) REG_##name = num,
enum asm_reg : unsigned
{
    #include "asm_reg.h"
};
#undef ASM_REG

const size_t ARG_MAX = 1;

/**
 * @brief 
 * Permissions required for CPU command's formal argument
 */
enum arg_perms : unsigned
{
    ARG_RD      = 01,
    ARG_WR      = 02,
    ARG_RDWR    = ARG_RD | ARG_WR,
    ARG_LABEL   = 04
};

enum arg_flags : unsigned
{
    AF_LAB  = 0x00,
    AF_NUM  = 0x20,
    AF_REG  = 0x40,
    AF_MEM  = 0x80
};

/**
 * @brief 
 * Description of CPU command formal argument
 */
struct asm_arg
{
    int*        val_ptr;
    arg_flags   flags;
    arg_perms   perms;
};

/**
 * @brief 
 * Inforamtion about specific command's formal
 * arguments
 */
struct cmd_args
{
    size_t arg_count;
    asm_arg arg_list[ARG_MAX];
};

/**
 * @brief 
 * Maximum number of arguments required for command
 */
const size_t MAX_CMD_ARGS = 1;

/**
 * @brief 
 * Required executable signature
 */
#define SIGNATURE {'M', 'K', 'B', 'S'}

const char STR_SIGN[5] = SIGNATURE;

/**
 * @brief 
 * Latest command-set version
 */
const unsigned int LATEST_VERSION = 4;

/**
 * @brief 
 * Length of executable file header
 */
const long HEADER_SIZE = 16;

#pragma pack(push)
#pragma pack(1)
/**
 * @brief 
 * Executable file header info
 */
struct file_header
{
    char signature[4];
    unsigned int version;
    size_t opcnt;
};
#pragma pack(pop)

/**
 * @brief 
 * If `file_header` fits in `HEADER_SIZE` bytes this is equal
 * to `HEADER_SIZE`, -1 otherwise (will cause compilation error)
 */
const long RZVD_SIZE = sizeof(file_header) <= HEADER_SIZE ? HEADER_SIZE : -1;

/**
 * @brief 
 * `file_header` padded to match `HEADER_SIZE`
 */
union padded_header
{
    struct file_header header;
    char __rzvd__[RZVD_SIZE];
};

#endif