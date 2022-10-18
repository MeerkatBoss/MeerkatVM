/**
 * @file asm_flags.h
 * @author MeerkatBoss (solodovnikov.ia@phystech.edu)
 * @brief List of assembler command-line arguments and flags.
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ASM_FLAGS_H
#define ASM_FLAGS_H

#include "argparser.h"

/**
 * @brief 
 * Output file name
 */
#define output_file (*output_file_addr_())

/**
 * @brief 
 * Listing file name
 */
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
        .long_tag = "output",
        .parameter_count = 1,
        .callback = set_output_file,
        .description = "Set output file. Default output file is a.out."
    },
    {
        .short_tag = 'l',
        .long_tag = "listing",
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