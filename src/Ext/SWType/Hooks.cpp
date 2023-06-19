
#pragma region  Incl
#include <Misc/AresData.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include "Body.h"

#include <NetworkEvents.h>

#include "NewSuperWeaponType/NuclearMissile.h"
#include "NewSuperWeaponType/LightningStorm.h"
#include "NewSuperWeaponType/Dominator.h"
#pragma endregion

//TODO : removing all SWTypeExt related call if possible 
//		 test AI auto targeting stuffs ,..

//TODO this shit
DEFINE_OVERRIDE_HOOK(0x6EFC70, TeamClass_IronCurtain, 5)
{
	GET(TeamClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pTeamMission, 0x4);
	GET_STACK(bool, barg3, 0x8);
	AresData::FireIronCurtain(pThis, pTeamMission, barg3);
	return 0x6EFE4F;
}

DEFINE_OVERRIDE_HOOK(0x6CEF84 , SuperWeaponTypeClass_GetAction, 7)
{
	GET_STACK(CellStruct*, pTarget, 0xC);
	GET(SuperWeaponTypeClass*, pType, ECX);

	const auto nAction = SWTypeExt::ExtData::GetAction(pType , pTarget);
	if (nAction != Action::None) {
		R->EAX(nAction);
	}

	return 0x6CEFD9;
}
DEFINE_OVERRIDE_HOOK(0x6AAEDF, SidebarClass_ProcessCameoClick_SuperWeapons, 6)
{

	GET(int, idxSW, ESI);
	SuperClass* pSuper = HouseClass::CurrentPlayer->Supers.GetItem(idxSW);
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type);

	// if this SW is only auto-firable, discard any clicks.
	// if AutoFire is off, the sw would not be firable at all,
	// thus we ignore the setting in that case.
	bool manual = !pData->SW_ManualFire && pData->SW_AutoFire;
	bool unstoppable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
		&& pData->SW_Unstoppable;

	// play impatient voice, if this isn't charged yet
	if (!pSuper->CanFire() && !manual)
	{
		VoxClass::PlayIndex(pData->EVA_Impatient);
		return 0x6AAFB1;
	}

	// prevent firing the SW if the player doesn't have sufficient
	// funds. play an EVA message in that case.
	if (!HouseClass::CurrentPlayer->CanTransactMoney(pData->Money_Amount))
	{
		VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
		pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::CurrentPlayer);
		return 0x6AAFB1;
	}

	if (!pData->SW_UseAITargeting.Get() || SWTypeExt::ExtData::IsTargetConstraintEligible(pSuper, true))
	{
		// disallow manuals and active unstoppables
		if (manual || unstoppable)
		{
			return 0x6AAFB1;
		}

		if (!((int)pSuper->Type->Action) || pData->SW_UseAITargeting.Get())
		{
			R->EAX(HouseClass::CurrentPlayer());
			return 0x6AAF10;
		}
		else
		{
			return 0x6AAF46;
		}
	}
	else
	{
		pData->PrintMessage(pData->Message_CannotFire, HouseClass::CurrentPlayer);
		return 0x6AAFB1;
	}
}

// play a customizable target selection EVA message
DEFINE_OVERRIDE_HOOK(0x6AAF9D, SidebarClass_ProcessCameoClick_SelectTarget, 5)
{
	GET(int, index, ESI);
	if (SuperClass* pSW = HouseClass::CurrentPlayer->Supers.GetItem(index))
	{
		if (SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW->Type))
		{
			VoxClass::PlayIndex(pData->EVA_SelectTarget);
		}
	}

	return 0x6AB95A;
}

DEFINE_OVERRIDE_HOOK(0x6A932B, StripClass_GetTip_MoneySW, 6)
{
	GET(SuperWeaponTypeClass*, pSW, EAX);

	if (SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW))
	{
		if (pData->Money_Amount < 0)
		{
			wchar_t* pTip = SidebarClass::TooltipBuffer;
			int length = SidebarClass::TooltipLength;

			// account for no-name SWs
			if (*reinterpret_cast<byte*>(0x884B8C) || !wcslen(pSW->UIName))
			{
				const wchar_t* pFormat = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_1);
				swprintf(pTip, length, pFormat, -pData->Money_Amount);
			}
			else
			{
				// then, this must be brand SWs
				const wchar_t* pFormat = StringTable::LoadStringA(GameStrings::TXT_MONEY_FORMAT_2);
				swprintf(pTip, length, pFormat, pSW->UIName, -pData->Money_Amount);
			}
			pTip[length - 1] = 0;

			// replace space by new line
			for (int i = wcslen(pTip); i >= 0; --i)
			{
				if (pTip[i] == 0x20)
				{
					pTip[i] = 0xA;
					break;
				}
			}

			// put it there
			R->EAX(pTip);
			return 0x6A93E5;
		}
	}

	return 0;
}

// 6CEE96, 5
DEFINE_OVERRIDE_HOOK(0x6CEE96, SuperWeaponTypeClass_FindIndex, 5)
{
	GET(const char*, TypeStr, EDI);

	const auto customType = NewSWType::FindFromTypeID(TypeStr);

	if (customType > SuperWeaponType::Invalid)
	{
		R->ESI(customType);
		return 0x6CEE9C;
	}

	return 0;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_OVERRIDE_HOOK(0x4AC20C, DisplayClass_LeftMouseButtonUp, 7)
{
	GET_STACK(Action, nAction, 0x9C);

	if (nAction < (Action)AresNewActionType::SuperWeaponDisallowed)
	{
		// get the actual firing SW type instead of just the first type of the
		// requested action. this allows clones to work for legacy SWs (the new
		// ones use SW_*_CURSORs). we have to check that the action matches the
		// action of the found type as the no-cursor represents a different
		// action and we don't want to start a force shield even tough the UI
		// says no.
		auto pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType());
		if (pSW && (pSW->Action != nAction))
		{
			pSW = nullptr;
		}

		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if (nAction == (Action)AresNewActionType::SuperWeaponDisallowed)
	{
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(Ares_CurrentSWType);
	return 0x4AC21C;
}

// decoupling sw anims from types
DEFINE_OVERRIDE_HOOK(0x4463F0, BuildingClass_Place_SuperWeaponAnimsA, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (auto pSuper = BuildingExt::GetFirstSuperWeapon(pThis))
	{
		R->EAX(pSuper);
		return 0x44643E;
	}

	return 0x446580;
}

DEFINE_OVERRIDE_HOOK(0x450F9E, BuildingClass_ProcessAnims_SuperWeaponsA, 6)
{
	GET(BuildingClass*, pThis, ESI);

	auto pSuper = BuildingExt::GetFirstSuperWeapon(pThis);

	if (!pSuper)
		return 0x451145;

	const auto miss = pThis->GetCurrentMission();
	if (miss == Mission::Construction || miss == Mission::Selling || pThis->Type->ChargedAnimTime > 990.0)
		return 0x451145;

	R->EDI(pThis->Type);
	R->EAX(pSuper);
	return 0x451030;
}

// EVA_Detected
DEFINE_OVERRIDE_HOOK(0x4468F4, BuildingClass_Place_AnnounceSW, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (auto pSuper = BuildingExt::GetFirstSuperWeapon(pThis))
	{
		auto pData = SWTypeExt::ExtMap.Find(pSuper->Type);

		pData->PrintMessage(pData->Message_Detected, pThis->Owner);

		if (pData->EVA_Detected == -1 && NewSWType::IsOriginalType(pSuper->Type->Type) && !pData->IsTypeRedirected())
		{
			R->EAX(pSuper->Type->Type);
			return 0x446943;
		}

		VoxClass::PlayIndex(pData->EVA_Detected);
	}

	return 0x44699A;
}

// EVA_Ready
// 6CBDD7, 6
DEFINE_OVERRIDE_HOOK(0x6CBDD7, SuperClass_AnnounceReady, 6)
{
	GET(SuperWeaponTypeClass*, pThis, EAX);
	auto pData = SWTypeExt::ExtMap.Find(pThis);

	pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);

	if (pData->EVA_Ready != -1 || !NewSWType::IsOriginalType(pThis->Type) || pData->IsTypeRedirected())
	{
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CBE68;
	}

	return 0;
}

// 6CC0EA, 9
DEFINE_OVERRIDE_HOOK(0x6CC0EA, SuperClass_AnnounceQuantity, 9)
{
	GET(SuperClass*, pThis, ESI);
	auto pData = SWTypeExt::ExtMap.Find(pThis->Type);

	pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);

	if (pData->EVA_Ready != -1 || !NewSWType::IsOriginalType(pThis->Type->Type) || pData->IsTypeRedirected())
	{
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CC17E;
	}

	return 0;
}

