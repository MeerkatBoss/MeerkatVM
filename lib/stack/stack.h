#ifndef CUSTOM_STACK_IMPL
#define CUSTOM_STACK_IMPL

/**
 * @file stack.h
 * @author MeerkatBoss
 * @brief Stack data structure
 * @version 0.1
 * @date 2022-09-25
 * 
 * @copyright Copyright (c) 2022
 * TODO:                   ^ usually here goes name :)
 * 
 * @note For usage with any desired type define
 * `element_t` type, `element_poison` constant,
 * `int IsPoison(element_t element)` function, and 
 * `ELEMENT_SPEC` with `ELEMENT_ARGS(element)` macros.
 * 
 * @warning This header DOES NOT support separate compilation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "_stack_interface.h"
#include "logger.h"

/**
 * @brief 
 * Recalculate hash values in stack
 * @param[inout] stack `Stack` instance
 */
void        StackRecalculateHash_   (Stack* stack);

/**
 * @brief
 * Calculate stack hash value
 * 
 * @param[in] stack `Stack` instance
 * @return Hash value
 */
hash_t      GetStackHash_           (const Stack* stack);

/**
 * @brief 
 * Check stack integrity
 * 
 * @param[in] stack `Stack` instance
 * @param[in] func  calling fuction name
 * @param[in] file  calling file name
 * @param[in] line  calling line number
 * @return zero upon success, some combination of
 * `ErrorFlags` otherwise
 */
unsigned int    StackCheck      (const Stack* stack);

/**
 * @brief 
 * Check stack buffer integrity
 * 
 * @param[in] stack `Stack` instance
 * @return zero upon success, some combination of
 * `ErrorFlags` otherwise
 */
unsigned int    StackDataCheck_ (const Stack* stack);

/**
 * @brief 
 * Print `Stack` contents
 * 
 * @param[in] stack `Stack` instance
 * @param[in] func  calling fuction name
 * @param[in] file  calling file name
 * @param[in] line  calling line number
 */
unsigned int    StackAssert_       (const Stack*  stack,
                              const char*   func,
                              const char*   file,
                              size_t        line,
                              int force_dump);

/**
 * @brief 
 * Print `Stack` contents
 * 
 * @param[in] stack `Stack` instance
 */
#define StackAssert(stack) StackAssert_(stack,          \
                                    __PRETTY_FUNCTION__,\
                                    __FILE__, __LINE__, 0)

#define StackDump(stack) StackAssert_(stack,          \
                                    __PRETTY_FUNCTION__,\
                                    __FILE__, __LINE__, 1)

#define TRY_ASSIGN_PTR(ptr, value) if (ptr) {*(ptr) = value;}

inline enum ErrorFlags GetErrorFlag(int condition, enum ErrorFlags flag)
{
    return (condition) ? flag : STK_NO_ERROR;
}


/**
 * @brief 
 * Stack capacity growth coefficient
 */
const double stack_growth_ = 2;

/**
 * @brief 
 * Stack default capacity
 */
const size_t default_cap_ = 16;

/**
 * @brief 
 * Grow stack if needed so that it will be ready
 * to accept new element
 * @param[inout] stack `Stack` instance
 * @return zero upon success, non-zero otherwise
 */
int         StackTryGrow_       (Stack* stack);

/**
 * @brief 
 * Ensure stack has no eccess capacity
 * @param[inout] stack `Stack` instance
 * @return zero upon success, non-zero otherwise
 */
int         StackTryShrink_     (Stack* stack);

/**
 * @brief 
 * (Re)allocate array with size `new_size`,
 * add canaries before array start and after
 * its end.
 * @param[inout] old_array pointer to beginning of an
 * array, returned by call to `ReallocWithCanary_` or
 * `NULL`
 * @param[in] old_size length of old array. This parameter
 * is ignored if `old_array` is `NULL`
 * @param[in] new_size required new length of array
 * @return pointer to beginning of allocated array
 * or `NULL` upon failure
 * 
 * @warning If `old_array` is not `NULL` and it
 * was not returned by previous call to `ReallocWithCanary_`,
 * the result is undefined
 * 
 * @warning If new_size is 0 the result is undefined
 */
