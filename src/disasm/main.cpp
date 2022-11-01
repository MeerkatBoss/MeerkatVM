#include "logger.h"
#include "argparser.h"

#include "disasm_utils.h"
#include "disasm_config.h"
#include "disasm_flags.h"

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

    argc--;
    argv++;

    int parsed = parse_args((size_t)argc, argv, TAG_COUNT, TAGS);
    LOG_ASSERT_ERROR(parsed != -1, return STATUS_INPUT_ERROR,
        "Invalid command-line options", NULL);
    LOG_ASSERT_ERROR(argc - parsed  >= 1, return STATUS_INPUT_ERROR,
        "Input file not specified", NULL);
    LOG_ASSERT_ERROR(argc - parsed  == 1, return STATUS_INPUT_ERROR,
        "Too many files specified", NULL);

    const char* filepath = argv[argc - 1];

    disasm_state* state = disasm_ctor(filepath, OUTPUT_FILE);
    disassemble(state);

    freopen(OUTPUT_FILE, "w", state->output);  /* reset output */

    state->ip = 0;
    LOG_ASSERT_ERROR(disassemble(state) == 0, return 1,
        "Disassembly of '%s' failed.", filepath);
    disasm_dtor(state);

    return 0;
}
