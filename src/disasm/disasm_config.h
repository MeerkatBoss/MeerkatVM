#ifndef ASM_CONFIG_H
#define ASM_CONFIG_H

#include <stddef.h>

const char** output_file_addr_(void);
size_t* max_labels_addr_(void);

/**
 * @brief 
 * Output file name
 * 
 */
#define OUTPUT_FILE (*output_file_addr_())

/**
 * @brief 
 * Maximum allowed number of labels
 */
#define MAX_LABELS (*max_labels_addr_())

#endif