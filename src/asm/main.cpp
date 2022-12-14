/**
 * @file main.cpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
 * @brief Code assembler for virtual machine
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <fcntl.h>
#include <unistd.h>

#include "asm_parser.h"
#include "asm_flags.h"
#include "logger.h"

enum status
{
    STATUS_SUCCESS,
    STATUS_INPUT_ERROR,
    STATUS_RUNTIME_ERROR
};

int main(int argc, char** argv)
{
#ifndef NLOGS
    add_default_file_logger();
#endif
    add_logger(
        {
            .name = "Console output",
            .stream = stdout,
            .logging_level = LOG_ERROR,
            .settings_mask = LGS_USE_ESCAPE | LGS_KEEP_OPEN
        });

    argc--;
    argv++;

    int parsed = parse_args((size_t)argc, argv, TAG_COUNT, TAGS);
    LOG_ASSERT_ERROR(parsed != -1, return STATUS_INPUT_ERROR,
        "Invalid command-line options", NULL);
    LOG_ASSERT_ERROR(argc - parsed  >= 1, return STATUS_INPUT_ERROR,
        "Input file not specified", NULL);
    LOG_ASSERT_ERROR(argc - parsed  == 1, return STATUS_INPUT_ERROR,
        "Too many files specified", NULL);

    const char* filepath = argv[argc - 1];

    assembly_state *state = assembly_state_ctor(filepath);
    LOG_ASSERT(state != NULL, return STATUS_INPUT_ERROR);

    int status = assemble(state);
    LOG_ASSERT_ERROR(status != -1,
        {
            assembly_state_dtor(state);
            return STATUS_INPUT_ERROR;
        },
        "Assembly of file '%s' failed.", filepath);

    if (LISTING_FILE)
        print_listing(LISTING_FILE, state);

    status = create_executable(OUTPUT_FILE, state);

    assembly_state_dtor(state);

    return status == 0 ? STATUS_SUCCESS : STATUS_INPUT_ERROR;
}
