#include "logger.h"
#include "disasm_utils.h"

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

    LOG_ASSERT_ERROR(argc == 2, return -1,
        "Invalid program usage", NULL);

    disasm_state* state = disasm_ctor(argv[1], "disasm.txt");
    disassemble(state);

    freopen("disasm.txt", "w", state->output);  /* reset output */

    state->ip = 0;
    LOG_ASSERT_ERROR(disassemble(state) == 0, return 1,
        "Disassembly of '%s' failed.", argv[1]);
    disasm_dtor(state);

    return 0;
}
