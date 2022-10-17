#include <strings.h>
#include <string.h>
#include <ctype.h>

#include "logger.h"
#include "asm_utils.h"

/**
 * @brief 
 * Check if string consists only of whitespace characters
 * @param[in] str Checked string
 * @return 0 if string is not empty, non-zero otherwise
 */
static int strempty(const char* str);

void print_listing(const char* filename, const TextLines* text_lines, const void* cmd_array)
{
    if (!filename) return;
    #define BYTE_FORMAT "%02x"
    const int ASM_WIDTH = 32;

    FILE* listing = fopen(filename, "w");
    LOG_ASSERT_ERROR(listing != NULL, return,
        "Could not open listing file \'%s\'", filename);
    
    const unsigned int *byte_cmd = (const unsigned int*)cmd_array;
    size_t ip = 0;

    for (size_t i = 0; i < text_lines->line_count; i++)
    {
        if (strempty(text_lines->lines[i].line)) continue;
        int printed = 0;
        int total_chars = 0;
        int cur_cmd = byte_cmd[ip++];

        fprintf(listing, "[0x%04zx] " BYTE_FORMAT "%n", ip, cur_cmd, &printed);
        int cmd_mask = AF_NUM - 1;
        size_t arg_count = COMMANDS[cur_cmd & cmd_mask].arg_count;
        total_chars += printed;

        for (size_t j = 0; j < arg_count; j++)
        {   
            fprintf(listing, " " BYTE_FORMAT "%n", byte_cmd[ip++], &printed);
            total_chars += printed;

            if((cur_cmd & (AF_REG | AF_NUM)) == (AF_REG | AF_NUM))
            {
                fprintf(listing, " " BYTE_FORMAT "%n", byte_cmd[ip++], &printed);
                total_chars += printed;
            }
        }
        fprintf(listing, "%*s %s\n", ASM_WIDTH - total_chars,
                "|", text_lines->lines[i].line);
    }

    #undef BYTE_FORMAT
}

/**
 * @brief Parse command argument
 * 
 * @param[in] strargs  String representation of an argument
 * @param[out] n_read  Number of characters read from 'strargs'
 * @param[out] buffer  Command array
 * @param[out] arg_size Number of used array cells upon success 
 * @return `cmd_flags` upon success, -1 otherwise
 */
static int asm_parse_arg(const char* strargs, int *n_read, void *buffer, int *arg_size);


/**
 * @brief Assemble all command arguments
 * 
 * @param[in] args    Formal arguments description
 * @param[in] strargs String containing arguments
 * @param[out] buffer Output buffer
 * @return number of occupied array cells upon success, -1 otherwise
 */
static int assemble_args(cmd_args args, const char* strargs, void* buffer);

