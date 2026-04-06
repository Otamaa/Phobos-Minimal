#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <Helpers\Macro.h>

bool OverlayTypeExtData::CanBeBuiltOn(int overlayTypeIndex, BuildingTypeClass* pBuildingType, bool requireToBeRemovable)
{
	auto const pOverlayType = OverlayTypeClass::Array->Items[overlayTypeIndex];
	auto const pTypeExt = OverlayTypeExtContainer::Instance.Find(pOverlayType);

	if (!pTypeExt->IsCanBeBuiltOn)
		return false;

	if (((pBuildingType && pBuildingType->Wall) || pOverlayType->Wall) && !pTypeExt->CanBeBuiltOn_Remove)
		return false;

	return requireToBeRemovable ? pTypeExt->CanBeBuiltOn_Remove : true;
}

void OverlayTypeExtData::RemoveOverlayFromCell(int overlayTypeIndex, CellClass* pCell, HouseClass* pSource)
{
	if (overlayTypeIndex != -1 && OverlayTypeClass::Array->Items[overlayTypeIndex]->Wall)
	{
		if (pSource && pCell->WallOwnerIndex == pSource->ArrayIndex)
			pSource->SellWall(pCell->MapCoords, true);
		else
			pCell->ReduceWall(-1);
	}
	else
	{
		pCell->OverlayTypeIndex = -1;
		pCell->OverlayData = 0;
		pCell->RecalcAttributes(-1);
	}
}

bool OverlayTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr) || parseFailAddr)
		return false;

	auto pThis = this->This();
	INI_EX exINI(pINI);

	//auto const pArtINI = &CCINIClass::INI_Art();
	const char* pArtSection = pThis->ImageFile;
	const char* pSection = pThis->ID;

	this->Palette.Read(exINI , pArtSection, "Palette");
	this->ZAdjust.Read(exINI, pArtSection, "ZAdjust");
	this->IsCanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");
	this->CanBeBuiltOn_Remove.Read(exINI, pSection, "CanBeBuiltOn.Remove");

	return true;
}

// =============================
// load / save

template <typename T>
void OverlayTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->ZAdjust)
		.Process(this->IsCanBeBuiltOn)
		.Process(this->CanBeBuiltOn_Remove)
		;
}


// =============================
// container
OverlayTypeExtContainer OverlayTypeExtContainer::Instance;

void OverlayTypeExtContainer::LoadFromINI(OverlayTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}


		// Rules first 
		// Other files 
		// when this doesnt match the case it will causing weirdd issues like some value wont be initialized or replaced to default value after parsing
		switch (ptr->Initialized)
		{
		case InitState::Blank:
		{
			if (pINI == CCINIClass::INI_Rules())
			{
				ptr->SetInitState(InitState::Inited);
				//ptr->Initialize();
			}
			[[fallthrough]];
		}
		case InitState::Inited:
		case InitState::Ruled:
		{
			ptr->LoadFromINI(pINI, parseFailAddr);
			ptr->SetInitState(InitState::Ruled);
			[[fallthrough]];
		}
		default:
			break;
		}
	}

}

void OverlayTypeExtContainer::WriteToINI(OverlayTypeClass* key, CCINIClass* pINI)
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
// =============================
// container hooks

ASMJIT_PATCH(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	OverlayTypeExtContainer::Instance.Allocate(pItem);

	return 0;
} ASMJIT_PATCH_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)

ASMJIT_PATCH(0x5FE3F6, OverlayTypeClass_DTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);
	OverlayTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

bool FakeOverlayTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->OverlayTypeClass::LoadFromINI(pINI);
	OverlayTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF664, FakeOverlayTypeClass::_ReadFromINI)