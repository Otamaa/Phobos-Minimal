#include "Body.h"

#include "NewSuperWeaponType/LightningStorm.h"
#include "NewSuperWeaponType/Dominator.h"
#include "NewSuperWeaponType/NuclearMissile.h"

#include <Misc/DamageArea.h>
#include <Utilities/Helpers.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>

// this is a complete rewrite of LightningStorm::Start.
ASMJIT_PATCH(0x539EB0, LightningStorm_Start, 5)
{
	const auto pSuper = SW_LightningStorm::CurrentLightningStorm;

	if (!pSuper)
	{
		// legacy way still needed for triggers.
		return 0;
	}

	GET(int const, duration, ECX);
	GET(int const, deferment, EDX);
	GET_STACK(CellStruct, cell, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	auto const pType = pSuper->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);

	auto ret = false;

	// generate random cell if the passed ones are empty
	if (cell == CellStruct::Empty)
	{
		auto const& Bounds = MapClass::Instance->MapCoordBounds;
		auto& Random = ScenarioClass::Instance->Random;
		while (!MapClass::Instance->CellExists(cell))
		{
			cell.X = static_cast<short>(Random.RandomFromMax(Bounds.Right));
			cell.Y = static_cast<short>(Random.RandomFromMax(Bounds.Bottom));
		}
	}

	// yes. set them even if the Lightning Storm is active.
	LightningStorm::Coords = cell;
	LightningStorm::Owner = pOwner;

	if (!LightningStorm::IsActive)
	{
		if (deferment)
		{
			// register this storm to start soon
			if (!LightningStorm::Deferment
				|| LightningStorm::Deferment >= deferment)
			{
				LightningStorm::Deferment = deferment;
			}
			LightningStorm::Duration = duration;
			ret = true;
		}
		else
		{
			// start the mayhem. not setting this will create an
			// infinite loop. not tested what happens after that.
			LightningStorm::Duration = duration;
			LightningStorm::StartTime = Unsorted::CurrentFrame;
			LightningStorm::IsActive = true;

			// blackout
			auto const outage = pExt->Weather_RadarOutage.Get(
				RulesClass::Instance->LightningStormDuration);
			if (outage > 0)
			{
				if (pExt->Weather_RadarOutageAffects == AffectedHouse::Owner)
				{
					if (!pOwner->Defeated)
					{
						pOwner->CreateRadarOutage(outage);
					}
				}
				else if (pExt->Weather_RadarOutageAffects != AffectedHouse::None)
				{
					for (auto const& pHouse : *HouseClass::Array)
					{
						if (pExt->IsHouseAffected(
							pOwner, pHouse, pExt->Weather_RadarOutageAffects))
						{
							if (!pHouse->Defeated)
							{
								pHouse->CreateRadarOutage(outage);
							}
						}
					}
				}
			}

			if (HouseClass::CurrentPlayer)
			{
				HouseClass::CurrentPlayer->RecheckRadar = true;
			}

			// let there be light
			ScenarioClass::Instance->UpdateLighting();

			// activation stuff
			if (pExt->Weather_PrintText.Get(
				RulesClass::Instance->LightningPrintText))
			{
				pExt->PrintMessage(pExt->Message_Activate, pSuper->Owner);
			}

			auto const sound = pExt->SW_ActivationSound.Get(
				RulesClass::Instance->StormSound);
			if (sound != -1)
			{
				VocClass::PlayGlobal(sound, Panning::Center, 1.0);
			}

			if (pExt->SW_RadarEvent)
			{
				RadarEventClass::Create(
					RadarEventType::SuperweaponActivated, cell);
			}

			MapClass::Instance->RedrawSidebar(1);
		}
	}

	R->EAX(ret);
	return 0x539F80;
}

