#pragma once

// For counting variadic macro arguments.
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define VA_NARGS2(...) ((int)(sizeof((int[]){ __VA_ARGS__ })/sizeof(int)))

// The ubiquitous stringify macros for formatting strings.
#ifndef STRINGIZE
#define STRINGIZE_HELPER(str) #str
#define STRINGIZE(str) STRINGIZE_HELPER(str)
#define STRINGIZE_JOIN(str1, str2) STRINGIZE_HELPER(str1 ## str2)
#endif // STRINGIZE

// These allow evaluation of compiler specific attributes and intrinics on GCC like compilers.
// If they don't exist we want them to evaluate to false.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

/**
 *  Returns the count of items in a built-in C array. This is a common technique
 *  which is often used to help properly calculate the number of items in an
 *  array at runtime in order to prevent overruns, etc.
 *
 *  Example usage :
 *      int array[95];
 *      size_t arrayCount = ARRAY_SIZE(array);     // arrayCount is 95.
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif // !ARRAY_SIZE

/**
 *  Use to determine the size of a public struct member:
 *
 *  Example usage:
 *   typedef struct _ABC {
 *       sint32    A;
 *       sint32    B;
 *       sint16    C;
 *   } ABC, * PTR_ABC
 *
 *   SIZE_OF(struct _ABC, C)
 */
#ifndef SIZE_OF
#define SIZE_OF(typ, id) sizeof(((typ *)0)->id)
#endif // !SIZE_OF

/**
 *  Returns the absolute value of the number.
 */
#ifdef ABS
#undef ABS
#endif
#define ABS(a, b) (a < 0) ? -a : a;

/**
 *  Returns the minimum of the two numbers.
 */
#ifdef MIN
#undef MIN
#endif
#define MIN(a, b) (b < a) ? b : a;

/**
 *  Returns the maximum of the two numbers.
 */
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) (b > a) ? b : a;

#define NO_CONSTRUCT_CLASS(classname)\
classname() = delete;\
classname(const classname&) = delete;\
classname(classname&&) = delete; \
classname& operator=(const classname& other) = delete;
