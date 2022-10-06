#ifndef ASM_UTILS_H
#define ASM_UTILS_h

#include "assembler.h"
#include "argparser.h"
#include "text_lines.h"

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

/**
 * @brief Get the command by name
 * 
 * @param[in] str Command name
 * @param[out] n_read Number of charachters read from `str`
 * @return pointer to mathcing `command_description` upon success,
 * `NULL` otheriwse
 */
const command_description* get_description(const char* str, int* n_read);

/**
 * @brief Get command arguments
 * 
 * @param[in] command Command description
 * @param[in] strargs String containing command arguments
 * @param[out] buffer Output buffer
 * @return 0 upon success, -1 otherwise
 */
int get_args(const command_description* command, const char* strargs, void* buffer);

/**
 * @brief 
 * Print listing if listing file was specified
 * @param[in] text_lines Array of lines
 * @param[in] cmd_array  Array of parsed commands
 */
void try_print_listing(const TextLines text_lines, const void* cmd_array);


#endif