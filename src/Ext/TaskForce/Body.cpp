#include "Body.h"
#include <CCINIClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#pragma optimize("", off )
bool __fastcall FakTaskForceClass::_LoadEntryINI(CCINIClass* pINI, discard_t, TaskForceType type)
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

	const char* const section = "TaskForces";

	const int count = pINI->GetKeyCount(section);

	if (count <= 0)
		return false;

	for (int index = 0; index < count; ++index) {

		const char* entryKey = pINI->GetKeyName(section, index);
		std::string value(24, '\0');
		const int len = pINI->ReadString(section, entryKey, "", value.data(), 24);

		if (len <= 0)
			continue;

		value.resize(static_cast<size_t>(len));

		TaskForceClass* pTag = TaskForceClass::FindOrAllocate(value.c_str());
		size_t intID = String_To_ID(value.c_str());
		Debug::Log("TaskForce[%s - %x] want to remap as [%x] \n", value.c_str(), pTag, intID);
		//PhobosSwizzle::Instance.Here_I_Am((void*)ID, pType);

		if (pTag) {
			pTag->LoadFromINI(pINI);
			pTag->Type = type;
		}
	}

	return true;
}
#pragma optimize("", on )
DEFINE_FUNCTION_JUMP(LJMP, 0x6E8220, FakTaskForceClass::_LoadEntryINI);
DEFINE_FUNCTION_JUMP(CALL, 0x6879B4, FakTaskForceClass::_LoadEntryINI);
DEFINE_FUNCTION_JUMP(CALL, 0x6879BD, FakTaskForceClass::_LoadEntryINI);

bool FakTaskForceClass::_LoadFromINI(CCINIClass* pINI)
{
	pINI->Reset();

	if (!this->AbstractTypeClass::LoadFromINI(pINI))
		return false;

	this->CountEntries = 0;
	for (int i = 0; i < 6; ++i) {
		char buffer[128];

		if (pINI->ReadString(this->ID, std::to_string(i).c_str(), "", buffer) > 0) {
			this->Entries[this->CountEntries].Read(buffer);
			int v4 = this->CountEntries;
			if (this->Entries[v4].Type) {
				this->CountEntries = v4 + 1;
			}
		}
	}

	this->Group = pINI->ReadInteger(this->ID, "Group", this->Group);
	return true;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F46E4, FakTaskForceClass::_LoadFromINI);
DEFINE_FUNCTION_JUMP(LJMP , 0x6E8420, FakTaskForceClass::_LoadFromINI);

bool FakTaskForceClass::_WriteToINI(CCINIClass* pINI)
{
	if (!this->AbstractTypeClass::SaveToINI(pINI)) {
		return 0;
	}

	for (int i = 0; i < this->CountEntries; ++i) {
		pINI->WriteString(this->ID, std::to_string(i).c_str(), this->Entries[i].Write());
	}

	pINI->Reset();
	pINI->WriteInteger(this->ID, "Group", this->Group, false);

	return true;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6E8510, FakTaskForceClass::_WriteToINI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F46E8, FakTaskForceClass::_WriteToINI);