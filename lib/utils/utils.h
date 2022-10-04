#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @author your name (you@domain.com)
 * @brief Miscellanious utility functions
 * @version 0.1
 * @date 2022-09-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stddef.h>

typedef unsigned long long hash_t;

/**
 * @brief
 * Calculate hash value of data
 * @param[in] data   Memory area 
 * @param[in] length Memory area size
 * @return Calculated hash
 */
hash_t GetHash(const void* data, size_t length);

/**
 * @brief 
 * Check if pointer is readable
 * @param[in] ptr Pointer to be checked
 * @return 1 if pointer is readable, 0 otherwise
 */
int CanReadPointer(const void *ptr);

#endif