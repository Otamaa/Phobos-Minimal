#include "PlaceVeinholeMonster.h"

#include <Utilities/GeneralUtils.h>

#include <WWMouseClass.h>
#include <VeinholeMonsterClass.h>

const char* PlaceVeinholeMonster::GetName() const
{
	return "Place Veinhole Monster";
}

const wchar_t* PlaceVeinholeMonster::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_PLACEVEIN", L"Place Veinhole Monster");
}

const wchar_t* PlaceVeinholeMonster::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* PlaceVeinholeMonster::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_PLACEVEIN_DESC", L"Place Veinhole Monster.");
}

bool created = false;

void PlaceVeinholeMonster::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	if (!created)
	{
		CellStruct nPos = WWMouseClass::Instance->GetCellUnderCursor();

		if (VeinholeMonsterClass::IsCellEligibleForVeinHole(nPos))
		{
			auto pCell = MapClass::Instance->TryGetCellAt(nPos);

			if (!pCell)
				return;

			//this dummy overlay placed so it can be replace later with real veins
			for (int i = 0; i < 8; ++i)
			{
				auto v11 = pCell->GetAdjacentCell(i);
				v11->OverlayTypeIndex = 0x7E; //dummy image -> replaced with vein ? 
				;
				v11->OverlayData = 30u; //max it out
				v11->RedrawForVeins();
			}

			pCell->OverlayTypeIndex = 0xA7; //VeiholeDummy -> used to place veinhole monster
			pCell->OverlayData = 0;
			++Unsorted::ScenarioInit();
			auto pVein = GameCreate<VeinholeMonsterClass>(&nPos);
			--Unsorted::ScenarioInit();
			created = true;

			pVein->RegisterAffectedCells();
		}
	}
}