// AI SW targeting submarines
DEFINE_OVERRIDE_HOOK(0x50CFAA, HouseClass_PickOffensiveSWTarget, 0xA)
{
	// reset weight
	R->ESI(0);

	// mark as ineligible
	R->Stack8(0x13, 0);

	return 0x50CFC9;
}

DEFINE_OVERRIDE_HOOK(0x457630, BuildingClass_SWAvailable, 9)
{
	GET(BuildingClass*, pThis, ECX);
	R->EAX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GetSuperWeaponIndex(1));
	return 0x457688;
}

DEFINE_OVERRIDE_HOOK(0x457690, BuildingClass_SW2Available, 9)
{
	GET(BuildingClass*, pThis, ECX);
	R->EAX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GetSuperWeaponIndex(1));
	return 0x4576E8;
}

DEFINE_OVERRIDE_HOOK(0x43BE50, BuildingClass_DTOR_HasAnySW, 6)
{
	GET(BuildingClass*, pThis, ESI);

	return (BuildingExt::GetFirstSuperWeaponIndex(pThis) != -1)
		? 0x43BEEAu : 0x43BEF5u;
}


DEFINE_OVERRIDE_HOOK(0x449716, BuildingClass_Mi_Guard_HasFirstSW, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->FirstActiveSWIdx() != -1 ? 0x4497AFu : 0x449762u;
}

DEFINE_OVERRIDE_HOOK(0x4FAE72, HouseClass_SWFire_PreDependent, 6)
{
	GET(HouseClass*, pThis, EBX);

	// find the predependent SW. decouple this from the chronosphere.
	// don't use a fixed SW type but the very one acutually fired last.
	SuperClass* pSource = nullptr;
	if (auto pExt = HouseExt::ExtMap.Find(pThis))
	{
		pSource = pThis->Supers.GetItemOrDefault(pExt->SWLastIndex);
	}

	R->ESI(pSource);

	return 0x4FAE7B;
}

DEFINE_OVERRIDE_HOOK(0x6CC2B0, SuperClass_NameReadiness, 5)
{
	GET(SuperClass*, pThis, ECX);
	auto pData = SWTypeExt::ExtMap.Find(pThis->Type);

	// complete rewrite of this method.

	auto text = &pData->Text_Preparing;
	if (pThis->IsOnHold)
	{
		// on hold
		text = &pData->Text_Hold;
	}
	else
	{
		if (pThis->Type->UseChargeDrain)
		{
			switch (pThis->ChargeDrainState)
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
			if (pThis->IsCharged)
			{
				text = &pData->Text_Ready;
			}
		}
	}

	R->EAX(text->Get().empty() ? nullptr : text->Get().Text);
	return 0x6CC352;
}

// #896002: darken SW cameo if player can't afford it
DEFINE_OVERRIDE_HOOK(0x6A99B7, StripClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);

	auto pSW = HouseClass::CurrentPlayer->Supers.GetItem(idxSW);
	auto pExt = SWTypeExt::ExtMap.Find(pSW->Type);

	bool darken = false;
	if (pSW->IsCharged && !pSW->Owner->CanTransactMoney(pExt->Money_Amount)
		|| (pExt->SW_UseAITargeting && !SWTypeExt::ExtData::IsTargetConstraintEligible(pSW, true)))
	{
		darken = true;
	}

	R->BL(darken);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4F9004, HouseClass_Update_TrySWFire, 7)
{
	GET(HouseClass*, pThis, ESI);
	bool isHuman = R->AL() != 0;

	if (isHuman)
	{
		// update the SWs for human players to support auto firing.
		pThis->AI_TryFireSW();
	}
	else if (!pThis->Type->MultiplayPassive)
	{
		return 0x4F9015;
	}

	return 0x4F9038;
}

DEFINE_OVERRIDE_HOOK(0x6CBF5B , SuperClass_GetCameoChargeStage_ChargeDrainRatio, 9)
{
	GET_STACK(int, rechargeTime1, 0x10);
	GET_STACK(int, rechargeTime2, 0x14);
	GET_STACK(int, timeLeft, 0xC);

	GET(SuperWeaponTypeClass*, pType, EBX);
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pType);

	// use per-SW charge-to-drain ratio.
	double percentage = 0.0;
	double ratio = pData->GetChargeToDrainRatio();
	if (std::fabs(rechargeTime2 * ratio) > 0.001)
	{
		percentage = 1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio);
	}

	R->EAX(int(percentage * 54.0));
	return 0x6CC053;
}

DEFINE_OVERRIDE_HOOK(0x6CC053, SuperClass_GetCameoChargeStage_FixFullyCharged, 5)
{
	R->EAX(R->EAX() > 54 ? 54 : R->EAX());
	return 0x6CC066;
}

// a ChargeDrain SW expired - fire it to trigger status update
DEFINE_OVERRIDE_HOOK(0x6CBD86, SuperClass_Progress_Charged, 7)
{
	GET(SuperClass* const, pThis, ESI);
	SWTypeExt::ExtData::Deactivate(pThis, CellStruct::Empty, true);
	return 0;
}


