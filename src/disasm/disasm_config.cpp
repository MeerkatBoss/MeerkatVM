#include "disasm_config.h"

const char** output_file_addr_(void)
{
    static const char* _output_file = "a.disasm";

    return &_output_file;
}

size_t* max_labels_addr_(void)
{
    static size_t _max_labels = 256;

    return &_max_labels;
}
