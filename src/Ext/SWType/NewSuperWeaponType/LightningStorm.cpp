#include "LightningStorm.h"

#include <Utilities/Helpers.h>
#include <Ext/BuildingType/Body.h>

SuperClass* SW_LightningStorm::CurrentLightningStorm = nullptr;

std::vector<const char*> SW_LightningStorm::GetTypeString() const
{
	return { "NewLS" };
}

bool SW_LightningStorm::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags SW_LightningStorm::Flags() const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pThis->Type);

		auto duration = pData->Weather_Duration.Get(RulesClass::Instance->LightningStormDuration);
		auto deferment = pData->SW_Deferment.Get(RulesClass::Instance->LightningDeferment);


		if(!pData->Weather_UseSeparateState) {
			CurrentLightningStorm = pThis;

			LightningStorm::Start(duration, deferment, Coords, pThis->Owner);
		}
		else
		{
			this->newStateMachine(duration,deferment, Coords, pThis);
		}

		return true;
	}
	return false;
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW->Type);

	// only one Lightning Storm allowed
	if (LightningStorm::Active || LightningStorm::HasDeferment())
	{
		if (IsPlayer)
		{
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

void SW_LightningStorm::Initialize(SWTypeExt::ExtData* pData)
{ 
	// Defaults to Lightning Storm values
	pData->Weather_DebrisMin = 2;
	pData->Weather_DebrisMax = 4;
	pData->Weather_IgnoreLightningRod = false;
	pData->Weather_ScatterCount = 1;

	pData->Weather_RadarOutageAffects = AffectedHouse::Enemies;

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_WeatherDeviceReady);
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_LightningStormReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_LightningStormCreated);

	pData->Message_Launch = GameStrings::TXT_LIGHTNING_STORM_APPROACHING();
	pData->Message_Activate = GameStrings::TXT_LIGHTNING_STORM();
	pData->Message_Abort = GameStrings::LightningStormActive_msg();

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LightningStorm;
	pData->CursorType = int(MouseCursorType::LightningStorm);

	//
	pData->Weather_UseSeparateState = true;

}

void SW_LightningStorm::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ 
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->Weather_Duration.Read(exINI, section, "Lightning.Duration");
	pData->Weather_RadarOutage.Read(exINI, section, "Lightning.RadarOutage");
	pData->Weather_HitDelay.Read(exINI, section, "Lightning.HitDelay");
	pData->Weather_ScatterDelay.Read(exINI, section, "Lightning.ScatterDelay");
	pData->Weather_ScatterCount.Read(exINI, section, "Lightning.ScatterCount");
	pData->Weather_Separation.Read(exINI, section, "Lightning.Separation");
	pData->Weather_PrintText.Read(exINI, section, "Lightning.PrintText");
	pData->Weather_IgnoreLightningRod.Read(exINI, section, "Lightning.IgnoreLightningRod");
	pData->Weather_DebrisMin.Read(exINI, section, "Lightning.DebrisMin");
	pData->Weather_DebrisMax.Read(exINI, section, "Lightning.DebrisMax");
	pData->Weather_CloudHeight.Read(exINI, section, "Lightning.CloudHeight");
	pData->Weather_BoltExplosion.Read(exINI, section, "Lightning.BoltExplosion");
	pData->Weather_RadarOutageAffects.Read(exINI, section, "Lightning.RadarOutageAffects");
	pData->Weather_Clouds.Read(exINI, section, "Lightning.Clouds");
	pData->Weather_Bolts.Read(exINI, section, "Lightning.Bolts");
	pData->Weather_Debris.Read(exINI, section, "Lightning.Debris");
	pData->Weather_Sounds.Read(exINI, section, "Lightning.Sounds");
	pData->Weather_UseSeparateState.Read(exINI, section, "Lighning.UseSeparateState");
}

WarheadTypeClass* SW_LightningStorm::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->LightningWarhead);
}

int SW_LightningStorm::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->LightningDamage);
}

SWRange SW_LightningStorm::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange(RulesClass::Instance->LightningCellSpread) : pData->SW_Range;
}

