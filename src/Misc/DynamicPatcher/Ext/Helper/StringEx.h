#pragma once

#include <Windows.h>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>


//
// Lowercases string
//
template <typename T>
inline std::basic_string<T> lowercase(const std::basic_string<T>& s)
{
	return KratosCRT::lowercase(s);
}

//
// Uppercases string
//
template <typename T>
inline std::basic_string<T> uppercase(const std::basic_string<T>& s)
{
	return KratosCRT::uppercase(s);
}

inline std::string& trim(std::string& s)
{
	return KratosCRT::trim(s);
}

inline void split(std::string& s, std::string& delim, std::vector<std::string>* result)
{
	KratosCRT::split(s , delim , result);
}

inline std::string subreplace(std::string resource, std::string sub, std::string replace)
{
	return KratosCRT::subreplace(resource , sub , replace);
}

inline int Wchar2Char(const wchar_t* wcharStr, char* charStr)
{
	return KratosCRT::Wchar2Char(wcharStr , charStr);
}
inline std::string WString2String(std::wstring wstr)
{
	return KratosCRT::WString2String(wstr);
}

inline int Char2Wchar(const char* charStr, wchar_t* wcharStr)
{
	return KratosCRT::Char2Wchar(charStr , wcharStr);
}
inline std::wstring String2WString(std::string str)
{
	return KratosCRT::String2WString(str);
}

inline bool IsNotNone(std::string val)
{
	return KratosCRT::IsNotNone(val);
}

inline void ClearIfGetNone(std::vector<std::string>& value)
{
	return KratosCRT::ClearIfGetNone(value);
}

inline std::string GetUUID()
{
	return KratosCRT::GetUUID();
}
inline std::string GetUUIDShort()
{
	return KratosCRT::GetUUIDShort();
}
