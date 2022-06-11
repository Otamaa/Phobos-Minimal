#include "Template.h"

#include "INIParser.h"
#include "Debug.h"

bool ArmorType::Read(INI_EX &parser, const char *pSection, const char *pKey, bool Allocate)
{
	int buffer = this->Value;
	if (parser.ReadArmor(pSection, pKey, &buffer))
	{
		this->Value = buffer;
		return true;
	}
	else if (!parser.empty())
	{
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid ArmorType");
	}
	return false;
}
