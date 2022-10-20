#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "asm_parser.h"
#include "logger.h"

#define HEADER      (state->header)
#define CMD         (state->cmd)
#define CMD_LEN     (state->cmd_size)
#define IP          (state->ip)
#define LABELS      (state->labels)
#define DEFS        (state->definitions)
#define FIXUP       (state->fixups)
#define STATE       (state->result)
#define LINE_CNT    (state->program.line_count)
#define LINE(i)     (state->program.lines[i].line)
#define LINE_NUM    (state->line_num)
#define LINE_ADDR   (state->line_addr)

/**
 * @brief 
 * Parse `line` grammar rule
 * @param[in] line Program line
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_line(const char* line, assembly_state* state);

/**
 * @brief 
 * Parse `def` grammar rule
 * @param[in] str Program string
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_def(const char* str, assembly_state* state);

/**
 * @brief 
 * Parse `code` grammar rule
 * @param[in] str Program string
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_code(const char* str, assembly_state* state);

/**
 * @brief 
 * Parse `label` grammar rule
 * @param[in] str Program string
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_label(const char* str, assembly_state* state);

/**
 * @brief 
 * Parse `command` grammar rule
 * @param[in] str Program string
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_command(const char* str, assembly_state* state);

/**
 * @brief 
 * Parse `arg_list` grammar rule
 * @param[in] str Program string
 * @param[in] args Argument list description
 * @param[inout] state Current assembly state
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_arg_list(const char* str, cmd_args args, assembly_state* state);

/**
 * @brief 
 * Parse `arg` grammar rule
 * @param[inout] str Program string.
 * Delimiting charachters, such as '[', ']' and '+' will be replaced
 * with null-terminator
 * @param[inout] state Current assembly state
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_arg(char* str, assembly_state* state, unsigned* flags);

/**
 * @brief
 * Parse `sum_arg` grammar rule
 * @param[inout] str Program string.
 * Delimiting charachters, such as '+' will be replaced
 * with null-terminator
 * @param[inout] state Current assembly state
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_sum_arg(char* str, assembly_state* state, unsigned* flags);

/**
 * @brief 
 * Parse `lit_arg` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_lit_arg(const char* str, int* value, unsigned* flags);

/**
 * @brief 
 * Parse `register` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_register(const char* str, int *value);

/**
 * @brief 
 * Parse `register` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_number  (const char* str, int *value);

int assemble(assembly_state* state)
{
    for (LINE_NUM = 0; LINE_NUM < LINE_CNT; LINE_NUM++)
    {
        LINE_ADDR[LINE_NUM] = IP;

        int res = asm_parse_line(LINE(LINE_NUM), state);
        LOG_ASSERT(res != -1, return -1);
    }
    HEADER = {.header = {.signature = SIGNATURE,
                         .version   = LATEST_VERSION,
                         .opcnt     = IP}};
    return 0;
}

static int asm_parse_line(const char* line, assembly_state* state)
{
    char first_char = '\0';
    sscanf(line, " %c", &first_char);
    if (first_char == '%')
        return asm_parse_def(line, state);
    return asm_parse_code(line, state);
}

/* TODO: definition support */
static int asm_parse_def(const char* str, assembly_state* state)
{
    return -1;
}

static int asm_parse_code(const char* str, assembly_state* state)
{
    return asm_parse_command(str, state);
}

/* TODO: label support*/
static int asm_parse_label(const char* str, assembly_state* state)
{
    return -1;
}

