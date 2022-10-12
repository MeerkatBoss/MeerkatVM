#include "logger.h"
#include "vcpu_utils.h"
enum status
{
    STATUS_SUCCESS,
    STATUS_INPUT_ERROR,
    STATUS_RUNTIME_ERROR
};

int main(int argc, char** argv)
{
    add_default_file_logger();

    LOG_ASSERT_ERROR(argc == 2, return STATUS_INPUT_ERROR,
        "Incorrect program usage.", NULL);
    
    size_t cmd_count = 0;
    int* commands = read_cmd_array(argv[1], &cmd_count);
    LOG_ASSERT_ERROR(commands != NULL, return STATUS_INPUT_ERROR,
        "Corrupt program file.", NULL);

    Stack stack = {};
    int constructed = StackCtor(&stack);
    LOG_ASSERT_ERROR(constructed == 0, return STATUS_RUNTIME_ERROR,
        "Error initializing stack.", NULL);

    size_t iptr = 0;
    while(iptr < cmd_count)
    {
        size_t shift = 0;
        int status = execute_command(commands + iptr, &stack, &shift);
        LOG_ASSERT_ERROR(status != -1, return STATUS_INPUT_ERROR,
            "Invalid command sequence.", NULL);
        if (status == 1) break;
        iptr += shift;
    }

    free(commands);
    StackDtor(&stack);
    
    return STATUS_SUCCESS;
}
