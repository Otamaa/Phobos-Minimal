#pragma once

#include <type_traits>
#include <Base/Always.h>

template <typename T> requires std::is_enum_v<T>
struct EnumFlagHelper
{
	using type = std::underlying_type_t<T>;

	COMPILETIMEEVAL EnumFlagHelper(T value)
	{
		this->value = static_cast<type>(value);
	}

	COMPILETIMEEVAL explicit operator bool() const
	{
		return value != type();
	}

	COMPILETIMEEVAL operator T() const
	{
		return static_cast<T>(this->value);
	}

	COMPILETIMEEVAL explicit operator type() const
	{
		return value;
	}

private:

	type value;
};

#define MAKE_ENUM_FLAGS(FLAG_ENUM_NAME) \
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator& (EnumFlagHelper<FLAG_ENUM_NAME> lhs, EnumFlagHelper<FLAG_ENUM_NAME> rhs) { \
		using type = std::underlying_type_t<FLAG_ENUM_NAME>; \
		return static_cast<FLAG_ENUM_NAME>(static_cast<type>(lhs) & static_cast<type>(rhs)); \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator& (EnumFlagHelper<FLAG_ENUM_NAME> lhs, FLAG_ENUM_NAME rhs) { \
		return lhs & EnumFlagHelper<FLAG_ENUM_NAME>(rhs); \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator& (FLAG_ENUM_NAME lhs, EnumFlagHelper<FLAG_ENUM_NAME> rhs) { \
		return EnumFlagHelper<FLAG_ENUM_NAME>(lhs) & rhs; \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator& (FLAG_ENUM_NAME lhs, FLAG_ENUM_NAME rhs) { \
		return EnumFlagHelper<FLAG_ENUM_NAME>(lhs) & rhs; \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL FLAG_ENUM_NAME& operator&= (FLAG_ENUM_NAME &lhs, FLAG_ENUM_NAME rhs) { \
		return lhs = EnumFlagHelper<FLAG_ENUM_NAME>(lhs) & rhs; \
	} \
	\
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator| (EnumFlagHelper<FLAG_ENUM_NAME> lhs, EnumFlagHelper<FLAG_ENUM_NAME> rhs) { \
		using type = std::underlying_type_t<FLAG_ENUM_NAME>; \
		return static_cast<FLAG_ENUM_NAME>(static_cast<type>(lhs) | static_cast<type>(rhs)); \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator| (EnumFlagHelper<FLAG_ENUM_NAME> lhs, FLAG_ENUM_NAME rhs) { \
		return lhs | EnumFlagHelper<FLAG_ENUM_NAME>(rhs); \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator| (FLAG_ENUM_NAME lhs, EnumFlagHelper<FLAG_ENUM_NAME> rhs) { \
		return EnumFlagHelper<FLAG_ENUM_NAME>(lhs) | rhs; \
	} \
	 \
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator| (FLAG_ENUM_NAME lhs, FLAG_ENUM_NAME rhs) { \
		return EnumFlagHelper<FLAG_ENUM_NAME>(lhs) | rhs; \
	} \
	\
	OPTIONALINLINE COMPILETIMEEVAL FLAG_ENUM_NAME& operator|= (FLAG_ENUM_NAME &lhs, FLAG_ENUM_NAME rhs) { \
		return lhs = EnumFlagHelper<FLAG_ENUM_NAME>(lhs) | rhs; \
	} \
	\
	\
	OPTIONALINLINE COMPILETIMEEVAL EnumFlagHelper<FLAG_ENUM_NAME> operator~ (FLAG_ENUM_NAME rhs) { \
		using type = std::underlying_type_t<FLAG_ENUM_NAME>; \
		return static_cast<FLAG_ENUM_NAME>(~static_cast<type>(rhs)); \
	} \
