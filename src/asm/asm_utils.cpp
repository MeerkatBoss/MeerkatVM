#include <strings.h>

#include "logger.h"
#include "asm_utils.h"


const char** output_file_addr_()
{
    static const char* _output_file = "a.out";
    return &_output_file;
}

const char** listing_file_addr_()
{
    static const char* _listing_file = NULL;
    return &_listing_file;
}


int set_output_file(const char* const * strargs)
{
    static int called = 0;

    LOG_ASSERT(strargs != NULL, return -1);

    LOG_ASSERT_ERROR(!called, return -1,
        "Output file specified more than once", NULL);
    output_file = *strargs;
    called = 1;
    return 0;
}

int add_listing(const char* const* strargs)
{
    static int called = 0;

    LOG_ASSERT(strargs != NULL, return -1);

    LOG_ASSERT_ERROR(!called, return -1,
        "Listing file specified more than once", NULL);
    listing_file = *strargs;
    called = 1;
    return 0;
}

const command_description* get_description(const char* str, int* n_read)
{
    static const int BUFFER_SIZE = 256;
    static char buffer[BUFFER_SIZE] = "";
    sscanf(str, " %s%n", buffer, n_read);

    for (size_t i = 0; i < COMMAND_COUNT; i++)
        if (strcasecmp(COMMANDS[i].name, buffer) == 0)
            return &COMMANDS[i];
    return NULL;
}

int get_args(const command_description* command, const char* strargs, void* buffer)
{
    int *int_buf = (int*) buffer;   /* Buffer is int for now */
    
    for (size_t i = 0; i < command->arg_count; i++)
    {
        int n_read = 0;
        int read_status = sscanf(strargs, "%d %n", int_buf + i, &n_read);
        if (read_status != 1)
            return -1;
        strargs += n_read;
    }

    return 0;
}

void try_print_listing(const TextLines text_lines, const void* cmd_array)
{
    if (!listing_file) return;

    FILE* listing = fopen(listing_file, "w");
    LOG_ASSERT_ERROR(listing != NULL, return,
        "Could not open listing file \'%s\'", listing_file);
    
    const int *int_cmd = (const int*)cmd_array;

    for (size_t i = 0; i < text_lines.line_count; i++)
    {
        fprintf(listing, "%zu:\t%18s\t%d", i + 1, text_lines.lines[i].line, *int_cmd);
        size_t arg_count = COMMANDS[*int_cmd].arg_count;
        int_cmd++;
        for (size_t j = 0; j < arg_count; j++)
            fprintf(listing, " %d", *(int_cmd++));
        fputc('\n', listing);
    }
}
