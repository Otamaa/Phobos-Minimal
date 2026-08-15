#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Macro.h>

#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>
#include <Ext/SWType/NewSuperWeaponType/Dominator.h>

#include <Notifications.h>
void NOINLINE SWChargePool::EnsureAccumulating(SuperClass* pThis)
{
	// Accumulation is desired again — re-enable the flag first
	SWChargePool::Get(pThis->Owner, pThis->Type)->IsAllowedToAccumulate = true;

	if (pThis->IsOnHold || pThis->RechargeTimer.IsTicking())
		return;

	if (pThis->RechargeTimer.TimeLeft > 0)
	{
		// Paused mid-cycle — Resume() keeps the remaining time.
		// Start() here would throw away accumulated progress.
		pThis->RechargeTimer.Resume();
	}
	else
	{
		// Fully stopped — begin a fresh cycle
		const int t = pThis->GetRechargeTime();
		pThis->RechargeTimer.Start((t > 0) ? t : 1);
	}
}

void NOINLINE SWChargePool::SetPoolFull(SuperClass* pThis)
{
	pThis->IsCharged = true;
	if (pThis->RechargeTimer.IsTicking())
		pThis->RechargeTimer.Pause();   // freeze visually at 0

	SWChargePool::Get(pThis->Owner, pThis->Type)->IsAllowedToAccumulate = false;
}

void NOINLINE SWChargePool::BeginRecharge(SuperClass* pThis)
{
	pThis->IsCharged = false;
	SWChargePool::Get(pThis->Owner, pThis->Type)->IsAllowedToAccumulate =  true;
	const int t = pThis->GetRechargeTime();
	pThis->RechargeTimer.Start((t > 0) ? t : 1);
}

