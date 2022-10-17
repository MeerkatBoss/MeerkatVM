#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "vcpu_utils.h"
#include "text_lines.h"
#include "logger.h"
#include "assembler.h"

proc_state proc_ctor(const char* program_file)
{
    char* bytes = NULL;
    size_t file_size = map_file(program_file, &bytes, 0);
    LOG_ASSERT_ERROR(file_size >= HEADER_SIZE, return {},
        "Could not open file \'%s\'", program_file);
    padded_header *hdr = (padded_header*) bytes;
    LOG_ASSERT_ERROR(strncmp(hdr->header.signature, STR_SIGN, 4) == 0,
        return {},
        "Invalid signature \'%.4s\'", hdr->header.signature);
    LOG_ASSERT_ERROR(hdr->header.version == LATEST_VERSION,
        return {},
        "Version %u not supported", hdr->header.version);
    
    Stack* value_stack = (Stack*) calloc(1, sizeof(*value_stack));
    Stack* call_stack  = (Stack*) calloc(1, sizeof(*value_stack));
    StackCtor(value_stack);
    StackCtor(call_stack);

    return {
        .program_length = hdr->header.opcnt,
        .mapping_size   = file_size,
        .registers      = {0, 0, 0, 0, 0},
        .cmd            = (int*)(bytes + HEADER_SIZE),
        .value_stack    = value_stack,
        .call_stack     = call_stack
    };
}


void proc_dtor(proc_state* cpu)
{
    int err = munmap((char*)cpu->cmd - HEADER_SIZE, cpu->mapping_size);
    LOG_ASSERT_ERROR(err != -1, return,
        "Invalid cpu instance", NULL);
    StackDtor(cpu->value_stack);
    StackDtor(cpu->call_stack);
    free(cpu->value_stack);
    free(cpu->call_stack);
}

static asm_arg next_parameter(const int* cmd_array, int* ip);

#define CMD                 (cpu->cmd)
#define STACK               (cpu->value_stack)
#define CALL_STACK          (cpu->call_stack)
#define IP                  (cpu->registers[REG_IP])
#define PUSH(value)         StackPush(cpu->value_stack, (value))
#define POP                 StackPopCopy(cpu->value_stack, NULL)
#define NEXT_ARG            next_parameter(CMD, &IP)
#define STOP                return 0
#define ASSERT(cond, msg)   LOG_ASSERT_ERROR(cond, return -1, msg, NULL)

int proc_run(proc_state *cpu)
{

    #define ASM_CMD(name, num, args, code, ...)\
        case num: code; break;

    while ((size_t)IP < cpu->program_length)
    {
        switch (CMD[IP])
        {
        #include "asm_cmd.h"
        
        default:
            LOG_ASSERT_ERROR(0, return -1,
                "Failed to execute command at [%x]", IP);
            break;
        }

        IP++;
        StackDump(cpu->value_stack);
    }

    #undef ASM_CMD

    return 0;

}

#undef CMD
#undef STACK
#undef CALL_STACK
#undef IP
#undef PUSH
#undef POP
#undef NEXT_ARG
#undef STOP
#undef ASSERT

static asm_arg next_parameter(const int* cmd_array, int* ip)
{
    static int argument = 0;

    argument = cmd_array[++*ip];

    return {
        .val_ptr = &argument,
        .perms   = ARG_RD
    };
}

