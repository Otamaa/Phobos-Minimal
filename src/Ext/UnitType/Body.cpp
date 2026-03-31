#include "Body.h"
#include <Utilities/Macro.h>

#include <TerrainTypeClass.h>

UnitTypeExtContainer UnitTypeExtContainer::Instance;

bool UnitTypeExtData::LoadFromINI(CCINIClass * pINI, bool parseFailAddr)
{
	if (!this->FootTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	auto pThis = this->This();
	const char* pSection = pThis->ID;

	INI_EX exINI(pINI);
	this->DefaultMirageDisguises.Read(exINI, pSection, "DefaultMirageDisguises");

	return true;
}

void UnitTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void UnitTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}

ASMJIT_PATCH(0x7472B1, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x747316, UnitTypeClass_DTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeUnitTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	UnitTypeExtContainer::Instance.Find(this)->Initialize();
	bool status = this->UnitTypeClass::LoadFromINI(pINI);
	UnitTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F627C, FakeUnitTypeClass::_ReadFromINI)