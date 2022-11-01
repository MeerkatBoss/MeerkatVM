#include <stdio.h>
#include <string.h>

#include "argparser.h"
#include "logger.h"

static const arg_tag* find_tag(const char* str, size_t tag_count, const arg_tag* tags);
static int compare_tag(const char* str, const arg_tag* tag);

int parse_args(size_t arg_count, const char* const* strargs, size_t tag_count, const arg_tag* tags)
{
    LOG_ASSERT(strargs != NULL, return -1);
    LOG_ASSERT(tags != NULL, return -1);

    for (size_t i = 0; i < arg_count; i++)
    {
        LOG_ASSERT(strargs[i] != NULL, return -1);
        const arg_tag* tag = find_tag(strargs[i], tag_count, tags);

        if (tag == NULL)
            return i;

        int call_result = tag->callback(strargs + i + 1);

        LOG_ASSERT_ERROR(
            call_result != -1, return -1,
            "Error parsing command-line option \'%s\'", strargs[i]);
        
        i += tag->parameter_count;
    }

    return arg_count;
}

void print_help(size_t tag_count, const arg_tag* tags)
{
    for (size_t i = 0; i < tag_count; i++)
    {
        LOG_ASSERT(tags[i].short_tag != '\0' || tags[i].long_tag != NULL, return);

        if (tags[i].short_tag != '\0')
            printf("-%c ", tags[i].short_tag);
        if (tags[i].long_tag != NULL)
            printf("--%s ", tags[i].long_tag);
        
        printf("- %s\n", tags->description);
    }
}

static const arg_tag* find_tag(const char* str, size_t tag_count, const arg_tag* tags)
{
    LOG_ASSERT(str[0] != '\0', return NULL);
    
    for (size_t i = 0; i < tag_count; i++)
        if (compare_tag(str, &tags[i]))
            return &tags[i];

    return NULL;
}

static int compare_tag(const char* str, const arg_tag* tag)
{
    int n_dashes = 0;

    while(*str == '-')
    {
        str++;
        n_dashes++;
    }

    return
        (n_dashes == 1 && str[1] == '\0' && str[0] == tag->short_tag) ||
        (n_dashes == 2 && strcmp(str, tag->long_tag) == 0);
}
