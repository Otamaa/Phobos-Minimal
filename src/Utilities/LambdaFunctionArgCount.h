#pragma once

#include <type_traits>

// https://stackoverflow.com/questions/78638714/count-the-number-of-arguments-in-generic-lambda

struct any_argument
{
	template <typename T>
	operator T && () const;
};

template <typename Lambda, typename Is, typename = void>
struct can_accept_impl
	: std::false_type
{
};

template <typename Lambda, std::size_t ...Is>
struct can_accept_impl < Lambda, std::index_sequence<Is...>,
	decltype(std::declval<Lambda>()(((void)Is, any_argument {})...), void()) >
	: std::true_type
{
};

template <typename Lambda, std::size_t N>
struct can_accept
	: can_accept_impl<Lambda, std::make_index_sequence<N>>
{
};


template <typename Lambda, std::size_t Max, std::size_t N, typename = void>
struct lambda_details_impl
	: lambda_details_impl<Lambda, Max, N - 1>
{
};

template <typename Lambda, std::size_t Max, std::size_t N>
struct lambda_details_impl<Lambda, Max, N, std::enable_if_t<can_accept<Lambda, N>::value>>
{
	static COMPILETIMEEVAL bool is_variadic = (N == Max);
	static COMPILETIMEEVAL std::size_t argument_count = N;
};

template <typename Lambda, std::size_t Max = 50>
struct lambda_details
	: lambda_details_impl<Lambda, Max, Max>
{
};