bool CloneableLighningStormStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(this->CloudsPresent, RegisterForChange)
		.Process(this->CloudsManifest, RegisterForChange)
		.Process(this->BoltsPresent, RegisterForChange)
		.Process(this->ActualDuration)
		.Process(this->StartTime)
		.Process(this->Deferment)
		.Process(this->IsActive)
		.Process(this->TimeToEnd)
		.Success();
}

bool CloneableLighningStormStateMachine::Save(PhobosStreamWriter& Stm) const
{
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(this->CloudsPresent)
		.Process(this->CloudsManifest)
		.Process(this->BoltsPresent)
		.Process(this->ActualDuration)
		.Process(this->StartTime)
		.Process(this->Deferment)
		.Process(this->IsActive)
		.Process(this->TimeToEnd)
		.Success();
}

void CloneableLighningStormStateMachine::Update()
{

	// remove all bolts from the list that are halfway done
	for (auto i = BoltsPresent.Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = BoltsPresent[i])
		{
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				BoltsPresent.RemoveAt(i);
			}
		}
	}

	// find the clouds that should strike right now
	for (auto i = CloudsManifest.Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = CloudsManifest[i])
		{
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				auto const crdStrike = pAnim->GetCoords();
				Strike2(crdStrike);
				CloudsManifest.RemoveAt(i);
			}
		}
	}

	// all currently present clouds have to disappear first
	if (CloudsPresent.Count <= 0)
	{
		// end the lightning storm
		if (TimeToEnd)
		{
			if (IsActive)
			{
				IsActive = false;
				Coords = CellStruct::Empty;
			}
			TimeToEnd = false;
		}
	}
	else
	{
		for (auto i = CloudsPresent.Count - 1; i >= 0; --i)
		{
			if (auto const pAnim = CloudsPresent[i])
			{
				auto pAnimImage = pAnim->Type->GetImage();
				if (pAnim->Animation.Value >= pAnimImage->Frames - 1)
				{
					CloudsPresent.RemoveAt(i);
				}
			}
		}
	}

	auto const pExt = this->GetTypeExtData();

	// is inactive
	if (!IsActive || TimeToEnd)
	{
		auto deferment = Deferment;

		// still counting down?
		if (deferment > 0)
		{
			--deferment;
			Deferment = deferment;

			// still waiting
			if (deferment)
			{
				if (deferment % 225 == 0)
				{
					if (pExt->Weather_PrintText.Get(
						RulesClass::Instance->LightningPrintText))
					{
						pExt->PrintMessage(pExt->Message_Launch, Super->Owner);
					}
				}
			}
			else
			{
				// launch the storm
				Start(Coords, ActualDuration, 0);
			}
		}

		return;
	}

	// does this Lightning Storm go on?
	auto const duration = ActualDuration;
	if (duration != -1 && duration + StartTime < Unsorted::CurrentFrame)
	{
		// it's over already
		TimeToEnd = true;
		return;
	}

	// deterministic damage. the very target cell.
	auto const hitDelay = pExt->Weather_HitDelay.Get(
		RulesClass::Instance->LightningHitDelay);
	if (hitDelay > 0 && (Unsorted::CurrentFrame % hitDelay == 0))
	{
		Strike(Coords);
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
			auto const ret = Coords + CellStruct{
				static_cast<short>(offsetX), static_cast<short>(offsetY) };

			// don't even try if this is invalid
			if (!MapClass::Instance->CellExists(ret))
			{
				return CellStruct::Empty;
			}

			// out of range?
			if (!isRectangle && ret.DistanceFrom(Coords) > range.WidthOrRange)
			{
				return CellStruct::Empty;
			}

			// if we respect lightning rods, start looking for one.
			if (!pExt->Weather_IgnoreLightningRod)
			{
				// if, by coincidence, this is a rod, hit it.
				auto const pCell = MapClass::Instance->GetCellAt(ret);
				auto const pCellBld = pCell->GetBuilding();

				if (pCellBld && pCellBld->Type->LightningRod)
				{
					return ret;
				}

				// if a lightning rod is next to this, hit that instead. naive.
				if (auto const pObj = pCell->FindTechnoNearestTo(
					Point2D::Empty, false, pCellBld))
				{
					if (auto const pBld = specific_cast<BuildingClass*>(pObj))
					{
						if (pBld->Type->LightningRod)
						{
							return pBld->GetMapCoords();
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
				for (auto const& pCloud : CloudsPresent)
				{
					auto const cellCloud = pCloud->GetMapCoords();
					auto const dist = std::abs(cellCloud.X - ret.X)
						+ std::abs(cellCloud.Y - ret.Y);

					if (dist < separation)
					{
						return CellStruct::Empty;
					}
				}
			}

			return ret;
		};

		// generate a new place to strike
		if (height > 0 && width > 0 && MapClass::Instance->CellExists(Coords))
		{
			for (int k = pExt->Weather_ScatterCount; k > 0; --k)
			{
				auto const cell = GetRandomCoords();
				if (cell != CellStruct::Empty)
				{
					// found a valid position. strike there.
					Strike(cell);
					break;
				}
			}
		}
	}

}

void CloneableLighningStormStateMachine::Strike2(CoordStruct const& nCoord)
{

	auto const pData = this->GetTypeExtData();
	auto const pCell = MapClass::Instance->GetCellAt(nCoord);
	auto const coords = pCell->GetCoordsWithBridge();

	if (coords != CoordStruct::Empty)
	{

		// create a bolt animation
		if (auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			auto const rnd = ScenarioClass::Instance->Random.Random();
			auto const pAnimType = it.at(rnd % it.size());

			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, coords))
			{
				BoltsPresent.AddItem(pAnim);
			}
		}

		// play lightning sound
		if (auto const it = pData->Weather_Sounds.GetElements(
			RulesClass::Instance->LightningSounds))
		{
			auto const rnd = ScenarioClass::Instance->Random.Random();
			VocClass::PlayAt(it.at(rnd % it.size()), coords, nullptr);
		}

		auto debris = false;
		auto const pBld = pCell->GetBuilding();

		auto const& empty = Point2D::Empty;
		auto const pObj = pCell->FindTechnoNearestTo(empty, false, nullptr);
		auto const isInfantry = specific_cast<InfantryClass*>(pObj) != nullptr;

		// empty cell action
		if (!pBld && !pObj)
		{
			debris = Helpers::Alex::is_any_of(
				pCell->LandType,
				LandType::Road,
				LandType::Rock,
				LandType::Wall,
				LandType::Weeds);
		}

		// account for lightning rods
		auto damage = pData->GetNewSWType()->GetDamage(pData);
		if (!pData->Weather_IgnoreLightningRod)
		{
			if (auto const pBldObj = specific_cast<BuildingClass*>(pObj))
			{
				auto const pBldType = pBldObj->Type;
				if (pBldType->LightningRod)
				{
					// multiply the damage, but never go below zero.
					auto const pBldExt = BuildingTypeExt::ExtMap.Find(pBldType);
					damage = MaxImpl(static_cast<int>(
						damage * pBldExt->LightningRod_Modifier), 0);
				}
			}
		}

		// cause mayhem
		if (damage)
		{
			auto pWarhead = pData->GetNewSWType()->GetWarhead(pData);
			MapClass::FlashbangWarheadAt(
				damage, pWarhead, coords, false, SpotlightFlags::None);
			MapClass::DamageArea(
				coords, damage, nullptr, pWarhead, true, Super->Owner);

			// fancy stuff if damage is dealt
			auto const pAnimType = MapClass::SelectDamageAnimation(
				damage, pWarhead, pCell->LandType, coords);
			GameCreate<AnimClass>(pAnimType, coords);
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
					auto const rnd = ScenarioClass::Instance->Random.Random();
					auto const pAnimType = it.at(rnd % it.size());

					GameCreate<AnimClass>(pAnimType, coords);
				}
			}
		}
	}
}