static int asm_parse_command(const char* str, assembly_state* state)
{
    static const int BUFFER_SIZE = 256;
    static char buffer[BUFFER_SIZE] = "";

    int n_read = 0;
    if (strempty(str)) return 0;
    sscanf(str, " %s%n", buffer, &n_read);

    #define ASM_CMD(name, num, args, ...)                               \
        if (strcasecmp(buffer, #name) == 0)                             \
        {                                                               \
            LOG_ASSERT(IP < CMD_LEN, return -1);                        \
            CMD[IP] = num;                                              \
            int parsed = asm_parse_arg_list(str + n_read, args, state); \
            LOG_ASSERT_ERROR(parsed != -1, return -1,                   \
                "Invalid arguments given to '%s': '%s'",                \
                buffer, str);                                           \
        } else

    #include "asm_cmd.h"
    /* else */  LOG_ASSERT_ERROR(0, {STATE = ASM_UNCMD; return -1;}, 
                        "{%zu}: Unknown command '%s'",
                        LINE_NUM + 1, buffer);

    #undef ASM_CMD
    return 0;
}

static int asm_parse_arg_list(const char* str, cmd_args args, assembly_state* state)
{
    const size_t BUFLEN = 32;
    char BUFFER[BUFLEN] = "";

    int *command = &CMD[IP++];
    
    for (size_t i = 0; i < args.arg_count; i++)
    {
        int n_read = 0;
        
        int read = sscanf(str, "%s%n", BUFFER, &n_read);
        LOG_ASSERT_ERROR(read == 1, {STATE = ASM_ARGCNT; return -1;},
            "{%zu} Invalid number of arguments: %zu expected, but %zu found",
            LINE_NUM + 1, args.arg_count, i);
        str += n_read;

        unsigned flags = 0;
        int parsed = asm_parse_arg(BUFFER, state, &flags);

        LOG_ASSERT(parsed != -1, return -1);

        /* writable argument - memory or register without constant */
        unsigned perms = (flags & AF_MEM) || ((flags & AF_REG) && !(flags & AF_NUM))
                            ? ARG_RDWR
                            : ARG_RD;

        LOG_ASSERT_ERROR((perms & args.arg_list[i].perms) == args.arg_list[i].perms,
            {STATE = ASM_TYPE; return -1;},
            "{%zu} Argument '%s': invalid argument type",
            LINE_NUM + 1, BUFFER);
        
        *command |= (signed) flags;
    }

    LOG_ASSERT_ERROR(strempty(str), {STATE = ASM_ARGCNT; return -1;},
        "{%zu} Too many arguments given", LINE_NUM + 1);

    return 0;
}

static int asm_parse_arg(char* str, assembly_state* state, unsigned* flags)
{
    size_t char_cnt = strlen(str);
    
    if (*str == '[')   /* Memory access */
    {
        LOG_ASSERT_ERROR(str[char_cnt - 1] == ']',
            {STATE = ASM_INVAL; return -1;},
            "{%zu} Malformed memory access argument: '%s'",
            LINE_NUM + 1, str);
        
        *flags |= AF_MEM;
        str[char_cnt - 1] = '\0';
        str++;
    }

    return asm_parse_sum_arg(str, state, flags);
}

static int asm_parse_sum_arg(char* str, assembly_state* state, unsigned* flags)
{
    char* plusptr = strchr(str, '+');
    if (plusptr == NULL) /* no sum */
        return asm_parse_lit_arg(str, &CMD[IP++], flags);

    /* two arguments */
    *plusptr = '\0';
    const char* argstr1 = str;
    const char* argstr2 = plusptr + 1;
    unsigned flags1 = 0;
    int arg1 = asm_parse_lit_arg(argstr1, &CMD[IP++], &flags1);
    int arg2 = asm_parse_lit_arg(argstr2, &CMD[IP++], flags);
    *flags |= flags1;
    
    LOG_ASSERT_ERROR(arg1 != -1,
        {STATE = ASM_INVAL; return -1;},
        "{%zu} Could not parse literal: '%s'",
        LINE_NUM + 1, argstr1);

    LOG_ASSERT_ERROR(arg2 != -1,
        {STATE = ASM_INVAL; return -1;},
        "{%zu} Could not parse literal: '%s'",
        LINE_NUM + 1, argstr2);

    unsigned sum_flags = (*flags) & (AF_NUM | AF_REG);

    LOG_ASSERT_ERROR(sum_flags != AF_REG,
        {STATE = ASM_INVAL; return -1;},
        "{%zu} Sum of registers is not a valid argument.",
        LINE_NUM);

    if (sum_flags == AF_NUM) /* same argument type */
    {
        IP--;
        CMD[IP - 1] += CMD[IP];
        CMD[IP] = 0;

        return 0;
    }

    if (flags1 == AF_NUM) /* swap so that register comes before number*/
    {
        CMD[IP - 2] ^= CMD[IP - 1];
        CMD[IP - 1] ^= CMD[IP - 2];
        CMD[IP - 2] ^= CMD[IP - 1];
    }

    return 0;
}

static int asm_parse_lit_arg(const char* str, int* value, unsigned* flags)
{
    if(asm_parse_register(str, value) != -1)
    {
        *flags |= AF_REG;
        return 0;
    }

    if(asm_parse_number  (str, value) != -1)
    {
        *flags |= AF_NUM;
        return 0;
    }

    return -1;
}

static int asm_parse_register(const char* str, int *value)
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

static int asm_parse_number(const char* str, int *value)
{
    int n_read = 0;
    int res = sscanf(str, "%d%n", value, &n_read);
    if (res == 1)
    {
        LOG_ASSERT_ERROR(strempty(str + n_read), return -1,
            "Invalid number literal '%s'", str);
        return 0;
    }
    return -1;
}
