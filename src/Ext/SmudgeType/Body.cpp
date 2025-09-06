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

bool SmudgeTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new SmudgeTypeExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "SmudgeTypeExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return true;
}

bool SmudgeTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
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

HRESULT __stdcall FakeSmudgeTypeClass::_Load(IStream* pStm)
{
	auto hr = this->SmudgeTypeClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = SmudgeTypeExtContainer::Instance.ReadDataFromTheByteStream(this,
			SmudgeTypeExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeSmudgeTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->SmudgeTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = SmudgeTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F353C, FakeSmudgeTypeClass::_Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F3540, FakeSmudgeTypeClass::_Save)