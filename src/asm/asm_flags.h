#ifndef ASM_FLAGS_H
#define ASM_FLAGS_H

#include "argparser.h"

#define output_file (*output_file_addr_())
#define listing_file (*listing_file_addr_())

const char** output_file_addr_();
const char** listing_file_addr_();

/**
 * @brief Set the output file name
 * 
 * @param[in] strargs Array of strings containing arguments
 * @return 0 upon success, -1 otherwise
 */
int set_output_file(const char* const* strargs);

/**
 * @brief Set the listing file name
 * 
 * @param[in] strargs Array of strings containing arguments
 * @return 0 upon success, -1 otherwise
 */
int add_listing(const char* const* strargs);

/**
 * @brief 
 * Command-line arguments tags
 */
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

/**
 * @brief 
 * Length of `TAGS` array
 */
const size_t TAG_COUNT = sizeof(TAGS)/sizeof(*TAGS);

#endif