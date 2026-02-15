#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#include <Utilities/Macro.h>

#include <Phobos.SaveGame.h>

#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>
#include <Ext/SWType/NewSuperWeaponType/Dominator.h>

#include <Notifications.h>

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
void SuperExtData::UpdateSuperWeaponStatuses(HouseClass* pHouse)
{
	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && !pHouse->IsObserver())
	{
		if (pHouse->Supers.Count > 0) {
			pHouse->Supers.for_each([pHouse](SuperClass* pSuper) {
				auto pExt = SuperExtContainer::Instance.Find(pSuper);
				pExt->Statusses.reset();

				// Update stockpile max based on building count
				auto pTypeExt = pExt->Type;

				if (pTypeExt->SW_Stockpile > 0) {
					if (pTypeExt->SW_Stockpile_TieToBuilding) {

						int buildingCount = 0;
						pHouse->Buildings.for_each([&](BuildingClass* pBld) {
							 if (pBld->IsAlive && !pBld->InLimbo
								 && BuildingExtContainer::Instance.Find(pBld)->HasSuperWeapon(pSuper->Type->ArrayIndex, true)
								 ) {
								 buildingCount++;
							 }
							});
						pExt->StockpileMax = std::min(buildingCount, (int)pTypeExt->SW_Stockpile);
					} else {
						pExt->StockpileMax = pTypeExt->SW_Stockpile;
					}

					// Clamp if buildings destroyed
					if (pExt->StockpileCount > pExt->StockpileMax)
						pExt->StockpileCount = pExt->StockpileMax;
				}

				//if AlwaysGranted and SWAvaible
				pExt->Statusses.PowerSourced = !pSuper->IsPowered();
				if (pExt->Type->SW_AlwaysGranted && pExt->Type->IsAvailable(pHouse))
				{
					pExt->Statusses.Available = true;
					pExt->Statusses.Charging = true;
					pExt->Statusses.PowerSourced = true;
				}
			});
		}

		pHouse->Buildings.for_each([=](BuildingClass* pBld) {
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				bool PowerChecked = false;
				bool HasPower = false;

				// check for upgrades. upgrades can give super weapons, too.
				for (const auto type : pBld->GetTypes())
				{
					if (auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(const_cast<BuildingTypeClass*>(type)))
					{
						for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i)
						{
							const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i);

							if (idxSW >= 0)
							{
								const auto pSuperExt = SuperExtContainer::Instance.Find(pHouse->Supers[idxSW]);
								auto& status = pSuperExt->Statusses;

								if (!status.Charging)
								{
									if (pSuperExt->Type->IsAvailable(pHouse))
									{
										status.Available = true;

										if (!PowerChecked)
										{
											HasPower = pBld->HasPower
												&& !pBld->IsUnderEMP()
												&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExt_ExtData::IsOperated(pBld));

											PowerChecked = true;
										}

										if (!status.Charging && HasPower)
										{
											status.PowerSourced = true;

											if (!pBld->IsBeingWarpedOut()
												&& (pBld->CurrentMission != Mission::Construction)
												&& (pBld->CurrentMission != Mission::Selling)
												&& (pBld->QueuedMission != Mission::Construction)
												&& (pBld->QueuedMission != Mission::Selling))
											{
												status.Charging = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		});

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		const bool hasPower = pHouse->HasFullPower();
		const bool isCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
		const bool bIsSWShellEnabled = Unsorted::SWAllowed || isCampaign;

		if (!hasPower || !bIsSWShellEnabled) {
			pHouse->Supers.for_each([&](SuperClass* pSuper) {

				const auto pExt = SuperExtContainer::Instance.Find(pSuper);
				auto& nStatus = pExt->Statusses;

				// turn off super weapons that are disallowed.
				if (!bIsSWShellEnabled && pSuper->Type->DisableableFromShell)
				{
					nStatus.Available = false;
				}

				// if the house is generally on low power,
				// powered super weapons aren't powered
				if (!hasPower && pSuper->IsPowered())
				{
					nStatus.PowerSourced &= hasPower;
				}
			});
		}
	}
}


// =============================
// load / save

template <typename T>
void SuperExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Name)
		.Process(this->Type, true)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)
		.Process(this->CameoFirstClickDone)
		.Process(this->FirstClickAutoFireDone)
		//.Process(this->Firer)
		.Process(this->Statusses)

		.Process(this->MusicTimer)
		.Process(this->MusicActive)

		.Process(this->StockpileCount)
		.Process(this->StockpileMax)
		.Process(this->LastLaunchBuildingIndex)
		;
}

// =============================
// container
SuperExtContainer SuperExtContainer::Instance;

bool SuperExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(SuperExtContainer::ClassName))
	{
		auto& container = root[SuperExtContainer::ClassName];

		for (auto& entry : container[SuperExtData::ClassName])
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

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, SuperExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool SuperExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[SuperExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : SuperExtContainer::Array)
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

	first_layer[SuperExtData::ClassName] = std::move(_extRoot);

	return true;
}

