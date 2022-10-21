#include "asm_flags.h"
#include "logger.h"

const char** output_file_addr_()
{
    static const char* _output_file = "a.out";
    return &_output_file;
}

const char** listing_file_addr_()
{
    static const char* _listing_file = NULL;
    return &_listing_file;
}

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

int add_listing(const char* const* strargs)
{
    static int called = 0;

    LOG_ASSERT(strargs != NULL, return -1);

    LOG_ASSERT_ERROR(!called, return -1,
        "Listing file specified more than once", NULL);
    LISTING_FILE = *strargs;
    called = 1;
    return 0;
}