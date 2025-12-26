#include "Body.h"
#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>

UnitTypeExtContainer UnitTypeExtContainer::Instance;

bool UnitTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(UnitTypeExtContainer::ClassName))
	{
		auto& container = root[UnitTypeExtContainer::ClassName];

		for (auto& entry : container[UnitTypeExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, UnitTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool UnitTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[UnitTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : UnitTypeExtContainer::Array)
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

	first_layer[UnitTypeExtData::ClassName] = std::move(_extRoot);

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