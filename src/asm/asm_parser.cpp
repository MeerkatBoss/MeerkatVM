#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "asm_parser.h"
#include "logger.h"

static const long ADDR_UNSET = -1;

#define HEADER      (state->header)
#define CMD         (state->cmd)
#define CMD_LEN     (state->cmd_size)
#define IP          (state->ip)
#define LABELS      (state->labels)
#define LB_CNT      (state->label_cnt)
#define DEFS        (state->definitions)
#define DEF_CNT     (state->def_cnt)
#define FIXUP       (state->fixups)
#define FX_CNT      (state->fix_cnt)
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
 * @param[inout] state Current assembly state
 * @param[out] value Parsed literal
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_lit_arg(const char* str, assembly_state* state, int* value, unsigned* flags);

/**
 * @brief 
 * Parse `register` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_register(const char* str, int *value, unsigned* flags);

/**
 * @brief 
 * Parse `number` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[out] value Parsed literal
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_number  (const char* str, int *value, unsigned* flags);

/**
 * @brief 
 * Parse `name` grammar rule
 * 
 * @param[in]  str   Literal string
 * @param[inout] state Current assembly state
 * @param[out] value Parsed literal
 * @param[out] flags `arg_flags` of an argument
 * @return 0 upon success, -1 otherwise
 */
static int asm_parse_name  (const char* str, assembly_state* state,int *value, unsigned* flags);

/**
 * @brief 
 * Add new label to array
 * @param[in] name Label name
 * @param[in] addr Label address
 * @param[inout] state Current assembly state
 * @return int 
 */
static int add_label(const char* name, long addr, assembly_state* state);

/**
 * @brief 
 * Find label with given name in label array
 * 
 * @param[in] name Label name
 * @param[in] state Assembly state
 * @return pointer to label or NULL
 */
static asm_label* find_label (const char* name, const assembly_state* state);

/**
 * @brief 
 * Find definition with given name in definition array
 * 
 * @param[in] name Definition name
 * @param[in] state Assembly state
 * @return pointer to definition or NULL
 */
static asm_def *find_def     (const char *name, const assembly_state *state);

/**
 * @brief 
 * Fix unset label addresses in command array
 * @param[inout] state Assembly state
 * @return 0 upon success, -1 otherwise
 */
static int        run_fixup  (assembly_state* state);

/**
 * @brief 
 * Check if string is a valid identifier
 * @param[in] str String to be checked
 * @return 1 if string is a valid identifier, 0 otherwise
 */
static int check_name(const char* str);

int assemble(assembly_state* state)
{
    for (LINE_NUM = 0; LINE_NUM < LINE_CNT; LINE_NUM++)
    {
        LINE_ADDR[LINE_NUM] = IP;

        int res = asm_parse_line(LINE(LINE_NUM), state);
        LOG_ASSERT(res != -1, continue);
    }
    int fixup = run_fixup(state);
    LOG_ASSERT(fixup != -1, return -1);

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
    static const int BUFFER_SIZE = 256;
    static char BUFFER[BUFFER_SIZE] = "";

    int value = 0;
    int n_read = 0;

    int success = sscanf(str, " %%def %s %d%n", BUFFER, &value, &n_read);

    LOG_ASSERT_ERROR(success == 2, 
        { STATE |= ASM_WDEF; return -1;},
        "{%zu} Malformed constant definition",
        LINE_NUM + 1);
    LOG_ASSERT_ERROR(strempty(BUFFER + n_read), 
        { STATE |= ASM_WDEF; return -1;},
        "{%zu} Malformed constant definition",
        LINE_NUM + 1);
    LOG_ASSERT_ERROR(check_name(BUFFER),
        { STATE |= ASM_NAME; return -1;},
        "{%zu} '%s' is not a valid constant name",
        LINE_NUM + 1, BUFFER);
    LOG_ASSERT_ERROR(find_def(BUFFER, state) == NULL,
        {STATE |= ASM_REDEF; return -1;},
        "{%zu} Constant redefinition: '%s'",
        LINE_NUM + 1, BUFFER);
    LOG_ASSERT_ERROR(find_label(BUFFER, state) == NULL,
        {STATE |= ASM_REDEF; return -1;},
        "{%zu} Label '%s' redefinition",
        LINE_NUM + 1, BUFFER);
    LOG_ASSERT_ERROR(DEF_CNT < MAX_LABELS,
        {STATE |= ASM_DEFOVF; return -1;},
        "Maximum number of defined constants exceeded", NULL);
    
    strcpy(DEFS[DEF_CNT].name, BUFFER);
    DEFS[DEF_CNT].value = value;
    DEF_CNT++;

    return 0;
}

