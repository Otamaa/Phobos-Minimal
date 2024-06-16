#include "Anchor.h"

void Anchor::Read(INI_EX& parser, const char* pSection, const char* pFlagFormat)
{
	char flagName[0x40];

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Horizontal");
	this->Horizontal.Read(parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Vertical");
	this->Vertical.Read(parser, pSection, flagName);
}