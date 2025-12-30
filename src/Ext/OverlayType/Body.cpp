#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <Helpers\Macro.h>

#include <Phobos.SaveGame.h>

bool OverlayTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr) || parseFailAddr)
		return false;

	auto pThis = this->This();
	INI_EX exINI(pINI);

	//auto const pArtINI = &CCINIClass::INI_Art();
	auto pArtSection = pThis->ImageFile;

	this->Palette.Read(exINI , pArtSection, "Palette");
	this->ZAdjust.Read(exINI, pArtSection, "ZAdjust");

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
		;
}


// =============================
// container
OverlayTypeExtContainer OverlayTypeExtContainer::Instance;

bool OverlayTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(OverlayTypeExtContainer::ClassName))
	{
		auto& container = root[OverlayTypeExtContainer::ClassName];

		for (auto& entry : container[OverlayTypeExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, OverlayTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool OverlayTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[OverlayTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : OverlayTypeExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer); // write all data to stream

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[OverlayTypeExtData::ClassName] = std::move(_extRoot);
	return true;
}

void OverlayTypeExtContainer::LoadFromINI(OverlayTypeClass* key, CCINIClass* pINI, bool parseFailAddr)
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