element_t*    ReallocWithCanary_  (element_t* old_array,
                                size_t old_size,
                                size_t new_size);

/**
 * @brief 
 * Free memory allocated by `ReallocWithCanary_`
 * @param[inout] ptr Memory to be freed
 */
void        FreeWithCanary_     (element_t* ptr);

int StackCtor_(Stack* stack,
                const char* name,
                const char* func_name,
                const char* file_name,
                size_t line_num)
{
    element_t* data = ReallocWithCanary_(NULL, 0, default_cap_);

    if (!data)
        return -1; // TODO: What about an enum for errors?)
    
    _ON_CANARY(
        canary_t canary = CANARY ^ (canary_t)stack;
    )
    *stack = {
        _ON_CANARY(     .canary_start_  = canary,)
                        .data           = data,
                        .size           = 0,
                        .capacity       = default_cap_,
        _ON_HASH(       .hash_          = 0,)
        _ON_HASH(       .data_hash_     = 0,)
        _ON_DEBUG_INFO(
                        .debug_         = {
                            .name       = name,
                            .func_name  = func_name,
                            .file_name  = file_name,
                            .line_num   = line_num
                        },
        )
        _ON_CANARY(     .canary_end_    = canary,)
    };

    StackRecalculateHash_(stack);
    
    log_message(MSG_TRACE, "Constructed stack at %p", stack);
    return 0;
}

void StackDtor(Stack* stack)
{
    if (StackAssert(stack) != STK_NO_ERROR)
        return;
    FreeWithCanary_(stack->data);
    *stack = {};
    log_message(MSG_TRACE, "Destroyed stack at %p", stack);
}

unsigned int StackPush(Stack* stack, element_t value)
{
    unsigned int err = StackAssert(stack);
    if (err) return err;

    int push_status = StackTryGrow_(stack);
    if (push_status < 0)
    {
        log_message(MSG_WARNING, "Failed to push to stack %p. Not enough memory", stack);
        return STK_NO_MEMORY;
    }
    
    stack->data[stack->size++] = value;

    StackRecalculateHash_(stack);

    return STK_NO_ERROR;
}

unsigned int StackPop (Stack* stack)
{
    unsigned int err = StackAssert(stack);
    if (err) return err;

    if (stack->size == 0)
    {
        log_message(MSG_WARNING, "Attempt to pop empty stack %p spotted.", stack);
        return STK_EMPTY;
    }

    stack->data[--stack->size] = element_poison;
    StackTryShrink_(stack);

    StackRecalculateHash_(stack);

    return STK_NO_ERROR;
}

element_t StackPopCopy    (Stack* stack, unsigned int* err = NULL)
{
    unsigned int err_flags = StackAssert(stack);
    if (err_flags)
    {
        TRY_ASSIGN_PTR(err, err_flags);
        return element_poison;
    }

    if (stack->size == 0)
    {
        TRY_ASSIGN_PTR(err, STK_EMPTY);
        return element_poison;
    }
    
    element_t result = stack->data[stack->size - 1];
    stack->data[--stack->size] = element_poison;
    StackTryShrink_(stack);

    _ON_HASH(
        stack->hash_ = 0;
        stack->data_hash_ = GetHash(stack->data, stack->capacity);
        stack->hash_      = GetHash(stack, sizeof(*stack));
    )

    TRY_ASSIGN_PTR(err, STK_NO_ERROR);
    return result;
}

element_t* StackPeek      (const Stack* stack, unsigned int* err = NULL)
{
    unsigned int err_flags = StackAssert(stack);
    if (err_flags)
    {
        TRY_ASSIGN_PTR(err, err_flags);
        return NULL;
    }

    TRY_ASSIGN_PTR(err, err_flags);
    return stack->data + stack->size;
}