static int SWCharges_GetPerCycle(SuperWeaponTypeClass* pType)
{
	const auto pTypeExt = SWTypeExtContainer::Instance.Find(pType);
	const int perCycle = pTypeExt->SW_ChargesPerCycle;
	return (perCycle < 1) ? 1 : perCycle;  // never below 1
}

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
void SuperExtData::UpdateSuperWeaponStatuses(HouseClass* pHouse)
{
	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && !pHouse->IsObserver()) {
		if (pHouse->Supers.Count > 0) {
			pHouse->Supers.for_each([pHouse](SuperClass* pSuper) {
				auto pExt = SuperExtContainer::Instance.Find(pSuper);
				pExt->Statusses.reset();

				//if AlwaysGranted and SWAvaible
				pExt->Statusses.PowerSourced = !pSuper->IsPowered();

				if (pExt->Type->SW_AlwaysGranted && SWTypeExtData::IsAvailable(pHouse, pSuper)) {
					pExt->Statusses.SetAllState(true);
				}
			});
		}

		pHouse->Buildings.for_each([=](BuildingClass* pBld) {
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				bool PowerChecked = false;
				bool HasPower = false;
				auto pBldExt = BuildingExtContainer::Instance.Find(pBld);

				if(!pBldExt->Supers.empty()){ 
					for (auto pSuper : pBldExt->Supers) {
						const auto pSuperExt = SuperExtContainer::Instance.Find(pSuper);
						auto& status = pSuperExt->Statusses;

						if (!status.Charging) {
							if (SWTypeExtData::IsAvailable(pHouse, pSuper)) {
								status.Available = true;

								if (!PowerChecked) {
									HasPower = pBld->HasPower
										&& !pBld->IsUnderEMP()
										&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExtData::IsOperated(pBld));

									PowerChecked = true;
								}

								if (!status.Charging && HasPower) {
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
				}else{
					// check for upgrades. upgrades can give super weapons, too.
					for (const auto type : pBld->GetTypes()) {
						if (auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(const_cast<BuildingTypeClass*>(type))) {
							for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i) {
								const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i);

								if (idxSW >= 0) {
									auto pSuper = pHouse->Supers[idxSW];
									const auto pSuperExt = SuperExtContainer::Instance.Find(pSuper);
									auto& status = pSuperExt->Statusses;

									if (!status.Charging) {
										if (SWTypeExtData::IsAvailable(pHouse, pSuper)) {
											status.Available = true;

											if (!PowerChecked) {
												HasPower = pBld->HasPower
													&& !pBld->IsUnderEMP()
													&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExtData::IsOperated(pBld));

												PowerChecked = true;
											}

											if (!status.Charging && HasPower) {
												status.PowerSourced = true;

												if (!pBld->IsBeingWarpedOut()
													&& (pBld->CurrentMission != Mission::Construction)
													&& (pBld->CurrentMission != Mission::Selling)
													&& (pBld->QueuedMission != Mission::Construction)
													&& (pBld->QueuedMission != Mission::Selling)) {
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
			}
		});

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		const bool hasPower = pHouse->HasFullPower();
		const bool isCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
		const bool bIsSWShellEnabled = Unsorted::SWAllowed() || isCampaign;

		pHouse->Supers.for_each([&](SuperClass* pSuper) {

			if (!hasPower || !bIsSWShellEnabled) {
				const auto pExt = SuperExtContainer::Instance.Find(pSuper);
				auto& nStatus = pExt->Statusses;

				// turn off super weapons that are disallowed.
				if (!bIsSWShellEnabled && pSuper->Type->DisableableFromShell) {
					nStatus.Available = false;
				}

				// if the house is generally on low power,
				// powered super weapons aren't powered
				if (!hasPower && pSuper->IsPowered()) {
					nStatus.PowerSourced &= hasPower;
				}
			}

			if (pSuper->Granted) {
				const int maxCharges = SWChargePool::GetMax(pSuper->Type);
				if (maxCharges >= 0) {
					const int dischargeAmt = SWChargePool::GetDischarge(pSuper->Type);
					auto pPool = SWChargePool::Get(pHouse, pSuper->Type);

					if (pPool->Charges >= maxCharges) {
						// Pool full — freeze timer
						SWChargePool::SetPoolFull(pSuper);
					} else if (pPool->Charges >= dischargeAmt) {
						// Ready to fire, still accumulating.
						// Keep IsCharged=true + StartTime=-1 (Paused).
						// Restart timer ONLY if it's fully stopped
						// (not ticking AND TimeLeft==0) — meaning we
						// just Paused() it from the full-pool path or
						// _AI completed a cycle and Paused for us.
						pSuper->IsCharged = true;

						// restarting. A timer Pause()d mid-cycle has
						// TimeLeft > 0 and was never revived. Resume/Start
						// via helper instead.
						SWChargePool::EnsureAccumulating(pSuper);

					} else if (!pSuper->IsCharged) {
						// Below threshold — resume/restart toward next charge.
						// (not ticking, TimeLeft > 0) previously fell through
						// the `!IsTicking()` branch but old code Start()ed a
						// full fresh cycle, losing progress. Helper resumes.
						SWChargePool::EnsureAccumulating(pSuper);
					}
				}
			}
		});
	}
}

// =============================
// load / save

template <typename T>
void SuperExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Name)
		.Process(this->Type)
		.Process(this->Firer)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)
		.Process(this->CameoFirstClickDone)
		.Process(this->FirstClickAutoFireDone)
		.Process(this->IsFromBuilding)
		.Process(this->Statusses)
		.Process(this->Data)
		;
}

// =============================
// container
SuperExtContainer SuperExtContainer::Instance;

// .cpp file
SuperExtData::SuperExtData(SuperClass* pObj) : AbstractExtended(pObj)
{
	this->Type = SWTypeExtContainer::Instance.Find(pObj->Type);
	this->Name = pObj->Type->ID;
	this->AbsType = SuperClass::AbsID;
}

LauchData* SuperExtData::GetLauchDataPtr(SuperClass* pFor)
{
	return &SuperExtContainer::Instance.Find(pFor)->Data;
}

void SuperExtData::UpdateLauchDataTimer(SuperClass* pFor)
{
	auto nData = SuperExtData::GetLauchDataPtr(pFor);

	if ((nData->LastFrame & 0x80000000) != 0)
		nData->LastFrame = Unsorted::CurrentFrame();
}

void SuperExtData::UpdateLauchData(SuperClass* pFor)
{
	SuperExtData::GetLauchDataPtr(pFor)->Update();
}

bool SuperExtData::CanFire(SuperClass* pFor)
{
	const int nAmount = SWTypeExtContainer::Instance.Find(pFor->Type)->SW_Shots;

	if (nAmount < 0)
		return true;

	return SuperExtData::GetLauchDataPtr(pFor)->Count < nAmount;
}
// =============================
// container hooks

ASMJIT_PATCH(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
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
	this->_GetExtData()->InvalidatePointer(target, all, target->WhatAmI());

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

	return (*text)->empty() ? nullptr: (*text)->Text;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC2B0, FakeSuperClass::_NameReadiness)

int FakeSuperClass::_GetAnimStage()
{
	if (!this->Granted)
		return 0;

	auto pType = this->Type;
	auto pTypeExt = SWTypeExtContainer::Instance.Find(pType);

	int rechargeTime = this->GetRechargeTime();
	int delayTime = this->RechargeTimer.TimeLeft;
	int started = this->RechargeTimer.StartTime;

	if (started != -1) {
		int elapsed = Unsorted::CurrentFrame() - started;
		delayTime = (elapsed >= delayTime) ? 0 : (delayTime - elapsed);
	}

	double progress = 0.0;

	if (pTypeExt->UseWeeds) {
		if (this->IsCharged) return 54;
		if (pTypeExt->UseWeeds_StorageTimer) {
			int p = int(54.0 * this->Owner->OwnedWeed.GetTotalAmount()
						/ (double)pTypeExt->UseWeeds_Amount);
			return (p > 54) ? 54 : p;
		}

		return 0;
	}

	if (pType->UseChargeDrain) {
		if (this->ChargeDrainState == ChargeDrainState::Draining) {
			const double ratio = pTypeExt->GetChargeToDrainRatio();
			progress = (Math::abs(rechargeTime * ratio) > 0.001)
				? 1.0 - (rechargeTime * ratio - delayTime) / (rechargeTime * ratio)
				: 0.0;
		} else {
			rechargeTime = this->GetRechargeTime();
			progress = (double)(rechargeTime - delayTime) / rechargeTime;
		}
	} else {
		// FIX 3a: only short-circuit to 54 when pool is FULL or feature off.
		// When accumulating (IsCharged=true but Charges < MaxCharges),
		// show real timer progress so the clock animates.
		const int maxCharges = SWChargePool::GetMax(pType);
		const bool isAccumulating = (maxCharges >= 0)
			&& (SWChargePool::Get(this->Owner, pType)->Charges < maxCharges);

		if (this->IsCharged && !isAccumulating)
			return 54;   // pool full or feature off — no clock

		rechargeTime = this->GetRechargeTime();

		if (rechargeTime <= 0)
			return 0;

		progress = (double)(rechargeTime - delayTime) / rechargeTime;
	}

	int stage = (int)(progress * 54.0);
	return (stage > 54) ? 54 : stage;
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
	if (!this->_GetTypeExtData()->SuperWeaponSidebar_Allow.Get(FakeRulesClass::Instance->SuperWeaponSidebar_AllowByDefault))
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

	return FlashSidebarTabFrames && FlashSidebarTabFrames + this->ReadinessFrame > Unsorted::CurrentFrame();
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
		int curSW = Unsorted::CurrentSWType();

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

	// Vanilla: exit when IsCharged && !UseChargeDrain
	 // New:     also continue if pool has room (timer must tick)
	bool isNeedToCharge = false;
	if (this->IsCharged && !this->Type->UseChargeDrain) {
		const int maxCharges = SWChargePool::GetMax(this->Type);

		if (maxCharges >= 0) {
			// Pool feature on — only exit if pool is full
			// (timer already paused, nothing to accumulate)
			auto pPool = SWChargePool::Get(this->Owner, this->Type);
			if (pPool->Charges >= maxCharges)
				return false;
			else isNeedToCharge = true;
			// else: fall through — timer still needs to run
		} else {
			return false;   // feature off — vanilla exit
		}
	}

	if (this->IsOnHold)
		return false;

	// Timer not running — check if CameoChargeState needs reset
	if (this->RechargeTimer.StartTime == -1) {
		if (this->CameoChargeState != -1) {
			this->CameoChargeState = -1;
			return true;
		}
		return false;
	}

	// === Hook: SuperClass_AI_UseWeeds (0x6CBD2C) ===
	const auto pTypeExt = this->_GetTypeExtData();
	bool forceCharged = false;

	// UseWeeds: charge from weed storage instead of timer
	if (pTypeExt->UseWeeds)
	{
		if (this->Owner->OwnedWeed.GetTotalAmount() >= pTypeExt->UseWeeds_Amount)
		{
			this->Owner->OwnedWeed.RemoveAmount(
				static_cast<float>(pTypeExt->UseWeeds_Amount), 0);
			this->RechargeTimer.Start(0);
			forceCharged = true;

			// run drain checks even on weed-triggered charge.
			// Pass remainingDelay=0 so modulo fires immediately (timeLeft%delay==0).
			if (!pTypeExt->ApplyDrainMoney(0, this->Owner))
			{
				// Insufficient funds — abort the weed charge this frame.
				// Reset timer so we try again next tick.
				this->RechargeTimer.Start(1);
				forceCharged = false;
				// Early return via the existing cameo-stage path
				int animStage = this->GetCameoChargeState();
				if (this->CameoChargeState != animStage)
				{
					this->CameoChargeState = animStage;
					return true;
				}
				return false;
			}

			if (!pTypeExt->ApplyDrainBattlePoint(0, this->Owner))
			{
				this->RechargeTimer.Start(1);
				forceCharged = false;
				int animStage = this->GetCameoChargeState();
				if (this->CameoChargeState != animStage)
				{
					this->CameoChargeState = animStage;
					return true;
				}
				return false;
			}
		}
		else
		{
			const float weedFraction =
				static_cast<float>(this->Owner->OwnedWeed.GetTotalAmount())
				/ static_cast<float>(pTypeExt->UseWeeds_Amount);

			const float animThreshold =
				static_cast<float>(pTypeExt->UseWeeds_ReadinessAnimationPercentage)
				/ 100.0f;

			const int rechargerValue = (weedFraction >= animThreshold) ? 15 : 915;

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

		if (!pTypeExt->ApplyDrainBattlePoint(remainingDelay, this->Owner))
			remainingDelay = 0;

		// Timer still running — check cameo stage update
		if (remainingDelay != 0)
		{
			int stage = this->GetCameoChargeState();

			if (stage != this->CameoChargeState) {
				this->CameoChargeState = stage;
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
			this->ChargeDrainState = ChargeDrainState::None;
			SWTypeExtData::Deactivate(this, CellStruct::Empty, true);
			this->RechargeTimer.Start(this->GetRechargeTime());
			return true;
		}
		else
		{
			// ChargeDrain doesn't use the charge pool — unchanged
			this->ChargeDrainState = ChargeDrainState::Ready;
			this->IsCharged = true;
			return true;
		}
	}

	// --------------------------------------------------------
	// PATCH: charge pool accumulation replaces bare IsCharged=true
	// --------------------------------------------------------
	{
		const int maxCharges = SWChargePool::GetMax(pType);
		const int perCycle = SWCharges_GetPerCycle(pType);
		const int dischargeAmt = SWChargePool::GetDischarge(pType);
		auto pLauchData = SuperExtData::GetLauchDataPtr(this);

		auto StartRecharge = [this]()
			{
				this->IsCharged = false;
				const int t = this->GetRechargeTime();
				this->RechargeTimer.Start((t > 0) ? t : 1);
			};

		if (maxCharges >= 0)
		{
			auto pPool = SWChargePool::Get(this->Owner, pType);

			// VirtualCharge first-grant guard
			const bool isVirtualFirstGrant =
				pTypeExt->SW_VirtualCharge && pLauchData->Count == 0;

			if (!isVirtualFirstGrant)
			{
				const int available = maxCharges - pPool->Charges;
				const int toAdd = (perCycle < available) ? perCycle : available;
				pPool->Charges += toAdd;
			}

			if (isVirtualFirstGrant)
			{
				// Honour vanilla VirtualCharge first-grant behaviour
				this->IsCharged = true;
			}
			else if (pPool->Charges >= maxCharges)
			{
				// Pool full — freeze timer, SW stays ready
				this->IsCharged = true;
				this->RechargeTimer.Pause();
			}
			else if (pPool->Charges >= dischargeAmt)
			{
				// Ready to fire. Keep timer PAUSED to prevent
				// _Discharged's StartTime guard from passing
				// unexpectedly. UpdateSuperWeaponStatuses will
				// restart the timer for continued accumulation.
				SWChargePool::SetPoolFull(this);   // IsCharged=true + Pause()
				SuperExtData::UpdateSuperWeaponStatuses(this->Owner);

				// Announce only on first ready transition
				pTypeExt->OnSuperReady(isPlayer);
			}
			else
			{
				// Below threshold — not ready yet
				StartRecharge();
			}
		}
		else
		{
			// Feature disabled — vanilla
			this->IsCharged = true;
			pTypeExt->OnSuperReady(isPlayer);
		}

		this->ReadinessFrame = Unsorted::CurrentFrame();
		return true;
	}
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CBCA0, FakeSuperClass::_AI)

#include <ThemeClass.h>

void FakeSuperClass::_GlobalAI()
{
	// if (!this->Granted)
	// 	return;

	// const auto pTypeExt = this->_GetTypeExtData();
	// auto pSuperExt = this->_GetExtData();
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4044, FakeSuperClass::_GlobalAI)

void FakeSuperClass::_SetCharge(int charge)
{
	const auto pTypeExt = this->_GetTypeExtData();

	if (pTypeExt->UseWeeds || !this->Granted || charge < 0 || charge > 100)
		return;

	const int maxCharges = SWChargePool::GetMax(this->Type);

	if (maxCharges >= 0 && charge == 100)
	{
		// Pool feature active and caller wants full charge —
		// mirror _AI increment logic exactly
		auto pPool = SWChargePool::Get(this->Owner, this->Type);
		pPool->Increment(maxCharges);

		if (pPool->CanAccumulate(maxCharges))
			SWChargePool::BeginRecharge(this);
		else
			SWChargePool::SetPoolFull(this);

		return;
	}

	// vanilla path (feature off OR charge < 100)
	const int rechargeTime = this->GetRechargeTime();
	const int remainingDelay = rechargeTime
		- static_cast<int>(charge * 0.01f * rechargeTime);

	if (remainingDelay == 0)
		this->IsCharged = true;

	this->RechargeTimer.Start(remainingDelay);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CC1E0, FakeSuperClass::_SetCharge)

void FakeSuperClass::_Forced_Charge(bool isPlayer)
{
	if (!this->Granted)
		return;

	const int maxCharges = SWChargePool::GetMax(this->Type);

	if (maxCharges >= 0)
	{
		// Fill the pool completely
		auto pPool = SWChargePool::Get(this->Owner, this->Type);
		pPool->Charges = maxCharges;
	 	SWChargePool::SetPoolFull(this);
	}
	else
	{
		// Vanilla behaviour
	this->IsCharged = true;
	this->RechargeTimer.Start(0);
	}

	const auto pData = this->_GetTypeExtData();

	pData->OnSuperReady(isPlayer);

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

		if (this->Type->UseChargeDrain
			&& this->ChargeDrainState == ChargeDrainState::Draining)
		{
			SWTypeExtData::Deactivate(this, CellStruct::Empty, false);
			this->ChargeDrainState = ChargeDrainState::Charging;
		}

		// PATCH: clear pool on last removal of this SW type
		const int maxCharges = SWChargePool::GetMax(this->Type);
		if (maxCharges >= 0)
		{
			// Check if any other granted SW of this type still exists
			// for this house. Only wipe the pool when the last one goes.
			bool otherExists = false;
			this->Owner->Supers.for_each([&](SuperClass* pOther)
			{
				if (pOther != this
					&& pOther->Type == this->Type
					&& pOther->Granted)
				{
					otherExists = true;
				}
			});

			if (!otherExists)
			{
				auto pPool = SWChargePool::Get(this->Owner, this->Type);
				pPool->Charges = 0;
				pPool->IsAllowedToAccumulate = true;
			}
		}

		ret = true;
	}

	return ret;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB7B0, FakeSuperClass::_Remove)

bool FakeSuperClass::_Recharge(bool player)
{	
	if (auto v3 = this->Animation) {
		v3->RemainingIterations = 0;
		this->Animation = 0;
		PointerExpiredNotification::NotifyInvalidAnim->Remove(this);
	}

	if (this->AnimationGotInvalid) {
		PointerExpiredNotification::NotifyInvalidAnim->Remove(this);
		this->AnimationGotInvalid = 0;
	}
	if (!this->Granted 
		|| this->IsCharged 
		|| this->IsOnHold && !this->Type->PreClick)
	{
		return 0;
	}

	this->ReadinessFrame = -1;
	this->RechargeTimer.Start(this->GetRechargeTime());

	if (this->Type->UseChargeDrain) {
		this->ChargeDrainState = ChargeDrainState::Charging;
	}
	return 1;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB830, FakeSuperClass::_Recharge)

bool FakeSuperClass::_Discharged(bool isPlayer, CellStruct* pCell)
{
	auto const pType = this->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);
	auto const pOwner = this->Owner;
	auto const pHouseExt = HouseExtContainer::Instance.Find(pOwner);

	if (!pType->UseChargeDrain)
	{
		if (!pType->PostClick)
		{
			const int maxCharges = SWChargePool::GetMax(pType);

			if (maxCharges >= 0)
			{
				// FIX 1+2: pool gate replaces vanilla StartTime check
				const int dischargeAmt = SWChargePool::GetDischarge(pType);
				auto pPool = SWChargePool::Get(pOwner, pType);

				if (!this->Granted || !this->IsCharged || pPool->Charges < dischargeAmt)
					return false;

				// FIX 1: block unexpected auto-fire from 0x4FAE97.
				// That path always passes CellStruct::Empty.
				// SWs that require player targeting (Action == SuperWeaponAllowed,
				// not AI-targeting) should NEVER fire at an empty cell legitimately.
				// Player clicks always provide a real cell.
				// SWs with Action::None or SW_UseAITargeting fire to Empty on purpose.
				const bool requiresPlayerTarget =
					!pExt->SW_UseAITargeting
					&& (PhobosNewActionType)pType->Action == PhobosNewActionType::SuperWeaponAllowed;

				if (requiresPlayerTarget && *pCell == CellStruct::Empty)
					return false;
			}
			else
			{
				// Vanilla: use timer guard (feature off)
				if (this->RechargeTimer.StartTime == -1
					|| !this->Granted
					|| !this->IsCharged)
					return false;
			}
		}

		if (!pOwner->CanTransactMoney(pExt->Money_Amount))
		{
			if (pOwner->IsCurrentPlayer())
				pExt->UneableToTransactMoney(pOwner);
			return false;
		}

		if (!pHouseExt->CanTransactBattlePoints(pExt->BattlePoints_Amount))
		{
			if (pOwner->IsCurrentPlayer())
				pExt->UneableToTransactBattlePoints(pOwner);
			return false;
		}

		auto pNewType = SWTypeHandler::get_Handler(pExt->HandledType);

		if (pNewType->AbortFire(this, isPlayer))
			return false;

		this->_Place(pCell, isPlayer);

		if (!pType->PostClick && !pType->PreClick)
			this->IsCharged = false;

		int maxCharges = SWChargePool::GetMax(pType);
		if (maxCharges >= 0 && !pType->PostClick && !pType->PreClick)
		{
			const int dischargeAmt = SWChargePool::GetDischarge(pType);
			auto pPool = SWChargePool::Get(pOwner, pType);

			pPool->DecrementBy(dischargeAmt);

			const bool willRemove = this->OneTime || !SuperExtData::CanFire(this);

			if (!willRemove)
			{
				if (pPool->Charges >= dischargeAmt)
				{
					this->IsCharged = true;

					if (pPool->Charges >= maxCharges)
						SWChargePool::SetPoolFull(this);
					else
						// ("freeze visually at 0"). That snapshotted
						// TimeLeft > 0 and no restart path could pass its
						// `TimeLeft <= 0` guard -> timer stuck forever.
						// Non-full pool must keep ticking.
						SWChargePool::EnsureAccumulating(this);
				}
				else
				{
					this->IsCharged = false;
					// paused mid-cycle timers (TimeLeft > 0, not ticking).
					// Helper resumes them instead of leaving them dead.
					SWChargePool::EnsureAccumulating(this);
				}
			}
		}

		if (this->OneTime || !SuperExtData::CanFire(this))
		{
			this->OneTime = false;
			return this->Lose();
		}
		else if (pType->ManualControl)
		{
			const auto time = this->GetRechargeTime();
			this->CameoChargeState = -1;
			this->RechargeTimer.Start(time);
			this->RechargeTimer.Pause();
		}
		else if (!pType->PreClick && !pType->PostClick)
		{
			if (maxCharges >= 0)
			{
				auto pPool = SWChargePool::Get(pOwner, pType);
				// Only kill anim if we're going back to recharging state
				// If still ready (charges remain), leave anim alone
				if (pPool->Charges < SWChargePool::GetDischarge(pType))
					this->_Recharge(isPlayer);
			}
			else
			{
				this->_Recharge(isPlayer);
			}
		}
	}
	else
	{
		// ChargeDrain — unchanged
		if (this->ChargeDrainState == ChargeDrainState::Draining)
		{
			this->ChargeDrainState = ChargeDrainState::Ready;
			auto const left = this->RechargeTimer.GetTimeLeft();
			auto const duration = int(this->GetRechargeTime()
									- (left / pExt->GetChargeToDrainRatio()));
			this->RechargeTimer.Start(duration);
			pExt->Deactivate(this, *pCell, isPlayer);
		}
		else if (this->ChargeDrainState == ChargeDrainState::Ready)
		{
			this->ChargeDrainState = ChargeDrainState::Draining;
			auto const left = this->RechargeTimer.GetTimeLeft();
			auto const duration = int(
				(this->GetRechargeTime() - left)
				* pExt->GetChargeToDrainRatio());
			this->RechargeTimer.Start(duration);
			this->_Place(pCell, isPlayer);
		}
	}

	return false;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB920, FakeSuperClass::_Discharged)
DEFINE_FUNCTION_JUMP(CALL, 0x4FAE97, FakeSuperClass::_Discharged)

bool FakeSuperClass::_Suspend(bool on)
{
	auto ret = false;

	if (this->Granted
		&& !this->OneTime
		&& this->CanHold
		&& on != this->IsOnHold)
	{
		if (on)
		{
			// Suspending — always Pause regardless of pool state
			this->RechargeTimer.Pause();
		}
		else
		{
			// Resuming — check pool state for ManualControl
			const int maxCharges = SWChargePool::GetMax(this->Type);

			if (maxCharges >= 0)
			{
				auto pPool = SWChargePool::Get(this->Owner, this->Type);

				if (pPool->Charges >= maxCharges)
				{
					// Pool full — ManualControl or not, stay frozen
					SWChargePool::SetPoolFull(this);
				}
				else
				{
					// BUGFIX: bare Resume() on a timer whose TimeLeft
						// hit 0 while suspended resumes as already-expired
						// -> _AI treats it as a completed cycle -> free
						// charge on unhold. Helper Start()s fresh instead
						// when TimeLeft == 0, Resume()s when > 0.
					SWChargePool::EnsureAccumulating(this);
				}
			}
			else if (this->Type->ManualControl)
			{
				// Feature off, vanilla ManualControl — Pause as normal
				this->RechargeTimer.Pause();
			}
			else
			{
				// Feature off, non-ManualControl — Resume as normal
				this->RechargeTimer.Resume();
			}
		}

		this->IsOnHold = on;

		if (this->Type->UseChargeDrain)
		{
			// ChargeDrain path unchanged
			if (on)
			{
				if (this->ChargeDrainState == ChargeDrainState::Draining)
				{
					SWTypeExtData::Deactivate(this, CellStruct::Empty, false);
					const auto nTime = this->GetRechargeTime();
					const auto nRation = this->RechargeTimer.GetTimeLeft()
						/ SWTypeExtContainer::Instance.Find(this->Type)
						->GetChargeToDrainRatio();
					this->RechargeTimer.Start(int(nTime - nRation));
					this->RechargeTimer.Pause();
				}
				this->ChargeDrainState = ChargeDrainState::None;
			}
			else
			{
				const auto pTypeExt = SWTypeExtContainer::Instance.Find(this->Type);
				this->ChargeDrainState = ChargeDrainState::Charging;

				if (!pTypeExt->SW_InitialReady
					|| SuperExtData::GetLauchDataPtr(this)->Count)
				{
					this->RechargeTimer.Start(this->GetRechargeTime());
				}
				else
				{
					this->ChargeDrainState = ChargeDrainState::Ready;
					this->ReadinessFrame = Unsorted::CurrentFrame();
					this->IsCharged = true;
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

	//auto pExt = SuperExtContainer::Instance.Find(this);
	auto pSuperExt = SWTypeExtContainer::Instance.Find(pType);

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

			auto data = SuperExtData::GetLauchDataPtr(this);
			const int nCharge = (!pSuperExt->SW_InitialReady || data->Count)
				? this->GetRechargeTime() : 0;

			this->RechargeTimer.Start(nCharge);

			auto nFrame = Unsorted::CurrentFrame();
			if (pSuperExt->SW_VirtualCharge)
			{
				if ((data->LastFrame & 0x80000000) == 0)
				{
					this->RechargeTimer.StartTime = data->LastFrame ;
					nFrame = data->LastFrame ;
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

			SuperExtData::UpdateLauchDataTimer(this);
			result = true;
		}
	}

	// Quiet/onHold mode: suspend timer (only for non-onetime supers)
	if (onHold && this->Granted && !this->OneTime
		&& !this->IsOnHold && this->CanHold)
	{
		this->RechargeTimer.Pause();
		this->IsOnHold = true;
	}

	// OneTime supers always force-charge immediately
	if (this->OneTime)
		this->ClickFire(announce);

	return result;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6CB560, FakeSuperClass::_Grant)

HRESULT __stdcall FakeSuperClass::__Load(IStream* pStm)
{
	HRESULT hr = this->SuperClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!SuperExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F3FFC, FakeSuperClass::__Load)

HRESULT __stdcall FakeSuperClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->SuperClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!SuperExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4000, FakeSuperClass::__Save)