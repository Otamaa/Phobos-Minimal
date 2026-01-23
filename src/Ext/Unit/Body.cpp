#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/UnitType/Body.h>

#include <Locomotor/Cast.h>

#include <SlaveManagerClass.h>
#include <GameModeOptionsClass.h>
#include <Ext/Anim/Body.h>
#include <ScenarioClass.h>
#include <GameOptionsClass.h>

#include <Phobos.SaveGame.h>

UnitExtContainer UnitExtContainer::Instance;

bool UnitExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(UnitExtContainer::ClassName))
	{
		auto& container = root[UnitExtContainer::ClassName];

		for (auto& entry : container[UnitExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, UnitExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool UnitExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[UnitExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : UnitExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[UnitExtData::ClassName] = std::move(_extRoot);

	return true;
}

bool UnitExtContainer::HasDeployingAnim(TechnoTypeClass* pUnitType) {
	return pUnitType->DeployingAnim || !TechnoTypeExtContainer::Instance.Find(pUnitType)->DeployingAnims.empty();
}

bool UnitExtContainer::CheckDeployRestrictions(FootClass* pUnit, bool isDeploying)
{
	// Movement restrictions.
	if (isDeploying && pUnit->Locomotor->Is_Moving_Now())
		return true;

	FacingClass* currentDir = &pUnit->PrimaryFacing;
	bool isJumpjet = false;

	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
	{
		// Jumpjet rotating is basically half a guarantee it is also moving and
		// may not be caught by the Is_Moving_Now() check.
		if (isDeploying && pJJLoco->Facing.Is_Rotating())
			return true;

		currentDir = &pJJLoco->Facing;
		isJumpjet = true;
	}

	// Facing restrictions.
	auto const pType = GET_TECHNOTYPE(pUnit);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const defaultFacing = (FacingType)(RulesClass::Instance->DeployDir >> 5);
	auto const facing = pTypeExt->DeployDir.Get(defaultFacing);

	if (facing == FacingType::None || (!pTypeExt->DeployDir.isset() && !HasDeployingAnim(pType)))
		return false;

	if (facing != (FacingType)currentDir->Current().GetFacing<8>())
	{
		auto dir = DirStruct();
		dir.SetFacing<8>((size_t)facing);

		if (isDeploying)
		{
			static_cast<UnitClass*>(pUnit)->Deploying = true;

			if (isJumpjet)
				currentDir->Set_Desired(dir);

			pUnit->Locomotor->Do_Turn(dir);

			return true;
		}
		else
		{
			currentDir->Set_Desired(dir);
		}
	}

	return false;
}

void UnitExtContainer::CreateDeployingAnim(UnitClass* pUnit, bool isDeploying)
{
	if (!pUnit->DeployAnim)
	{
		auto const pType = pUnit->Type;
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pAnimType = !pTypeExt->DeployingAnims.empty() ?
			GeneralUtils::GetItemForDirection<AnimTypeClass*>(pTypeExt->DeployingAnims, pUnit->PrimaryFacing.Current())
			: pUnit->Type->DeployingAnim;

		auto const pAnim = GameCreate<AnimClass>(pAnimType, pUnit->Location, 0, 1, 0x600, 0,
			!isDeploying && pTypeExt->DeployingAnim_ReverseForUndeploy);

		pUnit->DeployAnim = pAnim;
		pAnim->SetOwnerObject(pUnit);
		AnimExtData::SetAnimOwnerHouseKind(pAnim, pUnit->Owner, nullptr, pUnit, false, true);
		auto const pExt = UnitExtContainer::Instance.Find(pUnit);

		if (pTypeExt->DeployingAnim_UseUnitDrawer)
		{
			pAnim->LightConvert = pUnit->GetRemapColour();
			pAnim->IsBuildingAnim = true; // Hack to make it use tint in drawing code.
		}

		// Set deploy animation timer. Required to be independent from animation lifetime due
		// to processing order / pointer invalidation etc. adding additional delay - simply checking
		// if the animation is still there wouldn't work well as it'd lag one frame behind I believe.
		const int rate = pAnimType->Normalized ?
			GameOptionsClass::Instance->GetAnimSpeed(pAnimType->Rate) :
			pAnimType->Rate;

		auto& timer = pExt->SimpleDeployerAnimationTimer;

		if (pAnimType->Reverse || pAnim->Reverse)
			timer.Start(pAnim->Animation.Stage * rate);
		else
			timer.Start(pAnimType->End * rate);
	}
}

void FakeUnitClass::_Deploy()
{
	auto pThis = this;
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return;

	if (!pThis->Deployed)
	{
		if (!pThis->InAir && pType->DeployToLand && pThis->GetHeight() > 0)
			pThis->InAir = true;

		if (pThis->Deploying && pThis->DeployAnim)
		{
			auto const pExt = UnitExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = true;
				pThis->Deploying = false;
			}
		}
		else if (!pThis->InAir)
		{
			if (UnitExtContainer::CheckDeployRestrictions(pThis, true))
				return;

			if (UnitExtContainer::HasDeployingAnim(pType))
			{
				UnitExtContainer::CreateDeployingAnim(pThis, true);
				pThis->Deploying = true;
			}
			else
			{
				pThis->Deployed = true;
				pThis->Deploying = false; // DeployDir != -1 + no DeployingAnim case needs this reset here.
			}
		}
	}

	if (pThis->Deployed)
	{
		int maxAmmo = -1;

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

		if (pTypeExt->Convert_Deploy)
			maxAmmo = pTypeExt->Convert_Deploy->Ammo;

		TechnoExtData::HandleOnDeployAmmoChange(pThis, maxAmmo);

		VocClass::SafeImmedietelyPlayAt(pType->DeploySound, pThis->Location);
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x739AC0 , FakeUnitClass::_Deploy)

void FakeUnitClass::_UnDeploy()
{
	auto pThis = this;
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return;

	if (pThis->Deployed)
	{
		if (pThis->Undeploying && pThis->DeployAnim)
		{
			auto const pExt = UnitExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = false;
				pThis->Undeploying = false;
				auto cell = CellStruct::Empty;
				pThis->NearbyLocation(&cell, pThis);
				auto const pCell = MapClass::Instance->GetCellAt(cell);
				pThis->SetDestination(pCell, true);
			}
		}
		else
		{
			if (UnitExtContainer::HasDeployingAnim(pType))
			{
				UnitExtContainer::CreateDeployingAnim(pThis, false);
				pThis->Undeploying = true;
			}
			else
			{
				pThis->Deployed = false;
			}
		}

		if (pThis->IsDisguised())
			pThis->Disguised = false;

		if (!pThis->Deployed)
		{
			TechnoExtData::HandleOnDeployAmmoChange(pThis);

			VocClass::SafeImmedietelyPlayAt(pType->UndeploySound, pThis->Location);
		}
	}

}

DEFINE_FUNCTION_JUMP(LJMP, 0x739CD0, FakeUnitClass::_UnDeploy)

int FakeUnitClass::_Mission_AreaGuard()
{
	auto pTypeExt = this->_GetTypeExtData();
	auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	if (this->SlaveManager
			&& !(nFrame < 0 || nFrame + this->CurrentMissionStartTime >= Unsorted::CurrentFrame)) {

		this->SlaveManager->Guard();
		return static_cast<int>((this->GetCurrentMissionControl()->Rate
				* 900) + ScenarioClass::Instance->Random(1, 5));

	} else {

		if (TechnoExtData::CannotMove(this)) {

			if (this->CanPassiveAcquireTargets() && this->TargetingTimer.Completed())
				this->TargetAndEstimateDamage(&this->Location, ThreatType::Range);

			int delay = 1;

			if (!this->Target) {
				this->UpdateIdleAction();
				delay = static_cast<int>(this->GetCurrentMissionControl()->Rate
					* 900) + ScenarioClass::Instance->Random(1, 5);
			}

			return delay;
		}
	}

	return FootClass::Mission_AreaGuard();
}

DEFINE_FUNCTION_JUMP(VTABLE , 0x7F5E90 , FakeUnitClass::_Mission_AreaGuard)
DEFINE_FUNCTION_JUMP(CALL , 0x744100, FakeUnitClass::_Mission_AreaGuard)

#include <Ext/WarheadType/Body.h>
#include <Ext/AnimType/Body.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Hooks.Otamaa.h>

#include <RadarEventClass.h>

DamageState FakeUnitClass::_Take_Damage(int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse)
{
	DamageState _res = DamageState::Unaffected;
	if (this->DeathFrameCounter > 0) {
		return _res;
	}

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(warhead);
	bool isPlayerControlled = this->Owner->ControlledByCurrentPlayer();
	bool selected = this->IsSelected && isPlayerControlled;

	if (!ignoreDefenses) {
		if (auto pBld = cast_to<BuildingClass*>(this->GetRadioContact())) {
			// #895584: ships not taking damage when repaired in a shipyard. bug
			// was that the logic that prevented units from being damaged when
			// exiting a war factory applied here, too. added the Naval check.
			if (pBld->Type->WeaponsFactory
				&& !pBld->Type->Naval
				&& MapClass::Instance->TryGetCellAt(this->Location)->GetBuilding() == pBld) {
				return _res;
			}
		}
	}

	_res = FakeFootClass::__Take_Damage(this, discard_t(), damage, distance, warhead, source, ignoreDefenses, PreventsPassengerEscape, sourceHouse);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (_res == DamageState::NowDead)
	{
		if (this->IsAttackedByLocomotor && this->Type->Crashable)
			this->IsAttackedByLocomotor = false;

		//this cause desync ?
		if (!this->Type->Voxel && this->Type->Strength > 0)
		{
			if (this->Type->MaxDeathCounter > 0
				&& !this->InLimbo
				&& !this->IsCrashing
				&& !this->IsSinking
				&& !this->TemporalTargetingMe
				&& !this->IsInAir()
				&& this->DeathFrameCounter <= 0
				)
			{

				this->Stun();
				const auto loco = this->Locomotor.GetInterfacePtr();

				if (loco->Is_Moving_Now())
					loco->Stop_Moving();

				this->DeathFrameCounter = 1;
			}
		}
	}

	if (_res != DamageState::PostMortem && this->DeathFrameCounter > 0) {
		return DamageState::PostMortem;
	}

	auto _CurCoord = this->GetCoords();
	auto _CurCell = CellClass::Coord2Cell(_CurCoord);
	auto pType = this->Type;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto pExt = TechnoExtContainer::Instance.Find(this);

	if (_res != DamageState::PostMortem)
	{
		if (_res != DamageState::NowDead)
		{
			if (_res != DamageState::Unaffected)
			{

				if (this->Type->Harvester
					&& pWHExt->Malicious
					&& !pWHExt->Nonprovocative
					&& this->Owner == HouseClass::CurrentPlayer()
					&& !this->Owner->IsObserver())
				{
					if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, _CurCell))
					{
						VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack());
					}
				}

				if (!this->HaveAttackMoveTarget
					&& source
					&& source->IsAlive
					&& !source->IsSinking
					&& !source->IsCrashing
					&& !source->TemporalTargetingMe
					&& !this->IsTethered
					&& this->Owner->IsAlliedWith(source->Owner)
					&& isPlayerControlled) {

					if (this->ShouldCrushIt(source)) {
						this->SetDestination(source, true);
						this->QueueMission(Mission::Move, false);
						return _res;
					}

					if ((pType->Harvester || pType->Weeder)
						&& this->GetPipFillLevel() > 0
						&& this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
					{
						TechnoClass* pDock = nullptr;
						for (int i = 0; i < pType->Dock.Count; ++i) {
							pDock = this->FindDockingBay(pType->Dock.Items[i], 0, false);
							if (pDock)
								break;
						}

						if (pType->Dock.Count > 0 && !pDock)
						{
							return _res;
						}

						if (!this->HasLinkOrFreeSlot(pDock) && this->SendCommand(RadioCommand::RequestLink, pDock) == RadioCommand::AnswerPositive) {
							this->QueueMission(Mission::Enter, false);
						}
					}
				}
			}

			return _res;
		}

		if (auto pBunker = cast_to<BuildingClass*>(this->BunkerLinkedItem)) {
			pBunker->ClearBunker();
		}

		if (pType->DeathFrames <= 0)
		{
			bool ShouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

			if (!pTypeExt->Sinkable.Get(ShouldSink)
			   || this->GetCell()->LandType != LandType::Water
			   || this->WarpingOut
			   || this->OnBridge
			   || this->GetHeight() > 0)
			{
				this->Destroyed(source);

				if (this->GetHeight() <= 10
				  && this->IsABomb
				  && (this->GetCell()->LandType == LandType::Water))
				{
					GameCreate<AnimClass>(RulesClass::Instance->Wake, this->Location, 0, 1, AnimFlag(0x600), 0, 0);
					auto coord_splash = this->Location;
					coord_splash += CoordStruct(0, 0, 5);
					GameCreate<AnimClass>(RulesClass::Instance->SplashList.Items[RulesClass::Instance->SplashList.Count - 1], this->Location, 0, 1, AnimFlag(0x600), 0, 0);
				} else {

					pExt->ReceiveDamage = true;
					AnimTypeExtData::ProcessDestroyAnims(this, source, warhead);
					this->Explode();
				}
			} else {
				this->Destroyed(source);
				this->Health = 1;
				this->IsAlive = 1;
				this->IsSinking = 1;
				this->Stun();
			}
		} else {
			if (this->DeathFrameCounter == -1) {
				this->DeathFrameCounter = 0;
				this->Destroyed(source);
			}

			this->Health = 1;
			this->IsAlive = 1;
		}

		this->Mark(MarkType::Remove);

		if (this->Passengers.NumPassengers > 0 && this->Passengers.GetFirstPassenger()) {
			if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit) {
				auto pPassenger = this->Passengers.GetFirstPassenger();
				auto pPassengerExt = TechnoExtContainer::Instance.Find(pPassenger);

				if (pPassengerExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pPassengerExt->OriginalPassengerOwner, false);

				while (pPassenger->NextObject)
				{
					pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
					pPassengerExt = TechnoExtContainer::Instance.Find(pPassenger);

					if (pExt->OriginalPassengerOwner)
						pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
				}
			}
		}

		if (pType->OpenTopped)
			this->MarkPassengersAsExited();

		TechnoExt_ExtData::SpawnSurvivors(this, source, selected, ignoreDefenses, PreventsPassengerEscape);

		if (GameModeOptionsClass::Instance->Crates)
		{
			if (pType->CrateGoodie
			  && (ScenarioClass::Instance->TruckCrate && !pType->IsTrain
				  || ScenarioClass::Instance->TrainCrate && pType->IsTrain))
			{

				const auto crate_cell = MapClass::Instance->NearByLocation(this->GetMapCoords(), SpeedType::Track, ZoneType::None, MovementZone::Normal, 0, 1, 1, true, false, false, true, CellStruct::Empty, false, false);
				if (crate_cell.IsValid())
				{
					MapClass::Instance->Place_Crate(crate_cell, PowerupEffects(0x14));
				}
			}
		}

		if ((!pType->Crashable || !this->Crash(source)) && !this->IsSinking)
			this->UnInit();
	}

	return _res;
}

ASMJIT_PATCH(0x73544D, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x7359DC, UnitClass_DTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeUnitClass::_Detach(AbstractClass* target, bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	this->UnitClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5C98, FakeUnitClass::_Detach)