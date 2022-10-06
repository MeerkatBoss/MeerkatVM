#ifndef ASSEMBLER_H
#define ASSEMBLER_H

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

#include <stddef.h>

/**
 * @brief 
 * Byte-codes of virtual CPU instructions
 */
enum asm_command
{
    CMD_NOP     = 0x00,
    CMD_ADD     = 0x01,
    CMD_INC     = 0x02,
    CMD_SUB     = 0x03,
    CMD_DEC     = 0x04,
    CMD_MUL     = 0x05,
    CMD_DIV     = 0x06,
    CMD_PUSH    = 0x07,
    CMD_POP     = 0x08,
    CMD_HALT    = 0x09
};

/**
 * @brief 
 * Description for virtual CPU command
 */
struct command_description
{
    asm_command command;
    const char* name;
    size_t      arg_count;
};

/**
 * @brief 
 * All commands array
 */
const command_description COMMANDS[] =
{
    {.command = CMD_NOP,    .name = "nop",  .arg_count = 0},
    {.command = CMD_ADD,    .name = "add",  .arg_count = 0},
    {.command = CMD_INC,    .name = "inc",  .arg_count = 0},
    {.command = CMD_SUB,    .name = "sub",  .arg_count = 0},
    {.command = CMD_DEC,    .name = "dec",  .arg_count = 0},
    {.command = CMD_MUL,    .name = "mul",  .arg_count = 0},
    {.command = CMD_DIV,    .name = "div",  .arg_count = 0},
    {.command = CMD_PUSH,   .name = "push", .arg_count = 1},
    {.command = CMD_POP,    .name = "pop",  .arg_count = 0},
    {.command = CMD_HALT,   .name = "halt", .arg_count = 0}
};

/**
 * @brief 
 * Length of `COMMANDS` array
 */
const size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(*COMMANDS);

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
const unsigned int LATEST_VERSION = 0;

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