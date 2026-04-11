#include "TranslucencyLevel.h"

#include <Utilities/INIParser.h>
#include <Utilities/SavegameDef.h>

bool TranslucencyLevel::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	int buf;
	if (parser.ReadInteger(pSection, pKey, &buf))
	{
		*this = buf;
		return true;
	}

	return false;
}

bool TranslucencyLevel::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->value, RegisterForChange);
}

bool TranslucencyLevel::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->value);
}