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
		//rename DUMMYOLD -> VEINS from Ts to make veins working
		/* idx 126
		* [VEINS]
			Image=VEINS
			Name=Tiberium Veins
			RadarColor=0,0,92
			IsVeins=true
			Land=Weeds
		*
		*
		*/
		CellStruct nPos = WWMouseClass::Instance->GetCellUnderCursor();

		++Unsorted::ScenarioInit();
		if (VeinholeMonsterClass::IsCellEligibleForVeinHole(nPos))
		{
			auto pCell = MapClass::Instance->TryGetCellAt(nPos);

			if (!pCell)
				return;

			//this dummy overlay placed so it can be replace later with real veins
			for (int i = 0; i < 8; ++i)
			{
				auto v11 = pCell->GetAdjacentCell((FacingType)i);
				v11->OverlayTypeIndex = 178//VEINHOLEDUMMY
					; //dummy for spawning veins ?
				;
				v11->OverlayData = 0u; //max it out
				v11->RedrawForVeins();
			}

			pCell->OverlayTypeIndex = 167; //USELESS -> replace with VEINHOLE
			pCell->OverlayData = 0;

			auto pVein = GameCreate<VeinholeMonsterClass>(&nPos);
			pVein->RegisterAffectedCells();

			created = true;
		}
		--Unsorted::ScenarioInit();
	}
}