// SW was lost (source went away)
DEFINE_OVERRIDE_HOOK(0x6CB7B0, SuperClass_Lose, 6)
{
	GET(SuperClass* const, pThis, ECX);
	auto ret = false;

	if (pThis->Granted)
	{
		pThis->IsCharged = false;
		pThis->Granted = false;

		SuperClass::ShowTimers->Remove(pThis);

		// changed
		if (pThis->Type->UseChargeDrain && pThis->ChargeDrainState == ChargeDrainState::Draining)
		{
			SWTypeExt::ExtData::Deactivate(pThis, CellStruct::Empty, false);
			pThis->ChargeDrainState = ChargeDrainState::Charging;
		}

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB810;
}

// activate or deactivate the SW
DEFINE_OVERRIDE_HOOK(0x66CB920, SuperClass_ClickFire, 5)
{
	GET(SuperClass* const, pThis, ECX);
	GET_STACK(bool const, isPlayer, 0x4);
	GET_STACK(CellStruct const* const, pCell, 0x8);

	retfunc<bool> ret(R, 0x6CBC9C);

	auto const pType = pThis->Type;
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

	auto const pOwner = pThis->Owner;

	if (pType->UseChargeDrain)
	{

		// AI get non-draining SWs
		if (!pOwner->IsControlledByHuman_())
		{
			if (!pOwner->FirestormActive)
			{
				pThis->Launch(*pCell, isPlayer);
			}
			else
			{
				SWTypeExt::ExtData::Deactivate(pThis, *pCell, isPlayer);
			}
		}
		else
		{
			if (pThis->ChargeDrainState == ChargeDrainState::Draining)
			{
				// deactivate for human players
				pThis->ChargeDrainState = ChargeDrainState::Ready;
				auto const left = pThis->RechargeTimer.GetTimeLeft();

				auto const duration = Game::F2I(pThis->GetRechargeTime()
					- (left / pExt->GetChargeToDrainRatio()));
				pThis->RechargeTimer.Start(duration);

				SWTypeExt::ExtData::Deactivate(pThis, *pCell, isPlayer);

			}
			else if (pThis->ChargeDrainState == ChargeDrainState::Ready)
			{
				// activate for human players
				pThis->ChargeDrainState = ChargeDrainState::Draining;
				auto const left = pThis->RechargeTimer.GetTimeLeft();

				auto const duration = Game::F2I(
					(pThis->GetRechargeTime() - left)
					* pExt->GetChargeToDrainRatio());
				pThis->RechargeTimer.Start(duration);

				pThis->Launch(*pCell, isPlayer);
			}
		}

		return ret(false);
	}

	//change done
	if ((pThis->RechargeTimer.StartTime < 0
		|| !pThis->Granted
		|| !pThis->IsCharged)
		&& !pType->PostClick)
	{
		return ret(false);
	}

	// auto-abort if no money
	if (!pOwner->CanTransactMoney(pExt->Money_Amount))
	{
		if (pOwner->IsCurrentPlayer())
		{
			VoxClass::PlayIndex(pExt->EVA_InsufficientFunds);
			pExt->PrintMessage(pExt->Message_InsufficientFunds, pOwner);
		}
		return ret(false);
	}

	// can this super weapon fire now?
	if (auto const pNewType = NewSWType::GetNewSWType(pExt))
	{
		if (pNewType->AbortFire(pThis, isPlayer))
		{
			return ret(false);
		}
	}

	pThis->Launch(*pCell, isPlayer);

	// the others will be reset after the PostClick SW fired
	if (!pType->PostClick && !pType->PreClick)
	{
		pThis->IsCharged = false;
	}

	if (pThis->OneTime || !pExt->CanFire(pOwner))
	{
		// remove this SW
		pThis->OneTime = false;
		return ret(pThis->Lose());
	}
	else if (pType->ManualControl)
	{
		// set recharge timer, then pause
		const auto time = pThis->GetRechargeTime();
		pThis->CameoChargeState = -1;
		pThis->RechargeTimer.Start(time);
		pThis->RechargeTimer.Pause();
	}
	else
	{
		if (!pType->PreClick && !pType->PostClick)
		{
			pThis->StopPreclickAnim(isPlayer);
		}
	}

	return ret(false);
}

// rewriting OnHold to support ChargeDrain
DEFINE_OVERRIDE_HOOK(0x6CB4D0, SuperClass_SetOnHold, 6)
{
	GET(SuperClass*, pThis, ECX);
	GET_STACK(bool const, onHold, 0x4);

	auto ret = false;

	if (pThis->Granted
		&& !pThis->OneTime
		&& pThis->CanHold
		&& onHold != pThis->IsOnHold)
	{
		if (onHold || pThis->Type->ManualControl)
		{
			pThis->RechargeTimer.Pause();
		}
		else
		{
			pThis->RechargeTimer.Resume();
		}

		pThis->IsOnHold = onHold;

		if (pThis->Type->UseChargeDrain)
		{
			if (onHold)
			{
				if (pThis->ChargeDrainState == ChargeDrainState::Draining)
				{
					SWTypeExt::ExtData::Deactivate(pThis, CellStruct::Empty, false);
					const auto nTime = pThis->GetRechargeTime();
					const auto nRation = pThis->RechargeTimer.GetTimeLeft() / RulesClass::Instance->ChargeToDrainRatio;
					pThis->RechargeTimer.Start(nTime - nRation);
					pThis->RechargeTimer.Pause();
				}

				pThis->ChargeDrainState = ChargeDrainState::None;
			}
			else
			{
				const auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);
				auto pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

				pThis->ChargeDrainState = ChargeDrainState::Charging;

				if (!pExt->SW_InitialReady || pHouseExt->GetShotCount(pThis->Type).Count)
				{
					pThis->RechargeTimer.Start(pThis->GetRechargeTime());
				}
				else
				{
					pThis->ChargeDrainState = ChargeDrainState::Ready;
					pThis->ReadinessFrame = Unsorted::CurrentFrame();
					pThis->IsCharged = true;
				}
			}
		}

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB555;
}

DEFINE_OVERRIDE_HOOK(0x6CBD6B, SuperClass_Update_DrainMoney, 8)
{
	// draining weapon active. take or give money. stop,
	// if player has insufficient funds.
	GET(SuperClass*, pSuper, ESI);
	GET(int, timeLeft, EAX);

	if (timeLeft > 0 && pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining)
	{
		if (SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type))
		{
			int money = pData->Money_DrainAmount;
			if (money != 0 && pData->Money_DrainDelay > 0)
			{
				if (!(timeLeft % pData->Money_DrainDelay))
				{
					auto pOwner = pSuper->Owner;

					// only abort if SW drains money and there is none
					if (!pOwner->CanTransactMoney(money))
					{
						if (pOwner->IsControlledByHuman())
						{
							VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
							pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::CurrentPlayer);
						}
						return 0x6CBD73;
					}

					// apply drain money
					pOwner->TransactMoney(money);
				}
			}
		}
	}

	return (timeLeft ? 0x6CBE7C : 0x6CBD73);
}

