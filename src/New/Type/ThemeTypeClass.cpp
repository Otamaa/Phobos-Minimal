#include "ThemeTypeClass.h"

#include <ThemeClass.h>

#include <Helpers/Macro.h>

Enumerable<ThemeTypeClass>::container_t Enumerable<ThemeTypeClass>::Array;

const char* Enumerable<ThemeTypeClass>::GetMainSection()
{
	return "Themes";
}

void ThemeTypeClass::LoadFromINI(CCINIClass* pINI)
{
	this->NextText.Read(pINI, this->Name.data(), "Next");
	this->HousesText.Read(pINI, this->Name.data(), "RequiredHouses");
	this->Side = pINI->ReadSide(this->Name.data(), "Side", -1);

	INI_EX exINI(pINI);
	this->Normal.Read(exINI, this->Name.data(), "Normal");
	this->Repeat.Read(exINI, this->Name.data(), "Repeat");
	this->UIName.Read(exINI, this->Name.data(), "Name");
}

/* these hooks detaching the theme from the game timer
ASMJIT_PATCH(0x406FC6, sub_406F70_ThemeClass_AI_RemoveFromGameTimer, 0x5)
{
	return 0x406FD0;
}

ASMJIT_PATCH(0x406FE2, sub_406F70_ThemeClass_AI_AfterTimer, 0xA)
{
	ThemeClass::Instance->AI();
	return 0;
}
*/

ASMJIT_PATCH(0x7206FB, ThemeClass_AllocateType, 0x8)
{
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x38, 0x4));
	GET(char*, pSection, EBP);

	ThemeTypeClass::FindOrAllocate(pSection)->LoadFromINI(pINI);

	return 0;
}

ASMJIT_PATCH(0x721171, ThemeClass_IsAvailable_Handle, 0x6)
{
	GET_STACK(int, index, 0x4);
	enum { ReturnFalse = 0x72117B, ReturnTrue = 0x7211CE };

	auto const pThemeType = ThemeTypeClass::TryFindFromIndex(index);

	if (!pThemeType || !pThemeType->Normal.Get())
		return ReturnFalse;

	auto const pPlayer = HouseClass::CurrentPlayer();
	if (!pPlayer)
		return ReturnFalse;

	if (pThemeType->Side.Get() >= 0 && pPlayer->Type->SideIndex != pThemeType->Side.Get())
		return ReturnFalse;

	if (strcmp(pThemeType->HousesText.data(), ""))
	{
		char* context = nullptr;
		strcpy(Phobos::readBuffer, pThemeType->HousesText.data());

		bool getHouse = false;
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			if (!strcmp(pPlayer->Type->get_ID(), cur))
			{
				getHouse = true;
				break;
			}
		}

		if (!getHouse)
			return ReturnFalse;
	}

	return 0;
}

ASMJIT_PATCH(0x7209B0, ThemeClass_GetUIName, 0x7)
{
	GET_STACK(int, index, 0x4);

	auto const pThemeType = ThemeTypeClass::TryFindFromIndex(index);

	if (!pThemeType) {
		R->EAX(L"\0");
		return 0x7209C6;
	}

	R->EAX(pThemeType->UIName.Get().Text);
	return 0x7209C6;
}

ASMJIT_PATCH(0x720A69, ThemeClass_AI_Play, 0x6)
{
	GET(ThemeClass*, pThis, ESI);

	int idx = pThis->QueuedTheme;

	if (pThis->LastTheme >= 0 &&
		((pThis->LastTheme == idx && pThis->CurrentTheme == idx) ||
			pThis->LastTheme != idx))
	{
		if (auto const pThemeExt = ThemeTypeClass::Array[pThis->LastTheme].get())
		{
			if (!pThemeExt->Repeat &&
				strcmp(pThemeExt->NextText.data(), ""))
			{
				int next = ThemeClass::Instance->FindIndex(pThemeExt->NextText.data());

				if (next >= 0 && next != pThis->LastTheme)
				{
					idx = next;
				}
			}
		}
	}

	pThis->Play(idx);
	return 0x720A74;
}
