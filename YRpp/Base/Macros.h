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

#ifndef _useSTD
#define MinImpl(a,b) (((a) < (b)) ? (a) : (b))
#define MaxImpl(a,b) (((a) > (b)) ? (a) : (b))
#else
#define MinImp std::min
#define MaxImpl std::max
#endif

#define LessOrEqualTo(a, b) (a <= b) ? (a) : (b);
#define MoreOrEqualTo(a, b) (a >= b) ? (a) : (b);

#define PRAGMA(X)					 __pragma(#X)
#define PRAGMA_DISABLEWARNING()      PRAGMA(warning(push))
#define PRAGMA_DISABLEWARNING_S(x)   PRAGMA(warning(disable : x))
#define PRAGMA_DISABLEWARNING_POP()	 PRAGMA(warning(pop))

#define R1 {return 1;}
#define R0 {return 0;}
#define RX {}
#define RT(type) {return type();}

#define NOVTABLE __declspec(novtable)
#define NOINLINE [[msvc::noinline]]
#define UNUSED  [[maybe_unused]]
#define NORETURN [[noreturn]]
#define NOTHROW noexcept
#define SELECTANY __declspec(selectany)
#define NAKED __declspec(naked)
#define NAKEDNOINLINE [[msvc::noinline]] __declspec(naked)

#ifdef _DEBUG
#define FORCEDINLINE inline
#else
#define FORCEDINLINE __forceinline
#endif

#define OPTIONALINLINE inline
#define COMPILETIMEEVAL constexpr

#define ALIGN(val) __declspec(align(val))
#define ALIGNOF(type) (sizeof(type) - sizeof(type) + __alignof(type))
#define ALIGNAS(byte_alignment) __declspec(align(byte_alignment))

#define SAFE_RELEASE(ptr) {if(ptr) delete[] ptr;}

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define DEFINE_CLSID(_addrs) __declspec(uuid(_addrs))
#define CLASS_NAME(type) #type

#define COMPILE_TIME_SIZEOF(t) \
template<int s> struct SIZEOF_ ## t ## _IS; \
struct foo { int a,b; }; \
SIZEOF_ ## t ## _IS<sizeof(t)> SIZEOF_ ## t ## _IS;

 // A macro to disallow the copy constructor and operator= functions
 // This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/*
 *	This set of macros is necessary because a type argument can contain commas, which would split it into multiple arguments.
 *	To circumvent this, we can enclose the argument in parentheses.
 *	However, parentheses are significant, so we then need a macro to remove them so as
 *	to not affect our final result.
 *	Source: https://groups.google.com/a/isocpp.org/g/std-proposals/c/Ngl_vTAdddA
 */
#define UNPAREN( ... ) UNPAREN __VA_ARGS__
#define DONE_UNPAREN

#define CAT_LIT(A, ...) A ## __VA_ARGS__
#define CAT(A, ...) CAT_LIT(A, __VA_ARGS__)
#define UIP(...) CAT( DONE_, UNPAREN __VA_ARGS__  )

#define BYTE0(x) ((unsigned char)((x) >>  0))
#define BYTE1(x) ((unsigned char)((x) >>  8))
#define BYTE2(x) ((unsigned char)((x) >> 16))
#define BYTE3(x) ((unsigned char)((x) >> 24))

// ── Move-only: for value types with unique_ptr/CustomPalette members ──
// Use on: CustomPalette, HugeBar, BannerTypeClass, GameConfig, etc.
#define MOVEABLE_ONLY(ClassName)                              \
    ClassName(ClassName&&) noexcept = default;                 \
    ClassName& operator=(ClassName&&) noexcept = default;     \
    ClassName(const ClassName&) = delete;                      \
    ClassName& operator=(const ClassName&) = delete

// ── Non-transferable: for pool-managed / identity-bound objects ──
// Use on: all ExtData classes, ObjectPool, etc.
#define NON_TRANSFERABLE(ClassName)                            \
    ClassName(ClassName&&) = delete;                           \
    ClassName& operator=(ClassName&&) = delete;                \
    ClassName(const ClassName&) = delete;                      \
    ClassName& operator=(const ClassName&) = delete

// ── Non-copyable (but moveable with no noexcept promise) ──
// Use when move is needed but noexcept can't be guaranteed
#define NON_COPYABLE(ClassName)                               \
    ClassName(ClassName&&) = default;                          \
    ClassName& operator=(ClassName&&) = default;               \
    ClassName(const ClassName&) = delete;                      \
    ClassName& operator=(const ClassName&) = delete