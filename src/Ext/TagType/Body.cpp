#include "Body.h"
#include <CCINIClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <TriggerTypeClass.h>

bool __fastcall FakeTagTypeClass::_LoadEntryINI(CCINIClass* pINI)
{
	auto String_To_ID = [](const char* str)
		{
			int result = 0;

			if (!str)
			{
				return result;
			}

			while (*str)
			{
				const char ch = *str;

				if (!std::isxdigit(static_cast<unsigned char>(ch)))
				{
					break;
				}

				result *= 16;

				if (ch >= '0' && ch <= '9')
				{
					result += ch - '0';
				}
				else
				{
					result += std::toupper(static_cast<unsigned char>(ch)) - 'A' + 10;
				}

				++str;
			}

			return result;
		};

	const char* const section = "Tags";

	const int count = pINI->GetKeyCount(section);

	if (count <= 0)
		return false;

	for (int index = 0; index < count; ++index)
	{
		// Get the entry key (the tag ID string, e.g. "01EA0000")
		const char* entryKey = pINI->GetKeyName(section, index);

		// Read the value string — vanilla uses a 24-byte fixed buffer.
		// VERIFY: 24 is the max length enforced by vanilla. Harmless to use
		// std::string here since Get_String copies into our buffer anyway.
		std::string value(24, '\0');
		const int len = pINI->ReadString(section, entryKey, "", value.data(), 24);

		if (len <= 0)
			continue;

		value.resize(static_cast<size_t>(len));//arent used just placeholder

		// Assembly 0x6E5F27-0x6E5F49:
		// <none> and none are sentinel values meaning "no tag".
		// Still register null with SwizzleManager below — intentional.
		TagTypeClass* pTag = TagTypeClass::FindOrAllocate(entryKey);
		// If <none>/none or alloc failed: pTag remains nullptr.
		// SwizzleManager still gets registered with nullptr — matches assembly.

		// Assembly 0x6E5FA3-0x6E5FB1:
		// Register this pointer against the hashed entry key.
		size_t ID = String_To_ID(entryKey);
		Debug::Log("TagType[%s - %x] want to remap as [%x] \n", entryKey, pTag , ID);
		//PhobosSwizzle::Instance.Here_I_Am((void*)ID, pType);

		if (pTag) {
			pTag->LoadFromINI(pINI);
		}
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6E5ED0, FakeTagTypeClass::_LoadEntryINI);
DEFINE_FUNCTION_JUMP(CALL, 0x6879CB, FakeTagTypeClass::_LoadEntryINI);

bool FakeTagTypeClass::_LoadFromINI(CCINIClass* pINI)
{
	pINI->Reset();

	char buffer[128];

	if (!pINI->ReadString( "Tags", this->ID, "", buffer, sizeof(buffer))) {
		return false;
	}

	this->Persistence = (TriggerPersistence)std::atoi(std::strtok(buffer, ","));

	if (char* token = std::strtok(nullptr, ",")) {
		std::strncpy(this->Name, token, 48);
		this->Name[48] = '\0';
	} else {
		this->Name[0] = '\0';
	}

	this->FirstTrigger = TriggerTypeClass::FindOrAllocate(std::strtok(nullptr, ","));

	return true;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6E6080, FakeTagTypeClass::_LoadFromINI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4628, FakeTagTypeClass::_LoadFromINI);

bool FakeTagTypeClass::_WriteToINI(CCINIClass* pINI)
{
	std::string _result;

	if (auto pTrig = this->FirstTrigger) {
		_result = fmt::format("{},{},{}", (int)this->Persistence, this->Name, pTrig->ID);
	} else {
		_result = fmt::format("{},<none>", this->Name);
	}

	pINI->WriteString("Tags", this->ID, _result.c_str());

	return true;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6E6160, FakeTagTypeClass::_WriteToINI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F462C, FakeTagTypeClass::_WriteToINI);