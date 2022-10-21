/**
 * @file argparser.h
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
 * @brief Program for disassembling virtual processor instructions.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "logger.h"
#include "disasm_utils.h"

int main(int argc, char** argv)
{
    add_default_file_logger();
    add_logger(
        {
            .name = "Console output",
            .stream = stdout,
            .logging_level = LOG_ERROR,
            .settings_mask = LGS_USE_ESCAPE | LGS_KEEP_OPEN
        });

    LOG_ASSERT_ERROR(argc == 2, return -1,
        "Invalid program usage", NULL);

    disasm_state* state = disasm_ctor(argv[1]);
    int result = disassemble("disasm.txt", state);
    state->ip = 0;
    result = disassemble("disasm.txt", state);
    disasm_dtor(state);

    return result;
}
