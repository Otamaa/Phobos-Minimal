 #include "Body.h"

#include <Utilities/Macro.h>

#include <Ext/Anim/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <TechnoClass.h>
#include <BuildingClass.h>
#include <SlaveManagerClass.h>
#include <SuperClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <SpawnManagerClass.h>
#include <CaptureManagerClass.h>

#include <Misc/Ares/Hooks/Header.h>

void FakeTemporalClass::CreateWarpAwayAnimation(WeaponTypeClass* pWeapon)
{
	if (auto pAnimType = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead)->Temporal_WarpAway.Get(RulesClass::Instance()->WarpAway)) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, this->Target->Location, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
			this->Owner ? this->Owner->Owner : nullptr,
			this->Target->Owner,
			this->Owner,
				false, false
		);
	}
}

void HandleBuildingDestruction(TemporalClass* pTemporal, BuildingClass* building)
{
	// Handle occupiers
	if (building->GetOccupantCount() > 0) {
		building->KillOccupants(nullptr);
	}

	// Handle superweapon suspension
	if (pTemporal->SourceSW) {
		pTemporal->SourceSW->SetOnHold(false);
		pTemporal->SourceSW = 0;
	}

	if (const auto pTunnelData = HouseExtData::GetTunnelVector(building->Type, building->Owner))
		TunnelFuncs::DestroyTunnel(&pTunnelData->Vector, building, pTemporal->Owner);

	if (building->Type->Helipad
		&& building->RadioLinks.Items
		&& building->RadioLinks.IsAllocated
		&& building->RadioLinks.IsInitialized
		) {
		for (auto i = 0; i < building->RadioLinks.Capacity; ++i) {
			if (auto const pAir = cast_to<AircraftClass*>(building->RadioLinks[i])) {
				if (pAir->IsAlive && !pAir->InLimbo && !pAir->TemporalTargetingMe) {
					const auto pExt = TechnoTypeExtContainer::Instance.Find(pAir->Type);

					if (pAir->IsInAir() || !AircraftTypeExtContainer::Instance.Find(pAir->Type)->ExtendedAircraftMissions_FastScramble
									.Get(RulesExtData::Instance()->ExpandAircraftMission)) {
						if ((pExt->Crashable.isset() && !pExt->Crashable) || !pAir->Crash(pTemporal->Owner)) {
							TechnoExtData::HandleRemove(pAir, pTemporal->Owner, false, false);
						}
					} else {
						//Ask plane to fly
						building->SendCommand(RadioCommand::AnswerLeave, pAir);
						pAir->DockedTo = nullptr;
					}
				}
			}
		}
	}

	if (building->Type->Radar)
		building->Owner->RecheckRadar = true;
}

// bugfix #379: Temporal friendly kills give veterancy
// bugfix #1266: Temporal kills gain double experience
void HandleDestruction(TemporalClass* pTemporal , TechnoClass* target , WeaponTypeClass* pWeapon)
{
	auto pOwnerExt = TechnoExtContainer::Instance.Find(pTemporal->Owner);
	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (target->IsMouseHovering)
		target->IsMouseHovering = false;

	if (pWarheadExt->Supress_LostEva)
		pOwnerExt->SupressEVALost = true;

	if (target && target->IsSelected)
		target->Deselect();

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto const pTargetExt = TechnoExtContainer::Instance.Find(target);

	if (auto pTargetShield = pTargetExt->GetShield()) {
		if (pTargetShield->IsAvailable()) {
			pTargetShield->OnTemporalUpdate(pTemporal);
		}
	}

	pTargetExt->RadarJammer.reset();

	if (target && target->IsAlive) {
		AresAE::UpdateTempoal(&pTargetExt->AeData, target);

		if (target && target->IsAlive) {
			for (auto& ae : pTargetExt->PhobosAE) {
				if (ae) {
					ae->AI_Temporal();
				}
			}

			if (target && target->IsAlive) {
				pTargetExt->UpdateRearmInTemporal();

				auto pBuilding = cast_to<BuildingClass*,false>(target);
				bool erase = true;

				if (!pBuilding && pWeaponExt->Abductor_Temporal)
					erase = !AresWPWHExt::conductAbduction(pWeapon, pTemporal->Owner, target, CoordStruct::Empty);

				if (erase) {
					if (pBuilding) {
						HandleBuildingDestruction(pTemporal, pBuilding);
					}

					// Handle bunker connections for target
					if (auto pBunker = cast_to<BuildingClass*>(target->BunkerLinkedItem)) {
						pBunker->UnloadBunker();
					}

					// Handle cargo
					target->KillPassengers(pTemporal->Owner);

					// Handle slave manager
					if (auto pSlave = target->SlaveManager) {
						pSlave->Killed(pTemporal->Owner, nullptr);
					}

					target->Destroyed(pTemporal->Owner);
					target->RegisterDestruction(pTemporal->Owner);
					// issue #1437: crash when warping out buildings infantry wants to garrison
					target->IsAlive = false;
					target->UnInit();
					target->Owner->RecheckTechTree = true;
					target->Owner->RecheckPower = true;
				}
			}
		}
	}
}

