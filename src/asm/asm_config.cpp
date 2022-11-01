#include "asm_config.h"

const char** output_file_addr_(void)
{
    static const char* _output_file = "a.out";

    return &_output_file;
}

const char** listing_file_addr_(void)
{
    static const char* _listing_file = NULL;

    return &_listing_file;
}

size_t* max_labels_addr_(void)
{
    static size_t _max_labels = 64;

    return &_max_labels;
}

size_t* max_fixups_addr_(void)
{
    static size_t _max_fixups = MAX_LABELS * 4;

    return &_max_fixups;
}