static int asm_parse_code(const char* str, assembly_state* state)
{
    static const int BUFFER_SIZE = 256;
    static char BUFFER[BUFFER_SIZE] = "";
    const char *sep_ptr = strchr(str, ':');
    if (sep_ptr != NULL)
    {
        sscanf(str, " %[^:]", BUFFER);

        int label_parsed = asm_parse_label(BUFFER, state);
        LOG_ASSERT(label_parsed != -1, return -1);

        str = sep_ptr + 1;
    }
    sep_ptr = strchr(str, ';');
    
    if(sep_ptr != NULL)
    {
        size_t command_len = (size_t) (sep_ptr - str);
        LOG_ASSERT(command_len < BUFFER_SIZE - 1, return -1);

        strncpy(BUFFER, str, command_len);
        BUFFER[command_len] = '\0';
        str = BUFFER;
    }

    return asm_parse_command(str, state);
}

static int asm_parse_label(const char* str, assembly_state* state)
{
    LOG_ASSERT_ERROR(check_name(str),
        {STATE |= ASM_NAME; return -1;},
        "{%zu} '%s' is not a valid label name",
        LINE_NUM + 1, str);
    LOG_ASSERT_ERROR(find_def(str, state) == NULL,
        {STATE |= ASM_REDEF; return -1;},
        "{%zu} Ambiguous label definition: '%s'.",
        LINE_NUM + 1, str);
    
    asm_label* old_label = find_label(str, state);
    if (old_label != NULL)
    {
        LOG_ASSERT_ERROR(old_label->addr == ADDR_UNSET,
            {STATE |= ASM_REDEF; return -1;},
            "{%zu} Label '%s' redefinition.",
            LINE_NUM + 1, str);
        old_label->addr = (long) IP;
        return 0;
    }

    return add_label(str, (long) IP, state);
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
            LOG_ASSERT(parsed != -1, return -1);                        \
        } else

    #include "asm_cmd.h"
    /* else */  LOG_ASSERT_ERROR(0, {STATE |= ASM_UNCMD; return -1;}, 
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
        LOG_ASSERT_ERROR(read == 1, {STATE |= ASM_ARGCNT; return -1;},
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

        if (flags == 0)
            perms = ARG_LABEL;
        else
        {
            perms = ARG_RD;
            if ((flags & AF_MEM) || !(flags & AF_NUM))
                perms |= ARG_WR;
        }

        LOG_ASSERT_ERROR((perms & args.arg_list[i].perms) == args.arg_list[i].perms,
            {STATE |= ASM_TYPE; return -1;},
            "{%zu} Argument '%s': invalid argument type",
            LINE_NUM + 1, BUFFER);
        
        *command |= (signed) flags;
    }

    LOG_ASSERT_ERROR(strempty(str), {STATE |= ASM_ARGCNT; return -1;},
        "{%zu} Too many arguments given", LINE_NUM + 1);

    return 0;
}