// .cpp file
SuperExtData::SuperExtData(SuperClass* pObj) : AbstractExtended(pObj)
	// Large aggregates
	, Name()
	, Statusses()
	, MusicTimer()
	// Pointer
	, Type(nullptr)
	, SelectedFirer(nullptr)
	// int
	, StockpileCount(0)
	, StockpileMax(-1)
	, LastLaunchBuildingIndex(0)
	// CellStruct
	, Temp_CellStruct()
	// bools
	, Temp_IsPlayer(false)
	, CameoFirstClickDone(false)
	, FirstClickAutoFireDone(false)
	, MusicActive(false)
{
	this->Type = SWTypeExtContainer::Instance.Find(pObj->Type);
	this->Name = pObj->Type->ID;
	this->AbsType = SuperClass::AbsID;
}
// =============================
// container hooks

ASMJIT_PATCH(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);

	SuperExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x6CB1BD, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeSuperClass::_Detach(AbstractClass* target, bool all)
{
	this->_GetExtData()->InvalidatePointer(target, all);
	this->SuperClass::PointerExpired(target , all);
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4010, FakeSuperClass::_Detach)

const wchar_t* FakeSuperClass::_NameReadiness()
{
	const auto pData = this->_GetTypeExtData();

	// complete rewrite of this method.

	Valueable<CSFText>* text = &pData->Text_Preparing;

	if (this->IsOnHold)
	{
		// on hold
		text = &pData->Text_Hold;
	}
	else
	{
		if (this->Type->UseChargeDrain)
		{
			switch (this->ChargeDrainState)
			{
			case ChargeDrainState::Charging:
				// still charging
				text = &pData->Text_Charging;
				break;
			case ChargeDrainState::Ready:
				// ready
				text = &pData->Text_Ready;
				break;
			case ChargeDrainState::Draining:
				// currently active
				text = &pData->Text_Active;
				break;
			}
		}
		else
		{
			// ready
			if (this->IsCharged)
			{
				text = &pData->Text_Ready;
			}
		}
	}

	return (*text)->empty() ? L"" : (*text)->Text;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC2B0, FakeSuperClass::_NameReadiness)

