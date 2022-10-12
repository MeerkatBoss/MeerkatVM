#include <stdio.h>
#include <string.h>

#include "vcpu_utils.h"
#include "logger.h"
#include "assembler.h"

int* read_cmd_array(const char* filename, size_t* array_len)
{
    const size_t SIGNATURE_LENGTH = 4;
    FILE* program_file = fopen(filename, "r");   
    LOG_ASSERT_FATAL(program_file != NULL, "Cannot open file \'%s\'", filename);

    char signature_buf[SIGNATURE_LENGTH + 1] = "";
    unsigned int version = 0;
    size_t length = 0;

    int read_status = fscanf(program_file, "%4s %u %zu",
                            signature_buf, &version, &length);
    LOG_ASSERT(read_status == 3,                        return NULL);
    LOG_ASSERT(strcmp(signature_buf, STR_SIGN) == 0,    return NULL);
    LOG_ASSERT(version == LATEST_VERSION,               return NULL);

    int* array = (int*)calloc(length, sizeof(*array));

    for (size_t i = 0; i < length; i++)
    {
        read_status = fscanf(program_file, "%d", array + i);
        LOG_ASSERT(read_status == 1,
        {
            free(array);
            return NULL;
        });
    }
    *array_len = length;
    return array;
}

static unsigned int get_operands(Stack* stack, size_t n_op, int* op1, int* op2);

int execute_command(const int* cmd_array, Stack* stack, size_t* shift)
{
#define BINARY_OP_CASE(command, operation) case command:\
        err = get_operands(stack, 2, &op1, &op2);\
        LOG_ASSERT(!err, return -1);\
        err = StackPush(stack, op2 operation op1);\
        LOG_ASSERT(!err, return -1);\

    size_t iptr = 0;
    int command = cmd_array[iptr++];
    int op1 = 0, op2 = 0;
    unsigned int err = 0;
    switch (command)
    {
    case CMD_NOP: break;

    BINARY_OP_CASE(CMD_ADD, +) break;
    BINARY_OP_CASE(CMD_SUB, -) break;
    BINARY_OP_CASE(CMD_MUL, *) break;
    BINARY_OP_CASE(CMD_DIV, /) break;

    case CMD_INC:
        err = get_operands(stack, 1, &op1, NULL);
        LOG_ASSERT(!err, return -1);
        err = StackPush(stack, op1 + 1);
        LOG_ASSERT(!err, return -1);
        break;
    case CMD_DEC:
        err = get_operands(stack, 1, &op1, NULL);
        LOG_ASSERT(!err, return -1);
        err = StackPush(stack, op1 - 1);
        LOG_ASSERT(!err, return -1);
        break;
    case CMD_PUSH:
        op1 = cmd_array[iptr++];
        err = StackPush(stack, op1);
        LOG_ASSERT(!err, return -1);
        break;
    case CMD_POP:
        err = get_operands(stack, 1, &op1, NULL);
        LOG_ASSERT(!err, return -1);
        printf("%d\n", op1);
        break;

    case CMD_HALT:
        return 1;
        
    default:
        LOG_ASSERT(0 && "Invalid command encountered", return -1);
        break;
    }
    *shift = iptr;

    StackDump(stack);

    return 0;
}

static unsigned int get_operands(Stack* stack, size_t n_op, int* op1, int* op2)
{
    LOG_ASSERT(n_op == 1 || n_op == 2, return 1);
    
    LOG_ASSERT(op1 != NULL, return 1);
    unsigned int err = 0;
    *op1 = StackPopCopy(stack, &err);
    LOG_ASSERT(!err,        return 1);

    if (n_op == 2)
    {
        LOG_ASSERT(op2 != NULL, return 1);
        *op2 = StackPopCopy(stack, &err);

        LOG_ASSERT(!err,    return 1);
    }
    return 0;
}
