#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <Utilities/TemplateDefB.h>

#include <WeaponTypeClass.h>
#include <CCINIClass.h>
#include <AnimTypeClass.h>

template<typename T, bool Allocate = false, bool Unique = false>
static NOINLINE void ParseList(DynamicVectorClass<T>& List, CCINIClass* pINI, const char* section, const char* key) {
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

// == WeaponType ==
DEFINE_OVERRIDE_HOOK(0x772462, WeaponTypeClass_LoadFromINI_ListLength, 0x9)
{
	GET(WeaponTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, EDI);
	
	ParseList<AnimTypeClass*, true>(pThis->Anim, pINI, pSection, GameStrings::Anims());

	return 0x77255F;
}