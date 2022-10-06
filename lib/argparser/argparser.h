#ifndef ARGPARSER_H
#define ARGPARSER_H

/**
 * @file argparser.h
 * @author MeerkatBoss
 * @brief Command-line arguments parser
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stddef.h>

/**
 * @brief 
 * Callback used to parse argument parameters
 * @param[in] strargs Parameter array
 * @return 0 upon sucessful parse, -1 otherwise 
 */
typedef int arg_callback(const char* const* strparams);

/**
 * @brief 
 * Command-line argument description
 */
struct arg_tag
{
    char            short_tag;
    const char*     long_tag;
    size_t          parameter_count;
    arg_callback*   callback;
    const char*     description;
};

/**
 * @brief 
 * Parse command-line arguments based on `arg_tag` array
 * and call coresponding callbacks
 * 
 * @param[in] arg_count Length of `strargs` array
 * @param[in] strargs   Command-line arguments array
 * @param[in] tag_count Length of `tags` array
 * @param[in] tags      Argument descriptions array
 * @return 0 upon successful parse, -1 otherwise
 */
int parse_args(size_t arg_count, const char* const* strargs, size_t tag_count, const arg_tag* tags);

/**
 * @brief 
 * Print command-line options help based on array of
 * argument descriptions
 * 
 * @param[in] tag_count Length of `tags` array
 * @param[in] tags      Argument descriptions array
 */
void print_help(size_t tag_count, const arg_tag* tags);

#endif