int FakeSuperClass::_GetAnimStage()
{
	if (!this->Granted)
	{
		return 0;
	}

	auto pType = this->Type;
	auto pTypeExt = SWTypeExtContainer::Instance.Find(pType);
	// Get effective recharge time
	int customRechargeTime = this->CustomChargeTime;
	int rechargeTime = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;

	// Calculate remaining delay time
	int delayTime = this->RechargeTimer.TimeLeft;
	int started = this->RechargeTimer.StartTime;

	if (started != -1)
	{
		int elapsed = Unsorted::CurrentFrame - started;
		if (elapsed >= delayTime)
		{
			delayTime = 0;
		}
		else
		{
			delayTime -= elapsed;
		}
	}

	// Calculate progress ratio
	double progress = 0.0;
	if (pTypeExt->UseWeeds)
	{
		if (this->IsCharged)
			return 54;

		if (pTypeExt->UseWeeds_StorageTimer)
		{
			int wprogress = int(54.0 * this->Owner->OwnedWeed.GetTotalAmount() / (double)pTypeExt->UseWeeds_Amount);

			if (wprogress > 54)
				wprogress = 54;

			return wprogress;
		}
		else
		{
			return 0;
		}
	}

	if (pType->UseChargeDrain)
	{
		if (this->ChargeDrainState == ChargeDrainState::Draining)
		{
			// Draining state - reverse progress
			// [HOOK APPLIED: 0x6CBF5B - SuperClass_GetCameoChargeStage_ChargeDrainRatio]
			int rechargeTime1 = rechargeTime;
			int rechargeTime2 = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
			int timeLeft = delayTime;

			// Use per-SW charge-to-drain ratio instead of Rule->ChargeToDrainRatio

			const double ratio = pTypeExt->GetChargeToDrainRatio();

			if (Math::abs(rechargeTime2 * ratio) > 0.001)
			{
				progress = 1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio);
			}
			else
			{
				progress = 0.0;
			}
		}
		else
		{
			// Charging state
			int divisor = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
			progress = (double)(rechargeTime - delayTime) / divisor;
		}
	}
	else
	{
		// Non-ChargeDrain mode

		// Stockpile: show progress toward NEXT charge, not just "full"
		auto pTypeExt_ = SWTypeExtContainer::Instance.Find(pType);
		if (pTypeExt_->SW_Stockpile > 0)
		{
			auto pSuperExt = SuperExtContainer::Instance.Find(this);
			if (pSuperExt->StockpileCount >= pSuperExt->StockpileMax && pSuperExt->StockpileMax > 0)
			{
				return 54; // Full — all charges stocked
			}
			// Show progress toward next charge (timer is still running)
			int divisor = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
			if (divisor > 0)
			{
				progress = (double)(rechargeTime - delayTime) / divisor;
				int stage = (int)(progress * 54.0);
				if (stage > 54) stage = 54;
				if (stage < 0) stage = 0;
				return stage;
			}
			return 0;
		}

		if (this->IsCharged)
		{
			return 54;
		}

		// Normal charging
		int divisor = (customRechargeTime == -1) ? pType->RechargeTime : customRechargeTime;
		progress = (double)(rechargeTime - delayTime) / divisor;
	}

	// Convert to stage
	// [HOOK APPLIED: 0x6CC053 - SuperClass_GetCameoChargeStage_FixFullyCharged]
	int stage = (int)(progress * 54.0);

	// Clamp to 0-54 (original clamped to 53, hook fixes to 54)
	if (stage > 54)
	{
		stage = 54;
	}

	return stage;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CBEE0, FakeSuperClass::_GetAnimStage)
DEFINE_FUNCTION_JUMP(CALL, 0x6CBE7E, FakeSuperClass::_GetAnimStage)
DEFINE_FUNCTION_JUMP(CALL, 0x6CBE8A, FakeSuperClass::_GetAnimStage)