bool CloneableLighningStormStateMachine::Strike(CellStruct const& nCell)
{

	auto const pExt = this->GetTypeExtData();

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(nCell);
	auto coords = pCell->GetCoordsWithBridge();

	// create a cloud animation
	if (coords != CoordStruct::Empty)
	{
		// select the anim
		auto const itClouds = pExt->Weather_Clouds.GetElements(
			RulesClass::Instance->WeatherConClouds);
		auto const pAnimType = itClouds.at(
			ScenarioClass::Instance->Random.Random() % itClouds.size());

		// infer the height this thing will be drawn at.
		if (pExt->Weather_CloudHeight < 0)
		{
			if (auto const itBolts = pExt->Weather_Bolts.GetElements(
				RulesClass::Instance->WeatherConBolts))
			{
				auto const pBoltAnim = itBolts.at(0);
				pExt->Weather_CloudHeight = Game::F2I(
					((pBoltAnim->GetImage()->Height / 2) - 0.5)
					* CloudHeightFactor);
			}
		}
		coords.Z += pExt->Weather_CloudHeight;

		// create the cloud and do some book keeping.
		if (auto const pAnim = GameCreate<AnimClass>(pAnimType, coords))
		{
			CloudsManifest.AddItem(pAnim);
			CloudsPresent.AddItem(pAnim);
		}
	}

	return true;
}

