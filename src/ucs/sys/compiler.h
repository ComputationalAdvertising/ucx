/**
* Copyright (C) Mellanox Technologies Ltd. 2001-2014.  ALL RIGHTS RESERVED.
*
* See file LICENSE for terms.
*/

#ifndef UCS_COMPILER_H_
#define UCS_COMPILER_H_

#include "preprocessor.h"
#include "compiler_def.h"

#include <stddef.h>
#include <stdarg.h>

#ifndef ULLONG_MAX
#define ULLONG_MAX (__LONG_LONG_MAX__ * 2ULL + 1)
#endif


#ifdef __ICC
#  pragma warning(disable: 268)
#endif

/*
 * Assertions which are checked in compile-time
 *
 * Usage: UCS_STATIC_ASSERT(condition)
 */
#define UCS_STATIC_ASSERT(_cond) \
     switch(0) {case 0:case (_cond):;}

/* Aliasing structure */
#define UCS_S_MAY_ALIAS __attribute__((may_alias))

/* A function without side effects */
#define UCS_F_PURE   __attribute__((pure))

/* A function which does not return */
#define UCS_F_NORETURN __attribute__((noreturn))

/* A function which should not be optimized */
#if defined(HAVE_ATTRIBUTE_NOOPTIMIZE) && (HAVE_ATTRIBUTE_NOOPTIMIZE == 1)
#define UCS_F_NOOPTIMIZE __attribute__((optimize("O0")))
#else
#define UCS_F_NOOPTIMIZE
#endif

/* Avoid inlining the function */
#define UCS_F_NOINLINE __attribute__ ((noinline))

/* Shared library constructor and destructor */
#define UCS_F_CTOR __attribute__((constructor))
#define UCS_F_DTOR __attribute__((destructor))

/* Silence "defined but not used" error for static function */
#define UCS_F_MAYBE_UNUSED __attribute__((used))

/* Silence "uninitialized variable" for stupid compilers (gcc 4.1)
 * which can't optimize properly.
 */
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ == 1)) || !defined(__OPTIMIZE__))
#  define UCS_V_INITIALIZED(_v)  (_v = (typeof(_v))0)
#else
#  define UCS_V_INITIALIZED(_v)  ((void)0)
#endif

/* Unused variable */
#define UCS_V_UNUSED __attribute__((unused))

/* Aligned variable */
#define UCS_V_ALIGNED(_align) __attribute__((aligned(_align)))

/* Used for labels */
#define UCS_EMPTY_STATEMENT {}

/**
 * @return Offset of _member in _type. _type is a structure type.
 */
#define ucs_offsetof(_type, _member) \
    ((unsigned long)&( ((_type*)0)->_member ))

/**
 * Get a pointer to a struct containing a member.
 *
 * @param __ptr   Pointer to the member.
 * @param type    Container type.
 * @param member  Element member inside the container.

 * @return Address of the container structure.
 */
#define ucs_container_of(_ptr, _type, _member) \
    ( (_type*)( (char*)(void*)(_ptr) - ucs_offsetof(_type, _member) )  )


/**
 * Size of statically-declared array
 */
#define ucs_static_array_size(_array) \
    ({ \
        UCS_STATIC_ASSERT((void*)&(_array) == (void*)&((_array)[0])); \
        ( sizeof(_array) / sizeof((_array)[0]) ); \
    })

/**
 * @return Address of a derived structure. It must have a "super" member at offset 0.
 * NOTE: we use the built-in offsetof here because we can't use ucs_offsetof() in
 *       a constant expression.
 */
#define ucs_derived_of(_ptr, _type) \
    ({\
        UCS_STATIC_ASSERT(offsetof(_type, super) == 0) \
        ucs_container_of(_ptr, _type, super); \
    })

/**
 * Prevent compiler from reordering instructions
 */
#define ucs_compiler_fence()       asm volatile(""::: "memory")

/**
 * Prefetch cache line
 */
#define ucs_prefetch(p)            __builtin_prefetch(p)

/* Branch prediction */
#define ucs_likely(x)              __builtin_expect(x, 1)
#define ucs_unlikely(x)            __builtin_expect(x, 0)

/* Check if an expression is a compile-time constant */
#define ucs_is_constant(expr)      __builtin_constant_p(expr)

/* Special pointer value */
#define UCS_ERR_PTR(err)           ((void*)-(err + 1))

/* Helper macro for address arithmetic in bytes */
#define UCS_PTR_BYTE_OFFSET(_ptr, _offset) \
    ((void *)((uintptr_t)(_ptr) + (_offset)))

/**
 * Copy words from _src to _dst.
 *
 * @param _dst         Destination buffer.
 * @param _src         Source buffer.
 * @param _word_type   Type to use for copying.
 * @param _size        Number of bytes to copy.
 */
#define UCS_WORD_COPY(_dst, _src, _word_type, _size) \
    { \
        unsigned i; \
        for (i = 0; i < (_size) / sizeof(_word_type); ++i) { \
            *((_word_type*)(_dst) + i) = *((_word_type*)(_src) + i); \
        } \
    }

#define UCS_ALLOCA_MAX_SIZE  1200

/**
 * alloca which makes sure the size is small enough.
 */
#define ucs_alloca(_size) \
    ({ \
        ucs_assertv((_size) <= UCS_ALLOCA_MAX_SIZE, "alloca(%zu)", (size_t)(_size)); \
        alloca(_size); \
    })


/**
 * Define cache-line padding variable inside a structure
 *
 * @param ...    List of types, of the variables which should be padded to cache line.
 */
#define UCS_CACHELINE_PADDING(...) \
    char UCS_PP_APPEND_UNIQUE_ID(pad)[UCS_SYS_CACHE_LINE_SIZE - \
                                      UCS_CACHELINE_PADDING_MISALIGN(__VA_ARGS__)];
#define UCS_CACHELINE_PADDING_SIZEOF(_, _x) \
    + sizeof(_x)
#define UCS_CACHELINE_PADDING_MISALIGN(...) \
    ((UCS_PP_FOREACH(UCS_CACHELINE_PADDING_SIZEOF, _, __VA_ARGS__)) % UCS_SYS_CACHE_LINE_SIZE)

#endif
