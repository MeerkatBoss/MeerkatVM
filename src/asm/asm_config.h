#ifndef ASM_CONFIG_H
#define ASM_CONFIG_H

#include <stddef.h>

const char** output_file_addr_(void);
const char** listing_file_addr_(void);
size_t* max_labels_addr_(void);
size_t* max_fixups_addr_(void);

const size_t MAX_LABEL_LEN = 32;

/**
 * @brief 
 * Output file name
 * 
 */
#define OUTPUT_FILE (*output_file_addr_())

/**
 * @brief 
 * Listing file name
 */
#define LISTING_FILE (*listing_file_addr_())

/**
 * @brief 
 * Maximum allowed number of labels
 */
#define MAX_LABELS (*max_labels_addr_())

/**
 * @brief 
 * Maximum allowed number of fixups
 */
#define MAX_FIXUPS (*max_fixups_addr_())



#endif