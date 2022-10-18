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
    add_logger(
        {
            .name = "Console output",
            .stream = stdout,
            .logging_level = LOG_ERROR,
            .settings_mask = LGS_USE_ESCAPE | LGS_KEEP_OPEN
        });

    LOG_ASSERT_ERROR(argc == 2, return STATUS_INPUT_ERROR,
        "Incorrect program usage.", NULL);
    
    proc_state vcpu = proc_ctor(argv[1]);
    LOG_ASSERT_ERROR(vcpu.cmd != NULL, return STATUS_INPUT_ERROR,
        "Failed to read program.", NULL);

    int res = proc_run(&vcpu);
    LOG_ASSERT_ERROR(res != -1,
        {
            proc_dtor(&vcpu);
            return STATUS_INPUT_ERROR;
        },
        "Failed to execute program.", NULL);
    
    proc_dtor(&vcpu);
    return STATUS_SUCCESS;
}
