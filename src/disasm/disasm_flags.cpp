#include "disasm_flags.h"
#include "logger.h"
#include "disasm_config.h"

int set_output_file(const char* const * strargs)
{
    static int called = 0;

    LOG_ASSERT(strargs != NULL, return -1);

    LOG_ASSERT_ERROR(!called, return -1,
        "Output file specified more than once", NULL);
    OUTPUT_FILE = *strargs;
    called = 1;
    return 0;
}

int set_max_labels(const char* const *strargs)
{
    static int called = 0;

    LOG_ASSERT(strargs != NULL, return -1);
    LOG_ASSERT_ERROR(!called, return -1,
        "Max label count specified more than once", NULL);

    LOG_ASSERT_ERROR(sscanf(*strargs, "%zu", &MAX_LABELS) == 1,
        return -1,
        "Invalid value for max label count: '%s'", *strargs);
    called = 1;

    return 0;
}

[[noreturn]]
int show_help(const char* const*)
{
    const int name_width = -16; 

    for (size_t i = 0; i < TAG_COUNT; i++)
    {
        printf("\033[1m-%c, --%*s\033[22m %s\n",
            TAGS[i].short_tag,
            name_width,
            TAGS[i].long_tag,
            TAGS[i].description);
    }
    exit(0);
}