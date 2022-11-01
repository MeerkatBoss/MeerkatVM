#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "disasm_utils.h"
#include "assembler.h"
#include "logger.h"
#include "text_lines.h"

disasm_state* disasm_ctor(const char* input_file, const char* output_file)
{
    char* bytes = NULL;
    size_t file_size = map_file(input_file, &bytes, 0);
    LOG_ASSERT_ERROR(file_size >= HEADER_SIZE, return {},
        "Could not open file \'%s\'", input_file);
    padded_header *hdr = (padded_header*) bytes;
    LOG_ASSERT_ERROR(strncmp(hdr->header.signature, STR_SIGN, 4) == 0,
        return {},
        "Invalid signature \'%.4s\'", hdr->header.signature);
    LOG_ASSERT_ERROR(hdr->header.version == LATEST_VERSION,
        return {},
        "Version %u not supported", hdr->header.version);

    FILE* output = fopen(output_file, "w");
    LOG_ASSERT_ERROR(output != NULL, return NULL,
        "Could not open file '%s'", output_file);
    
    disasm_state* result = (disasm_state*)calloc(1, sizeof(*result));
    *result =  {
        .program_length = hdr->header.opcnt,
        .mapping_size   = file_size,
        .cmd            = (byte_t*)(bytes + HEADER_SIZE),
        .ip             = 0,
        .labels         = (long*) calloc(MAX_LABELS,
                                sizeof(long)),
        .label_cnt      = 0,
        .output         = output
    };

    return result;
}

void disasm_dtor(disasm_state* state)
{
    int err = munmap((char*)state->cmd - HEADER_SIZE, state->mapping_size);
    LOG_ASSERT_ERROR(err != -1, return,
        "Invalid cpu instance", NULL);

    fclose(state->output);
    free(state->labels);
    free(state);
}

#define IP          state->ip
#define CMD         state->cmd
#define PROG_LEN    state->program_length
#define LABELS      state->labels
#define LAB_CNT     state->label_cnt
#define OUTPUT      state->output

static long find_label(long addr, disasm_state* state);

static int disasm_args(cmd_args arg_descr, disasm_state* state);

int disassemble(disasm_state* state)
{
    static const unsigned cmd_mask = AF_NUM - 1;

    while(state->ip < state->program_length)
    {
        size_t arg_cnt = 0;

        if (find_label(state->ip, state))
            fprintf(OUTPUT, ".0x%08lx:\t", (unsigned long) state->ip);
        else
            fputs("\t\t", OUTPUT);

        #define ASM_CMD(name, num, args, ...)\
            case num:\
            fprintf(OUTPUT, "%s ", #name);\
            disasm_args(args, state);\
            break;
        
        switch(CMD[IP++] & cmd_mask)
        {
            #include "asm_cmd.h"
            default:
                LOG_ASSERT_ERROR(0, return -1,
                    "Unknown byte-code: '%x'",
                    CMD[IP - 1] & cmd_mask);
                break;
        }
        #undef ASM_CMD

        fputc('\n', OUTPUT);
    }

    return 0;
}

static int disasm_reg(disasm_state* state);
static int disasm_num(disasm_state* state);
static int disasm_label(disasm_state* state);

static int disasm_args(cmd_args arg_descr, disasm_state* state)
{
    unsigned char arg_mask = ~(AF_NUM - 1);

    byte_t arg_flags = CMD[IP - 1] & arg_mask;
    for (size_t i = 0; i < arg_descr.arg_count; i++)
    {

        if (arg_flags & AF_MEM) fputc('[', OUTPUT);
        if (arg_flags & AF_REG)
            LOG_ASSERT(disasm_reg(state) == 0, return -1);

        if ((arg_flags & AF_NUM) && (arg_flags & AF_REG))
            fputc('+', OUTPUT);

        if (arg_flags & AF_NUM)
            LOG_ASSERT(disasm_num(state) == 0, return -1);

        if (arg_flags & AF_MEM) fputc(']', OUTPUT);

        if (arg_flags == AF_LAB)
            LOG_ASSERT(disasm_label(state) == 0, return -1);
    }
    return 0;
}

static int disasm_reg(disasm_state* state)
{
    #define ASM_REG(name, num, ...)\
        case num: fputs(#name, OUTPUT); break;
    switch(CMD[IP++])
    {
        #include "asm_reg.h"
        default:
            LOG_ASSERT_ERROR(0, return -1,
                "Unknown register number: '%x'",
                CMD[IP - 1]);
            return -1;
    }
    #undef ASM_REG

    return 0;
}

static int disasm_num(disasm_state* state)
{
    int num = 0;
    memcpy(&num, (int*)(CMD + IP), sizeof(int));
    fprintf(OUTPUT, "%d", num);
    IP += sizeof(int);
    return 0;
}

static int disasm_label(disasm_state* state)
{
    int addr = 0;
    memcpy(&addr, (int*)(CMD + IP), sizeof(int));
    IP += sizeof(int);
    LABELS[LAB_CNT++] = addr;
    fprintf(OUTPUT, ".0x%08lx\t", (unsigned long) addr);
    return 0;
}

static long find_label(long addr, disasm_state* state)
{
    for(size_t i = 0; i < LAB_CNT; i++)
        if(LABELS[i] == addr) return 1;
    return 0;
}
