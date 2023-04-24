#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <Utilities/TemplateDefB.h>

#include <WeaponTypeClass.h>
#include <CCINIClass.h>
#include <AnimTypeClass.h>

template<typename T, bool Allocate = false, bool Unique = false>
static void ParseList(DynamicVectorClass<T>& List, CCINIClass* pINI, const char* section, const char* key) {
	if (pINI->ReadString(section, key, Phobos::readDefval, Phobos::readBuffer)) {
		List.Clear();
		char* context = nullptr;

		if constexpr (std::is_pointer<T>())
		{
			using BaseType = std::remove_pointer_t<T>;

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur;
				 cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				BaseType* buffer = nullptr;
				if constexpr (Allocate)
				{
					buffer = BaseType::FindOrAllocate(cur);
				} else {
					buffer = BaseType::Find(cur);
				}

				if (buffer) {
					if constexpr (!Unique) {
						List.AddItem(buffer);
					} else {
						List.AddUnique(buffer);
					}
				}
				else if (!INIClass::IsBlank(cur) && BaseType::Array->IsAllocated && BaseType::Array->Count > 0)
				{
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
		else
		{
			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur;
				 cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{

				T buffer = T();
				if (Parser<T>::TryParse(cur, &buffer)) {
					List.AddItem(buffer);
				} else if (!INIClass::IsBlank(cur)) {
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
	}
};

DEFINE_OVERRIDE_HOOK(0x7274AF, TriggerTypeClass_LoadFromINI_Read_Events, 5)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7274C8, TriggerTypeClass_LoadFromINI_Strtok_Events, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x727529, TriggerTypeClass_LoadFromINI_Read_Actions, 5)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x727544, TriggerTypeClass_LoadFromINI_Strtok_Actions, 5)
{
	R->EDX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4750EC, INIClass_ReadHouseTypesList, 7)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x475107, INIClass_ReadHouseTypesList_Strtok, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x47527C, INIClass_GetAlliesBitfield, 7)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x475297, INIClass_GetAlliesBitfield_Strtok, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

// == WeaponType ==
DEFINE_OVERRIDE_HOOK(0x772462, WeaponTypeClass_LoadFromINI_ListLength, 0x9)
{
	GET(WeaponTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, EDI);
	
	ParseList<AnimTypeClass*, true>(pThis->Anim, pINI, pSection, GameStrings::Anim());

	return 0x77255F;
}