void FakeSuperClass::_Place(CellStruct* cell, bool player)
{
	//Debug::LogInfo("[%s - %x] Lauch [%s - %x] ", pSuper->Owner->get_ID() , pSuper->Owner, pSuper->Type->ID, pSuper);
	if (SWTypeExtData::Activate(this, *cell, player)) {
		this->_GetTypeExtData()->FireSuperWeapon(this, this->Owner, cell, player);
	}

	//Debug::LogInfo("Lauch [%x][%s] %s failed ", pSuper, pSuper->Owner->get_ID(), pSuper->Type->ID);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC390, FakeSuperClass::_Place)

bool FakeSuperClass::_IsToFlashTab()
{
	if (!this->_GetTypeExtData()->SuperWeaponSidebar_Allow)
		return false;

	if (this->IsOnHold) {
		return false;
	}

	if (this->Type->UseChargeDrain) {
		if (this->ChargeDrainState == ChargeDrainState::Charging) {
			return false;
		}
	} else if (!this->IsCharged) {
		return false;
	}

	int FlashSidebarTabFrames = this->Type->FlashSidebarTabFrames;
	if (FlashSidebarTabFrames == -1) {
		return true;
	}

	return FlashSidebarTabFrames && FlashSidebarTabFrames + this->ReadinessFrame > Unsorted::CurrentFrame;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CE1A0, FakeSuperClass::_IsToFlashTab)

bool FakeSuperClass::_AI(bool isPlayer)
{
	// Tick down special sound duration
	if (this->SpecialSoundDuration > 0)
		this->SpecialSoundDuration--;

	if (this->SpecialSoundDuration == 0) {
		this->SpecialSoundDuration = -1;
		VocClass::SafeImmedietelyPlayAt(this->Type->SpecialSound, &this->SpecialSoundLocation, 0);
	}

	// === Hook: SuperClass_AI_Animation (0x6CBCDE) ===
	// Dynamic ChronoWarp check instead of hardcoded SPC_CHRONOWARP == 4
	{
		bool isChronoWarp = false;
		int curSW = Unsorted::CurrentSWType;

		if (static_cast<size_t>(curSW) < static_cast<size_t>(SuperWeaponTypeClass::Array->Count))
			isChronoWarp = (SuperWeaponTypeClass::Array->Items[curSW]->Type == SuperWeaponType::ChronoWarp);

		if (!isChronoWarp && this->Animation != nullptr)
		{
			this->Animation->Invisible = true;
			this->Animation->Audio3.AudioEventHandleEnd();
		}
	}

	// Early out checks
	if (!this->Granted)
		return false;

	if (this->IsCharged && !this->Type->UseChargeDrain)
		return false;

	if (this->IsOnHold)
		return false;

	// Timer not running — check if CameoChargeState needs reset
	if (this->RechargeTimer.StartTime == -1)
	{
		if (this->CameoChargeState != -1)
		{
			this->CameoChargeState = -1;
			return true;
		}
		return false;
	}

	// === Hook: SuperClass_AI_UseWeeds (0x6CBD2C) ===
	const auto pTypeExt = this->_GetTypeExtData();
	const bool isStockpile = pTypeExt->SW_Stockpile > 0;
	bool forceCharged = false;

	// Stockpile handling
	if (isStockpile)
	{
		auto pSuperExt = SuperExtContainer::Instance.Find(this);

		if (pSuperExt->StockpileMax <= 0)
			return false;

		if (this->IsCharged)
		{
			if (pSuperExt->StockpileCount >= pSuperExt->StockpileMax)
				return false;

			// Accumulate next charge
			if (pTypeExt->UseWeeds)
			{
				if (this->Owner->OwnedWeed.GetTotalAmount() >= pTypeExt->UseWeeds_Amount)
				{
					this->Owner->OwnedWeed.RemoveAmount(
						static_cast<float>(pTypeExt->UseWeeds_Amount), 0);
					pSuperExt->StockpileCount++;
					return true;
				}
			}
			else
			{
				if (this->RechargeTimer.GetTimeLeft() <= 0)
				{
					pSuperExt->StockpileCount++;
					if (pSuperExt->StockpileCount < pSuperExt->StockpileMax)
						this->RechargeTimer.Start(this->GetRechargeTime());
					return true;
				}
			}

			// Charge not ready yet — check cameo update
			int animStage = this->GetCameoChargeState();
			if (this->CameoChargeState != animStage)
			{
				this->CameoChargeState = animStage;
				return true;
			}
			return false;
		}
		// Not yet charged — fall through to UseWeeds or vanilla timer
	}

	// UseWeeds: charge from weed storage instead of timer
	if (pTypeExt->UseWeeds)
	{
		if (this->Owner->OwnedWeed.GetTotalAmount() >= pTypeExt->UseWeeds_Amount)
		{
			this->Owner->OwnedWeed.RemoveAmount(
				static_cast<float>(pTypeExt->UseWeeds_Amount), 0);
			this->RechargeTimer.Start(0);
			forceCharged = true; // skip drain checks, go straight to charged
		}
		else
		{
			// Not enough weeds — set a fake timer for cameo animation
			const int rechargerValue =
				this->Owner->OwnedWeed.GetTotalAmount() >=
				pTypeExt->UseWeeds_ReadinessAnimationPercentage
				/ (100.0 * pTypeExt->UseWeeds_Amount)
				? 15 : 915;

			this->RechargeTimer.Start(rechargerValue);

			int animStage = this->GetCameoChargeState();
			if (this->CameoChargeState != animStage)
			{
				this->CameoChargeState = animStage;
				return true;
			}
			return false;
		}
	}

	if (!forceCharged)
	{
		// Vanilla timer: compute remaining time
		int remainingDelay = this->RechargeTimer.GetTimeLeft();

		// === Hook: SuperClass_AI_DrainMoney (0x6CBD6B) ===
		// Drain money/battle points while active; stop drain if insufficient
		if (!pTypeExt->ApplyDrainMoney(remainingDelay, this->Owner))
			remainingDelay = 0; // force timer complete (stop drain)
		else if (!pTypeExt->ApplyDrainBattlePoint(remainingDelay, this->Owner))
			remainingDelay = 0;

		// Timer still running — check cameo stage update
		if (remainingDelay != 0)
		{
			int stage = this->GetCameoChargeState();
			if (stage != this->CameoChargeState)
			{
				this->CameoChargeState = this->GetCameoChargeState();
				return true;
			}
			return false;
		}
	}

	// === Timer completed (or forced by UseWeeds / drain failure) ===
	SuperWeaponTypeClass* pType = this->Type;

	if (pType->UseChargeDrain)
	{
		if (this->ChargeDrainState == ChargeDrainState::Draining)
		{
			// === Hook: SuperClass_AI_Progress_Charged (0x6CBD86) ===
			// Deactivate replaces the original ChargeDrainState = None
			SWTypeExtData::Deactivate(this, CellStruct::Empty, true);
			this->RechargeTimer.Start(this->GetRechargeTime());
			return true;
		}
		else
		{
			// Charging finished — become ready
			this->ChargeDrainState = ChargeDrainState::Ready;
			this->IsCharged = true;
			return true;
		}
	}

	// Non-ChargeDrain: fully charged
	this->IsCharged = true;

	// === Hook: SuperClass_AI_AnnounceReady (0x6CBDD7) ===
	// Custom message + EVA replaces hardcoded ActsLike switch
	if (isPlayer) {
		pTypeExt->PrintMessage(pTypeExt->Message_Ready, HouseClass::CurrentPlayer);

		if (pTypeExt->EVA_Ready != -1)
			VoxClass::PlayIndex(pTypeExt->EVA_Ready);
	}

	this->ReadinessFrame = Unsorted::CurrentFrame();
	return true;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CBCA0, FakeSuperClass::_AI)

void FakeSuperClass::_SetCharge(int charge)
{
	const auto pTypeExt = this->_GetTypeExtData();

	// When stockpile SW first becomes charged, init the stockpile count
	// and restart the timer if we haven't reached max yet
	if (pTypeExt->SW_Stockpile > 0)
	{
		auto pSuperExt = this->_GetExtData();

		if (pSuperExt->StockpileCount <= 0)
		{
			pSuperExt->StockpileCount = 1;
		}

		// Keep charging for more stockpile
		if (pSuperExt->StockpileCount < pSuperExt->StockpileMax) {
			this->RechargeTimer.Start(this->GetRechargeTime());
		}

		// Let vanilla proceed to set IsCharged = true
		// return 0;
	}

	if (pTypeExt->UseWeeds || !this->Granted || charge < 0 || charge > 100)
		return;

	// Resolve recharge time
	const int rechargeTime = this->GetRechargeTime();
	// Remaining delay = rechargeTime * (1.0 - charge/100.0)
	const int remainingDelay = rechargeTime - static_cast<int>(charge * 0.01f * rechargeTime);

	if (remainingDelay == 0)
		this->IsCharged = true;

	this->RechargeTimer.Start(remainingDelay);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC1E0, FakeSuperClass::_SetCharge)

void FakeSuperClass::_Forced_Charge(bool isPlayer)
{
	if (!this->Granted)
		return;

	int charge = this->GetRechargeTime();
	this->IsCharged = true;
	this->RechargeTimer.Start(0);
	const auto pData = this->_GetTypeExtData();

	if (isPlayer) {
		pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);

		if (pData->EVA_Ready != -1) {
			VoxClass::PlayIndex(pData->EVA_Ready);
		}
	}

	this->ReadinessFrame = Unsorted::CurrentFrame();
	if (this->Type->UseChargeDrain)
		this->ChargeDrainState = ChargeDrainState::Ready;

}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC080, FakeSuperClass::_Forced_Charge)