bool CloneableLighningStormStateMachine::Start(CellStruct& cell, int nDuration, int nDeferment)
{
	auto pData = this->GetTypeExtData();

	if (!pData)
	{
		return false;
	}
	bool ret = false;

	// generate random cell if the passed ones are empty
	if (cell == CellStruct::Empty)
	{
		auto const& Bounds = MapClass::Instance->MapCoordBounds;
		auto& Random = ScenarioClass::Instance->Random;
		while (!MapClass::Instance->CellExists(cell))
		{
			cell.X = static_cast<short>(Random.RandomRanged(0, Bounds.Right));
			cell.Y = static_cast<short>(Random.RandomRanged(0, Bounds.Bottom));
		}
	}

	Coords = cell;

	if (!IsActive)
	{
		if (nDeferment)
		{
			// register this storm to start soon
			if (!Deferment
				|| Deferment >= nDeferment)
			{
				Deferment = nDeferment;
			}
			ActualDuration = nDuration;
			ret = true;
		}
		else
		{

			// start the mayhem. not setting this will create an
			// infinite loop. not tested what happens after that.
			ActualDuration = nDuration;
			StartTime = Unsorted::CurrentFrame;
			IsActive = true;

			// blackout
			auto const outage = pData->Weather_RadarOutage.Get(
				RulesClass::Instance->LightningStormDuration);
			if (outage > 0)
			{
				for (auto const pHouse : *HouseClass::Array)
				{
					if (pData->IsHouseAffected(
						Super->GetOwningHouse(), pHouse, pData->Weather_RadarOutageAffects))
					{
						if (!pHouse->Defeated)
						{
							pHouse->CreateRadarOutage(outage);
						}
					}
				}
			}
			if (HouseClass::CurrentPlayer)
			{
				HouseClass::CurrentPlayer->RecheckRadar = true;
			}

			// activation stuff
			if (pData->Weather_PrintText.Get(
				RulesClass::Instance->LightningPrintText))
			{
				pData->PrintMessage(pData->Message_Activate, Super->GetOwningHouse());
			}

			auto const sound = pData->SW_ActivationSound.Get(
				RulesClass::Instance->StormSound);
			if (sound != -1)
			{
				VocClass::PlayGlobal(sound, Panning::Center, 1.0);
			}

			if (pData->SW_RadarEvent)
			{
				RadarEventClass::Create(
					RadarEventType::SuperweaponActivated, Coords);
			}

			MapClass::Instance->RedrawSidebar(1);
		}
	}


	return ret;
}

void CloneableLighningStormStateMachine::InvalidatePointer(void* ptr, bool remove)
{
	AnnounceInvalidPointer(CloudsPresent, ptr);
	AnnounceInvalidPointer(CloudsManifest, ptr);
	AnnounceInvalidPointer(BoltsPresent, ptr);
}