// clear the chrono placement animation if not ChronoWarp
DEFINE_OVERRIDE_HOOK(0x6CBCDE, SuperClass_Update_Animation, 5)
{
	if (auto const pType = SuperWeaponTypeClass::Array->GetItemOrDefault(Unsorted::CurrentSWType))
	{
		if (pType->Type == SuperWeaponType::ChronoWarp)
		{
			return 0x6CBCFE;
		}
	}

	return 0x6CBCE3;
}

// used only to find the nuke for ICBM crates. only supports nukes fully.
DEFINE_OVERRIDE_HOOK(0x6CEEB0, SuperWeaponTypeClass_FindFirstOfAction, 8)
{
	GET(Action, action, ECX);

	SuperWeaponTypeClass* pFound = nullptr;

	// this implementation is as stupid as short sighted, but it should work
	// for the moment. as there are no actions any more, this has to be
	// reworked if powerups are expanded. for now, it only has to find a nuke.
	for (auto pType : *SuperWeaponTypeClass::Array)
	{
		if (pType->Action == action)
		{
			pFound = pType;
			break;
		}
		else
		{
			if (auto pNewSWType = NewSWType::GetNewSWType(SWTypeExt::ExtMap.Find(pType)))
			{
				if (pNewSWType->HandleThisType(SuperWeaponType::Nuke))
				{
					pFound = pType;
					break;
				}
			}
		}
	}

	// put a hint into the debug log to explain why we will crash now.
	if (!pFound)
	{
		Debug::FatalErrorAndExit("Failed finding an Action=Nuke or Type=MultiMissile super weapon to be granted by ICBM crate.");
	}

	R->EAX(pFound);
	return 0x6CEEE5;
}

DEFINE_OVERRIDE_HOOK(0x6D49D1, TacticalClass_Draw_TimerVisibility, 5)
{
	enum
	{
		DrawSuspended = 0x6D49D8,
		DrawNormal = 0x6D4A0D,
		DoNotDraw = 0x6D4A71
	};

	GET(SuperClass*, pThis, EDX);

	auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt->IsHouseAffected(pThis->Owner, HouseClass::CurrentPlayer(), pExt->SW_TimerVisibility))
		return DoNotDraw;

	return pThis->IsOnHold ? DrawSuspended : DrawNormal;
}

DEFINE_OVERRIDE_HOOK(0x6CB70C, SuperClass_Grant_InitialReady, 0xA)
{
	GET(SuperClass*, pSuper, ESI);

	pSuper->CameoChargeState = -1;

	if (pSuper->Type->UseChargeDrain)
		pSuper->ChargeDrainState = ChargeDrainState::Charging;

	auto pHouseExt = HouseExt::ExtMap.Find(pSuper->Owner);
	const auto pSuperExt = SWTypeExt::ExtMap.Find(pSuper->Type);

	auto& [frame, count] = pHouseExt->GetShotCount(pSuper->Type);

	const int nCharge = !pSuperExt->SW_InitialReady || count ? pSuper->GetRechargeTime() : 0;

	pSuper->RechargeTimer.Start(nCharge);
	auto nFrame = Unsorted::CurrentFrame();

	if (pSuperExt->SW_VirtualCharge)
	{
		const auto nChekedFrame = frame;
		if ((frame & 0x80000000) == 0)
		{
			pSuper->RechargeTimer.StartTime = nChekedFrame;
			nFrame = nChekedFrame;
		}
	}

	if (nFrame != -1)
	{
		auto nTimeLeft = nCharge + nFrame - Unsorted::CurrentFrame();
		if (nTimeLeft <= 0)
			nTimeLeft = 0;

		if (nTimeLeft <= 0)
		{
			pSuper->IsCharged = true;
			pSuper->ReadinessFrame = Unsorted::CurrentFrame();

			if (pSuper->Type->UseChargeDrain)
				pSuper->ChargeDrainState = ChargeDrainState::Ready;
		}
	}

	pHouseExt->UpdateShotCountB(pSuper->Type);

	return 0x6CB750;
}

DEFINE_DISABLE_HOOK(0x46B371, BulletClass_NukeMaker_ares)
DEFINE_OVERRIDE_HOOK(0x46B310, BulletClass_NukeMaker_Handle, 6)
{
	GET(BulletClass*, pThis, EBP);

	enum { ret = 0x46B53C };

	const auto pTarget = pThis->Target;
	if (!pTarget)
	{
		Debug::Log("Bullet[%s] Trying to Apply NukeMaker but has invalid target !\n", pThis->Type->ID);
		return ret;
	}

	WeaponTypeClass* pPaylod = nullptr;
	SuperClass* pNukeSW = nullptr;

	if (auto const pNuke = BulletExt::ExtMap.Find(pThis)->NukeSW)
	{
		pPaylod = SW_NuclearMissile::GetNukePayload(pNuke->Type);
		pNukeSW = pNuke;

	}
	else
	{
		pPaylod = WeaponTypeClass::Find(GameStrings::NukePayload());
	}

	if (!pPaylod || !pPaylod->Projectile)
	{
		Debug::Log("Bullet[%s] Trying to Apply NukeMaker but has invalid Payload Weapon or Payload Weapon Projectile !\n", pThis->Type->ID);
		return ret;
	}

	//TODO , interceptor
	if (auto pPayloadBullet = BulletTypeExt::ExtMap.Find(pPaylod->Projectile)
		->CreateBullet(pTarget, pThis->Owner, pPaylod))
	{
		pPayloadBullet->Limbo();
		BulletExt::ExtMap.Find(pPayloadBullet)->NukeSW = pNukeSW;
		const auto nTargetLoc = pTarget->GetCoords();
		const auto pTargetCell = MapClass::Instance->GetCellAt(nTargetLoc);
		auto nTargetLocBridge = pTargetCell->GetCoordsWithBridge();
		nTargetLocBridge.Z += pPaylod->Projectile->DetonationAltitude;
		//auto nCos = 0.00004793836; //std::cos(1.570748388432313); // Accuracy is different from the game 
		//auto nSin = 0.99999999885; //std::sin(1.570748388432313); // Accuracy is different from the game 
		constexpr double nX = 0.00004793836 * 0.00004793836 * -1.0;
		constexpr double nY = 0.00004793836 * 0.99999999885 * -1.0;
		constexpr double nZ = 0.99999999885 * -1.0;

		pPayloadBullet->MoveTo(nTargetLocBridge, { nX , nY , nZ });
	}

	return ret;
}

DEFINE_OVERRIDE_HOOK(0x653B3A, RadarClass_GetMouseAction_CustomSWAction, 7)
{
	GET_STACK(CellStruct*, pMapCoords, 0x0C);
	GET_STACK(DWORD, flag, 0x58);

	if (Unsorted::CurrentSWType() < 0)
		return 0x0;

	if ((flag & 0x50) != 0)
		return 0x653D6F;

	const auto nResult = SWTypeExt::ExtData::GetAction(SuperWeaponTypeClass::Array->GetItem(Unsorted::CurrentSWType), pMapCoords);

	if (nResult == Action::None)
		return 0x0;

	R->ESI(nResult);
	return 0x653CA3;
}

