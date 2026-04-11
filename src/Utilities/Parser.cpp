#include "Parser.h"

#include <cmath>

template<>
bool Parser<bool>::TryParse(const char* pValue, OutType* outValue)
{

	switch (toupper(static_cast<unsigned char>(*pValue)))
	{
	case '1':
	case 'T':
	case 'Y':
	{
		*outValue = true;
		return true;
	}
	case '0':
	case 'F':
	case 'N':
	{
		*outValue = false;
		return true;
	}
	}

	return false;
};

template<>
bool Parser<int>::TryParse(const char* pValue, OutType* outValue)
{
	const char* pFmt = nullptr;
	if (*pValue == '$')
	{
		pFmt = "$%x";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h')
	{
		pFmt = "%xh";
	}
	else
	{
		pFmt = "%d";
	}

	int buffer = 0;
	if (sscanf_s(pValue, pFmt, &buffer) == 1)
	{
		*outValue = buffer;
		return true;
	}
	return false;
}

template<>
bool Parser<double>::TryParse(const char* pValue, OutType* outValue)
{

	errno = 0;
	char* end = nullptr;

	// Nov 23, 2025 - Starkku: strtod() + cast result to float produces results
	// more similar to game's CRT functions than using sscanf_s.
	double value = strtod(pValue, &end);

	if (pValue == end || errno == ERANGE || !std::isfinite(value))
		return false;

	float floatValue = static_cast<float>(value);

	if (strchr(pValue, '%'))
	{
		floatValue *= 0.01f;
	}

	if (outValue)
	{
		*outValue = floatValue;
	}
	return true;
};

template<>
bool Parser<float>::TryParse(const char* pValue, OutType* outValue)
{
	double buffer = 0.0;
	if (Parser<double>::TryParse(pValue, &buffer))
	{
		if (outValue)
		{
			*outValue = static_cast<float>(buffer);
		}
		return true;
	}
	return false;
}

template<>
bool Parser<BYTE>::TryParse(const char* pValue, OutType* outValue)
{
	// no way to read unsigned char, use short instead.
	const char* pFmt = nullptr;
	if (*pValue == '$')
	{
		pFmt = "$%hx";
	}
	else if (tolower(static_cast<unsigned char>(pValue[strlen(pValue) - 1])) == 'h')
	{
		pFmt = "%hxh";
	}
	else
	{
		pFmt = "%hu";
	}

	WORD buffer;
	if (sscanf_s(pValue, pFmt, &buffer) == 1)
	{
		if (buffer <= UCHAR_MAX)
		{
			*outValue = static_cast<BYTE>(buffer);
			return true;
		}
	}
	return false;
};

template<>
bool Parser<short>::TryParse(const char* pValue, OutType* outValue)
{
	int buffer = 0;
	if (!Parser<int>::TryParse(pValue, &buffer))
		return false;

	if (buffer > std::numeric_limits<short>::max() || buffer < std::numeric_limits<short>::min())
		return false;

	*outValue = static_cast<short>(buffer);
	return true;
}