unsigned int StackCheck(const Stack* stack)
{
    if (!CanReadPointer(stack))
        return STK_BAD_PTR;

    unsigned int flags = STK_NO_ERROR;

    _ON_HASH(
    flags |= GetErrorFlag(GetStackHash_(stack) != stack->hash_, STK_WRONG_HASH);
    )

    _ON_CANARY(
    canary_t canary = CANARY ^ (canary_t)stack;
    flags |= GetErrorFlag(stack->canary_start_ != canary,       STK_DEAD_CANARY);
    flags |= GetErrorFlag(stack->canary_end_   != canary,       STK_DEAD_CANARY);
    )

    flags |= GetErrorFlag((long long)stack->size < 0,           STK_CORRUPTED_SIZE);
    flags |= GetErrorFlag(stack->size > stack->capacity,        STK_CORRUPTED_SIZE);

    flags |= GetErrorFlag((long long)stack->capacity < 0,       STK_CORRUPTED_CAP);

    if (flags & (STK_CORRUPTED_SIZE | STK_CORRUPTED_SIZE)) 
        return flags;

    flags |= StackDataCheck_(stack);

    return flags;
}

unsigned int StackDataCheck_(const Stack* stack)
{
    unsigned int flags = STK_NO_ERROR;

    if (!CanReadPointer(stack->data))
        return flags | STK_BAD_DATA_PTR;

    _ON_HASH(
    flags |= GetErrorFlag(
                stack->data_hash_ != GetHash(stack->data, stack->capacity),
                STK_WRONG_DATA_HASH);
    )

    _ON_CANARY(
    canary_t* start = ((canary_t*) stack->data)- 1;
    canary_t* end   =  (canary_t*)(stack->data + stack->capacity);
    
    if (!CanReadPointer(start) || !CanReadPointer(end))
        return flags | STK_BAD_DATA_PTR;

    flags |= GetErrorFlag(CANARY != *start || CANARY != *end, STK_CORRUPTED_DATA);
    )

    // TODO: Extractable
    for (size_t i = 0; i < stack->size; i++)
        if (!CanReadPointer(stack->data + i) || IsPoison(stack->data[i]))
            return flags | STK_CORRUPTED_DATA;

    for (size_t i = stack->size; i < stack->capacity; i++)
        if (!CanReadPointer(stack->data + i) || !IsPoison(stack->data[i]))
            return flags | STK_CORRUPTED_DATA;
    
    return flags;
}

unsigned int StackAssert_(const Stack* stack,
                            const char*  func,
                            const char*  file,
                            size_t       line,
                            int force = 0)
{
    unsigned int errs = StackCheck(stack);
    
    if (!errs && !force)
        return STK_NO_ERROR;

    message_level level = errs ? MSG_ERROR : MSG_INFO;

    log_message(level, "Dumping stack[%p] (status: %s)\n"
        "\tin %s:%zu in file \'%s\'\n",
            stack,
            errs ? "CORRUPTED" : "ok",
            func, line, file);
    
    _ON_DEBUG_INFO(
    log_message(level, "Stack \'%s\' declared in %s on line %zu, file \'%s\'\n",
            stack->debug_.name,
            stack->debug_.func_name,
            stack->debug_.line_num,
            stack->debug_.file_name);
    )


    if (errs)
        log_message(level, "Error flags: %o\n", errs);

    _ON_HASH(
    log_message(level, "Hash:"
            "\tstored: %#llx\n"
            "\tactual: %#llx\n",
            stack->hash_, GetStackHash_(stack));
    )

    // TODO: Extractable!
    _ON_CANARY(
    log_message(level, "Canary state:\n"
        "\tstart: %#016llx ^ %#016llx\n"
        "\tend  : %#016llx ^ %#016llx\n",
        CANARY, stack->canary_start_ ^ CANARY,
        CANARY, stack->canary_end_   ^ CANARY);
    )

    // TODO: Extractable!
    log_message(level, "Elements stored: %zu\n"
        "Total capacity : %zu\n",
        stack->size,
        stack->capacity);
    
    _ON_HASH(
        log_message(level, "Data hash:\n"
                "\tstored: %#llx\n"
                "\tactual: %#llx\n",
                stack->data_hash_,
                GetHash(stack->data, stack->capacity));
    )
    
    // TODO: Extractable!
    log_message(level, "Data[%p]:\n", stack->data);
    if (errs & STK_BAD_DATA_PTR) // TODO: Mix of conditinally and uncoditionally compilated, can you reduce?
    {
        log_message(level, "\tNOT READABLE\n");
        return errs;
    }

    _ON_CANARY(
    canary_t* start = ((canary_t*) stack->data)- 1;

    if (CanReadPointer(start))
        log_message(level, "\tcanary: %#016llx\n", *start);
    else
        log_message(level, "\tcanary: NOT READABLE\n");
    )

    for (size_t i = 0; i < stack->capacity; i++)
    {
        if (!CanReadPointer(stack->data + i) || IsPoison(stack->data[i]))
            log_message(level, "\t%c[%zu]: %s",
                    CanReadPointer(stack->data + i)
                        ? "POISON"
                        : "NOT READABLE");
            
        log_message(level, "\t%c[%zu]: " ELEMENT_SPEC,
            i < stack->size ? '*' : ' ',
            i,
            ELEMENT_ARGS(stack->data[i]));
    }

    _ON_CANARY(
    canary_t* end = (canary_t*)(stack->data + stack->capacity);

    if (CanReadPointer(end))
        log_message(level, "\tcanary: %#016llx\n", *end);
    else
        log_message(level, "\tcanary: NOT READABLE\n");
    )

    return errs;
}

