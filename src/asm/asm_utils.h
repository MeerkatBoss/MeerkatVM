#ifndef ASM_UTILS_H
#define ASM_UTILS_h

#include "argparser.h"

const char* output_file = "a.out";
const char* listing_file = NULL;

int set_output_file(const char** strargs);
int add_listing(const char** strargs);

const arg_tag TAGS[] = 
{
    {
        .short_tag = 'o',
        .long_tag = "output-to",
        .parameter_count = 1,
        .callback = set_output_file,
        .description = "Set output file. Default output file is a.out."
    },
    {
        .short_tag = 'l',
        .long_tag = "add-listing",
        .parameter_count = 1,
        .callback = add_listing,
        .description = "Output assembly listing to specified file."
    }
};

const size_t TAG_COUNT = sizeof(TAGS)/sizeof(*TAGS);



#endif