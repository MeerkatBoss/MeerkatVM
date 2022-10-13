#ifndef ASM_CMD
#error "macro ASM_CMD was not defined"
#endif

#define _ ,

ASM_CMD(NOP, 0x00, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(ADD, 0x01, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(INC, 0x02, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(SUB, 0x03, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(DEC, 0x04, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(MUL, 0x05, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(DIV, 0x06, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(PUSH,0x07, {.arg_count = 1 _ .arg_list = {{.perms = ARG_RD}}})

ASM_CMD(POP, 0x08, {.arg_count = 0 _ .arg_list = {}})

ASM_CMD(HALT,0x09, {.arg_count = 0 _ .arg_list = {}})

#undef _