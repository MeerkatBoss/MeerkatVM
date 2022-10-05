#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "asm_utils.h"
#include "logger.h"
#include "text_lines.h"

enum status
{
    STATUS_SUCCESS,
    STATUS_INPUT_ERROR,
    STATUS_RUNTIME_ERROR
};

int main(int argc, char** argv)
{
    add_default_file_logger();
    LOG_ASSERT_ERROR(argc >= 2, return STATUS_INPUT_ERROR,
        "Incorrect program usage", NULL);
    int parsed = parse_args(argc - 2, argv + 1, TAG_COUNT, TAGS);
    LOG_ASSERT_ERROR(argc > 0, return STATUS_INPUT_ERROR,
        "Invalid command-line options", NULL);

    const char* filepath = argv[argc - 1];
    TextLines text_lines = read_file(filepath);
    LOG_ASSERT_ERROR(text_lines.text != NULL, return STATUS_INPUT_ERROR,
        "Could not read file \'%s\'", filepath);
    
    padded_header header = {};
    memset(&header, 0, HEADER_SIZE);    /* is this necessary? */

    for (size_t i = 0; i < text_lines.line_count; i++)
    {

    }


    return 0;
}
