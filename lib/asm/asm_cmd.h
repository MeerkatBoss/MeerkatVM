/**
 * @file asm_cmd.h
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
 * @brief Processor command definitions.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ASM_CMD
#error "macro ASM_CMD was not defined"
#endif

#define _ ,

#define ASM_JMP(name, num, cmp)\
    ASM_CMD(name, num, {.arg_count = 1 _ .arg_list = {{.perms = ARG_LABEL}}},\
    {\
        ASSERT(ST_SIZE >= 2, "Cannot pop empty stack");\
        int op1 = POP _ op2 = POP;\
        asm_arg argument = NEXT_ARG;\
        if (op2 cmp op1) IP = *argument.val_ptr - 1;\
    })

ASM_CMD(NOP, 0x00, {.arg_count = 0 _ .arg_list = {}}, {})

ASM_CMD(ADD, 0x01, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 2, "Cannot pop empty stack");
    PUSH(POP + POP);
})

ASM_CMD(INC, 0x02, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 1, "Cannot pop empty stack");
    PUSH(POP + 1);
})

ASM_CMD(SUB, 0x03, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 2, "Cannot pop empty stack");
    int op1 = POP _ op2 = POP;
    PUSH(op2 - op1);
})

ASM_CMD(DEC, 0x04, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 1, "Cannot pop empty stack");
    PUSH(POP - 1);
})

ASM_CMD(MUL, 0x05, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 2, "Cannot pop empty stack");
    PUSH(POP * POP);
})

ASM_CMD(DIV, 0x06, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 2, "Cannot pop empty stack");
    int op1 = POP _ op2 = POP;
    ASSERT(op1 != 0, "Attempted division by zero");
    PUSH(op2 / op1);
})

ASM_CMD(PUSH,0x07, {.arg_count = 1 _ .arg_list = {{.perms = ARG_RD}}},
{
    asm_arg argument = NEXT_ARG;
    PUSH(*argument.val_ptr);
})

ASM_CMD(POP, 0x08, {.arg_count = 1 _ .arg_list = {{.perms = ARG_WR}}},
{
    ASSERT(ST_SIZE >= 1, "Cannot pop empty stack");
    asm_arg argument = NEXT_ARG;
    ASSERT(argument.perms & ARG_WR, "Cannot pop to given address.");
    *argument.val_ptr = POP;
    if (argument.val_ptr == &REGS[REG_VBP])
        REFRESH;
})

ASM_CMD(OUT, 0x09, {.arg_count = 0 _ .arg_list = {}},
{
    ASSERT(ST_SIZE >= 1, "Cannot pop empty stack");
    printf("%d\n", POP);
})

ASM_CMD(HALT,0x0A, {.arg_count = 0 _ .arg_list = {}},
{
    STOP;
})

ASM_CMD(JMP, 0x0B, {.arg_count = 1 _ .arg_list = {{.perms = ARG_LABEL}}},
{
    asm_arg argument = NEXT_ARG;
    IP = *argument.val_ptr - 1;
})

ASM_JMP(JG,  0x0C, >)
ASM_JMP(JL,  0x0D, <)
ASM_JMP(JE,  0x0E, ==)
ASM_JMP(JNE, 0x0F, !=)
ASM_JMP(JGE, 0x10, >=)
ASM_JMP(JLE, 0x11, <=)

ASM_CMD(CALL,0x12, {.arg_count = 1 _ .arg_list = {{.perms = ARG_LABEL}}},
{
    asm_arg argument = NEXT_ARG;
    PUSH_CALL;
    IP = *argument.val_ptr - 1;
})

ASM_CMD(RET, 0x13, {},
{
    ASSERT(CALL_DEPTH >= 1, "No call to return from");
    IP = POP_CALL;
})

ASM_CMD(DUP, 0x14, {},
{
    ASSERT(ST_SIZE >= 1, "Cannot pop empty stack");
    int tmp = POP;
    PUSH(tmp);
    PUSH(tmp);
})

ASM_CMD(GET, 0x15, {},
{
    int val = 0;
    int success = scanf("%d", &val);
    ASSERT(success == 1, "Failed to read number");
    PUSH(val);
})


#undef _