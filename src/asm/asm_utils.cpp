#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include "logger.h"
#include "asm_utils.h"

static void add_def(const char* name, int value, assembly_state* state);

assembly_state* assembly_state_ctor(const char* filename)
{
    TextLines text_lines = read_file(filename);
    LOG_ASSERT_ERROR(text_lines.text != NULL, 
        {
            dispose_lines(&text_lines);
            return NULL;
        },
        "Could not read file \'%s\'", filename);

    assembly_state *result = (assembly_state*) calloc(1, sizeof(*result));
    size_t cmd_len = text_lines.line_count
                        * (MAX_CMD_ARGS*sizeof(int) + 2*sizeof(char));

    *result = {
        .header      = {},
        .program     = text_lines,
        .line_num    = 0,
        .line_addr   = (size_t*)calloc(text_lines.line_count,
                        sizeof(size_t)),
        .ip          = 0,
        .cmd         = (byte_t*) calloc(cmd_len, sizeof(char)),
        .cmd_size    = cmd_len,
        .labels      = (asm_label*) calloc(MAX_LABELS, sizeof(asm_label)),
        .label_cnt   = 0,
        .definitions = (asm_def*) calloc(MAX_LABELS, sizeof(asm_def)),
        .def_cnt     = 0,
        .fixups      = (fixup*) calloc(MAX_FIXUPS, sizeof(fixup)),
        .fix_cnt     = 0,
        .result      = ASM_SUCCESS
    };

    add_def(".sc_width", SCREEN_WIDTH, result);
    add_def(".sc_height", SCREEN_HEIGHT, result);
     
    return result;
}

void assembly_state_dtor(assembly_state* state)
{
    dispose_lines(&state->program);
    free(state->line_addr);
    free(state->cmd);
    free(state->labels);
    free(state->definitions);
    free(state->fixups);
    
    free(state);
}

int create_executable(const char* filename, const assembly_state* state)
{
    LOG_ASSERT(state->result == ASM_SUCCESS, return -1);

    int output_fd = creat(filename, S_IWUSR | S_IRUSR);
    LOG_ASSERT_ERROR(output_fd != -1, return -1,
        "Could not create output file '%s'", filename);
    write(output_fd, &state->header, sizeof(state->header));
    write(output_fd, state->cmd, state->ip*sizeof(*state->cmd));
    close(output_fd);
    
    return 0;
}

void print_listing(const char* filename, const assembly_state* state)
{
    #define LINE_CNT        (state->program.line_count)
    #define LINE(i)         (state->program.lines[i].line)
    #define ASM_START(i)    (state->line_addr[i])
    #define ASM_END(i)      ((i) < LINE_CNT - 1 ? state->line_addr[i + 1] : state->ip)
    #define CMD             (state->cmd)
    #define BYTE_FORMAT "%02x"

    if (!filename) return;
    LOG_ASSERT(state->result == ASM_SUCCESS, return);

    const int ASM_WIDTH = 32;

    FILE* listing = fopen(filename, "w");
    LOG_ASSERT_ERROR(listing != NULL, return,
        "Could not open listing file \'%s\'", filename);
    
    for (size_t i = 0; i < LINE_CNT; i++)
    {
        int printed = 0;
        int total_chars = 0;

        fprintf(listing, "[0x%04zx]%n", ASM_START(i), &printed);
        total_chars += printed;

        for (size_t j = ASM_START(i); j < ASM_END(i); j++)
        {   
            fprintf(listing, " " BYTE_FORMAT "%n", (unsigned)CMD[j], &printed);
            total_chars += printed;
        }
        fprintf(listing, "%*s\t%s\n", ASM_WIDTH - total_chars, "|", LINE(i));
    }

    fclose(listing);

    #undef LINE_CNT
    #undef LINE
    #undef ASM_START
    #undef ASM_END
    #undef CMD
    #undef BYTE_FORMAT
}

int strempty(const char* str)
{
    while(isspace(*str)) str++;
    return !*str;
}

static void add_def(const char* name, int value, assembly_state* state)
{
    strcpy(state->definitions[state->def_cnt].name, name);
    state->definitions[state->def_cnt].value = value;
    state->def_cnt++;
}