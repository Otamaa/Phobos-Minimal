#include "VersesData.h"

#include <Conversions.h>

bool VersesData::Parse(const char* str)
{
	auto const& [bValid, nVerses] = Conversions::Str2ArmorCheck(str, &this->Flags);

	if (bValid) {
		this->Verses = nVerses;
		return true;
	}

	return false;
}

void VersesData::Parse_NoCheck(const char* str)
{
	if (!str || !strlen(str))
		return;

	this->Verses = Conversions::Str2Armor(str, &this->Flags);
	this->LastParseIsValid = true;
}