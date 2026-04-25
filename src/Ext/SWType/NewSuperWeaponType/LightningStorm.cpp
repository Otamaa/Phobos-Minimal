#include "LightningStorm.h"

#include <Utilities/Helpers.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>

#include <SuperClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>

SuperClass* SW_LightningStorm::CurrentLightningStorm = nullptr;

SuperWeaponFlags SW_LightningStorm::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (!pThis->IsCharged)
		return false;

	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pThis->Type);

	auto duration = pData->Weather_Duration.Get(RulesClass::Instance->LightningStormDuration);
	auto deferment = pData->SW_Deferment.Get(RulesClass::Instance->LightningDeferment);

	if (!pData->Weather_UseSeparateState)
	{
		// Classic mode: uses the global singleton LightningStorm state
		CurrentLightningStorm = pThis;
		LightningStorm::Start(duration, deferment, Coords, pThis->Owner);
	}
	else
	{
		// Separate state mode: each activation gets its own state machine,
		// allowing multiple lightning storms to run simultaneously
		this->newStateMachine(duration, deferment, Coords, pThis, this->GetFirer(pThis, Coords, false));
	}

	return true;
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW->Type);

	if (!pData->Weather_UseSeparateState)
	{
		// Classic mode: only one global lightning storm allowed at a time
		if (LightningStorm::IsActive() || LightningStorm::HasDeferment())
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
	pData->Weather_UseSeparateState = false;

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_WeatherDeviceReady);
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_LightningStormReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_LightningStormCreated);

	pData->Message_Launch = GameStrings::TXT_LIGHTNING_STORM_APPROACHING();
	pData->Message_Activate = GameStrings::TXT_LIGHTNING_STORM();
	pData->Message_Abort = GameStrings::LightningStormActive_msg();

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LightningStorm;
	pData->CursorType = int(MouseCursorType::LightningStorm);
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

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

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
		.Process(this->Invoker, RegisterForChange)
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

// ---------------------------------------------------------------------------
// Helper — safely gets the halfway frame for an anim, returns -1 on failure.
// Guards against null anims, null types, and missing images.
// ---------------------------------------------------------------------------
static int GetAnimHalfwayFrame(AnimClass* pAnim)
{
	if (!pAnim || !pAnim->Type)
		return -1;

	auto const pImage = pAnim->Type->GetImage();
	if (!pImage || pImage->Frames <= 0)
		return -1;

	return pImage->Frames / 2;
}

// ---------------------------------------------------------------------------
// Helper — safely gets the last frame for an anim, returns -1 on failure.
// ---------------------------------------------------------------------------
static int GetAnimLastFrame(AnimClass* pAnim)
{
	if (!pAnim || !pAnim->Type)
		return -1;

	auto const pImage = pAnim->Type->GetImage();
	if (!pImage || pImage->Frames <= 0)
		return -1;

	return pImage->Frames - 1;
}

