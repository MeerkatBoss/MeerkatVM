/**
 * @file main.cpp
 * @author Solodovnikov Ivan (solodovnikov.ia@phystech.edu)
 * @brief Code assembler for virtual processor. Human readable text goes in, machine-readable binary goes out.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <fcntl.h>
#include <unistd.h>

#include "asm_utils.h"
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
    add_default_file_logger();
    add_logger(
        {
            .name = "Console output",
            .stream = stdout,
            .logging_level = LOG_ERROR,
            .settings_mask = LGS_USE_ESCAPE | LGS_KEEP_OPEN
        });

    LOG_ASSERT_ERROR(argc >= 2, return STATUS_INPUT_ERROR,
        "Incorrect program usage", NULL);
    int parsed = parse_args((size_t)argc - 2, argv + 1, TAG_COUNT, TAGS);
    LOG_ASSERT_ERROR(parsed != -1, return STATUS_INPUT_ERROR,
        "Invalid command-line options", NULL);

    const char* filepath = argv[argc - 1];
    TextLines text_lines = read_file(filepath);
    LOG_ASSERT_ERROR(text_lines.text != NULL, 
        {
            dispose_lines(&text_lines);
            return STATUS_INPUT_ERROR;
        },
        "Could not read file \'%s\'", filepath);

    assembly_state *result = (assembly_state*) calloc(1, sizeof(*result));
    int status = assemble(&text_lines, result);
    LOG_ASSERT_ERROR(status != -1,
        {
            dispose_lines(&text_lines);
            if (result->cmd)
                free(result->cmd);
            free(result);
            return STATUS_INPUT_ERROR;
        },
        "Assembly of file %s failed.", filepath);

    if (listing_file)
        print_listing(listing_file, &text_lines, result->cmd);

    int output_fd = creat(output_file, S_IWUSR | S_IRUSR);
    LOG_ASSERT_ERROR(output_fd != -1,
        {
            dispose_lines(&text_lines);
            free(result->cmd);
            free(result);
            return STATUS_INPUT_ERROR;
        },
        "Could not create output file %s", output_file);
    write(output_fd, &result->header, sizeof(result->header));
    write(output_fd, result->cmd, result->ip*sizeof(*result->cmd));
    close(output_fd);
    
    dispose_lines(&text_lines);
    free(result->cmd);
    free(result);
    return STATUS_SUCCESS;
}