int assemble(TextLines* text_lines, assembly_state* state)
{
    static const int BUFFER_SIZE = 256;
    static char buffer[BUFFER_SIZE] = "";

    #define HEADER  (state->header)
    #define CMD     (state->cmd)
    #define IP      (state->ip)

    size_t cmd_len = text_lines->line_count * (2*MAX_CMD_ARGS + 1);

    CMD = (int*)calloc(cmd_len, sizeof(*CMD));
    IP = 0;

    for (size_t i = 0; i < text_lines->line_count; i++)
    {
        int n_read = 0;
        const char* cur_line = text_lines->lines[i].line;
        if (strempty(cur_line)) continue;
        sscanf(cur_line, " %s%n", buffer, &n_read);

        #define ASM_CMD(name, num, args, ...)                                       \
            if (strcasecmp(buffer, #name) == 0)                                     \
            {                                                                       \
                LOG_ASSERT(IP < cmd_len, return -9999);                             \
                CMD[IP] = num;                                                      \
                int args_size = assemble_args(args, cur_line + n_read, CMD + IP);   \
                LOG_ASSERT_ERROR(args_size != -1, return -1,                        \
                    "Invalid arguments given to \'%s\' on line %zu: \'%s\'",        \
                    buffer, i + 1, cur_line);                                       \
                IP += args_size;                                                    \
            } else

        #include "asm_cmd.h"
        /* else */  LOG_ASSERT_ERROR(0, return -1, 
                            "Syntax error on line %zu: unknown command %s",
                            i, buffer);

        #undef ASM_CMD
    }
    HEADER = {.header = {.signature = SIGNATURE,
                         .version   = LATEST_VERSION,
                         .opcnt     = IP}};
    return 0;

    #undef HEADER
    #undef CMD
    #undef IP
}

static int assemble_args(cmd_args args, const char* strargs, void* buffer)
{
    int *int_buf = (int*) buffer;   /* Buffer is int for now */
    int *buf_start = int_buf;
    int cur_cmd = *buf_start;
    int_buf++;
    
    for (size_t i = 0; i < args.arg_count; i++)
    {
        int n_read = 0;
        int arg_size = 0;
        int arg_flags = asm_parse_arg(strargs, &n_read, int_buf, &arg_size);

        LOG_ASSERT(arg_flags != -1, return -1);
        /* writable argument - memory or register without constant */
        int perms = (arg_flags & AF_MEM) || ((arg_flags & AF_REG) && !(arg_flags & AF_NUM))
                            ? ARG_RDWR
                            : ARG_RD;

        LOG_ASSERT_ERROR((perms & args.arg_list[i].perms) == args.arg_list[i].perms,
            return -1,
            "Argument '%*s': invalid read/write permissions.", n_read, strargs);
        
        *buf_start |= arg_flags;
        int_buf    += arg_size;
        strargs    += n_read;
    }

    return int_buf - buf_start;
}

/**
 * @brief 
 * Parse assembly language literal
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @return `arg_flags` flag upon success, -1 otherwise
 */
static int parse_literal(const char* str, int* value);

static int asm_parse_arg(const char* strargs, int* n_read, void* buffer, int *arg_size)
{
    static const size_t BUFLEN = 256;
    static char BUFFER[BUFLEN] = "";

    int* cmd = (int*) buffer;
    int read_status = sscanf(strargs, "%s %n", BUFFER, n_read);

    LOG_ASSERT_ERROR(read_status == 1, return -1, "Expected argument", NULL);

    unsigned int flags = 0;
    int char_cnt = strlen(BUFFER);
    char *bufptr = BUFFER;
    
    if (*bufptr == '[')   /* Memory access */
    {
        LOG_ASSERT_ERROR(bufptr[char_cnt - 1] == ']', return -1,
            "Invalid memory access argument: '%s'", bufptr);
        
        flags |= AF_MEM;
        bufptr[char_cnt - 1] = '\0';
        bufptr++;
    }

    char* plusptr = strchr(bufptr, '+');
    if (plusptr == NULL) /* no sum */
    {
        int arg_flag = parse_literal(bufptr, cmd + 0);
        LOG_ASSERT_ERROR(arg_flag != -1, return -1,
            "Could not parse literal: '%s'", bufptr);
        flags |= arg_flag;
        *arg_size = 1;

        return flags;
    }

    /* two arguments */
    *plusptr = '\0';
    int arg1 = parse_literal(bufptr,      cmd + 0);
    int arg2 = parse_literal(plusptr + 1, cmd + 1);
    
    LOG_ASSERT_ERROR(arg1 != -1, return -1,
        "Could not parse literal: '%s'", bufptr);
    LOG_ASSERT_ERROR(arg2 != -1, return -1,
        "Could not parse literal: '%s'", plusptr + 1);

    if (arg1 == arg2) /* same argument type */
    {
        LOG_ASSERT_ERROR(arg1 != AF_REG, return -1,
            "Sum of registers is not a valid argument.", NULL);
        cmd[0] += cmd[1];
        cmd[1] = 0;
        flags |= AF_NUM;

        *arg_size = 1;

        return flags;
    }
    if (arg1 == AF_NUM) /* swap so that register comes before number*/
    {
        cmd[0] ^= cmd[1];
        cmd[1] ^= cmd[0];
        cmd[0] ^= cmd[1];
    }

    *arg_size = 2;

    flags |= arg1 | arg2;
    return flags;
}

static int try_parse_register(const char* str, int *value);
static int try_parse_number  (const char* str, int *value);

static int parse_literal(const char* str, int* value)
{
    if(try_parse_register(str, value) != -1) return AF_REG;

    if(try_parse_number  (str, value) != -1) return AF_NUM;

    return -1;
}

static int try_parse_register(const char* str, int *value)
{
    #define ASM_REG(name, num)\
        if(strcasecmp(str, #name) == 0)\
        {\
            *value = num;\
            return 0;\
        }

    #include "asm_reg.h"
    /*else*/ return -1;
    
    #undef ASM_REG
}

static int try_parse_number  (const char* str, int *value)
{
    return sscanf(str, "%d", value) - 1;
}

static int strempty(const char* str)
{
    while(isspace(*str)) str++;
    return !*str;
}
