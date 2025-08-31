#include "LightningStorm.h"

#include <Utilities/Helpers.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>

SuperClass* SW_LightningStorm::CurrentLightningStorm = nullptr;

std::vector<const char*> SW_LightningStorm::GetTypeString() const
{
	return { "NewLS" };
}

bool SW_LightningStorm::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags SW_LightningStorm::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pThis->Type);

		auto duration = pData->Weather_Duration.Get(RulesClass::Instance->LightningStormDuration);
		auto deferment = pData->SW_Deferment.Get(RulesClass::Instance->LightningDeferment);


		if(!pData->Weather_UseSeparateState) {
			CurrentLightningStorm = pThis;

			LightningStorm::Start(duration, deferment, Coords, pThis->Owner);
		}
		else
		{
			this->newStateMachine(duration,deferment, Coords, pThis , this->GetFirer(pThis, Coords, false));
		}

		return true;
	}
	return false;
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW->Type);

	if(!pData->Weather_UseSeparateState) {
		// only one Lightning Storm allowed
		if (LightningStorm::IsActive || LightningStorm::HasDeferment())
		{
			if (IsPlayer)
			{
				pData->PrintMessage(pData->Message_Abort, pSW->Owner);
			}
			return true;
		}
	}

	return false;
}

void SW_LightningStorm::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action::LightningStorm;
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
	pData->Weather_UseSeparateState = false;

}

bool SW_LightningStorm::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void SW_LightningStorm::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
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
	pData->Weather_UseSeparateState.Read(exINI, section, "Lightning.UseSeparateState");
	pData->Weather_LightningRodTypes.Read(exINI, section, "Lightning.LighningRodTypes");
}

WarheadTypeClass* SW_LightningStorm::GetWarhead(const SWTypeExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->LightningWarhead);
}

int SW_LightningStorm::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->LightningDamage);
}

SWRange SW_LightningStorm::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange(RulesClass::Instance->LightningCellSpread) : pData->SW_Range;
}

void SW_LightningStorm::ValidateData(SWTypeExtData* pData) const
{
	Debug::LogInfo("{} - {} SW Validating Data ---------------------------:", pData->This()->ID, this->GetTypeString()[0]);

	if (pData->Weather_BoltExplosion.isset()) {
		if (pData->Weather_BoltExplosion) {
			if (!pData->Weather_BoltExplosion->GetImage()) {
				Debug::LogInfo("Anim[{}] Has no proper Image!", pData->Weather_BoltExplosion->ID);
				Debug::RegisterParserError();
			}
		}
	}

	if (pData->Weather_Clouds.HasValue()) {
		for (auto& explo : pData->Weather_Clouds) {
			if (explo && !explo->GetImage()) {
				Debug::LogInfo("Anim[{}] Has no proper Image!", explo->ID);
				Debug::RegisterParserError();
			}
		}
	}

	if (pData->Weather_Bolts.HasValue()) {
		for (auto& explo : pData->Weather_Bolts) {
			if (explo && !explo->GetImage()) {
				Debug::LogInfo("Anim[{}] Has no proper Image!", explo->ID);
				Debug::RegisterParserError();
			}
		}
	}

	if (pData->Weather_Debris.HasValue()) {
		for (auto& explo : pData->Weather_Debris) {
			if (explo && !explo->GetImage()) {
				Debug::LogInfo("Anim[{}] Has no proper Image!", explo->ID);
				Debug::RegisterParserError();
			}
		}
	}

	Debug::LogInfo("-----------------------------------------------------");
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
		.Process(this->Invoker , RegisterForChange)
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
		.Process(this->Invoker)
		.Success();
}

