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

enum assembly_result
{
    ASM_SUCCESS     = 0,
    ASM_SYNTAX      = -1,
    ASM_LABEL_OVF   = -2,
    ASM_DEFS_OVF    = -3,
    ASM_FIXUP_OVF   = -4
};

const size_t MAX_LABEL_LEN = 32;
struct asm_label
{
    char name[32];
    ssize_t addr;
};

struct fixup
{
    size_t label_number;
    size_t addr;
};

struct asm_def
{
    char name[MAX_LABEL_LEN];
    int value;
};

const size_t MAX_LABELS = 64;
const size_t MAX_FIXUPS = MAX_LABELS * 4;

struct assembly_state
{
    padded_header   header;
    size_t          ip;
    int*            cmd;
    asm_label       labels[MAX_LABELS];
    asm_def         definitions[MAX_LABELS];
    fixup           fixups[MAX_FIXUPS];
    assembly_result result;
};

int assemble(TextLines* text_lines, assembly_state* state);

/**
 * @brief Get the command by name
 * 
 * @param[in] str Command name
 * @param[out] n_read Number of characters read from `str`
 * @return pointer to matching `command_description` upon success,
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