element_t* ReallocWithCanary_(element_t* old_array,
                            size_t   old_size,
                            size_t   new_size)
{
    _ON_CANARY(
        if (old_array == NULL) old_size = 0;

        /* Allocate result */
        void* allocated = realloc(
                /* Calculate real array start*/
                old_array
                    ? (canary_t*)old_array - 1  /* get real beginning */
                    : NULL,                     /* allocate new array */
                /* Ensure there is enough space for canaries */
                new_size*sizeof(element_t) + 2*sizeof(canary_t));
        
        if (allocated == NULL) // TODO: Check why? perror?
            return NULL;

        element_t* result = (element_t*)((canary_t*)allocated + 1);
        /* Fill new elements (if any) with poison*/
        
        for (size_t i = old_size; i < new_size; i++) // TODO: Trying to beat memset, huh?)
            result[i] = element_poison;

        /* Set canaries before and after array*/
        ((canary_t*) result)[-1]        = CANARY;
        *(canary_t*)(result + new_size) = CANARY;
        return result;
    )
    _NO_CANARY(
        return realloc(old_array, new_size);
    )

}

void FreeWithCanary_(element_t* ptr)
{
    free((canary_t*)ptr - 1);
}

inline size_t GetNewCapacity_(size_t size)
{
    return (size_t)round((double)size * stack_growth_);
}

inline size_t GetCapacityLimit_(size_t size)
{
    return (size_t)round((double)size * stack_growth_*stack_growth_);
}

int StackTryGrow_(Stack* stack)
{
    if (stack->size < stack->capacity)
        return 0;
    
    size_t new_capacity = GetNewCapacity_(stack->size);
    
    element_t* new_data = ReallocWithCanary_(stack->data,
                                           stack->capacity,
                                           new_capacity);
    if (!new_data)
        return -1; /* There is no bool in C*/
    
    stack->data     = new_data;
    stack->capacity = new_capacity;

    return 0;
}

int StackTryShrink_(Stack* stack)
{
    size_t capacity_limit = GetCapacityLimit_(stack->size);

    if (capacity_limit <= default_cap_ || stack->capacity < capacity_limit)
        return 0;

    size_t new_capacity = GetNewCapacity_(stack->size);

    if (new_capacity <= default_cap_)
        new_capacity = default_cap_;
    
    element_t* new_data = ReallocWithCanary_(
                                        stack->data,
                                        stack->capacity,
                                           new_capacity);
    if (!new_data)
        return -1; /* Still no bools in C */
    
    stack->data     = new_data;
    stack->capacity = new_capacity;

    return 0;
}

void StackRecalculateHash_(Stack* stack)
{
    _ON_HASH(
        stack->data_hash_ = GetHash(stack->data, stack->capacity);
        stack->hash_      = GetStackHash_(stack);
    )
}

hash_t GetStackHash_(const Stack* stack)
{
    _ON_HASH(
        hash_t old_hash = stack->hash_;
        const_cast<Stack*>(stack)->hash_ = 0;
        hash_t new_hash = GetHash(stack, sizeof(*stack));
        const_cast<Stack*>(stack)->hash_ = old_hash;
        return new_hash;
    )
    return 0;
}
#endif