void FakeTemporalClass::_Update()
{
	if (this->Target && this->Target->TemporalTargetingMe == this && this->NextTemporal) {
		this->Target->TemporalTargetingMe = 0;
		this->Target->BeingWarpedOut = 0;
		this->Detach();
		return;
	}
	bool shouldProcessMainLogic = true;

	if (this->Owner && this->Owner->InOpenToppedTransport && this->Target) {

		CoordStruct targetPos = this->Target->GetCoords();
		CoordStruct ownerPos = this->Owner->GetCoords();
		CoordStruct distanceVector = ownerPos - targetPos;

		int disatanceMax = RulesClass::Instance->OpenToppedWarpDistance;

		if (auto const pTransport = this->Owner->Transporter) {
			if (pTransport->IsAlive) {
				const auto& _cDiscance = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->OpenTopped_WarpDistance;
				if (_cDiscance.isset()) {
					disatanceMax = _cDiscance.Get();
				}
			}
		}

		if (distanceVector.Length() > (disatanceMax << 8)) {
			this->LetGo();
			shouldProcessMainLogic = false;
		}
	}

	if (shouldProcessMainLogic) {
		this->WarpRemaining -= TechnoExt_ExtData::GetWarpPerStep(this, 0);

		if (!this->Owner) {
			this->Detach();
			return;
		}

		if(this->WarpRemaining <= 0) {
			this->WarpRemaining = 0;//lets reset the warp remaning
			int WeaponIdx = TechnoExtContainer::Instance.Find(this->Owner)->idxSlot_Warp;
			auto pWpStruct = this->Owner->GetWeapon(WeaponIdx);
			auto pWeapon = pWpStruct->WeaponType;

			if (auto pTarget = this->Target) {
				this->CreateWarpAwayAnimation(pWeapon);

				if (pTarget && pTarget->IsAlive) {
					HandleDestruction(this, pTarget, pWeapon);
				}
			}

			this->ResetTemporalState();
			if(this->Owner)
				this->Owner->EnterIdleMode(false, 1);
		}
	}
}

