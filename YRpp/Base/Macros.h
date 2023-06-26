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

// Define some C++ keywords when standard is less than C++11, mainly for watcom support
#if __cplusplus <= 199711L && (!defined _MSC_VER || _MSC_VER < 1600)
#define nullptr NULL
#define override
#define final
#define static_assert(x, ...)
#define constexpr
#define noexcept
#endif

// These allow evaluation of compiler specific attributes and intrinics on GCC like compilers.
// If they don't exist we want them to evaluate to false.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

// This section defines some keywords controlling inlining and unused variables
// where the keywords needed differ between compilers.
#define __noinline __declspec(noinline)
#define __unused __pragma(warning(suppress : 4100 4101))
#define __mayalias
#define __noreturn __declspec(noreturn)
#define __nothrow __declspec(nothrow)
#define __selectany __declspec(selectany)
#define __novtable __declspec(novtable)

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

// Statements like:
// #pragma message(Reminder "Fix this problem!")
// Which will cause messages like:
// C:\Source\Project\main.cpp(47): Reminder: Fix this problem!
// to show up during compiles. Note that you can NOT use the
// words "error" or "warning" in your reminders, since it will
// make the IDE think it should abort execution. You can double
// click on these messages and jump to the line in question.
#define MakeString(M, L) M(L)
#define $Line MakeString(STRINGIZE, __LINE__)
#define Reminder __FILE__ "(" $Line ") : Reminder: "

#define NO_CONSTRUCT_CLASS(classname)\
classname() = delete;\
classname(const classname&) = delete;\
classname(classname&&) = delete; \
classname& operator=(const classname& other) = delete; 
