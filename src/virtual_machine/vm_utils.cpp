#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "vm_utils.h"
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
        .registers      = {0, 0, 0, 0, 0, 0},
        .cmd            = (byte_t*)(bytes + HEADER_SIZE),
        .value_stack    = value_stack,
        .call_stack     = call_stack,
        .memory         = (int*) calloc(MEM_SIZE, sizeof(int))
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
    free(cpu->memory);
}

static asm_arg next_parameter(proc_state* cpu);
static void refresh_screen(const proc_state* cpu);

#define CMD                 (cpu->cmd)
#define REGS                (cpu->registers)
#define STACK               (cpu->value_stack)
#define ST_SIZE             (cpu->value_stack->size)
#define CALL_STACK          (cpu->call_stack)
#define CALL_DEPTH          (cpu->call_stack->size)
#define PUSH_CALL           StackPush(CALL_STACK, IP)
#define POP_CALL            StackPopCopy(CALL_STACK, NULL)
#define RAM                 (cpu->memory)
#define IP                  (cpu->registers[REG_IP])
#define PUSH(value)         StackPush(cpu->value_stack, (value))
#define POP                 StackPopCopy(cpu->value_stack, NULL)
#define NEXT_ARG            next_parameter(cpu)
#define REFRESH             refresh_screen(cpu)
#define STOP                return 0
#define ASSERT(cond, msg)   LOG_ASSERT_ERROR(cond, return -1, msg, NULL)

int proc_run(proc_state *cpu)
{
    const unsigned cmd_mask = 0x1f;

    #define ASM_CMD(name, num, args, code, ...)\
        case num: code; break;

    while ((size_t)IP < cpu->program_length)
    {
        switch ((unsigned)CMD[IP++] & cmd_mask)
        {
        #include "asm_cmd.h"
        
        default:
            LOG_ASSERT_ERROR(0, return -1,
                "Failed to execute command '%x' at [%x]", CMD[IP], IP);
            break;
        }

        //proc_dump(cpu);
    }

    #undef ASM_CMD

    return 0;

}

static asm_arg next_parameter(proc_state* cpu)
{
    static int argument = 0;
    const unsigned flag_mask = 0xe0;

    byte_t flags = CMD[IP-1] & flag_mask;

    if (flags == AF_LAB)
    {
        memcpy(&argument, (int*)(CMD + IP), sizeof(int));
        IP += sizeof(int);
        return {
            .val_ptr = &argument,
            .perms   = ARG_LABEL
        };
    }

    int *val_ptr = &argument;

    if (flags & AF_REG)
    {
        byte_t reg_num = CMD[IP++];
        LOG_ASSERT_ERROR(reg_num < REG_COUNT, return {},
            "Invalid register number at 0x%*x", sizeof(IP)*2, IP);
        val_ptr = &REGS[reg_num];
    }
    if (flags & AF_NUM)
    {
        memcpy(&argument, (int*)(CMD + IP), sizeof(int));
        IP += sizeof(int);

        if (val_ptr != &argument)
        {
            argument += *val_ptr;
            val_ptr = &argument;
        }
    }
    if (flags & AF_MEM)
    {
        LOG_ASSERT_ERROR(*val_ptr < MEM_SIZE, return {},
            "Invalid memory address '0x%*x' at 0x%*x",
            sizeof(*val_ptr)*2, *val_ptr,
            sizeof(IP), IP);
        return {
            .val_ptr = &RAM[*val_ptr],
            .perms   = ARG_RDWR
        };
    }

    return {
        .val_ptr = val_ptr,
        .perms   = val_ptr == &argument ? ARG_RD : ARG_RDWR
    };
}

static void refresh_screen(const proc_state* cpu)
{
    puts("\033[H\033[2J");
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            if (RAM[REGS[REG_VBP] + i*SCREEN_WIDTH + j])
                putc('#', stdout);
            else
                putc('.', stdout);
        }
        putc('\n', stdout);
    }
}

void proc_dump(proc_state* cpu)
{
    static const unsigned dump_lines = 16;
    static const unsigned dump_columns = 16;
    static char STRBUF[2048] = "";
    char *bufptr = STRBUF;
    log_message(MSG_TRACE, "Dumping virtual machine state.");
    log_message(MSG_TRACE, "Registers:");

    #define ASM_REG(name, num) log_message(MSG_TRACE, "\t" #name "= %d", REGS[num]);
    #include "asm_reg.h"
    #undef ASM_REG

    log_message(MSG_TRACE, "Memory:");
    for (unsigned i = 0; i < dump_lines; i++)
    {
        int n_write = 0;
        bufptr = STRBUF;
        sprintf(bufptr, "[%04u]%n", i*dump_columns, &n_write);
        bufptr+=n_write;

        for (unsigned j = 0; j < dump_columns; j++)
        {
            sprintf(bufptr, " %04u%n", RAM[i*dump_columns + j], &n_write);
            bufptr += n_write;
        }
        log_message(MSG_TRACE, "%s", STRBUF);
    }

    log_message(MSG_TRACE, "Commands:");
    bufptr = STRBUF;
    for (unsigned i = 0; i < dump_lines; i++)
    {
        int n_write = 0;
        bufptr = STRBUF;
        sprintf(bufptr, "[0x%04x] %n", i*dump_columns, &n_write);
        bufptr += n_write;
        
        for (unsigned j = 0; j < dump_columns; j++)
        {
            if (IP == i*dump_columns + j)
                sprintf(bufptr, "[0x%02hhx]%n", i*dump_columns + j, &n_write);
            else
                sprintf(bufptr, " 0x%02hhx %n", i*dump_columns + j, &n_write);
            bufptr += n_write;
        }
        log_message(MSG_TRACE, "%s", STRBUF);
    }
    log_message(MSG_TRACE, "Data stack:");
    StackDump(STACK);
    log_message(MSG_TRACE, "Call stack:");
    StackDump(CALL_STACK);
}
