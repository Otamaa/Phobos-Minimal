#include "Anchor.h"
#include "TemplateDef.h"

void Anchor::Read(INI_EX& parser, const char* pSection, const char* pFlagFormat)
{
	char flagName[0x40];

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Horizontal");
	detail::read(this->Horizontal, parser, pSection, flagName);

	IMPL_SNPRNINTF(flagName, sizeof(flagName), pFlagFormat, "Vertical");
	detail::read(this->Vertical, parser, pSection, flagName);
}

template <typename T>
inline bool Serialize(Anchor* pThis , T& stm)
{
	return stm
		.Process(pThis->Horizontal)
		.Process(pThis->Vertical)
		.Success()
		//&& stm.RegisterChange(this)
		; // announce this type
}	

bool Anchor::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(this ,Stm);
}

bool Anchor::Save(PhobosStreamWriter& Stm) const
{
	return Serialize(const_cast<Anchor*>(this) , Stm);
}