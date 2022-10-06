#include <stdio.h>
#include <string.h>

#include "asm_utils.h"
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
    LOG_ASSERT_ERROR(argc >= 2, return STATUS_INPUT_ERROR,
        "Incorrect program usage", NULL);
    int parsed = parse_args(argc - 2, argv + 1, TAG_COUNT, TAGS);
    LOG_ASSERT_ERROR(parsed != -1, return STATUS_INPUT_ERROR,
        "Invalid command-line options", NULL);

    const char* filepath = argv[argc - 1];
    TextLines text_lines = read_file(filepath);
    LOG_ASSERT_ERROR(text_lines.text != NULL, return STATUS_INPUT_ERROR,
        "Could not read file \'%s\'", filepath);
    
    int *command_array = (int*)calloc(
            text_lines.line_count * (MAX_CMD_ARGS + 1),
            sizeof(*command_array));
    size_t instruction_ptr = 0;

    for (size_t i = 0; i < text_lines.line_count; i++)
    {
        /*log_message(MSG_TRACE, "IP = %zu", instruction_ptr);
        for (size_t j = 0; j < instruction_ptr + 1; j++)
            log_message(MSG_TRACE, "command_array[%02zu] = %d", j, command_array[j]);*/
        int n_read = 0;
        const char* cur_line = text_lines.lines[i].line;
        const command_description* command =
            get_description(cur_line, &n_read);
        
        LOG_ASSERT_ERROR(command != NULL, return STATUS_INPUT_ERROR,
            "Unknown command encountered on line %zu: \'%s\'",
            i + 1, cur_line);
        
        command_array[instruction_ptr++] = command->command;


        int args_ok = get_args(command, cur_line + n_read, command_array + instruction_ptr);
        LOG_ASSERT_ERROR(args_ok != -1, return STATUS_INPUT_ERROR,
            "Invalid arguments given to \'%s\' on line %zu: \'%s\'",
            command->name, i + 1, cur_line);

        instruction_ptr += command->arg_count;
    }
    try_print_listing(text_lines, command_array);

    /*
    padded_header preamble = {
                    .header = {.signature = SIGNATURE,
                     .version   = LATEST_VERSION,
                     .opcnt     = instruction_ptr}};*/

    FILE* output = fopen(output_file, "w");
    LOG_ASSERT_ERROR(output != NULL, return STATUS_INPUT_ERROR,
        "Could not open output file \'%s\'", output_file);
    
    fprintf(output, "%.4s %u\n%zu\n", STR_SIGN, LATEST_VERSION, instruction_ptr);

    for (size_t i = 0; i < instruction_ptr; i++)
        fprintf(output, "%u ", (unsigned int)command_array[i]);
    
    fclose(output);

    free(command_array);
    dispose_lines(&text_lines);
    return 0;
}