DEFINE_OVERRIDE_HOOK(0x5098F0, HouseClass_Update_AI_TryFireSW, 5)
{
	GET(HouseClass*, pThis, ECX);

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	bool AIFire = !pThis->IsControlledByHuman_();

	for (const auto pSuper : pThis->Supers)
	{
		if (pSuper->IsCharged && pSuper->ChargeDrainState != ChargeDrainState::Draining)
		{
			auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);
			if (AIFire || pExt->SW_AutoFire)
			{
				SWTypeExt::ExtData::TryFire(pSuper, false);
			}
		}
	}

	return 0x509AE7;
}

DEFINE_DISABLE_HOOK(0x4C78D6, Networking_RespondToEvent_SpecialPlace_ares)

DEFINE_HOOK(0x4C78EF, Networking_RespondToEvent_SpecialPlace, 9)
{
	GET(CellStruct*, pCell, EDX);
	//GET(int , nCRC , EAX);
	GET(NetworkEvent*, pEvent, ESI);
	GET(HouseClass*, pHouse, EDI);

	auto pSuper = pHouse->Supers.Items[pEvent->Checksum];
	auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

	if (pExt->SW_UseAITargeting)
	{
		if (!SWTypeExt::ExtData::TryFire(pSuper, 1) && pHouse == HouseClass::CurrentPlayer)
		{
			pExt->PrintMessage(pExt->Message_CannotFire, pHouse);
			return 0x4C78F8;
		}
	}
	else
	{
		pHouse->Fire_SW(pEvent->Checksum, *pCell);
	}

	return 0x4C78F8;
}

// cache all super weapon statuses
struct SWStatus
{
	bool Available; //0
	bool PowerSourced; //1
	bool Charging;
};

bool AlwaysGrantedSWTYpe;
// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
std::vector<SWStatus> GetSuperWeaponStatuses(HouseClass* pHouse)
{

	std::vector<SWStatus> Statuses(pHouse->Supers.Count, { false, false, false });

	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && pHouse == HouseClass::CurrentPlayer())
	{

		for (auto pSuper : pHouse->Supers)
		{
			//if InitialReady and SWAvaible
			const auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

			if (pExt->SW_InitialReady && pExt->IsAvailable(pHouse))
			{
				Statuses[pSuper->Type->ArrayIndex].Available = true;
				Statuses[pSuper->Type->ArrayIndex].Charging = true;
			}
		}

		for (auto pBld : pHouse->Buildings)
		{
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pBld);

				// the super weapon status update lambda.
				auto UpdateStatus = [pBld, pExt, &Statuses](int idxSW)
				{
					if (idxSW > -1)
					{
						auto& status = Statuses[idxSW];
						status.Available = true;

						if (!status.Charging
							&& pBld->HasPower
							&& !pBld->IsUnderEMP()
							&& (Is_Operated(pBld) || AresData::IsOperated(pBld)))
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
				};

				// check for upgrades. upgrades can give super weapons, too.
				for (const auto& pUpgrade : pBld->Upgrades)
				{
					if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.TryFind(pUpgrade))
					{
						const auto count = pUpgradeExt->GetSuperWeaponCount();
						for (auto i = 0; i < count; ++i)
						{
							UpdateStatus(pUpgradeExt->GetSuperWeaponIndex(i, pHouse));
						}
					}
				}

				// look for the main building.
				const auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
				const auto count = pBuildingExt->GetSuperWeaponCount();
				for (auto i = 0; i < count; ++i)
				{
					UpdateStatus(pBuildingExt->GetSuperWeaponIndex(i));
				}
			}
		}

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		auto hasPower = pHouse->HasFullPower();

		for (auto pSuper : pHouse->Supers)
		{
			auto index = pSuper->Type->ArrayIndex;
			auto& status = Statuses[index];

			// turn off super weapons that are disallowed.
			if (SessionClass::Instance->GameMode != GameMode::Campaign && !Unsorted::SWAllowed)
			{
				if (pSuper->Type->DisableableFromShell)
				{
					status.Available = false;
				}
			}

			// if the house is generally on low power,
			// powered super weapons aren't powered
			if (pSuper->IsPowered())
			{
				status.PowerSourced &= hasPower;
			}
		}
	}

	return Statuses;
}

DEFINE_OVERRIDE_HOOK(0x50AF10, HouseClass_UpdateSuperWeaponsOwned, 5)
{
	GET(HouseClass*, pThis, ECX);

	auto Statuses = GetSuperWeaponStatuses(pThis);

	// now update every super weapon that is valid.
	// if this weapon has not been granted there's no need to update
	for (auto pSuper : pThis->Supers)
	{
		if (pSuper->Granted)
		{
			auto pType = pSuper->Type;
			auto index = pType->ArrayIndex;
			auto& status = Statuses[index];

			// is this a super weapon to be updated?
			// sw is bound to a building and no single-shot => create goody otherwise
			bool isCreateGoody = (!pSuper->CanHold || pSuper->OneTime);
			if (!isCreateGoody || pThis->Defeated)
			{

				// shut down or power up super weapon and decide whether
				// a sidebar tab update is needed.
				bool update = false;
				if (!status.Available || pThis->Defeated)
				{
					update = (pSuper->Lose() && HouseClass::CurrentPlayer);
				}
				else if (status.Charging && !pSuper->IsPowered())
				{
					update = pSuper->IsOnHold && pSuper->SetOnHold(false);
				}
				else if (!status.Charging && !pSuper->IsPowered())
				{
					update = !pSuper->IsOnHold && pSuper->SetOnHold(true);
				}
				else if (!status.PowerSourced)
				{
					update = (pSuper->IsPowered() && pSuper->SetOnHold(true));
				}
				else
				{
					update = (status.PowerSourced && pSuper->SetOnHold(false));
				}

				// update only if needed.
				if (update)
				{
					// only the human player can see the sidebar.
					if (pThis->IsCurrentPlayer())
					{
						if (Unsorted::CurrentSWType == index)
						{
							Unsorted::CurrentSWType = -1;
						}
						int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0);
						MouseClass::Instance->RepaintSidebar(idxTab);
					}
					pThis->RecheckTechTree = true;
				}
			}
		}
	}

	return 0x50B1CA;
}

