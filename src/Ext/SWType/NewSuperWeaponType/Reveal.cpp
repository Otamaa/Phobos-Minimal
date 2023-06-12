#include "Reveal.h"

std::vector<const char*> SW_Reveal::GetTypeString() const
{
	return { "Reveal" };
}

bool SW_Reveal::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicReveal);
}

bool SW_Reveal::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	return true;
}

void SW_Reveal::Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) { }

void SW_Reveal::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("Reveal[%s] Init !\n", pData->Get()->ID);
}

void SW_Reveal::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

int SW_Reveal::GetSound(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->PsychicRevealActivateSound;
}

SWRange SW_Reveal::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Range->empty())
	{
		// real default values, that is, force max cellspread range of 10
		auto const radius = std::min(RulesClass::Instance->PsychicRevealRadius, 10);
		return SWRange(radius);
	}
	return pData->SW_Range;
}