// this is a complete rewrite of LightningStorm::Update.
ASMJIT_PATCH(0x53A6CF, LightningStorm_Update, 7)
{
	enum { Legacy = 0x53A8FFu, Handled = 0x53AB45u };

#pragma region NukeUpdate
	// switch lightning for nuke
	if (NukeFlash::Duration != -1)
	{
		if (NukeFlash::StartTime + NukeFlash::Duration < Unsorted::CurrentFrame)
		{
			if (NukeFlash::IsFadingIn())
			{
				NukeFlash::Status = NukeFlashStatus::FadeOut;
				NukeFlash::StartTime = Unsorted::CurrentFrame;
				NukeFlash::Duration = 15;
				ScenarioClass::Instance->UpdateLighting();
				MapClass::Instance->RedrawSidebar(1);
			}
			else if (NukeFlash::IsFadingOut())
			{
				SW_NuclearMissile::CurrentNukeType = nullptr;
				NukeFlash::Status = NukeFlashStatus::Inactive;
			}
		}
	}
#pragma endregion

	// update other screen effects
	PsyDom::Update();
	ChronoScreenEffect::Update();

#pragma region RemoveBoltThathalfwaydone
	// remove all bolts from the list that are halfway done
	for (auto i = LightningStorm::BoltsPresent->Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = LightningStorm::BoltsPresent->Items[i])
		{
			if (pAnim->Animation.Stage >= pAnim->Type->GetImage()->Frames / 2)
			{
				LightningStorm::BoltsPresent->RemoveAt(i);
			}
		}
	}
#pragma endregion

#pragma region cloudsthatshouldstrikerightnow
	// find the clouds that should strike right now
	for (auto i = LightningStorm::CloudsManifesting->Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = LightningStorm::CloudsManifesting->Items[i])
		{
			if (pAnim->Animation.Stage >= pAnim->Type->GetImage()->Frames / 2)
			{
				auto const crdStrike = pAnim->GetCoords();
				LightningStorm::Strike2(crdStrike);
				LightningStorm::CloudsManifesting->RemoveAt(i);
			}
		}
	}
