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
 * @brief Set the output file name
 * 
 * @param[in] strargs Array of strings containing arguments
 * @return 0 upon success, -1 otherwise
 */
int set_output_file(const char* const* strargs);

/**
 * @brief Set max labels count
 * 
 * @param[in] strargs Array of strings containing arguments
 * @return 0 upon success, -1 otherwise
 */
int set_max_labels(const char* const *strargs);

/**
 * @brief Display help message
 * 
 * @param[in] strargs Array of strings containing arguments
 * @return 0 upon success, -1 otherwise
 */
int show_help(const char* const* strargs);

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
        .description = "Set output file. Default output file is \033[3ma.disasm.\033[23m"
    },
    {
        .short_tag = 'L',
        .long_tag = "max-labels",
        .parameter_count = 1,
        .callback = set_max_labels,
        .description = "Set maximum allowed number of labels (different jump addresses)\n"
                        "\t\t\tin a program. Default value is 256."
    },
    {
        .short_tag = 'h',
        .long_tag = "help",
        .parameter_count = 0,
        .callback = show_help,
        .description = "Show command-line help and exit."
    }
};

/**
 * @brief 
 * Length of `TAGS` array
 */
const size_t TAG_COUNT = sizeof(TAGS)/sizeof(*TAGS);

#endif