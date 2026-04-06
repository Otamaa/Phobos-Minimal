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

void SmudgeTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
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

void SmudgeTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
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
//
ASMJIT_PATCH(0x6B52E1, SmudgeTypeClass_CTOR, 0x5)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.Allocate(pItem);
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

bool FakeSmudgeTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->SmudgeTypeClass::LoadFromINI(pINI);
	SmudgeTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F358C, FakeSmudgeTypeClass::_ReadFromINI)
