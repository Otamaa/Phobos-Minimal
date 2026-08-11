#pragma once

#include <Lib/utfcpp/utf8.h>
#include <filesystem>

inline std::string utf8_to_string(std::u8string& u8)
{
	return std::string(u8.begin(), u8.end());
}

inline std::string utf8_to_string(const std::u8string u8)
{
	return std::string(u8.begin(), u8.end());
}

inline std::string utf8_to_string(const char8_t* cu8)
{
	const std::u8string u8(cu8);
	return std::string(u8.begin(), u8.end());
}

inline std::filesystem::path utf8_to_path(std::u8string& u8)
{
	return std::filesystem::path(u8);
}

inline std::filesystem::path  utf8_to_path(const char8_t* cu8)
{
	return std::filesystem::path(std::u8string(cu8));
}