bool FakeSuperClass::_Remove()
{
	auto ret = false;

	if (this->Granted)
	{
		this->IsCharged = false;
		this->Granted = false;

		if (SuperClass::ShowTimers->erase(this))
		{
			std::ranges::sort(*SuperClass::ShowTimers,
			[](SuperClass* a, SuperClass* b) {
				 const auto aExt = SWTypeExtContainer::Instance.Find(a->Type);
				 const auto bExt = SWTypeExtContainer::Instance.Find(b->Type);
				 return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
			});
		}

		// changed
		if (this->Type->UseChargeDrain && this->ChargeDrainState == ChargeDrainState::Draining)
		{
			SWTypeExtData::Deactivate(this, CellStruct::Empty, false);
			this->ChargeDrainState = ChargeDrainState::Charging;
		}

		ret = true;
	}

	return ret;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB7B0, FakeSuperClass::_Remove)

bool FakeSuperClass::_Discharged(bool isPlayer, CellStruct* pCell)
{

	auto const pType = this->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);
	auto const pOwner = this->Owner;
	auto const pHouseExt = HouseExtContainer::Instance.Find(pOwner);

	if (!pType->UseChargeDrain)
	{
		//change done
		if ((this->RechargeTimer.StartTime < 0
			|| !this->Granted
			|| !this->IsCharged)
			&& !pType->PostClick)
		{
			return false;
		}

		// auto-abort if no resources
		if (!pOwner->CanTransactMoney(pExt->Money_Amount))
		{
			if (pOwner->IsCurrentPlayer())
			{
				pExt->UneableToTransactMoney(pOwner);
			}
			return false;
		}

		if (!pHouseExt->CanTransactBattlePoints(pExt->BattlePoints_Amount))
		{
			if (pOwner->IsCurrentPlayer())
			{
				pExt->UneableToTransactBattlePoints(pOwner);
			}
			return false;
		}

		auto pNewType = SWTypeHandler::get_Handler(pExt->HandledType);

		// can this super weapon fire now?
		if (pNewType->AbortFire(this, isPlayer))
		{
			return false;
		}

		this->Launch(*pCell, isPlayer);

		// the others will be reset after the PostClick SW fired
		if (!pType->PostClick && !pType->PreClick)
		{
			this->IsCharged = false;
		}

		if (this->OneTime || !pExt->CanFire(pOwner))
		{
			// remove this SW
			this->OneTime = false;
			return this->Lose();
		}
		else if (pType->ManualControl)
		{
			// set recharge timer, then pause
			const auto time = this->GetRechargeTime();
			this->CameoChargeState = -1;
			this->RechargeTimer.Start(time);
			this->RechargeTimer.Pause();

		}
		else if (!pType->PreClick && !pType->PostClick)
		{
			this->StopPreclickAnim(isPlayer);
		}
	}
	else
	{
		if (this->ChargeDrainState == ChargeDrainState::Draining)
		{
			// deactivate for human players
			this->ChargeDrainState = ChargeDrainState::Ready;
			auto const left = this->RechargeTimer.GetTimeLeft();

			auto const duration = int(this->GetRechargeTime()
				- (left / pExt->GetChargeToDrainRatio()));
			this->RechargeTimer.Start(duration);
			pExt->Deactivate(this, *pCell, isPlayer);
		}
		else if (this->ChargeDrainState == ChargeDrainState::Ready)
		{
			// activate for human players
			this->ChargeDrainState = ChargeDrainState::Draining;
			auto const left = this->RechargeTimer.GetTimeLeft();

			auto const duration = int(
					(this->GetRechargeTime() - left)
					* pExt->GetChargeToDrainRatio());
			this->RechargeTimer.Start(duration);

			this->Launch(*pCell, isPlayer);
		}
	}

	return false;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB920, FakeSuperClass::_Discharged)

