#include "Body.h"

#include <Helpers\Macro.h>

bool SmudgeTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr) || parseFailAddr)
		return false;

	auto pThis = This();
	const char* pSection = pThis->ID;

	INI_EX exINI(pINI);
	this->Clearable.Read(exINI, pSection, "Clearable");

	return true;
}

// =============================
// load / save

template <typename T>
void SmudgeTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Clearable)
		;

}

// =============================
// container
SmudgeTypeExtContainer SmudgeTypeExtContainer::Instance;
std::vector<SmudgeTypeExtData*> Container<SmudgeTypeExtData>::Array;
// =============================
// container hooks
//
ASMJIT_PATCH(0x6B52E1, SmudgeTypeClass_CTOR, 0x5)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x6B5391, SmudgeTypeClass_CTOR_NoInit, 0x7)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.AllocateNoInit(pItem);
	return 0;
}

ASMJIT_PATCH(0x6B61B5, SmudgeTypeClass_SDDTOR, 0x7)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

#include <Misc/Hooks.Otamaa.h>
#include <Ext/IsometricTileType/Body.h>

bool FakeSmudgeTypeClass::_CanPlaceHere(CellStruct* origin, bool underbuildings) {
	for (int h = 0; h < this->Height; h++)
	{
		for (int w = 0; w < this->Width; w++)
		{
			CellStruct trycell = *origin + CellStruct(w, h);
			CellClass* cell = MapClass::Instance->TryGetCellAt(trycell);
			if (!!MapClass::Instance->IsWithinUsableArea(trycell, true))
			{
				return false;
			}

			if (cell->SlopeIndex != 0)
			{
				return false;
			}

			if (cell->SmudgeTypeIndex != -1)
			{
				return false;
			}

			if (cell->OverlayTypeIndex != -1)
			{
				return false;
			}

			if (!underbuildings && cell->GetBuilding())
			{
				return false;
			}

			int ittype = cell->IsoTileTypeIndex;
			if (cell->IsoTileTypeIndex < 0 || cell->IsoTileTypeIndex >= IsometricTileTypeClass::Array->Count)
			{
				ittype = 0;
			}

			if (!IsometricTileTypeClass::Array->Items[ittype]->Morphable)
			{
				return false;
			}

			const auto isotype_ext = IsometricTileTypeExtContainer::Instance.Find(IsometricTileTypeClass::Array->Items[ittype]);

			if (!isotype_ext->AllowedSmudges.empty() && !isotype_ext->AllowedSmudges.Contains(this)) {
				return false;
			}
		}
	}

	return true;
}

ASMJIT_PATCH(0x6B57CD, SmudgeTypeClass_LoadFromINI, 0xA)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x208, -0x4));
	SmudgeTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x6B57DA);
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6B57DA, SmudgeTypeClass_LoadFromINI, 0xA)