#ifndef CUSTOM_STACK_H
#define CUSTOM_STACK_H

/**
 * @file _stack_interface.h
 * @author MeerkatBoss
 * @brief Stack data structure function definitions
 * @version 0.1
 * @date 2022-09-25
 * 
 * @warning This header is internal, It IS NOT supposed
 * to be included outside the scope of the library.
 * 
 */

#include <stddef.h>

#include "utils.h"

#ifndef USE_CUSTOM_ELEMENT
    /**
     * @brief 
     * Stack element type
     */
    typedef void* element_t;

    /**
     * @brief 
     * Invalid element value
     */
    const element_t element_poison = NULL;

    inline int IsPoison(element_t element) { return element == NULL; }

    /**
     * @brief 
     * Element printf specifier
     */
    #define ELEMENT_SPEC "%p"

    /**
     * @brief 
     * Element printf args
     * 
     * @param[in] element Printed element
     */
    #define ELEMENT_ARGS(element) (element)
#endif

#define STK_CANARY_PROT 01
#define STK_HASH_PROT   02
#define STK_DEBUG_INFO  04 

#ifndef STK_PROT_LEVEL
#define STK_PROT_LEVEL STK_CANARY_PROT | STK_HASH_PROT | STK_DEBUG_INFO
#endif

#if (STK_PROT_LEVEL & STK_CANARY_PROT)
    #define _ON_CANARY(...) __VA_ARGS__
    #define _NO_CANARY(...)
#else
    #define _ON_CANARY(...)
    #define _NO_CANARY(...) __VA_ARGS__
#endif

#if (STK_PROT_LEVEL & STK_HASH_PROT)
    #define _ON_HASH(...) __VA_ARGS__
    #define _NO_HASH(...)
#else
    #define _ON_HASH(...)
    #define _NO_HASH(...) __VA_ARGS__
#endif

#if (STK_PROT_LEVEL & STK_DEBUG_INFO)
    #define _ON_DEBUG_INFO(...) __VA_ARGS__
    #define _NO_DEBUG_INFO(...)
#else
    #define _ON_DEBUG_INFO(...)
    #define _NO_DEBUG_INFO(...) __VA_ARGS__
#endif

#ifndef NSTACK_CHECK
    #define _ON_STACK_CHECK(...) __VA_ARGS__
    #define _NO_STACK_CHECK(...)
#else
    #define _ON_STACK_CHECK(...)
    #define _NO_STACK_CHECK(...) __VA_ARGS__
#endif

typedef unsigned long long canary_t;

const canary_t CANARY = 0xD1AB011CA1C0C0A5ULL;  /* DIABOLICAL COCOAS*/

_ON_HASH(
    typedef unsigned long long hash_t;
)

enum ErrorFlags : unsigned int
{ 
    STK_NO_ERROR        = 00000,
    STK_EMPTY           = 00001,
    STK_NO_MEMORY       = 00002,
    STK_BAD_PTR         = 00004,
    STK_BAD_DATA_PTR    = 00010,
    STK_DEAD_CANARY     = 00020,
    STK_WRONG_HASH      = 00040,
    STK_WRONG_DATA_HASH = 00100,
    STK_CORRUPTED_SIZE  = 00200,
    STK_CORRUPTED_CAP   = 00400,
    STK_CORRUPTED_DATA  = 01000,
};

/**
 * @brief 
 * Debug information about variable
 */
struct debug_info_
{
    const char *name;       /* variable name */
    const char *func_name;  /* declaring function name */
    const char *file_name;  /* declaring file name */
    size_t      line_num;   /* declaration line */
                            /* ^--- I like my comments nice and C-style
                                Everything ISO ever touched is inherently evil
                            */
};

/**
 * @brief 
 * LIFO data structure
 */
struct Stack
{
    _ON_CANARY(     canary_t        canary_start_;)
                    element_t*        data;           /* stored elements */
                    size_t          size;           /* stored elements count*/
                    size_t          capacity;       /* maximum capacity */
    _ON_HASH(       hash_t          hash_;)
    _ON_HASH(       hash_t          data_hash_;)
    _ON_DEBUG_INFO( debug_info_     debug_;)        
    _ON_CANARY(     canary_t        canary_end_;)
};

/**
 * @brief 
 * Construct `Stack` instance from parameters
 * @param[out] stack     constructed instance
 * @param[in]  name      variable name. Used only if
 *                          `STK_PROT_LEVEL` & `STK_DEBUG_INFO` != 0
 * @param[in]  func_name declaring function name. Used only if
 *                          `STK_PROT_LEVEL` & `STK_DEBUG_INFO` != 0
 * @param[in]  file_name declaring file name. Used only if
 *                          `STK_PROT_LEVEL` & `STK_DEBUG_INFO` != 0
 * @param[in]  line_num  declaration line. Used only if
 *                          `STK_PROT_LEVEL` & `STK_DEBUG_INFO` != 0
 * @return zero upon successful construction, non-zero otherwise
 */
int     StackCtor_      (Stack* stack,
                        const char* name,
                        const char* func_name,
                        const char* file_name,
                        size_t line_num);

/**
 * @brief 
 * Construct `Stack`
 * 
 * @param[out] stack constructed instance
 * 
 * @return zero upon successful construction, non-zero otherwise
 */
#define StackCtor(       stack) StackCtor_(stack,               \
                                    #stack + (*#stack == '&'),  \
                                    __PRETTY_FUNCTION__,        \
                                    __FILE__, __LINE__);
// TODO: I'd just use a separate function to perform "&stack" => "stack"
//       transformation, befor calling StackCtor

/**
 * @brief 
 * Clean up `Stack` instance. Free associated resources
 * 
 * @param[inout] stack instance to be cleaned up
 */
void    StackDtor       (Stack* stack);

/**
 * @brief
 * Add element to stack
 * 
 * @param[inout] stack `Stack` instance
 * @param[in] value    value to be added
 * @return zero upon success, some combination of
 * `ErrorFlags` otherwise
 */
unsigned int StackPush  (Stack* stack, element_t value);

/**
 * @brief 
 * Remove top element from stack
 * 
 * @param[inout] stack `Stack` instance
 * @return zero upon success, some combination of
 * `ErrorFlags` otherwise
 */
unsigned int StackPop   (Stack* stack);

/**
 * @brief
 * Remove top element from stack
 * 
 * @param[inout] stack `Stack` instance
 * @param[out] err Error code, i.e some combination of
 * `ErrorFlags`. Parameter is ignored if set to NULL
 * @return Removed value
 */
element_t StackPopCopy    (Stack* stack, unsigned int* err);

/**
 * @brief
 * Get top element from stack
 * 
 * @param[inout] stack `Stack` instance
 * @param[out] err Error code, i.e some combination of
 * `ErrorFlags`. Parameter is ignored if set to NULL
 * @return Pointer to element in stack
 * 
 * @warning Pointer invalidates after call to `StackPop`
 * with the same `Stack` instance
 */
element_t* StackPeek      (const Stack* stack, unsigned int* err);

#endif