bool FakeSuperClass::_Suspend(bool on)
{
	auto ret = false;

	if (this->Granted
		&& !this->OneTime
		&& this->CanHold
		&& on != this->IsOnHold)
	{
		if (on || this->Type->ManualControl)
		{
			this->RechargeTimer.Pause();
		}
		else
		{
			this->RechargeTimer.Resume();
		}

		this->IsOnHold = on;

		if (this->Type->UseChargeDrain)
		{
			if (on)
			{
				if (this->ChargeDrainState == ChargeDrainState::Draining)
				{
					SWTypeExtData::Deactivate(this, CellStruct::Empty, false);
					const auto nTime = this->GetRechargeTime();
					const auto nRation = this->RechargeTimer.GetTimeLeft()
						/ SWTypeExtContainer::Instance.Find(this->Type)->GetChargeToDrainRatio();
					this->RechargeTimer.Start(int(nTime - nRation));
					this->RechargeTimer.Pause();
				}

				this->ChargeDrainState = ChargeDrainState::None;
			}
			else
			{
				const auto pExt = SWTypeExtContainer::Instance.Find(this->Type);

				this->ChargeDrainState = ChargeDrainState::Charging;

				if (!pExt->SW_InitialReady || HouseExtContainer::Instance.Find(this->Owner)
					->GetShotCount(this->Type).Count)
				{
					this->RechargeTimer.Start(this->GetRechargeTime());
				}
				else
				{
					this->ChargeDrainState = ChargeDrainState::Ready;
					this->ReadinessFrame = Unsorted::CurrentFrame();
					this->IsCharged = true;
					//this->IsOnHold = false;
				}
			}
		}

		ret = true;
	}

	return ret;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB4D0, FakeSuperClass::_Suspend)