#pragma endregion

	// all currently present clouds have to disappear first
	if (LightningStorm::CloudsPresent->Count <= 0)
	{
		// end the lightning storm
		if (LightningStorm::TimeToEnd)
		{
			if (LightningStorm::IsActive)
			{
				LightningStorm::IsActive = false;
				LightningStorm::Owner = nullptr;
				LightningStorm::Coords = CellStruct::Empty;
				SW_LightningStorm::CurrentLightningStorm = nullptr;
				ScenarioClass::Instance->UpdateLighting();
			}
			LightningStorm::TimeToEnd = false;
		}
	}
	else
	{
		for (auto i = LightningStorm::CloudsPresent->Count - 1; i >= 0; --i)
		{
			if (auto const pAnim = LightningStorm::CloudsPresent->Items[i])
			{
				auto pAnimImage = pAnim->Type->GetImage();
				if (pAnim->Animation.Stage >= pAnimImage->Frames - 1)
				{
					LightningStorm::CloudsPresent->RemoveAt(i);
				}
			}
		}
	}

	// check for presence of Ares SW
	auto const pSuper = SW_LightningStorm::CurrentLightningStorm;

	if (!pSuper)
	{
		// still support old logic for triggers
		return Legacy;
	}

	CellStruct coords = LightningStorm::Coords();

	auto const pType = pSuper->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);

	// is inactive
	if (!LightningStorm::IsActive || LightningStorm::TimeToEnd)
	{
		auto deferment = LightningStorm::Deferment();

		// still counting down?
		if (deferment > 0)
		{
			--deferment;
			LightningStorm::Deferment = deferment;

			// still waiting
			if (deferment)
			{
				if (deferment % 225 == 0)
				{
					if (pExt->Weather_PrintText.Get(
						RulesClass::Instance->LightningPrintText))
					{
						pExt->PrintMessage(pExt->Message_Launch, pSuper->Owner);
					}
				}
			}
			else
			{
				// launch the storm
				LightningStorm::Start(
					LightningStorm::Duration, 0, coords, LightningStorm::Owner);
			}
		}

		return Handled;
	}

	// does this Lightning Storm go on?
	auto const duration = LightningStorm::Duration();

	if (duration != -1 && duration + LightningStorm::StartTime < Unsorted::CurrentFrame)
	{
		// it's over already
		LightningStorm::TimeToEnd = true;
		return Handled;
	}

	// deterministic damage. the very target cell.
	auto const hitDelay = pExt->Weather_HitDelay.Get(
		RulesClass::Instance->LightningHitDelay);

	if (hitDelay > 0 && (Unsorted::CurrentFrame % hitDelay == 0))
	{
		LightningStorm::Strike(coords);
	}

	// random damage. somewhere in range.
	auto const scatterDelay = pExt->Weather_ScatterDelay.Get(
		RulesClass::Instance->LightningScatterDelay);

	if (scatterDelay > 0 && (Unsorted::CurrentFrame % scatterDelay == 0))
	{
		auto const range = pExt->GetNewSWType()->GetRange(pExt);
		auto const isRectangle = (range.height() <= 0);
		auto const width = range.width();
		auto const height = isRectangle ? width : range.height();

		auto const GetRandomCoords = [=]()
			{
				auto& Random = ScenarioClass::Instance->Random;
				auto const offsetX = Random.RandomRanged(-width / 2, width / 2);
				auto const offsetY = Random.RandomRanged(-height / 2, height / 2);
				auto const ret = coords + CellStruct {
					static_cast<short>(offsetX), static_cast<short>(offsetY) };

				// don't even try if this is invalid
				if (!MapClass::Instance->CellExists(ret))
				{
					return CellStruct::Empty;
				}

				// out of range?
				if (!isRectangle && ret.DistanceFrom(coords) > range.WidthOrRange)
				{
					return CellStruct::Empty;
				}

				// if we respect lightning rods, start looking for one.
				if (!pExt->Weather_IgnoreLightningRod)
				{
					// if, by coincidence, this is a rod, hit it.
					auto const pCell = MapClass::Instance->GetCellAt(ret);
					auto const pCellBld = pCell->GetBuilding();
					const auto& nRodTypes = pExt->Weather_LightningRodTypes;

					if (pCellBld && pCellBld->IsAlive && pCellBld->Type->LightningRod) {
						if (nRodTypes.empty() || nRodTypes.Contains(pCellBld->Type))
							return ret;
					}

					// if a lightning rod is next to this, hit that instead. naive.
					if (auto const pObj = pCell->FindTechnoNearestTo(
						Point2D::Empty, false, pCellBld)) {
						if (pObj->IsAlive) {
							if (auto const pBld = cast_to<BuildingClass*, false>(pObj)) {
								if (pBld->Type->LightningRod) {
									if (nRodTypes.empty() || nRodTypes.Contains(pBld->Type)) {
										return pBld->GetMapCoords();
									}
								}
							}
						}
					}
				}

				// is this spot far away from another cloud?
				auto const separation = pExt->Weather_Separation.Get(
					RulesClass::Instance->LightningSeparation);

				if (separation > 0)
				{
					// assume success and disprove.
					for (auto const& pCloud : LightningStorm::CloudsPresent.get())
					{
						auto const cellCloud = pCloud->GetMapCoords();
						auto const dist = Math::abs(cellCloud.X - ret.X)
							+ Math::abs(cellCloud.Y - ret.Y);

						if (dist < separation)
						{
							return CellStruct::Empty;
						}
					}
				}

				return ret;
			};

		// generate a new place to strike
		if (height > 0 && width > 0 && MapClass::Instance->CellExists(coords))
		{
			for (int k = pExt->Weather_ScatterCount; k > 0; --k)
			{
				auto const cell = GetRandomCoords();
				if (cell.IsValid())
				{
					// found a valid position. strike there.
					LightningStorm::Strike(cell);
					break;
				}
			}
		}
	}

	// jump over everything
	return Handled;
}

// create a cloud.
ASMJIT_PATCH(0x53A140, LightningStorm_Strike, 7)
{
	if (auto const pSuper = SW_LightningStorm::CurrentLightningStorm)
	{
		GET_STACK(CellStruct const, cell, 0x4);

		auto const pType = pSuper->Type;
		auto const pExt = SWTypeExtContainer::Instance.Find(pType);

		// get center of cell coords
		auto const pCell = MapClass::Instance->GetCellAt(cell);

		// create a cloud animation
		auto coords = pCell->GetCoordsWithBridge();
		if (coords.IsValid())
		{
			// select the anim
			auto const itClouds = pExt->Weather_Clouds.GetElements(
				RulesClass::Instance->WeatherConClouds);

			// infer the height this thing will be drawn at.
			if (pExt->Weather_CloudHeight < 0)
			{
				if (auto const itBolts = pExt->Weather_Bolts.GetElements(
					RulesClass::Instance->WeatherConBolts))
				{
					pExt->Weather_CloudHeight = GeneralUtils::GetLSAnimHeightFactor(itBolts[0], pCell, true);
				}
			}
			coords.Z += pExt->Weather_CloudHeight;

			if (auto const pAnimType = itClouds.at(ScenarioClass::Instance->Random.RandomFromMax(itClouds.size() - 1)))
			{
				if (pAnimType->GetImage())
				{
					// create the cloud and do some book keeping.
					auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
					pAnim->SetHouse(pSuper->Owner);
					LightningStorm::CloudsManifesting->AddItem(pAnim);
					LightningStorm::CloudsPresent->AddItem(pAnim);
				}
			}
		}

		R->EAX(true);
		return 0x53A2F1;
	}

	// legacy way for triggers.
	return 0;
}