DEFINE_OVERRIDE_HOOK(0x50B1D0, HouseClass_UpdateSuperWeaponsUnavailable, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (!pThis->Defeated)
	{
		auto Statuses = GetSuperWeaponStatuses(pThis);

		// update all super weapons not repeatedly available
		for (auto pSuper : pThis->Supers)
		{
			if (!pSuper->Granted || pSuper->OneTime)
			{
				auto index = pSuper->Type->ArrayIndex;
				auto& status = Statuses[index];

				if (status.Available)
				{
					pSuper->Grant(false, pThis->IsCurrentPlayer(), !status.PowerSourced);

					if (pThis->IsCurrentPlayer())
					{
						// hide the cameo (only if this is an auto-firing SW)
						auto pData = SWTypeExt::ExtMap.Find(pSuper->Type);
						if (pData->SW_ShowCameo || !pData->SW_AutoFire)
						{
							MouseClass::Instance->AddCameo(AbstractType::Special, index);
							int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0);
							MouseClass::Instance->RepaintSidebar(idxTab);
						}
					}
				}
			}
		}
	}

	return 0x50B36E;
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
DEFINE_OVERRIDE_HOOK(0x467E59, BulletClass_Update_NukeBall, 5)
{
	// changed the hardcoded way to just do this if the warhead is called NUKE
		// to a more universal approach. every warhead can get this behavior.
	GET(BulletClass* const, pThis, EBP);

	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	enum { Default = 0u, FireNow = 0x467F9Bu, PreImpact = 0x467EB6 };

	auto allowFlash = true;
	auto flashDuration = 0;

	// this is a bullet launched by a super weapon
	if (pExt->NukeSW && !pThis->WH->NukeMaker)
	{
		SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

		if (pThis->GetHeight() < 0)
		{
			pThis->SetHeight(0);
		}

		// cause yet another radar event
		auto const pSWTypeExt = SWTypeExt::ExtMap.Find(pExt->NukeSW->Type);

		if (pSWTypeExt->SW_RadarEvent)
		{
			auto const coords = pThis->GetMapCoords();
			RadarEventClass::Create(
				RadarEventType::SuperweaponActivated, coords);
		}

		allowFlash = pSWTypeExt->Lighting_Enabled;
		flashDuration = 30;
	}

	// does this create a flash?
	auto const duration = pWarheadExt->NukeFlashDuration.Get(flashDuration);

	if (allowFlash && duration > 0)
	{
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = duration;

		SWTypeExt::ChangeLighting(pExt->NukeSW->Type);
		MapClass::Instance->RedrawSidebar(1);
	}

	if (pWarheadExt->PreImpactAnim != -1)
	{
		R->EAX(pWarheadExt->PreImpactAnim);
		return PreImpact;
	}

	return FireNow;
}

DEFINE_OVERRIDE_HOOK(0x44CE46, BuildingClass_Mi_Missile_Pulsball, 5)
{
	GET(BuildingClass*, pThis, ESI);

	auto pPulseBall = AnimTypeClass::Find(GameStrings::PULSBALL);
	auto delay = 32;

	if (auto pSuper = AttachedSuperWeapon(pThis))
	{
		auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pPulseBall = pSWExt->EMPulse_PulseBall.Get(pPulseBall);
		delay = pSWExt->EMPulse_PulseDelay;
	}

	// also support no pulse ball
	if (pPulseBall)
	{
		auto flh = pThis->GetFLH(0, CoordStruct::Empty);
		if (auto pAnim = GameCreate<AnimClass>(pPulseBall, flh))
			pAnim->Owner = pThis->GetOwningHouse();
	}

	pThis->MissionStatus = 2;
	R->EAX(delay);
	return 0x44CEC2;
}

DEFINE_OVERRIDE_HOOK(0x44CCE7, BuildingClass_Mi_Missile_GenericSW, 6)
{
	enum { ProcessEMPulse = 0x44CD18, ReturnFromFunc = 0x44D599 };
	GET(BuildingClass* const, pThis, ESI);

	if (!pThis->Type->EMPulseCannon)
	{
		// originally, this part was related to chem missiles
		const auto pEMPTarget = EMPulseTarget(pThis);
		const auto pTarget = pEMPTarget ? pEMPTarget
			: MapClass::Instance->GetCellAt(pThis->Owner->NukeTarget);

		pThis->Fire(pTarget, 0);
		pThis->QueueMission(Mission::Guard, false);

		R->EAX(1);
		return ReturnFromFunc;
	}

	return ProcessEMPulse;
}

DEFINE_OVERRIDE_HOOK(0x44C9FF, BuildingClass_Mi_Missile_PsiWarn, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	auto const type = pThis->FiringSWType;

	if (auto const pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(type))
	{
		auto const pExt = SWTypeExt::ExtMap.Find(pSW);
		if (auto const& Anim = pExt->Nuke_PsiWarning)
		{
			R->EAX(Anim->ArrayIndex);
			return 0;
		}

		return 0x44CA7A;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44CB4C, BuildingClass_Mi_Missile_NukeTakeOff, 7)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(BulletClass*, pBullet, EDI);
	GET(CoordStruct*, pCoord, EAX);

	enum { DeleteBullet = 0x44CC42, SetUpNext = 0x44CCA7 };

	auto const type = pThis->FiringSWType;
	//auto nCos = 0.00004793836; //std::cos(1.570748388432313); // Accuracy is different from the game 
	//auto nSin = 0.99999999885; //std::sin(1.570748388432313); // Accuracy is different from the game 

	const auto nMult = pBullet->Type->Vertical ? 10.0 : 100.0;

	if (!pBullet->MoveTo(*pCoord, { 0.00004793836 * 0.00004793836 * nMult , 0.00004793836 * 0.99999999885 * nMult , 0.99999999885 * nMult }))
		return DeleteBullet;

	if (auto const pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(type))
	{
		if (auto const pAnimType = SWTypeExt::ExtMap.Find(pSW)->Nuke_TakeOff.Get(
			RulesClass::Instance->NukeTakeOff))
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, *pCoord))
			{
				if (!pAnim->ZAdjust)
					pAnim->ZAdjust = -100;

				pAnim->SetHouse(pThis->GetOwningHouse());
			}
		}
	}

	return SetUpNext;
}

DEFINE_OVERRIDE_HOOK(0x48A59A, MapClass_SelectDamageAnimation_LightningWarhead, 5)
{
	// override the lightning bolt explosion
	GET(WarheadTypeClass* const, pWarhead, ESI);
	if (auto const pSuper = SW_LightningStorm::CurrentLightningStorm)
	{
		auto const pData = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (NewSWType::GetNewSWType(pData)->GetWarhead(pData) == pWarhead)
		{
			auto const pAnimType = pData->Weather_BoltExplosion.Get(
				RulesClass::Instance->WeatherConBoltExplosion);

			if (pAnimType)
			{
				R->EAX(pAnimType);
				return 0x48A5AD;
			}
		}
	}

	return 0;
}