void FakeTemporalClass::_Detonate(TechnoClass* pTarget) 	{
	if (!pTarget || !this->Owner) return;

	// Clean up owner's existing temporal effect
	if (this->Owner->TemporalImUsing && this->Owner->TemporalImUsing->Target) {
		this->Owner->TemporalImUsing->LetGo();
	}

	if (!TechnoExt_ExtData::Warpable(this, pTarget) || this->Owner->TemporalTargetingMe)
		return;

	// bugfix #874 B: Temporal warheads affect Warpable=no units
	// it has been checked: this is warpable. free captured and destroy spawned units.
	if (pTarget->SpawnManager) {
		pTarget->SpawnManager->KillNodes();
	}

	if (pTarget->CaptureManager) {
		pTarget->CaptureManager->FreeAll();
	}

	this->Target = pTarget;
	auto pTargetType = pTarget->GetTechnoType();
	auto pUnit = cast_to<UnitClass*>(pTarget);
	auto pBuilding = cast_to<BuildingClass*>(pTarget);
	int wpIdx = TechnoExtContainer::Instance.Find(this->Owner)->idxSlot_Warp;
	auto pWeapon = this->Owner->GetWeapon(wpIdx)->WeaponType;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (TemporalClass* existingTemporal = pTarget->TemporalTargetingMe) {
		// Link to existing temporal chain
		this->NextTemporal = existingTemporal;
		this->PrevTemporal = existingTemporal->PrevTemporal;
		existingTemporal->PrevTemporal = this;

		if (this->PrevTemporal) {
			this->PrevTemporal->NextTemporal = this;
		}

	} else {

		pTarget->TemporalTargetingMe = this;

		if(pWHExt->Temporal_HealthFactor > 1.0){
			this->WarpRemaining = int(pTarget->GetHealthPercentage() * pWHExt->Temporal_HealthFactor);
		} else {
			this->WarpRemaining = 10 * pTargetType->Strength;
		}

		if(!pWHExt->Malicious && !pWHExt->Nonprovocative){
			if(auto pTargetOwner = pTarget->Owner) {
				if (pUnit) {
					if (pUnit->Type->Harvester && pTargetOwner == HouseClass::CurrentPlayer()) {
						auto nDest = pTarget->GetDestination();

						if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(nDest))) {
							VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack());
						}
					}

				} else if (pBuilding) {
					if (!pBuilding->Type->Insignificant && !pBuilding->IsStrange()) {
						((FakeHouseClass*)pTargetOwner)->_Attacked(pBuilding, pWeapon->Warhead);
					}

				}
			}
		}

	}

	pTarget->BeingWarpedOut = true;

	if (pTargetType->IsGattling) {
		pTarget->GattlingRateDown(1);
	}

	if (pBuilding) {
		auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
		pBldExt->MyPrismForwarding->RemoveFromNetwork(true);
		pBuilding->DisableTemporal();

		pBuilding->CashProductionTimer.Pause();

		for (size_t i = 0; i < std::size(pBuilding->Upgrades); ++i) {
			if (pBuilding->Upgrades[i]) {
				pBldExt->CashUpgradeTimers[i].Pause();
			}
		}

		if(pBuilding->Owner){
			if(pBuilding->Type->Radar)
				pBuilding->Owner->RecheckRadar = true;

			pBuilding->Owner->RecheckTechTree = 1;
		}
	}

	if (pTargetType->OpenTopped) {
		for (auto pPassenger = pTarget->Passengers.GetFirstPassenger();
			pPassenger;
			pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject)) {
			const auto pTemporal = pPassenger->TemporalImUsing;

			if (pTemporal && pTemporal->Target)
				pTemporal->LetGo();
		}
	}

	TemporalClass* targetTemporal = pTarget->TemporalImUsing;
	if (targetTemporal && targetTemporal->Target) {
		targetTemporal->LetGo();
	}

	if (pTarget->Owner && pTarget->Owner->IsControlledByHuman())
		pTarget->Deselect();

	pTarget->Mark(MarkType::Change);
}

// =============================
// load / save

template <typename T>
void TemporalExtData::Serialize(T& Stm) {


}

// =============================
// container
TemporalExtContainer TemporalExtContainer::Instance;
std::vector<TemporalExtData*> Container<TemporalExtData>::Array;

void Container<TemporalExtData>::Clear()
{
	Array.clear();
}

bool TemporalExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool TemporalExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

ASMJIT_PATCH(0x71A594, TemporalClass_CTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x71A5FF, TemporalClass_SDDTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x71B1DF, TemporalClass_SDDTOR, 0x7)

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F51DC, FakeTemporalClass::_Update);
DEFINE_FUNCTION_JUMP(LJMP, 0x71A760, FakeTemporalClass::_Update);
DEFINE_FUNCTION_JUMP(LJMP, 0x71AF20, FakeTemporalClass::_Detonate);

//ASMJIT_PATCH(0x71AB68, TemporalClass_Detach, 0x5)
//{
//	GET(TemporalClass*, pThis, ESI);
//	GET(AbstractClass*, target, EAX);
//
//	if (const auto pExt = TemporalExtContainer::Instance.TryFind(pThis))
//		pExt->InvalidatePointer(target, true);
//
//	return 0x0;
//}

HRESULT __stdcall FakeTemporalClass::_Load(IStream* pStm)
{
	HRESULT hr = this->TemporalClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = TemporalExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeTemporalClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->TemporalClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = TemporalExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5194, FakeTemporalClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5198, FakeTemporalClass::_Save)