static int asm_parse_arg(char* str, assembly_state* state, unsigned* flags)
{
    size_t char_cnt = strlen(str);
    
    if (*str == '[')   /* Memory access */
    {
        LOG_ASSERT_ERROR(str[char_cnt - 1] == ']',
            {STATE |= ASM_INVAL; return -1;},
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
        return asm_parse_lit_arg(str, state, &CMD[IP++], flags);

    /* two arguments */
    *plusptr = '\0';
    const char* argstr1 = str;
    const char* argstr2 = plusptr + 1;
    unsigned flags1 = 0;
    unsigned flags2 = 0;
    int arg1 = asm_parse_lit_arg(argstr1, state, &CMD[IP++], &flags1);
    int arg2 = asm_parse_lit_arg(argstr2, state, &CMD[IP++], &flags2);
    *flags |= flags1 | flags2;
    
    LOG_ASSERT_ERROR(arg1 != -1,
        {STATE |= ASM_INVAL; return -1;},
        "{%zu} Could not parse literal: '%s'",
        LINE_NUM + 1, argstr1);

    LOG_ASSERT_ERROR(arg2 != -1,
        {STATE |= ASM_INVAL; return -1;},
        "{%zu} Could not parse literal: '%s'",
        LINE_NUM + 1, argstr2);

    unsigned sum_flags = (*flags) & (AF_NUM | AF_REG);

    LOG_ASSERT_ERROR(flags1 != 0 && flags2 != 0,
        {STATE |= ASM_INVAL; return -1;},
        "{%zu} Sum containing label is not a valid argument.",
        LINE_NUM + 1);

    LOG_ASSERT_ERROR(sum_flags != AF_REG,
        {STATE |= ASM_INVAL; return -1;},
        "{%zu} Sum of registers is not a valid argument.",
        LINE_NUM + 1);

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

static int asm_parse_lit_arg(const char* str, assembly_state* state, int* value, unsigned* flags)
{
    if(asm_parse_register(str, value, flags) != -1)
        return 0;

    if(asm_parse_number  (str, value, flags) != -1)
        return 0;

    if(asm_parse_name    (str, state, value, flags) != -1)
        return 0;

    log_message(MSG_ERROR, "{%zu} Failed to parse literal: '%s'",
            LINE_NUM + 1, str);
    return -1;
}

static int asm_parse_register(const char* str, int *value, unsigned* flags)
{
    #define ASM_REG(name, num)          \
        if(strcasecmp(str, #name) == 0) \
        {   *flags |= AF_REG;           \
            *value = num;               \
            return 0;                   \
        }

    #include "asm_reg.h"
    /*else*/ return -1;
    
    #undef ASM_REG
}

static int asm_parse_number(const char* str, int *value, unsigned* flags)
{
    int n_read = 0;
    int res = sscanf(str, "%d%n", value, &n_read);
    if (res == 1)
    {
        LOG_ASSERT_ERROR(strempty(str + n_read), return -1,
            "Invalid number literal '%s'", str);
        *flags |= AF_NUM;
        return 0;
    }
    return -1;
}

static int asm_parse_name  (const char* str, assembly_state* state, int *value, unsigned* flags)
{
    if (check_name(str) == -1)
        return -1;
    
    asm_def* def_ptr = find_def(str, state);
    if (def_ptr != NULL)
    {
        *value = def_ptr->value;
        *flags |= AF_NUM;
        return 0;
    }
    
    asm_label* label_ptr = find_label(str, state);
    if (label_ptr != NULL)
    {
        *value = label_ptr->addr;
        /* flags are not set for label */
        return 0;
    }

    add_label(str, ADDR_UNSET, state);

    LOG_ASSERT_ERROR(FX_CNT < MAX_FIXUPS,
        {STATE |= ASM_FIXOVF; return -1;},
        "Maximum number of fixups exceeded", NULL);
    
    FIXUP[FX_CNT++] = {
        .label_number = LB_CNT - 1,
        .addr = IP - 1 /* IP already shifted */
    };
    *value = ADDR_UNSET;
    /* flags are not set for label */

    return 0;
}

static asm_label* find_label (const char* name, const assembly_state* state)
{
    for (size_t i = 0; i < LB_CNT; i++)
        if (strcmp(name, LABELS[i].name) == 0)
            return &LABELS[i];
    return NULL;
}

static asm_def* find_def (const char* name, const assembly_state* state)
{
    for (size_t i = 0; i < DEF_CNT; i++)
        if (strcmp(name, DEFS[i].name) == 0)
            return &DEFS[i];
    return NULL;
}

static int run_fixup (assembly_state* state)
{
    for (size_t i = 0; i < FX_CNT; i++)
    {
        size_t label_num = FIXUP[i].label_number;
        size_t addr      = FIXUP[i].addr;
        LOG_ASSERT_ERROR(LABELS[label_num].addr != ADDR_UNSET,
            {STATE |= ASM_NDEF; return -1;},
            "Label '%s' was not defined",
            LABELS[label_num].name);

        CMD[addr] = LABELS[label_num].addr;
    }
    return 0;
}

static int add_label(const char* name, long addr, assembly_state* state)
{
    LOG_ASSERT_ERROR(LB_CNT < MAX_LABELS,
        {STATE |= ASM_LABELOVF; return -1;},
        "Maximum number of labels exceeded", NULL);
    strcpy(LABELS[LB_CNT].name, name);
    LABELS[LB_CNT].addr = addr;
    LB_CNT++;
    return 0;
}

static int check_name(const char* str)
{
    if (!isalpha(*str) && *str != '_' && *str != '.')
        return 0;

    #define ASM_CMD(name,...)\
        if (strcasecmp(str, #name) == 0) return 0;
    #define ASM_REG(name,...)\
        if (strcasecmp(str, #name) == 0) return 0;

    #include "asm_cmd.h"
    #include "asm_reg.h"

    #undef ASM_CMD
    #undef ASM_REG

    while (*str)
    {
        if (!isalnum(*str) && *str != '_' && *str != '.')
            return 0;
        str++;
    }

    return 1;
}