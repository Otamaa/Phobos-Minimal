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
std::basic_string<T> lowercase(const std::basic_string<T>& s)
{
	std::basic_string<T> s2 = s;
	std::transform(s2.begin(), s2.end(), s2.begin(),
		[](const T v) { return static_cast<T>(std::tolower(v)); });
	return s2;
}

//
// Uppercases string
//
template <typename T>
std::basic_string<T> uppercase(const std::basic_string<T>& s)
{
	std::basic_string<T> s2 = s;
	std::transform(s2.begin(), s2.end(), s2.begin(),
		[](const T v) { return static_cast<T>(std::toupper(v)); });
	return s2;
}

std::string& trim(std::string& s)
{
	return KratosCRT::trim(s);
}

void split(std::string& s, std::string& delim, std::vector<std::string>* result)
{
	KratosCRT::split(s , delim , result);
}

std::string subreplace(std::string resource, std::string sub, std::string replace)
{
	return KratosCRT::subreplace(resource , sub , replace);
}

int Wchar2Char(const wchar_t* wcharStr, char* charStr)
{
	return KratosCRT::Wchar2Char(wcharStr , charStr);
}
std::string WString2String(std::wstring wstr)
{
	return KratosCRT::WString2String(wstr);
}

int Char2Wchar(const char* charStr, wchar_t* wcharStr)
{
	return KratosCRT::Char2Wchar(charStr , wcharStr);
}
std::wstring String2WString(std::string str)
{
	return KratosCRT::String2WString(str);
}

bool IsNotNone(std::string val)
{
	return KratosCRT::IsNotNone(val);
}

void ClearIfGetNone(std::vector<std::string>& value)
{
	return KratosCRT::ClearIfGetNone(value);
}

std::string GetUUID()
{
	return KratosCRT::GetUUID();
}
std::string GetUUIDShort()
{
	return KratosCRT::GetUUIDShort();
}
