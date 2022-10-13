#include <strings.h>

#include "logger.h"
#include "asm_utils.h"

void print_listing(const char* filename, const TextLines* text_lines, const void* cmd_array)
{
    if (!filename) return;
    #define BYTE_FORMAT "%02x"
    const long ASM_WIDTH = 32;

    FILE* listing = fopen(filename, "w");
    LOG_ASSERT_ERROR(listing != NULL, return,
        "Could not open listing file \'%s\'", filename);
    
    const unsigned int *byte_cmd = (const unsigned int*)cmd_array;
    size_t ip = 0;

    for (size_t i = 0; i < text_lines->line_count; i++)
    {
        int printed = 0;
        long total_chars = 0;

        fprintf(listing, "[%#06zx] " BYTE_FORMAT "%n", ip, byte_cmd[ip], &printed);
        size_t arg_count = COMMANDS[byte_cmd[ip]].arg_count;
        ip++;
        total_chars += printed;

        for (size_t j = 0; j < arg_count; j++)
        {
            fprintf(listing, " " BYTE_FORMAT "%n", byte_cmd[ip++], &printed);
            total_chars += printed;
        }
        fprintf(listing, "%*s %s\n", ASM_WIDTH - total_chars,
                "|", text_lines->lines[i].line);
    }

    #undef BYTE_FORMAT
}

/**
 * @brief Get the command by name
 * 
 * @param[in] str Command name
 * @param[out] n_read Number of characters read from `str`
 * @return pointer to matching `command_description` upon success,
 * `NULL` otheriwse
 */
static const command_description* get_cmd_description(const char* str, int* n_read);

/**
 * @brief Parse command argument
 * 
 * @param[in] strargs String representation of an argument
 * @param[out] n_read Number of characters read from 'strargs'
 * @return Formal argument description
 */
static asm_arg asm_parse_arg(const char* strargs, int *n_read);


/**
 * @brief Assemble all command arguments
 * 
 * @param[in] command Command description
 * @param[in] strargs String containing arguments
 * @param[out] buffer Output buffer
 * @return 0 upon success, -1 otherwise
 */
static int assemble_args(const command_description* command, const char* strargs, void* buffer);

int assemble(TextLines* text_lines, assembly_state* state)
{
    #define HEADER  (state->header)
    #define CMD     (state->cmd)
    #define IP      (state->ip)

    CMD = (int*)calloc(
            text_lines->line_count * (MAX_CMD_ARGS + 1),
            sizeof(*CMD));
    IP = 0;

    for (size_t i = 0; i < text_lines->line_count; i++)
    {
        int n_read = 0;
        const char* cur_line = text_lines->lines[i].line;
        const command_description* command =
            get_cmd_description(cur_line, &n_read);
        
        LOG_ASSERT_ERROR(command != NULL, return -1,
            "Unknown command encountered on line %zu: \'%s\'",
            i + 1, cur_line);
        
        CMD[IP++] = command->command;

        int args_ok = assemble_args(command, cur_line + n_read, CMD + IP);
        LOG_ASSERT_ERROR(args_ok != -1, return -1,
            "Invalid arguments given to \'%s\' on line %zu: \'%s\'",
            command->name, i + 1, cur_line);
        
        IP += command->arg_count;
    }
    HEADER = {.header = {.signature = SIGNATURE,
                         .version   = LATEST_VERSION,
                         .opcnt     = IP}};
    return 0;

    #undef HEADER
    #undef CMD
    #undef IP
}

static const command_description* get_cmd_description(const char* str, int* n_read)
{
    static const int BUFFER_SIZE = 256;
    static char buffer[BUFFER_SIZE] = "";
    sscanf(str, " %s%n", buffer, n_read);

    for (size_t i = 0; i < COMMAND_COUNT; i++)
        if (strcasecmp(COMMANDS[i].name, buffer) == 0)
            return &COMMANDS[i];
    return NULL;
}

static int assemble_args(const command_description* command, const char* strargs, void* buffer)
{
    int *int_buf = (int*) buffer;   /* Buffer is int for now */
    
    for (size_t i = 0; i < command->arg_count; i++)
    {
        int n_read = 0;
        asm_arg argument = asm_parse_arg(strargs, &n_read);
        *(int_buf++) = *argument.val_ptr;
        strargs += n_read;
    }

    return 0;
}

static asm_arg asm_parse_arg(const char* strargs, int *n_read)
{
    static int value = 0;

    int read_status = sscanf(strargs, "%d %n", &value, n_read);
    if (read_status != 1)
        return {};
    
    return {.val_ptr = &value, .perms = ARG_RD};
}