// this is a complete rewrite of LightningStorm::Start.
DEFINE_OVERRIDE_HOOK(0x539EB0, LightningStorm_Start, 5)
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
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

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

	if (!LightningStorm::Active)
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
			LightningStorm::Active = true;

			// blackout
			auto const outage = pExt->Weather_RadarOutage.Get(
				RulesClass::Instance->LightningStormDuration);
			if (outage > 0)
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
DEFINE_OVERRIDE_HOOK(0x53A6CF, LightningStorm_Update, 7)
{
	enum { Legacy = 0x53A8FFu, Handled = 0x53AB45u };

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

	// update other screen effects
	PsyDom::Update();
	ChronoScreenEffect::Update();

	// remove all bolts from the list that are halfway done
	for (auto i = LightningStorm::BoltsPresent->Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = LightningStorm::BoltsPresent->Items[i])
		{
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				LightningStorm::BoltsPresent->RemoveAt(i);
			}
		}
	}

	// find the clouds that should strike right now
	for (auto i = LightningStorm::CloudsManifesting->Count - 1; i >= 0; --i)
	{
		if (auto const pAnim = LightningStorm::CloudsManifesting->Items[i])
		{
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				auto const crdStrike = pAnim->GetCoords();
				LightningStorm::Strike2(crdStrike);
				LightningStorm::CloudsManifesting->RemoveAt(i);
			}
		}
	}

	// all currently present clouds have to disappear first
	if (LightningStorm::CloudsPresent->Count <= 0)
	{
		// end the lightning storm
		if (LightningStorm::TimeToEnd)
		{
			if (LightningStorm::Active)
			{
				LightningStorm::Active = false;
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
				if (pAnim->Animation.Value >= pAnimImage->Frames - 1)
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

	auto const coords = LightningStorm::Coords();

	auto const pType = pSuper->Type;
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

	// is inactive
	if (!LightningStorm::Active || LightningStorm::TimeToEnd)
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
		auto const range = NewSWType::GetNewSWType(pExt)->GetRange(pExt);
		auto const isRectangle = (range.height() <= 0);
		auto const width = range.width();
		auto const height = isRectangle ? width : range.height();

		auto const GetRandomCoords = [=]()
		{
			auto& Random = ScenarioClass::Instance->Random;
			auto const offsetX = Random.RandomRanged(-width / 2, width / 2);
			auto const offsetY = Random.RandomRanged(-height / 2, height / 2);
			auto const ret = coords + CellStruct{
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
				for (auto const& pCloud : LightningStorm::CloudsPresent.get())
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
DEFINE_OVERRIDE_HOOK(0x53A140, LightningStorm_Strike, 7)
{
	if (auto const pSuper = SW_LightningStorm::CurrentLightningStorm)
	{
		GET_STACK(CellStruct const, cell, 0x4);

		auto const pType = pSuper->Type;
		auto const pExt = SWTypeExt::ExtMap.Find(pType);

		// get center of cell coords
		auto const pCell = MapClass::Instance->GetCellAt(cell);

		// create a cloud animation
		if (auto coords = pCell->GetCoordsWithBridge())
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
						* LightningStorm::CloudHeightFactor);
				}
			}
			coords.Z += pExt->Weather_CloudHeight;

			// create the cloud and do some book keeping.
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, coords))
			{
				pAnim->SetHouse(pSuper->Owner);
				LightningStorm::CloudsManifesting->AddItem(pAnim);
				LightningStorm::CloudsPresent->AddItem(pAnim);
			}
		}

		R->EAX(true);
		return 0x53A2F1;
	}

	// legacy way for triggers.
	return 0;
}

// create bolt and damage area.
DEFINE_OVERRIDE_HOOK(0x53A300, LightningStorm_Strike2, 5)
{
	auto const pSuper = SW_LightningStorm::CurrentLightningStorm;

	if (!pSuper)
	{
		// legacy way for triggers
		return 0;
	}

	REF_STACK(CoordStruct const, refCoords, 0x4);

	auto const pType = pSuper->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(refCoords);
	const auto pNewSW = NewSWType::GetNewSWType(pData);

	if (auto const coords = pCell->GetCoordsWithBridge())
	{

		// create a bolt animation
		if (auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			auto const rnd = ScenarioClass::Instance->Random.Random();
			auto const pAnimType = it.at(rnd % it.size());

			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, coords))
			{
				pAnim->SetHouse(pSuper->Owner);
				LightningStorm::BoltsPresent->AddItem(pAnim);
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
		auto damage = pNewSW->GetDamage(pData);
		if (!pData->Weather_IgnoreLightningRod)
		{
			if (auto const pBldObj = specific_cast<BuildingClass*>(pObj))
			{
				auto const pBldType = pBldObj->Type;
				if (pBldType->LightningRod)
				{
					// multiply the damage, but never go below zero.
					auto const pBldExt = BuildingTypeExt::ExtMap.Find(pBldType);
					damage = MaxImpl(int(damage * pBldExt->LightningRod_Modifier), 0);
				}
			}
		}

		// cause mayhem
		if (damage)
		{
			auto const pWarhead = pNewSW->GetWarhead(pData);
			MapClass::FlashbangWarheadAt(
				damage, pWarhead, coords, false, SpotlightFlags::None);
			MapClass::DamageArea(
				coords, damage, nullptr, pWarhead, true, pSuper->Owner);

			// fancy stuff if damage is dealt
			auto const pAnimType = MapClass::SelectDamageAnimation(
				damage, pWarhead, pCell->LandType, coords);

			// Otamaa Add
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords))
				pAnim->SetHouse(pSuper->Owner);
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

					// Otamaa Add
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords))
						pAnim->SetHouse(pSuper->Owner);
				}
			}
		}
	}

	return 0x53A69A;
}