// ---------------------------------------------------------------------------
// Update — called every frame while the state machine exists
//
// Lifecycle:
//   1. Deferment countdown (prints warning messages periodically)
//   2. Storm active: deterministic strikes on target cell (HitDelay),
//      random strikes within range (ScatterDelay)
//   3. TimeToEnd set when duration expires
//   4. Waits for all clouds to finish their animations
//   5. Deactivates and restores lighting
// ---------------------------------------------------------------------------
void CloneableLighningStormStateMachine::Update()
{
	// --- Phase 1: Clean up finished bolts ---
	// Remove bolts that have reached their halfway point (visual effect done)
	this->BoltsPresent.remove_all_if([](AnimClass* pAnim)
 {
	 auto const half = GetAnimHalfwayFrame(pAnim);
	 return half < 0 || pAnim->Animation.Stage >= half;
	});

	// --- Phase 2: Process clouds that are ready to strike ---
	// When a cloud reaches its halfway frame, trigger the lightning strike
	// beneath it, then remove it from the manifest (so it only strikes once)
	this->CloudsManifest.remove_all_if([&](AnimClass* pAnim)
 {
	 auto const half = GetAnimHalfwayFrame(pAnim);
	 if (half < 0)
		 return true; // invalid anim, remove

	 if (pAnim->Animation.Stage >= half)
	 {
		 auto const crdStrike = pAnim->GetCoords();
		 this->Strike2(crdStrike);
		 return true; // struck — remove from manifest
	 }

	 return false; // not ready yet, keep
	});

	// --- Phase 3: Clean up finished cloud visuals ---
	// Must run before the empty check so invalidated/finished clouds
	// are removed promptly and don't block storm shutdown
	this->CloudsPresent.remove_all_if([](AnimClass* pAnim)
 {
	 auto const last = GetAnimLastFrame(pAnim);
	 return last < 0 || pAnim->Animation.Stage >= last;
	});

	// --- Phase 4: End-of-storm shutdown ---
	// All clouds must visually finish before we deactivate
	if (CloudsPresent.empty())
	{
		if (TimeToEnd)
		{
			if (IsActive)
			{
				IsActive = false;
				Coords = CellStruct::Empty;
			}
			TimeToEnd = false;
			ScenarioClass::Instance->UpdateLighting();
		}
	}

	auto const pExt = this->GetTypeExtData();

	// --- Phase 5: Deferment countdown (pre-activation) ---
	if (!IsActive || TimeToEnd)
	{
		auto deferment = Deferment;

		if (deferment > 0)
		{
			--deferment;
			Deferment = deferment;

			if (deferment)
			{
				// Print periodic warning messages during countdown
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
				// Countdown finished — launch the storm
				this->Start(Coords, ActualDuration, 0);
			}
		}

		return;
	}

	// --- Phase 6: Check if the storm has expired ---
	auto const duration = ActualDuration;
	if (duration != -1 && duration + StartTime < Unsorted::CurrentFrame())
	{
		TimeToEnd = true;
		return;
	}

	// --- Phase 7: Deterministic strike on the target cell ---
	auto const hitDelay = pExt->Weather_HitDelay.Get(
		RulesClass::Instance->LightningHitDelay);

	if (hitDelay > 0 && (Unsorted::CurrentFrame() % hitDelay == 0))
	{
		this->Strike(Coords);
	}

	// --- Phase 8: Random strikes within range ---
	auto const scatterDelay = pExt->Weather_ScatterDelay.Get(
		RulesClass::Instance->LightningScatterDelay);

	if (scatterDelay > 0 && (Unsorted::CurrentFrame() % scatterDelay == 0))
	{
		auto const range = pExt->GetNewSWType()->GetRange(pExt);
		auto const isRectangle = (range.height() <= 0);
		auto const width = range.width();
		auto const height = isRectangle ? width : range.height();

		// Generates a random cell within the storm's range, respecting
		// lightning rods (redirects strikes) and separation distance
		// (prevents clouds from clustering too close together)
		auto const GetRandomCoords = [=]()
			{
				auto& Random = ScenarioClass::Instance->Random;
				auto const offsetX = Random.RandomRanged(-width / 2, width / 2);
				auto const offsetY = Random.RandomRanged(-height / 2, height / 2);
				auto const ret = Coords + CellStruct {
					static_cast<short>(offsetX), static_cast<short>(offsetY) };

				if (!MapClass::Instance->CellExists(ret))
					return CellStruct::Empty;

				// Circular range check (only for non-rectangular ranges)
				if (!isRectangle && ret.DistanceFrom(Coords) > range.WidthOrRange)
					return CellStruct::Empty;

				// --- Lightning rod redirection ---
				if (!pExt->Weather_IgnoreLightningRod)
				{
					auto const pCell = MapClass::Instance->GetCellAt(ret);
					auto const pCellBld = pCell->GetBuilding();
					const auto& nRodTypes = pExt->Weather_LightningRodTypes;

					// Direct hit on a rod
					if (pCellBld && pCellBld->IsAlive && pCellBld->Type->LightningRod)
					{
						if (nRodTypes.empty() || nRodTypes.Contains(pCellBld->Type))
							return ret;
					}

					// Nearby rod attracts the strike
					if (auto const pObj = pCell->FindTechnoNearestTo(
						Point2D::Empty, false, pCellBld))
					{
						if (pObj->IsAlive)
						{
							if (auto const pBld = cast_to<BuildingClass*, false>(pObj))
							{
								if (pBld->Type->LightningRod)
								{
									if (nRodTypes.empty() || nRodTypes.Contains(pBld->Type))
									{
										return pBld->GetMapCoords();
									}
								}
							}
						}
					}
				}

				// --- Separation check ---
				// Reject if too close to an existing cloud
				auto const separation = pExt->Weather_Separation.Get(
					RulesClass::Instance->LightningSeparation);

				if (separation > 0)
				{
					for (auto const& pCloud : CloudsPresent)
					{
						if (!pCloud)
							continue;

						auto const cellCloud = pCloud->GetMapCoords();
						auto const dist = Math::abs(cellCloud.X - ret.X)
							+ Math::abs(cellCloud.Y - ret.Y);

						if (dist < separation)
							return CellStruct::Empty;
					}
				}

				return ret;
			};

		// Try up to ScatterCount times to find a valid random strike cell
		if (height > 0 && width > 0 && MapClass::Instance->CellExists(Coords))
		{
			for (int k = pExt->Weather_ScatterCount; k > 0; --k)
			{
				auto const cell = GetRandomCoords();
				if (cell.IsValid())
				{
					this->Strike(cell);
					break;
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Strike2 — executes the actual lightning strike at a coordinate
//
// Called when a cloud animation reaches its midpoint. Creates:
//   - A bolt animation (visual lightning)
//   - Lightning sound effect
//   - Warhead detonation (damage)
//   - Bolt explosion animation
//   - Metallic debris (if appropriate surface type or target destroyed)
//
// Lightning rods reduce damage via their modifier instead of blocking it.
// ---------------------------------------------------------------------------
void CloneableLighningStormStateMachine::Strike2(CoordStruct const& nCoord)
{
	if (nCoord == CoordStruct::Empty)
		return;

	auto const pData = this->GetTypeExtData();
	auto const pCell = MapClass::Instance->GetCellAt(nCoord);
	auto const coords = pCell->GetCoordsWithBridge();

	// --- Create bolt animation ---
	if (auto it = pData->Weather_Bolts.GetElements(
		RulesClass::Instance->WeatherConBolts))
	{
		if (!it.empty())
		{
			auto const randIndex = ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1);
			if (randIndex < it.size())
			{
				if (auto const pAnimType = it[randIndex])
				{
					if (pAnimType->GetImage())
					{
						auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
						AnimExtData::SetAnimOwnerHouseKind(pAnim, Super->Owner, nullptr, Invoker, false, false);
						BoltsPresent.push_back(pAnim);
					}
				}
			}
		}
	}

	// --- Play lightning sound ---
	if (auto const it = pData->Weather_Sounds.GetElements(
		RulesClass::Instance->LightningSounds))
	{
		if (!it.empty())
		{
			auto const rnd = ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1);
			if (rnd < it.size())
			{
				VocClass::SafeImmedietelyPlayAt(it[rnd], &coords, nullptr);
			}
		}
	}

	// --- Determine if debris should be created ---
	auto debris = false;
	auto const pBld = pCell->GetBuilding();

	auto const& empty = Point2D::Empty;
	auto const pObj = pCell->FindTechnoNearestTo(empty, false, nullptr);
	auto const isInfantry = pObj ? (cast_to<InfantryClass*>(pObj) != nullptr) : false;

	// Empty cell with certain land types generates debris
	if ((!pBld || !pBld->IsAlive) && (!pObj || !pObj->IsAlive))
	{
		debris = Helpers::Alex::is_any_of(
			pCell->LandType,
			LandType::Road,
			LandType::Rock,
			LandType::Wall,
			LandType::Weeds);
	}

	// --- Calculate damage (accounting for lightning rods) ---
	auto damage = pData->GetNewSWType()->GetDamage(pData);

	if (!pData->Weather_IgnoreLightningRod && pObj && pObj->IsAlive)
	{
		if (auto const pBldObj = cast_to<BuildingClass*>(pObj))
		{
			const auto& nRodTypes = pData->Weather_LightningRodTypes;
			auto const pBldType = pBldObj->Type;

			if (pBldType->LightningRod && (nRodTypes.empty() || nRodTypes.Contains(pBldType)))
			{
				auto const pBldExt = BuildingTypeExtContainer::Instance.Find(pBldType);
				damage = MaxImpl(int(damage * pBldExt->LightningRod_Modifier), 0);
			}
		}
	}

	// --- Deal damage ---
	if (damage)
	{
		auto pWarhead = pData->GetNewSWType()->GetWarhead(pData);

		if (!Invoker)
			Debug::LogInfo("LS[{} - {}] Invoker is nullptr, dealing damage without ownership!",
				(void*)Super, Super->Type->ID);

		WarheadTypeExtData::DetonateAt(pWarhead, MapClass::Instance->GetCellAt(coords), coords, Invoker, damage, Super->Owner);

		if (auto pBoltExt = pData->Weather_BoltExplosion.Get(RulesClass::Instance->WeatherConBoltExplosion))
		{
			auto pAnim = GameCreate<AnimClass>(pBoltExt, coords);
			pAnim->SetHouse(Super->Owner);
		}
	}

	// --- Check if the strike destroyed the target ---
	if (pObj != pCell->FindTechnoNearestTo(empty, false, nullptr))
	{
		debris = true;
	}

	// --- Create metallic debris ---
	if (auto const it = pData->Weather_Debris.GetElements(
		RulesClass::Instance->MetallicDebris))
	{
		// Dead infantry never generates debris
		if (!isInfantry && debris && !it.empty())
		{
			auto const count = ScenarioClass::Instance->Random.RandomRanged(
				pData->Weather_DebrisMin, pData->Weather_DebrisMax);

			for (int i = 0; i < count; ++i)
			{
				auto const randIndex = ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1);
				if (randIndex < it.size())
				{
					if (auto const pAnimType = it[randIndex])
					{
						AnimExtData::SetAnimOwnerHouseKind(
							GameCreate<AnimClass>(pAnimType, coords),
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

// ---------------------------------------------------------------------------
// Strike — creates a cloud animation at the given cell
//
// The cloud will later trigger Strike2 when it reaches its halfway frame
// (tracked via CloudsManifest). CloudsPresent tracks all active clouds
// for separation distance checks and end-of-storm shutdown gating.
// ---------------------------------------------------------------------------
bool CloneableLighningStormStateMachine::Strike(CellStruct const& nCell)
{
	if (nCell == CellStruct::Empty)
		return false;

	auto const pExt = this->GetTypeExtData();

	auto const pCell = MapClass::Instance->GetCellAt(nCell);
	auto coords = pCell->GetCoordsWithBridge();

	auto const itClouds = pExt->Weather_Clouds.GetElements(RulesClass::Instance->WeatherConClouds);

	// Infer cloud height from bolt animation if not explicitly set
	if (pExt->Weather_CloudHeight < 0)
	{
		if (auto const itBolts = pExt->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			if (!itBolts.empty() && itBolts[0])
			{
				pExt->Weather_CloudHeight = GeneralUtils::GetLSAnimHeightFactor(itBolts[0], pCell, false);
			}
		}
	}

	coords.Z += pExt->Weather_CloudHeight;

	if (!itClouds.empty())
	{
		auto const randIndex = ScenarioClass::Instance->Random.RandomFromMax(itClouds.size() - 1);
		if (randIndex < itClouds.size())
		{
			if (auto const pAnimType = itClouds[randIndex])
			{
				if (pAnimType->GetImage())
				{
					auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);
					AnimExtData::SetAnimOwnerHouseKind(pAnim, Super->Owner, nullptr, Invoker, false, false);
					CloudsManifest.push_back(pAnim);
					CloudsPresent.push_back(pAnim);
				}
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Start — begins (or schedules) the lightning storm
//
// If nDeferment > 0, the storm is scheduled to start after that many frames.
// If nDeferment == 0, the storm begins immediately:
//   - Radar outage applied to affected houses
//   - Lighting updated (screen darkening)
//   - Activation message and sound played
//   - Radar event created (if enabled)
// ---------------------------------------------------------------------------
bool CloneableLighningStormStateMachine::Start(CellStruct& cell, int nDuration, int nDeferment)
{
	auto pData = this->GetTypeExtData();
	bool ret = false;

	// Generate a random valid cell if none provided
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

	if (IsActive)
		return false;

	if (nDeferment)
	{
		// Schedule the storm to start after the deferment period
		if (!Deferment || Deferment >= nDeferment)
		{
			Deferment = nDeferment;
		}
		ActualDuration = nDuration;
		ret = true;
	}
	else
	{
		// --- Immediate activation ---
		ActualDuration = nDuration;
		StartTime = Unsorted::CurrentFrame;
		IsActive = true;

		// --- Radar outage ---
		auto const outage = pData->Weather_RadarOutage.Get(
			RulesClass::Instance->LightningStormDuration);

		if (outage > 0)
		{
			if (pData->Weather_RadarOutageAffects == AffectedHouse::Owner)
			{
				if (!Super->Owner->Defeated)
				{
					Super->Owner->CreateRadarOutage(outage);
				}
			}
			else if (pData->Weather_RadarOutageAffects != AffectedHouse::None)
			{
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

		if (HouseClass::CurrentPlayer())
		{
			HouseClass::CurrentPlayer->RecheckRadar = true;
		}

		ScenarioClass::Instance->UpdateLighting();

		// --- Activation feedback ---
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

	return ret;
}

void CloneableLighningStormStateMachine::InvalidatePointer(AbstractClass* ptr, bool remove, AbstractType  type)
{
	switch (type)
	{
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::Building:
	case AbstractType::Infantry:
		AnnounceInvalidPointer(Invoker, ptr, remove);
		break;
	case AbstractType::Anim:
		AnnounceInvalidPointer<AnimClass*>(CloudsPresent, ptr);
		AnnounceInvalidPointer<AnimClass*>(CloudsManifest, ptr);
		AnnounceInvalidPointer<AnimClass*>(BoltsPresent, ptr);
		break;
	default:
		break;
	}
}