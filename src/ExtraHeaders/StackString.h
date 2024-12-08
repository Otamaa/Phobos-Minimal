#pragma once

#include "StackContainer.h"

// StackString -----------------------------------------------------------------

template<size_t stack_capacity>
class StackString : public StackContainer<
	std::basic_string<char,
	std::char_traits<char>,
	StackAllocator<char, stack_capacity> >,
	stack_capacity>
{
public:
	constexpr StackString() : StackContainer<
		std::basic_string<char,
		std::char_traits<char>,
		StackAllocator<char, stack_capacity> >,
		stack_capacity>()
	{ }

private:
	DISALLOW_COPY_AND_ASSIGN(StackString);
};

// StackStrin16 ----------------------------------------------------------------

template<size_t stack_capacity>
class StackString16 : public StackContainer<
	std::basic_string<wchar_t,
	std::char_traits<wchar_t>,
	StackAllocator<wchar_t, stack_capacity> >,
	stack_capacity>
{
public:
	constexpr StackString16() : StackContainer<
		std::basic_string<wchar_t,
		std::char_traits<wchar_t>,
		StackAllocator<wchar_t, stack_capacity> >,
		stack_capacity>()
	{ }

private:
	DISALLOW_COPY_AND_ASSIGN(StackString16);
};
