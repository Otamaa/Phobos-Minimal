#include "Body.h"

#include <Helpers\Macro.h>
#include <Phobos.SaveGame.h>

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

bool SmudgeTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(SmudgeTypeExtContainer::ClassName))
	{
		auto& container = root[SmudgeTypeExtContainer::ClassName];

		for (auto& entry : container[SmudgeTypeExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, SmudgeTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool SmudgeTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[SmudgeTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : SmudgeTypeExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[SmudgeTypeExtData::ClassName] = std::move(_extRoot);

	return true;
}

void SmudgeTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
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
			if (!MapClass::Instance->IsWithinUsableArea(trycell, true))
			{
				return false;
			}

			if (!cell)  // Added null check for cell pointer
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