// create bolt and damage area.
ASMJIT_PATCH(0x53A300, LightningStorm_Strike2, 5)
{
	auto const pSuper = SW_LightningStorm::CurrentLightningStorm;

	if (!pSuper)
	{
		// legacy way for triggers
		return 0;
	}

	REF_STACK(CoordStruct const, refCoords, 0x4);

	auto const pType = pSuper->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(refCoords);
	const auto pNewSW = pData->GetNewSWType();
	auto coords = pCell->GetCoordsWithBridge();

	if (coords.IsValid())
	{
		// create a bolt animation
		if (auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			if (auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1)))
			{
				if (pAnimType->GetImage())
				{
					auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
					pAnim->SetHouse(pSuper->Owner);
					LightningStorm::BoltsPresent->AddItem(pAnim);
				}
			}
		}

		// play lightning sound
		if (auto const it = pData->Weather_Sounds.GetElements(
			RulesClass::Instance->LightningSounds))
		{
			VocClass::SafeImmedietelyPlayAt(it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1)), &coords, nullptr);
		}

		auto debris = false;
		auto const pBld = pCell->GetBuilding();

		auto const empty = Point2D::Empty;
		auto const pObj = pCell->FindTechnoNearestTo(empty, false, nullptr);
		auto const isInfantry = cast_to<InfantryClass*>(pObj) != nullptr;

		// empty cell action
		if ((!pBld || !pBld->IsAlive) && (!pObj || !pObj->IsAlive))
		{
			debris = Helpers::Alex::is_any_of(
				pCell->LandType,
				LandType::Road,
				LandType::Rock,
				LandType::Wall,
				LandType::Weeds);
		}

		// account for lightning rods
		auto damage = pNewSW->GetDamage(pData);
		if (!pData->Weather_IgnoreLightningRod)
		{
			if (auto const pBldObj = cast_to<BuildingClass*>(pObj))
			{
				const auto& nRodTypes = pData->Weather_LightningRodTypes;
				auto const pBldType = pBldObj->Type;

				if (pBldType->LightningRod && (nRodTypes.empty() || nRodTypes.Contains(pBldType)))
				{
					// multiply the damage, but never go below zero.
					damage = MaxImpl(int(damage *
						BuildingTypeExtContainer::Instance.Find(pBldType)->LightningRod_Modifier), 0);
				}
			}
		}

		// cause mayhem
		if (damage)
		{
			auto const pWarhead = pNewSW->GetWarhead(pData);

			MapClass::FlashbangWarheadAt(
				damage, pWarhead, coords, false, SpotlightFlags::None);

			DamageArea::Apply(
				&coords, damage, nullptr, pWarhead, true, pSuper->Owner);

			// fancy stuff if damage is dealt
			if (auto const pAnimType = MapClass::SelectDamageAnimation(
				damage, pWarhead, pCell->LandType, coords))
			{
				auto pAnim = GameCreate<AnimClass>(pAnimType, coords);
				pAnim->SetHouse(pSuper->Owner);
			}
		}

		// has the last target been destroyed?
		if (pObj != pCell->FindTechnoNearestTo(empty, false, nullptr))
		{
			debris = true;
		}

		// create some debris
		if (auto const it = pData->Weather_Debris.GetElements(
			RulesClass::Instance->MetallicDebris))
		{
			// dead infantry never generates debris
			if (!isInfantry && debris)
			{
				auto const count = ScenarioClass::Instance->Random.RandomRanged(
					pData->Weather_DebrisMin, pData->Weather_DebrisMax);

				for (int i = 0; i < count; ++i)
				{
					if (auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1)))
					{
						auto pAnim = GameCreate<AnimClass>(pAnimType, coords);
						pAnim->SetHouse(pSuper->Owner);
					}
				}
			}
		}
	}

	return 0x53A69A;
}
