#include "Anchor.h"

void Anchor::Read(INI_EX& parser, const char* pSection, const char* pFlagFormat)
{
	char flagName[0x40];

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Horizontal");
	detail::read(this->Horizontal, parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Vertical");
	detail::read(this->Vertical, parser, pSection, flagName);
}