bool FakeSuperClass::_Grant(bool oneTime, bool announce, bool onHold)
{
	if (this->Granted)
		return false;

	SuperWeaponTypeClass* pType = this->Type;
	bool result = false;

	this->Granted = true;
	this->OneTime = oneTime;
	this->BlinkState = false;
	this->unused_3C = pType->UIName; // UIName token

	// Resume from suspended (on-hold) state
	if (!oneTime && this->IsOnHold && this->CanHold)
	{
		if (!pType->ManualControl)
		{
			if (this->RechargeTimer.StartTime == -1)
				this->RechargeTimer.StartTime = Unsorted::CurrentFrame();
		}
		else
		{
			if (this->RechargeTimer.StartTime != -1)
			{
				const int remaining = (int)this->RechargeTimer.GetTimeLeft();
				this->RechargeTimer.TimeLeft = remaining;
				this->RechargeTimer.StartTime = -1;
			}
		}

		this->IsOnHold = false;
	}

	// === Hook: SuperClass_Grant_ShowTimer (0x6CB5EB) ===
	// Add to ShowTimers if type is visible and house is not observer
	if (pType->ShowTimer && !this->Owner->Type->MultiplayPassive)
	{
		if (SuperClass::ShowTimers->push_back(this))
		{
			std::ranges::sort(*SuperClass::ShowTimers,
				[](SuperClass* a, SuperClass* b)
				{
					const auto aExt = SWTypeExtContainer::Instance.Find(a->Type);
					const auto bExt = SWTypeExtContainer::Instance.Find(b->Type);
					return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
				});
		}
	}

	if (pType->ManualControl)
	{
		// ManualControl: initialize timer in paused state
		int rechargeTime = this->GetRechargeTime();

		this->CameoChargeState = -1;

		this->RechargeTimer.StartTime = Unsorted::CurrentFrame();
		this->RechargeTimer.TimeLeft = rechargeTime;

		// Immediately pause: snapshot remaining and stop
		if (this->RechargeTimer.StartTime != -1)
		{
			int elapsed = Unsorted::CurrentFrame() - this->RechargeTimer.StartTime;
			if (elapsed >= this->RechargeTimer.TimeLeft)
				this->RechargeTimer.TimeLeft = 0;
			else
				this->RechargeTimer.TimeLeft -= elapsed;

			this->RechargeTimer.StartTime = -1;
		}
	}
	else
	{
		// Standard (non-ManualControl) initialization

		// Clean up existing animation tracking
		if (this->Animation != nullptr) {
			this->Animation->RemainingIterations = 0;
			this->Animation = nullptr;
			PointerExpiredNotification::NotifyInvalidAnim->Remove(this);
		}

		if (this->AnimationGotInvalid) {
			PointerExpiredNotification::NotifyInvalidAnim->Remove(this);
			this->AnimationGotInvalid = false;
		}

		// Start recharge timer if conditions allow
		if (this->Granted && !this->IsCharged
			&& (!this->IsOnHold || pType->PreClick))
		{
			// === Hook: SuperClass_Grant_InitialReady (0x6CB70C) ===
			this->CameoChargeState = -1;

			if (pType->UseChargeDrain)
				this->ChargeDrainState = ChargeDrainState::Charging;

			auto pHouseExt = HouseExtContainer::Instance.Find(this->Owner);
			const auto pSuperExt = SWTypeExtContainer::Instance.Find(pType);

			if (pSuperExt->SW_Stockpile > 0)
			{
				auto pExt = SuperExtContainer::Instance.Find(this);
				if (pExt->StockpileMax < 0)
					pExt->StockpileMax = pSuperExt->SW_Stockpile;
			}

			auto const [frame, count] = pHouseExt->GetShotCount(pType);
			const int nCharge = (!pSuperExt->SW_InitialReady || count)
				? this->GetRechargeTime() : 0;

			this->RechargeTimer.Start(nCharge);

			auto nFrame = Unsorted::CurrentFrame();
			if (pSuperExt->SW_VirtualCharge)
			{
				if ((frame & 0x80000000) == 0)
				{
					this->RechargeTimer.StartTime = frame;
					nFrame = frame;
				}
			}

			if (nFrame != -1)
			{
				auto nTimeLeft = nCharge + nFrame - Unsorted::CurrentFrame();
				if (nTimeLeft <= 0)
				{
					this->IsCharged = true;
					this->ReadinessFrame = Unsorted::CurrentFrame();
					if (pType->UseChargeDrain)
						this->ChargeDrainState = ChargeDrainState::Ready;
				}
			}

			pHouseExt->UpdateShotCountB(pType);
			result = true;
		}
	}

	// Quiet/onHold mode: suspend timer (only for non-onetime supers)
	if (onHold && this->Granted && !this->OneTime
		&& !this->IsOnHold && this->CanHold)
	{
		this->RechargeTimer.Stop();
		this->IsOnHold = true;
	}

	// OneTime supers always force-charge immediately
	if (this->OneTime)
		this->ClickFire(announce);

	return result;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB560, FakeSuperClass::_Grant)