#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "disasm_utils.h"
#include "assembler.h"
#include "logger.h"
#include "text_lines.h"

disasm_state* disasm_ctor(const char* filename)
{
    char* bytes = NULL;
    size_t file_size = map_file(filename, &bytes, 0);
    LOG_ASSERT_ERROR(file_size >= HEADER_SIZE, return {},
        "Could not open file \'%s\'", filename);
    padded_header *hdr = (padded_header*) bytes;
    LOG_ASSERT_ERROR(strncmp(hdr->header.signature, STR_SIGN, 4) == 0,
        return {},
        "Invalid signature \'%.4s\'", hdr->header.signature);
    LOG_ASSERT_ERROR(hdr->header.version == LATEST_VERSION,
        return {},
        "Version %u not supported", hdr->header.version);
    
    disasm_state* result = (disasm_state*)calloc(1, sizeof(*result));
    *result =  {
        .program_length = hdr->header.opcnt,
        .mapping_size   = file_size,
        .cmd            = (int*)(bytes + HEADER_SIZE),
        .ip             = 0,
        .labels         = (long*) calloc(MAX_LABELS,
                                sizeof(long)),
        .label_cnt      = 0
    };

    return result;
}

void disasm_dtor(disasm_state* state)
{
    int err = munmap((char*)state->cmd - HEADER_SIZE, state->mapping_size);
    LOG_ASSERT_ERROR(err != -1, return,
        "Invalid cpu instance", NULL);
    free(state->labels);
    free(state);
}

static long find_label(long addr, disasm_state* state);

static inline cmd_args get_arg(cmd_args arg) {return arg;}

int disassemble(const char* output_file, disasm_state* state)
{
    static const unsigned cmd_mask = AF_NUM - 1;

    FILE* output = fopen(output_file, "w");
    LOG_ASSERT_ERROR(output != NULL, return -1,
        "Could not open file '%s'", output_file);

    while(state->ip < state->program_length)
    {
        size_t arg_cnt = 0;

        if (find_label(state->ip, state))
            fprintf(output, ".0x%08lx:\t", (unsigned long) state->ip);
        else
            fputs("\t\t", output);

        #define ASM_CMD(name, num, args, ...)\
            case num:\
            arg_cnt = get_arg(args).arg_count;\
            fprintf(output, "%s ", #name);\
            break;
        
        switch(state->cmd[state->ip++] & cmd_mask)
        {
            #include "asm_cmd.h"
            default:
                LOG_ASSERT_ERROR(0, return -1,
                    "Unknown byte-code: '%x'",
                    state->cmd[state->ip - 1] & cmd_mask);
                break;
        }

        #undef ASM_CMD
        if (arg_cnt)
        {
            unsigned arg_flags = state->cmd[state->ip - 1] & ~cmd_mask;
            if (arg_flags & AF_MEM) fputc('[', output);
            if (arg_flags & AF_REG)
            {
                #define ASM_REG(name, num, ...)\
                    case num: fputs(#name, output); break;
                switch(state->cmd[state->ip++])
                {
                    #include "asm_reg.h"
                    default:
                        LOG_ASSERT_ERROR(0, return -1,
                            "Unknown register number: '%x'",
                            state->cmd[state->ip - 1]);
                        break;
                }
                #undef ASM_REG
                if (arg_flags & AF_NUM)
                    fputc('+', output);
            }
            if (arg_flags & AF_NUM)
                fprintf(output, "%d", state->cmd[state->ip++]);
            if (arg_flags & AF_MEM) fputc(']', output);

            if (arg_flags == AF_LAB)
            {
                long addr = state->cmd[state->ip++];
                state->labels[state->label_cnt++] = addr;
                fprintf(output, ".0x%08lx\t", (unsigned long) addr);
            }
        }
        fputc('\n', output);
    }
    fclose(output);
    return 0;
}

static long find_label(long addr, disasm_state* state)
{
    for(size_t i = 0; i<state->label_cnt; i++)
        if(state->labels[i] == addr) return 1;
    return 0;
}
