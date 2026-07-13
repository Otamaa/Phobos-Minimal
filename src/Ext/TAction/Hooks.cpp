#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <OverlayTypeClass.h>
#include <LightSourceClass.h>
#include <RadSiteClass.h>
#include <VocClass.h>
#include <ScenarioClass.h>
#include <TriggerClass.h>
#include <ThemeClass.h>

#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <TriggerTypeClass.h>

ASMJIT_PATCH(0x6DD791, TActionClass_ReadINI_MaskedTActions, 0xB)
{
	GET(TActionClass*, pThis, EBP);

	switch (pThis->ActionKind)
	{
	case TriggerAction::PlayAnimAt:
	{
		if(pThis->Value < 0) {
			// default parsing but if the result is still not valid index
			//try again with text
			pThis->Value = AnimTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::PlayAnimAt;
		}

		break;
	}
	default:
		switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
		{
		case PhobosTriggerAction::WinByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::Win;
			break;
		case PhobosTriggerAction::LoseByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::Lose;
			break;
		case PhobosTriggerAction::ProductionBeginsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::ProductionBegins;
			break;
		case PhobosTriggerAction::AllToHuntByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::AllToHunt;
			break;
		case PhobosTriggerAction::FireSaleByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::FireSale;
			break;
		case PhobosTriggerAction::PlayMovieByID:
			for (int i = 0; i < MovieInfoArray->Count; ++i) {
				if (MovieInfoArray->Items[i] == pThis->Text) {
					pThis->Value = i;
					break;
				}
			}
			pThis->ActionKind = TriggerAction::PlayMovie;
			break;
		case PhobosTriggerAction::AutocreateBeginsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::AutocreateBegins;
			break;
		case PhobosTriggerAction::ChangeHouseByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::ChangeHouse;
			break;
		case PhobosTriggerAction::PlayMusicThemeByID:
			pThis->Value = ThemeClass::Instance->FindIndex(pThis->Text);
			pThis->ActionKind = TriggerAction::PlayMusicTheme;
			break;
		case PhobosTriggerAction::AddOneTimeSuperWeaponByID:
			pThis->Value = SuperWeaponTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::AddOneTimeSuperWeapon;
			break;
		case PhobosTriggerAction::AddRepeatingSuperWeaponByID:
			pThis->Value = SuperWeaponTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::AddRepeatingSuperWeapon;
			break;
		case PhobosTriggerAction::AllChangeHouseByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::AllChangeHouse;
			break;
		case PhobosTriggerAction::MakeAllyByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::MakeAlly;
			break;
		case PhobosTriggerAction::MakeEnemyByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::MakeEnemy;
			break;
		case PhobosTriggerAction::PlayAnimAtByID:
			pThis->Value = AnimTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::PlayAnimAt;
			break;
		case PhobosTriggerAction::DoExplosionAtByID:
			pThis->Value = WeaponTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::DoExplosionAt;
			break;
		case PhobosTriggerAction::CreateVoxelAnimByID:
			pThis->Value = VoxelAnimTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::CreateVoxelAnim;
			break;
		case PhobosTriggerAction::AITriggersBeginByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::AITriggersBegin;
			break;
		case PhobosTriggerAction::AITriggersStopByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::AITriggersStop;
			break;
		case PhobosTriggerAction::ParticleAnimByID:
			pThis->Value = ParticleSystemTypeClass::FindIndexById(pThis->Text);
			pThis->ActionKind = TriggerAction::ParticleAnim;
			break;
		case PhobosTriggerAction::MakeHouseCheerByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::MakeHouseCheer;
			break;
		case PhobosTriggerAction::DestroyAllByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::DestroyAll;
			break;
		case PhobosTriggerAction::DestroyAllBuildingsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::DestroyAllBuildings;
			break;
		case PhobosTriggerAction::DestroyAllLandUnitsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::DestroyAllLandUnits;
			break;
		case PhobosTriggerAction::DestroyAllNavalUnitsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::DestroyAllNavalUnits;
			break;
		case PhobosTriggerAction::MindControlBaseByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::MindControlBase;
			break;
		case PhobosTriggerAction::RestoreMindControlledBaseByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::RestoreMindControlledBase;
			break;
		case PhobosTriggerAction::RestoreStartingUnitsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::RestoreStartingUnits;
			break;
		case PhobosTriggerAction::RestoreStartingBuildingsByID:
			pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->Text);
			pThis->ActionKind = TriggerAction::RestoreStartingBuildings;
			break;

		default:
			break;
		}
		break;
	}
	

	return 0;
}