// completely replace the PsyDom::Fire() method.
DEFINE_OVERRIDE_HOOK(0x53B080, PsyDom_Fire, 5)
{
	if (SuperClass* pSuper = SW_PsychicDominator::CurrentPsyDom)
	{
		SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSuper->Type);
		HouseClass* pFirer = PsyDom::Owner;
		CellStruct cell = PsyDom::Coords;
		auto pNewData = NewSWType::GetNewSWType(pData);
		CellClass* pTarget = MapClass::Instance->GetCellAt(cell);
		CoordStruct coords = pTarget->GetCoords();

		// blast!
		if (pData->Dominator_Ripple)
		{
			if (auto pBlast = GameCreate<IonBlastClass>(coords))
			{
				pBlast->DisableIonBeam = TRUE;
			}
		}

		// tell!
		if (pData->SW_RadarEvent)
		{
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, cell);
		}

		// anim
		PsyDom::Anim = nullptr;
		if (AnimTypeClass* pAnimType = pData->Dominator_SecondAnim.Get(RulesClass::Instance->DominatorSecondAnim))
		{
			CoordStruct animCoords = coords;
			animCoords.Z += pData->Dominator_SecondAnimHeight;
			PsyDom::Anim = GameCreate<AnimClass>(pAnimType, animCoords);
		}

		// kill
		auto damage = pNewData->GetDamage(pData);
		if (damage > 0)
		{
			if (auto pWarhead = pNewData->GetWarhead(pData))
			{
				MapClass::Instance->DamageArea(coords, damage, nullptr, pWarhead, true, pFirer);
			}
		}

		// capture
		if (pData->Dominator_Capture)
		{
			DynamicVectorClass<FootClass*> Minions;

			auto Dominate = [pData, pFirer, &Minions](TechnoClass* pTechno) -> bool
			{
				TechnoTypeClass* pType = pTechno->GetTechnoType();

				// don't even try.
				if (pTechno->IsIronCurtained())
				{
					return true;
				}

				// ignore BalloonHover and inair units.
				if (pType->BalloonHover || pTechno->IsInAir())
				{
					return true;
				}

				// ignore units with no drivers
				if (Is_DriverKilled(pTechno))
				{
					return true;
				}

				// SW dependent stuff
				if (!pData->IsHouseAffected(pFirer, pTechno->Owner))
				{
					return true;
				}

				if (!pData->IsTechnoAffected(pTechno))
				{
					return true;
				}

				// ignore mind-controlled
				if (pTechno->MindControlledBy && !pData->Dominator_CaptureMindControlled)
				{
					return true;
				}

				// ignore permanently mind-controlled
				if (pTechno->MindControlledByAUnit && !pTechno->MindControlledBy
					&& !pData->Dominator_CapturePermaMindControlled)
				{
					return true;
				}

				// ignore ImmuneToPsionics, if wished
				if (TechnoExt::IsPsionicsImmune(pTechno) && !pData->Dominator_CaptureImmuneToPsionics)
				{
					return true;
				}

				// free this unit
				if (pTechno->MindControlledBy)
				{
					pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
				}

				// capture this unit, maybe permanently
				pTechno->SetOwningHouse(pFirer);
				pTechno->MindControlledByAUnit = pData->Dominator_PermanentCapture;

				// remove old permanent mind control anim
				if (pTechno->MindControlRingAnim)
				{
					pTechno->MindControlRingAnim->UnInit();
					pTechno->MindControlRingAnim = nullptr;
				}

				// create a permanent capture anim
				if (AnimTypeClass* pAnimType = pData->Dominator_ControlAnim.Get(RulesClass::Instance->PermaControlledAnimationType))
				{
					CoordStruct animCoords = pTechno->GetCoords();
					animCoords.Z += pType->MindControlRingOffset;
					pTechno->MindControlRingAnim = GameCreate<AnimClass>(pAnimType, animCoords);
					if (pTechno->MindControlRingAnim)
					{
						pTechno->MindControlRingAnim->SetOwnerObject(pTechno);
					}
				}

				// add to the other newly captured minions.
				if (FootClass* pFoot = generic_cast<FootClass*>(pTechno))
				{
					Minions.AddItem(pFoot);
				}

				return true;
			};

			// every techno in this area shall be one with Yuri.
			auto range = pNewData->GetRange(pData);
			Helpers::Alex::DistinctCollector<TechnoClass*> items;
			Helpers::Alex::for_each_in_rect_or_spread<TechnoClass>(cell, range.WidthOrRange, range.Height, items);
			items.apply_function_for_each(Dominate);

			// the AI sends all new minions to hunt
			if (!PsyDom::Owner->IsControlledByHuman_())
			{
				for (auto const& pFoot : Minions)
				{
					pFoot->QueueMission(Mission::Hunt, false);
				}
			}
		}

		// skip everything
		return 0x53B3EC;
	}
	return 0;
}

// replace entire function
DEFINE_OVERRIDE_HOOK(0x53C280, ScenarioClass_UpdateLighting, 5)
{
	auto lighting = SWTypeExt::GetLightingColor();

	auto scen = ScenarioClass::Instance();
	if (lighting.HasValue)
	{
		// something changed the lighting
		scen->AmbientTarget = lighting.Ambient;
		scen->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, 1);
	}
	else
	{
		// default lighting
		scen->AmbientTarget = scen->AmbientOriginal;
		scen->RecalcLighting(-1, -1, -1, 0);
	}

	return 0x53C441;
}

DEFINE_OVERRIDE_HOOK(0x555E50, LightConvertClass_CTOR_Lighting, 5)
{
	GET(LightConvertClass*, pThis, ESI);

	auto lighting = SWTypeExt::GetLightingColor();

	if (lighting.HasValue)
	{
		if (pThis->Color1.Red == -1)
		{
			pThis->Color1.Red = 1000;
			pThis->Color1.Green = 1000;
			pThis->Color1.Blue = 1000;
		}
	}
	else
	{
		lighting.Red = pThis->Color1.Red;
		lighting.Green = pThis->Color1.Green;
		lighting.Blue = pThis->Color1.Blue;
	}

	pThis->UpdateColors(lighting.Red, lighting.Green, lighting.Blue, lighting.HasValue);

	return 0x55606C;
}

DEFINE_OVERRIDE_HOOK(0x4555D5, BuildingClass_IsPowerOnline_KeepOnline, 5)
{
	GET(BuildingClass*, pThis, ESI);
	bool Contains = false;

	if (auto pOwner = pThis->GetOwningHouse())
	{
		auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);
		for (auto const& pBatt : pOwnerExt->Batteries)
		{
			const auto pExt = SWTypeExt::ExtMap.Find(pBatt->Type);
			if (pExt->Battery_KeepOnline.empty())
				continue;

			auto const Iter = std::find_if(pExt->Battery_KeepOnline.begin(),
						pExt->Battery_KeepOnline.end(),
				[&](const BuildingTypeClass* pItem) {
					return pItem == pThis->Type;
				});

			if (Iter != pExt->Battery_KeepOnline.end())
				Contains = true;
		}
	}

	R->EDI(Contains ? 0 : 2);
	return  Contains ? 0x4555DA : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x508E66, HouseClass_UpdateRadar_Battery, 8)
{
	GET(HouseClass*, pThis, ECX);
	auto pOwnerExt = HouseExt::ExtMap.Find(pThis);
	return pOwnerExt->Batteries.size() > 0 ? 0x508E87 : 0x508F2F;
}

// DEFINE_OVERRIDE_HOOK(0x44019D, BuildingClass_Update_Battery, 6)
// {
// 	GET(BuildingClass*, pThis, ESI);
//
// 	const auto& nVec = OverpoweredBuildingType(pThis->Owner);
//
// 	for (auto nPos = nVec.begin(); nPos != nVec.end(); ++nPos)
// 	{
// 		if ((*nPos) == pThis->Type)
// 			pThis->IsOverpowered = true;
// 	}
//
// 	return 0x0;
//
// }

DEFINE_OVERRIDE_HOOK(0x44019D, BuildingClass_Update_Battery, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (auto pOwner = pThis->GetOwningHouse()) {

		auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);
		for (auto const& pBatt : pOwnerExt->Batteries) {

			const auto pExt = SWTypeExt::ExtMap.Find(pBatt->Type);
			if (pExt->Battery_Overpower.empty())
				continue;

			auto const Iter = std::find_if(pExt->Battery_Overpower.begin(), 
						pExt->Battery_Overpower.end(),
				[&](const BuildingTypeClass* pItem) {
					 return !pThis->IsOverpowered && pItem == pThis->Type;
				});

			if (Iter != pExt->Battery_Overpower.end())
				pThis->IsOverpowered = true;
		}
	}

	return 0x0;
}