void CloneableLighningStormStateMachine::Update()
{
	// remove all bolts from the list that are halfway done
	this->BoltsPresent.remove_all_if([](AnimClass* pAnim) {
		return !pAnim || pAnim->Animation.Stage >= pAnim->Type->GetImage()->Frames / 2;
	});

	// find the clouds that should strike right now
	this->CloudsManifest.remove_all_if([&](AnimClass* pAnim) {
		if (!pAnim)
			return true;

		if (pAnim->Animation.Stage >= pAnim->Type->GetImage()->Frames / 2) {
			auto const crdStrike = pAnim->GetCoords();
			this->Strike2(crdStrike);
		}

		return false;
	});

	// all currently present clouds have to disappear first
	if (CloudsPresent.empty())
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
		this->CloudsPresent.remove_all_if([&](AnimClass* pAnim) {
			return !pAnim || pAnim->Animation.Stage >= pAnim->Type->GetImage()->Frames - 1;
		});

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
				this->Start(Coords, ActualDuration, 0);
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
		this->Strike(Coords);
	}

	// random damage. somewhere in range.
	auto const scatterDelay = pExt->Weather_ScatterDelay.Get(
		RulesClass::Instance->LightningScatterDelay);

	if (scatterDelay > 0 && (Unsorted::CurrentFrame % scatterDelay == 0))
	{
		auto const range = Type->GetRange(pExt);
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
				const auto& nRodTypes = pExt->Weather_LightningRodTypes;

				if (pCellBld && pCellBld->IsAlive && pCellBld->Type->LightningRod) {
					if (nRodTypes.empty() || nRodTypes.Contains(pCellBld->Type))
						return ret;
				}

				// if a lightning rod is next to this, hit that instead. naive.
				if (auto const pObj = pCell->FindTechnoNearestTo(
					Point2D::Empty, false, pCellBld)) {
					if (auto const pBld = cast_to<BuildingClass*, false>(pObj)) {
						if(pBld->IsAlive && pBld->Type->LightningRod) {
							if (nRodTypes.empty() || nRodTypes.Contains(pBld->Type)) {
								return pBld->GetMapCoords();
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
				for (auto const& pCloud : CloudsPresent)
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
		if (height > 0 && width > 0 && MapClass::Instance->CellExists(Coords))
		{
			for (int k = pExt->Weather_ScatterCount; k > 0; --k)
			{
				auto const cell = GetRandomCoords();
				if (cell != CellStruct::Empty)
				{
					// found a valid position. strike there.
					this->Strike(cell);
					break;
				}
			}
		}
	}
}

void CloneableLighningStormStateMachine::Strike2(CoordStruct const& nCoord)
{
	if (nCoord != CoordStruct::Empty)
	{
		auto const pData = this->GetTypeExtData();
		auto const pCell = MapClass::Instance->GetCellAt(nCoord);
		auto const coords = pCell->GetCoordsWithBridge();
		// create a bolt animation
		if (auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			if(auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1))) {
				if(pAnimType->GetImage()) {
				 auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
				 AnimExtData::SetAnimOwnerHouseKind(pAnim, Super->Owner, nullptr, Invoker, false, false);
				 BoltsPresent.push_back(pAnim);
				}
			}
		}

		// play lightning sound
		if (auto const it = pData->Weather_Sounds.GetElements(
			RulesClass::Instance->LightningSounds))
		{
			auto const rnd = ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1);
			VocClass::SafeImmedietelyPlayAt(it.at(rnd), &coords, nullptr);
		}

		auto debris = false;
		auto const pBld = pCell->GetBuilding();

		auto const& empty = Point2D::Empty;
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
		auto damage = Type->GetDamage(pData);
		if (!pData->Weather_IgnoreLightningRod) {
			if(pObj->IsAlive) {
				if (auto const pBldObj = cast_to<BuildingClass*>(pObj))
				{
					const auto& nRodTypes = pData->Weather_LightningRodTypes;
					auto const pBldType = pBldObj->Type;

					if (pBldType->LightningRod && (nRodTypes.empty() || nRodTypes.Contains(pBldType)))
					{
						// multiply the damage, but never go below zero.
						auto const pBldExt = BuildingTypeExtContainer::Instance.Find(pBldType);
						damage = MaxImpl(int(damage * pBldExt->LightningRod_Modifier), 0);
					}
				}
			}
		}

		// cause mayhem
		if (damage)
		{
			auto pWarhead = Type->GetWarhead(pData);

			if (!Invoker)
				Debug::LogInfo("LS[{} - {}] Invoked is nullptr, dealing damage without ownership !! ", (void*)Super, Super->Type->ID);

			WarheadTypeExtData::DetonateAt(pWarhead, MapClass::Instance->GetCellAt(coords), coords, Invoker, damage ,Super->Owner);

			if(auto pBoltExt = pData->Weather_BoltExplosion.Get(RulesClass::Instance->WeatherConBoltExplosion)){
				auto pAnim = GameCreate<AnimClass>(pBoltExt, coords);
				pAnim->SetHouse(Super->Owner);
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

				for (int i = 0; i < count; ++i) {
					if(auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1))){
						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords),
							Super->Owner,
							nullptr,
							Invoker,
							false, false
						);
					}
				}
			}
		}
	}
}

bool CloneableLighningStormStateMachine::Strike(CellStruct const& nCell)
{
	auto const pExt = this->GetTypeExtData();

	// create a cloud animation
	if (nCell != CellStruct::Empty)
	{
		// get center of cell coords
		auto const pCell = MapClass::Instance->GetCellAt(nCell);
		auto coords = pCell->GetCoordsWithBridge();

		// select the anim
		auto const itClouds = pExt->Weather_Clouds.GetElements(RulesClass::Instance->WeatherConClouds);

		// infer the height this thing will be drawn at.
		if (pExt->Weather_CloudHeight < 0) {
			if (auto const itBolts = pExt->Weather_Bolts.GetElements(
				RulesClass::Instance->WeatherConBolts)) {
				pExt->Weather_CloudHeight = GeneralUtils::GetLSAnimHeightFactor(itBolts[0], pCell, false);
			}
		}

		coords.Z += pExt->Weather_CloudHeight;

		if(auto const pAnimType = itClouds.at(ScenarioClass::Instance->Random.RandomFromMax(itClouds.size() - 1))) {
			if (pAnimType->GetImage()) {
				// create the cloud and do some book keeping.
				auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
				AnimExtData::SetAnimOwnerHouseKind(pAnim, Super->Owner, nullptr, Invoker, false, false);
				CloudsManifest.push_back(pAnim);
				CloudsPresent.push_back(pAnim);
			}
		}
	}

	return true;
}

bool CloneableLighningStormStateMachine::Start(CellStruct& cell, int nDuration, int nDeferment)
{
	auto pData = this->GetTypeExtData();
	bool ret = false;

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
				if(pData->Weather_RadarOutageAffects == AffectedHouse::Owner){
					if (!Super->Owner->Defeated) {
						Super->Owner->CreateRadarOutage(outage);
					}
				} else if(pData->Weather_RadarOutageAffects != AffectedHouse::None){
					for (auto const& pHouse : *HouseClass::Array)
					{
						if (pData->IsHouseAffected(
							Super->Owner, pHouse, pData->Weather_RadarOutageAffects))
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

void CloneableLighningStormStateMachine::InvalidatePointer(AbstractClass* ptr, bool remove)
{
	AnnounceInvalidPointer(Invoker, ptr, remove);
	AnnounceInvalidPointer<AnimClass*> (CloudsPresent, ptr);
	AnnounceInvalidPointer<AnimClass*>(CloudsManifest, ptr);
	AnnounceInvalidPointer<AnimClass*>(BoltsPresent, ptr);
}
