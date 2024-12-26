
#pragma region  Incl
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Sidebar/Body.h>

#include "Body.h"

#include <CCToolTip.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>

#include "NewSuperWeaponType/NuclearMissile.h"
#include "NewSuperWeaponType/LightningStorm.h"
#include "NewSuperWeaponType/Dominator.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Utilities/Macro.h>
#include <New/Entity/SWFirerClass.h>
#pragma endregion

DEFINE_HOOK_AGAIN(0x55B6F8, LogicClass_Updata, 0xC) //_End
DEFINE_HOOK(0x55AFB3, LogicClass_Update, 0x6) //_Early
{
	SWFirerClass::Update();
	SWStateMachine::UpdateAll();
	HouseExtData::UpdateAutoDeathObjects();
	HouseExtData::UpdateTransportReloaders();

	for (auto pHouse : *HouseClass::Array) {
		auto pExt = HouseExtContainer::Instance.Find(pHouse);
		AresHouseExt::UpdateTogglePower(pHouse);
	}

	//auto pCellbegin = MapClass::Instance->Cells.Items;
	//auto pCellCount = MapClass::Instance->Cells.Capacity;
	//auto pCellend = MapClass::Instance->Cells.Items + pCellCount;

	//for (auto begin = pCellbegin; begin != pCellend; ++begin)
	//{
	//	if(auto pCell = *begin)
	//	{
	//		std::wstring pText((size_t)(0x18 + 1), L'#');
	//		mbstowcs(&pText[0], std::to_string((int)pCell->Flags).c_str(), 0x18);
	//
	//		Point2D pixelOffset = Point2D::Empty;
	//		int width = 0, height = 0;
	//		BitFont::Instance->GetTextDimension(pText.c_str(), &width, &height, 120);
	//		pixelOffset.X -= (width / 2);
	//
	//		auto pos = TacticalClass::Instance->CoordsToView(pCell->GetCoords());
	//		pos += pixelOffset;
	//		auto bound = DSurface::Temp->Get_Rect_WithoutBottomBar();
	//
	//		auto const pOWner = HouseClass::CurrentPlayer();
	//
	//		if (!(pos.X < 0 || pos.Y < 0 || pos.X > bound.Width || pos.Y > bound.Height))
	//		{
	//			Point2D tmp { 0,0 };
	//			Fancy_Text_Print_Wide(tmp, pText.c_str(), DSurface::Temp(), bound, pos, ColorScheme::Array->Items[pOWner->ColorSchemeIndex), 0, TextPrintType::Center, 1);
	//		}
	//	}
	//}

	return 0x0;
}

DEFINE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(FakeSuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	//Debug::Log("[%s - %x] Lauch [%s - %x] \n", pSuper->Owner->get_ID() , pSuper->Owner, pSuper->Type->ID, pSuper);
	if ( SWTypeExtData::Activate(pSuper, *pCell, isPlayer) ) {
		pSuper->_GetTypeExtData()->FireSuperWeapon(pSuper, pSuper->Owner, pCell, isPlayer);
	}

	//Debug::Log("Lauch [%x][%s] %s failed \n", pSuper, pSuper->Owner->get_ID(), pSuper->Type->ID);
	return 0x6CDE40;
}

DEFINE_HOOK(0x6CEA92, SuperWeaponType_LoadFromINI_ParseAction, 0x6)
{
	GET(FakeSuperWeaponTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, EBX);
	GET(Action, result, ECX);

	INI_EX exINI(pINI);
	const auto pSection = pThis->ID;
	//Nullable<CursorTypeClass*> Cursor {};
	//Nullable<CursorTypeClass*> NoCursor {};

	//Cursor.Read(exINI, pSection, "Cursor");
	//NoCursor.Read(exINI, pSection, "NoCursor");

	if (exINI.ReadString(pSection, GameStrings::Action) > 0)
	{
		bool found = false;

		//there is no way to tell how this shit suppose to work without properly checking
		//so this one is kind a hack to allow it
		for (int i = 0; i < (int)SuperWeaponTypeClass::ActionTypeName.c_size(); ++i) {
			if (IS_SAME_STR_(SuperWeaponTypeClass::ActionTypeName[i], exINI.value())) {
				result = ((Action)(i));
				found = true;
			}
		}

		if (!found && IS_SAME_STR_("Custom", exINI.value()))
		{
			result = Action(AresNewActionType::SuperWeaponAllowed);
			found = true;
		}

		if (!found && NewSWType::FindFromTypeID(exINI.value()) != SuperWeaponType::Invalid) {
			result = Action(AresNewActionType::SuperWeaponAllowed);
			found = true;
		}

		pThis->_GetExtData()->LastAction = result;
	}


	R->EAX(result);
	return 0x6CEAA0;
}

#include <Conversions.h>
#include <TranslateFixedPoints.h>
#include <Commands/ToggleDesignatorRange.h>

//TODO : integrate this better inside ares SW ecosystems
DEFINE_HOOK(0x6CBEF4, SuperClass_AnimStage_UseWeeds, 0x6)
{
	enum
	{
		Ready = 0x6CBFEC,
		NotReady = 0x6CC064,
		ProgressInEax = 0x6CC066
	};

	GET(SuperClass*, pSuper, ECX);
	GET(FakeSuperWeaponTypeClass*, pSWType, EBX);

	auto pExt = pSWType->_GetExtData();

	if (pExt->UseWeeds)
	{
		if (pSuper->IsCharged)
			return Ready;

		if (pExt->UseWeeds_StorageTimer)
		{
			int progress = int(54.0 * pSuper->Owner->OwnedWeed.GetTotalAmount() / (double)pExt->UseWeeds_Amount);
			if (progress > 54)
				progress = 54;
			R->EAX(progress);
			return ProgressInEax;
		}
		else
		{
			return NotReady;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6CBD2C, SuperClass_AI_UseWeeds, 0x6)
{
	enum
	{
		NothingChanged = 0x6CBE9D,
		SomethingChanged = 0x6CBD48,
		Charged = 0x6CBD73
	};

	GET(FakeSuperClass*, pSuper, ESI);

	const auto pExt = pSuper->_GetTypeExtData();

	if (pExt->UseWeeds) {

		if (pSuper->Owner->OwnedWeed.GetTotalAmount() >= pExt->UseWeeds_Amount) {
			pSuper->Owner->OwnedWeed.RemoveAmount(static_cast<float>(pExt->UseWeeds_Amount), 0);
			pSuper->RechargeTimer.Start(0); // The Armageddon is here
			return Charged;
		}

		const int RechargerValue =
			pSuper->Owner->OwnedWeed.GetTotalAmount() >= pExt->UseWeeds_ReadinessAnimationPercentage / (100.0 * pExt->UseWeeds_Amount)
			// The end is nigh!
			? 15 : 915; // 61 seconds > 60 seconds (animation activation threshold)
			;

		pSuper->RechargeTimer.Start(RechargerValue);


		int animStage = pSuper->GetCameoChargeState();
		if (pSuper->CameoChargeState != animStage)
		{
			pSuper->CameoChargeState = animStage;
			return SomethingChanged;
		}

		return NothingChanged;
	}

	return 0;
}

// This is pointless for SWs using weeds because their charge is tied to weed storage.
DEFINE_HOOK(0x6CC1E6, SuperClass_SetSWCharge_UseWeeds, 0x5)
{
	enum { Skip = 0x6CC251 };

	GET(FakeSuperClass*, pSuper, EDI);
	return pSuper->_GetTypeExtData()->UseWeeds ? Skip : 0;
}

DEFINE_HOOK(0x6CEC19, SuperWeaponType_LoadFromINI_ParseType, 0x6)
{
	GET(FakeSuperWeaponTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, EBX);

	INI_EX exINI(pINI);
	const auto pSection = pThis->ID;

	if (exINI.ReadString(pSection, GameStrings::Type()) > 0)
	{
		for (int i = 0; i < (int)SuperWeaponTypeClass::SuperweaponTypeName.c_size(); ++i)
		{
			if (IS_SAME_STR_(SuperWeaponTypeClass::SuperweaponTypeName[i], exINI.value()))
			{
				pThis->Type = (SuperWeaponType)(i);
				pThis->_GetExtData()->HandledType = NewSWType::GetHandledType((SuperWeaponType)(i));
			}
		}

		if (pThis->Type == SuperWeaponType::Invalid)
		{
			const auto customType = NewSWType::FindFromTypeID(exINI.value());
			if (customType > SuperWeaponType::Invalid)
			{
				pThis->Type = customType;
			}
		}
	}

	if (exINI.ReadString(pSection, GameStrings::PreDependent()) > 0)
	{
		for (int i = 0; i < (int)SuperWeaponTypeClass::SuperweaponTypeName.c_size(); ++i)
		{
			if (IS_SAME_STR_(SuperWeaponTypeClass::SuperweaponTypeName[i], exINI.value()))
			{
				pThis->PreDependent = (SuperWeaponType)(i);
			}
		}

		if (pThis->PreDependent == SuperWeaponType::Invalid)
		{
			const auto customType = NewSWType::FindFromTypeID(exINI.value());
			if (customType > SuperWeaponType::Invalid)
			{
				pThis->PreDependent = customType;
			}
		}
	}

	return 0x6CECEF;
}

DEFINE_HOOK(0x6DBE74, Tactical_SuperLinesCircles_ShowDesignatorRange, 0x7)
{
	if (!ToggleDesignatorRangeCommandClass::ShowDesignatorRange || Unsorted::CurrentSWType < 0)
		return 0;

	const auto pSuperType = SuperWeaponTypeClass::Array()->Items[Unsorted::CurrentSWType];
	const auto pExt = SWTypeExtContainer::Instance.Find(pSuperType);

	if (!pExt->ShowDesignatorRange)
		return 0;

	for (const auto pCurrentTechno : *TechnoClass::Array)
	{
		const auto pCurrentTechnoType = pCurrentTechno->GetTechnoType();
		const auto pOwner = pCurrentTechno->Owner;
		const auto IsCurrentPlayer = pOwner->IsCurrentPlayer();

		if (!pCurrentTechno->IsAlive
			|| pCurrentTechno->InLimbo
			|| (!IsCurrentPlayer && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))                  // Ally objects are never designators or inhibitors
			|| (IsCurrentPlayer && !pExt->SW_Designators.Contains(pCurrentTechnoType))               // Only owned objects can be designators
			|| (!pOwner->IsAlliedWith(HouseClass::CurrentPlayer) && !pExt->SW_Inhibitors.Contains(pCurrentTechnoType)))  // Only enemy objects can be inhibitors
		{
			continue;
		}

		const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pCurrentTechnoType);

		const float radius = float((IsCurrentPlayer
			? pTechnoTypeExt->DesignatorRange
			: pTechnoTypeExt->InhibitorRange).Get(pCurrentTechnoType->Sight));

		CoordStruct coords = pCurrentTechno->GetCenterCoords();
		coords.Z = MapClass::Instance->GetCellFloorHeight(coords);
		Draw_Radial_Indicator(false,
		true,
		coords,
		pCurrentTechno->Owner->Color,
		radius,
		false,
		true);
	}

	return 0;
}

DEFINE_HOOK(0x41F0F1, AITriggerClass_IC_Ready, 0xA)
{
	enum { advance = 0x41F0FD, breakloop = 0x41F10D };
	GET(FakeSuperClass*, pSuper, EDI);

	return (pSuper->Type->Type == SuperWeaponType::IronCurtain
		&& pSuper->_GetTypeExtData()->IsAvailable(pSuper->Owner))
		? breakloop : advance;
}

// these thing picking first SW type with Chronosphere breaking the AI , bruh
// should check if the SW itself avaible before deciding it!
DEFINE_HOOK(0x6EFF05, TeamClass_ChronosphereTeam_PickSuper_IsAvail_A, 0x9)
{
	GET(FakeSuperClass*, pSuper, EAX);
	GET(HouseClass*, pOwner, EBP);

	return pSuper->_GetTypeExtData()->IsAvailable(pOwner) ?
		0x0//allow
		: 0x6EFF1C;//advance
}

DEFINE_HOOK(0x6F01BA, TeamClass_ChronosphereTeam_PickSuper_IsAvail_B, 0x9)
{
	GET(FakeSuperClass*, pSuper, EAX);
	GET(HouseClass*, pOwner, EDI);

	return pSuper->_GetTypeExtData()->IsAvailable(pOwner) ?
		0x0//allow
		: 0x6F01D3;//advance
}

DEFINE_HOOK(0x41F180, AITriggerClass_Chrono, 0x5)
{
	GET(AITriggerTypeClass*, pThis, ECX);
	GET_STACK(HouseClass*, pOwner, 0x4);
	GET_STACK(HouseClass*, pEnemy, 0x8);

	if (!pOwner || pOwner->Supers.Count <= 0) {
		R->EAX(false);
		return 0x41F1BA;
	}

	//Debug::Log("AITrigger[%s] With Owner[%s] Enemy[%s].\n", pThis->ID, pOwner->get_ID(), pEnemy->get_ID());
	auto iter = pOwner->Supers.find_if([pOwner](SuperClass* pItem) {
		return (pItem->Type->Type == SuperWeaponType::ChronoSphere
			&& SWTypeExtContainer::Instance.Find(pItem->Type)->IsAvailable(pOwner));
	});

	if (iter == pOwner->Supers.end() || !(*iter)->Granted) {
		R->EAX(false);
		return 0x41F1BA;
	}
	auto pSuper = *iter;
	auto v8 = pSuper->RechargeTimer.StartTime;
	auto v9 = pSuper->RechargeTimer.TimeLeft;
	const auto rechargePercent = (1.0 - RulesClass::Instance->AIMinorSuperReadyPercent);
	const auto rechargeTime = (double)pSuper->GetRechargeTime();

	if (v8 == -1) {
		const auto result1 = rechargePercent >= (v9 - (Unsorted::CurrentFrame - v8) / rechargeTime);
		R->EAX(result1);
		return 0x41F1BA;
	}

	const auto chargeTime = Unsorted::CurrentFrame - v8;
	if (chargeTime < v9) {
		v9 = (v9 - chargeTime);
		const auto result2 = rechargePercent >= (v9 / rechargeTime);
		R->EAX(result2);
		return 0x41F1BA;
	}

	const auto result3 = rechargePercent >= (0 / rechargeTime);
	R->EAX(result3);
	return 0x41F1BA;
}

#include <Ext/Team/Body.h>

DEFINE_HOOK(0x6EFC70, TeamClass_IronCurtain, 5)
{
	GET(TeamClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pTeamMission, 0x4);
	//GET_STACK(bool, barg3, 0x8);

	auto pTeamExt = TeamExtContainer::Instance.Find(pThis);
	const auto pLeader = pThis->FetchLeader();

	if (!pLeader)
	{
		pThis->StepCompleted = true;
		return 0x6EFE4F;
	}
	const auto pOwner = pThis->Owner;

	if (pOwner->Supers.Count <= 0)
	{
		pThis->StepCompleted = true;
		return 0x6EFE4F;
	}

	const bool havePower = pOwner->HasFullPower();
	SuperClass* obtain = nullptr;
	bool found = false;

	for (const auto& pSuper : pOwner->Supers)
	{
		const auto pExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

		if (!found && pExt->SW_AITargetingMode == SuperWeaponAITargetingMode::IronCurtain && pExt->SW_Group == pTeamMission->Argument)
		{
			if (!pExt->IsAvailable(pOwner))
				continue;

			// found SW that already charged , just use it and return
			if (pSuper->IsCharged && (havePower || !pSuper->IsPowered()))
			{
				obtain = pSuper;
				found = true;

				continue;
			}

			if(!obtain && pSuper->Granted)
			{
				double rechargeTime = (double)pSuper->GetRechargeTime();
				double timeLeft = (double)pSuper->RechargeTimer.GetTimeLeft();

				if ((1.0 - RulesClass::Instance->AIMinorSuperReadyPercent) < (timeLeft / rechargeTime))
				{
					obtain = pSuper;
					found = false;
					continue;
				}
			}
		}
	}

	if (found) {
		auto nCoord = pThis->SpawnCell->GetCoords();
		pOwner->Fire_SW(obtain->Type->ArrayIndex, CellClass::Coord2Cell(nCoord));
		pThis->StepCompleted = true;
		return 0x6EFE4F;
	}

	if(!found) {
		pThis->StepCompleted = true;
	}
	return 0x6EFE4F;
}

DEFINE_HOOK(0x6CEF84, SuperWeaponTypeClass_GetAction, 7)
{
	GET_STACK(CellStruct*, pMapCoords, 0x0C);
	GET(SuperWeaponTypeClass*, pType, ECX);

	const auto nAction = SWTypeExtData::GetAction(pType, pMapCoords);

	if (nAction != Action::None)
	{
		R->EAX(nAction);
		return 0x6CEFD9;
	}

	//use vanilla action
	return 0x0;
}

// 6CEE96, 5
DEFINE_HOOK(0x6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
{
	GET(const char*, TypeStr, EDI);
	auto customType = NewSWType::FindFromTypeID(TypeStr);
	if (customType > SuperWeaponType::Invalid)
	{
		R->ESI(customType);
		return 0x6CEE9C;
	}
	return 0;
}

DEFINE_HOOK(0x6AAEDF, SidebarClass_ProcessCameoClick_SuperWeapons, 6)
{
	enum
	{
		RetImpatientClick = 0x6AAFB1,
		SendSWLauchEvent = 0x6AAF10,
		ClearDisplay = 0x6AAF46
	};

	GET(int, idxSW, ESI);

	SuperClass* pSuper = HouseClass::CurrentPlayer->Supers.Items[idxSW];
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

	// if this SW is only auto-firable, discard any clicks.
	// if AutoFire is off, the sw would not be firable at all,
	// thus we ignore the setting in that case.
	const bool manual = !pData->SW_ManualFire && pData->SW_AutoFire;
	const bool unstoppable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
		&& pData->SW_Unstoppable;

	// play impatient voice, if this isn't charged yet
	if (!manual && !pSuper->CanFire())
	{
		VoxClass::PlayIndex(pSuper->Type->ImpatientVoice);
		return RetImpatientClick;
	}

	// prevent firing the SW if the player doesn't have sufficient
	// funds. play an EVA message in that case.
	if (!HouseClass::CurrentPlayer->CanTransactMoney(pData->Money_Amount))
	{
		VoxClass::PlayIndex(pData->EVA_InsufficientFunds);
		pData->PrintMessage(pData->Message_InsufficientFunds, HouseClass::CurrentPlayer);
		return RetImpatientClick;
	}

	if (!pData->SW_UseAITargeting.Get() || SWTypeExtData::IsTargetConstraintsEligible(pSuper, true))
	{
		// disallow manuals and active unstoppables
		if (manual || unstoppable)
		{
			return RetImpatientClick;
		}

		if (pSuper->Type->Action == Action::None || pData->SW_UseAITargeting.Get())
		{
			R->EAX(HouseClass::CurrentPlayer());
			return SendSWLauchEvent;
		}

		return ClearDisplay;
	}
	else
	{
		const auto pHouseID = HouseClass::CurrentPlayer->get_ID();
		Debug::Log("[%s - %x] SW [%s - %x] CannotFire\n", pHouseID, HouseClass::CurrentPlayer(), pSuper->Type->ID, pSuper);
		pData->PrintMessage(pData->Message_CannotFire, HouseClass::CurrentPlayer);
	}

	return RetImpatientClick;
}

// play a customizable target selection EVA message
DEFINE_HOOK(0x6AAF9D, SidebarClass_ProcessCameoClick_SelectTarget, 5)
{
	GET(int, index, ESI);
	VoxClass::PlayIndex(SWTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer->Supers.Items[index]->Type)->EVA_SelectTarget);
	return 0x6AB95A;
}

// 4AC20C, 7
// translates SW click to type
DEFINE_HOOK(0x4AC20C, DisplayClass_LeftMouseButtonUp, 7)
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

	R->EAX(SWTypeExtData::CurrentSWType);
	return 0x4AC21C;
}

DEFINE_HOOK(0x653B3A, RadarClass_GetMouseAction_CustomSWAction, 7)
{
	GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));
	REF_STACK(MouseEvent, flag, 0x58);

	enum
	{
		CheckOtherCases = 0x653CA3,
		DrawMiniCursor = 0x653CC0,
		NothingToDo = 0,
		DifferentEventFlags = 0x653D6F
	};

	if (Unsorted::CurrentSWType() < 0)
		return NothingToDo;

	if ((flag & (MouseEvent::RightDown | MouseEvent::RightUp)))
		return DifferentEventFlags;

	const auto nResult = SWTypeExtData::GetAction(SuperWeaponTypeClass::Array->Items[Unsorted::CurrentSWType], &MapCoords);

	if (nResult == Action::None)
		return NothingToDo; //vanilla action

	R->ESI(nResult);
	return CheckOtherCases;
}

// decoupling sw anims from types
DEFINE_HOOK(0x4463F0, BuildingClass_Place_SuperWeaponAnimsA, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (auto pSuper = BuildingExtData::GetFirstSuperWeapon(pThis))
	{
		// Do not display SuperAnimThree for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
		if (pSuper->RechargeTimer.StartTime == 0
			&& pSuper->RechargeTimer.TimeLeft == 0
			&& !SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_InitialReady
		) {
			R->ECX(pThis);
			return 0x4464F6;
		}

		R->EAX(pSuper);
		return 0x44643E;
	}
	else
	{
		if (pThis->Type->SuperWeapon >= 0)
		{
			const bool bIsDamage = !pThis->IsGreenHP();
			const int nOcc = pThis->GetOccupantCount();
			pThis->Game_PlayNthAnim(BuildingAnimSlot::SpecialThree, bIsDamage, nOcc > 0, 0);
		}
	}

	return 0x446580;
}

DEFINE_HOOK(0x450F9E, BuildingClass_ProcessAnims_SuperWeaponsA, 6)
{
	GET(BuildingClass*, pThis, ESI);
	const auto pSuper = BuildingExtData::GetFirstSuperWeapon(pThis);

	if (!pSuper)
		return 0x451145;

	const auto miss = pThis->GetCurrentMission();
	if (miss == Mission::Construction || miss == Mission::Selling || pThis->Type->ChargedAnimTime > 990.0)
		return 0x451145;

	// Do not advance SuperAnim for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
	if (pSuper->RechargeTimer.StartTime == 0
		&& pSuper->RechargeTimer.TimeLeft == 0
		&& !SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_InitialReady)
		return 0x451048;

	R->EDI(pThis->Type);
	R->EAX(pSuper);
	return 0x451030;
}

// EVA_Detected
DEFINE_HOOK(0x4468F4, BuildingClass_Place_AnnounceSW, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (auto pSuper = BuildingExtData::GetFirstSuperWeapon(pThis))
	{
		if (pSuper->Owner->IsNeutral())
			return 0x44699A;

		const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

		pData->PrintMessage(pData->Message_Detected, pThis->Owner);

		if (pData->EVA_Detected == -1 && pData->IsOriginalType() && !pData->IsTypeRedirected())
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
DEFINE_HOOK(0x6CBDD7, SuperClass_AI_AnnounceReady, 6)
{
	GET(FakeSuperWeaponTypeClass*, pThis, EAX);
	const auto pData = pThis->_GetExtData();

	pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);

	if (pData->EVA_Ready != -1 || !pData->IsOriginalType() || pData->IsTypeRedirected())
	{
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CBE68;
	}

	return 0;
}

// 6CC0EA, 9
DEFINE_HOOK(0x6CC0EA, SuperClass_ForceCharged_AnnounceQuantity, 9)
{
	GET(FakeSuperClass*, pThis, ESI);
	const auto pData = pThis->_GetTypeExtData();

	pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);

	if (pData->EVA_Ready != -1 || !pData->IsOriginalType() || pData->IsTypeRedirected())
	{
		VoxClass::PlayIndex(pData->EVA_Ready);
		return 0x6CC17E;
	}

	return 0;
}

//DEFINE_HOOK(0x6CBDB7  , SuperClass_Upadate_ChargeDrainSWReady , 0x6)
//{
//	GET(SuperClass* , pThis , ESI);
//	const auto pData = SWTypeExtContainer::Instance.Find(pThis->Type);
//
//	pData->PrintMessage(pData->Message_Ready, HouseClass::CurrentPlayer);
//
//	if (pData->EVA_Ready != -1) {
//		VoxClass::PlayIndex(pData->EVA_Ready);
//	}
//
//	return 0x0;
//}

// AI SW targeting submarines
DEFINE_HOOK(0x50CFAA, HouseClass_PickOffensiveSWTarget, 0xA)
{
	// reset weight
	R->ESI(0);

	// mark as ineligible
	R->Stack8(0x13, 0);

	return 0x50CFC9;
}

DEFINE_HOOK(0x457630, BuildingClass_SWAvailable, 9)
{
	GET(BuildingClass*, pThis, ECX);

	auto nSuper = pThis->Type->SuperWeapon2;
	if (nSuper >= 0 && !HouseExtData::IsSuperAvail(nSuper, pThis->Owner))
		nSuper = -1;

	R->EAX(nSuper);
	return 0x457688;
}

DEFINE_HOOK(0x457690, BuildingClass_SW2Available, 9)
{
	GET(BuildingClass*, pThis, ECX);

	auto nSuper = pThis->Type->SuperWeapon2;
	if (nSuper >= 0 && !HouseExtData::IsSuperAvail(nSuper, pThis->Owner))
		nSuper = -1;

	R->EAX(nSuper);
	return 0x4576E8;
}

DEFINE_HOOK(0x43BE50, BuildingClass_DTOR_HasAnySW, 6)
{
	GET(BuildingClass*, pThis, ESI);

	return (BuildingExtData::GetFirstSuperWeaponIndex(pThis) != -1)
		? 0x43BEEAu : 0x43BEF5u;
}

DEFINE_HOOK(0x449716, BuildingClass_Mi_Guard_HasFirstSW, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->FirstActiveSWIdx() != -1 ? 0x4497AFu : 0x449762u;
}

DEFINE_HOOK(0x4FAE72, HouseClass_SWFire_PreDependent, 6)
{
	GET(HouseClass*, pThis, EBX);

	// find the predependent SW. decouple this from the chronosphere.
	// don't use a fixed SW type but the very one acutually fired last.
	R->ESI(pThis->Supers.GetItemOrDefault(HouseExtContainer::Instance.Find(pThis)->SWLastIndex));

	return 0x4FAE7B;
}

DEFINE_HOOK(0x6CC2B0, SuperClass_NameReadiness, 5)
{
	GET(FakeSuperClass*, pThis, ECX);

	const auto pData = pThis->_GetTypeExtData();

	// complete rewrite of this method.

	Valueable<CSFText>* text = &pData->Text_Preparing;

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

	R->EAX((*text)->empty() ? nullptr : (*text)->Text);
	return 0x6CC352;
}

// #896002: darken SW cameo if player can't afford it
DEFINE_HOOK(0x6A99B7, StripClass_Draw_SuperDarken, 5)
{
	GET(int, idxSW, EDI);

	const auto pSW = HouseClass::CurrentPlayer->Supers.Items[idxSW];
	const auto pExt = SWTypeExtContainer::Instance.Find(pSW->Type);

	bool darken = false;
	if (pSW->IsCharged && !pSW->Owner->CanTransactMoney(pExt->Money_Amount)
		|| (pExt->SW_UseAITargeting && !SWTypeExtData::IsTargetConstraintsEligible(pSW, true)))
	{
		darken = true;
	}

	R->BL(darken);
	return 0;
}

DEFINE_HOOK(0x4F9004 ,HouseClass_Update_TrySWFire, 7)
{
	enum { UpdateAIExpert = 0x4F9015 , Continue = 0x4F9038};

	GET(HouseClass*, pThis, ESI);

	//Debug::Log("House[%s - %x , calling %s\n" , pThis->get_ID() , pThis ,__FUNCTION__);
	if(R->AL()) { // HumanControlled
		pThis->AI_TryFireSW();
		return Continue;
	}

	return pThis->Type->MultiplayPassive ? Continue : UpdateAIExpert;
}

DEFINE_HOOK(0x6CBF5B, SuperClass_GetCameoChargeStage_ChargeDrainRatio, 9)
{
	GET_STACK(int, rechargeTime1, 0x10);
	GET_STACK(int, rechargeTime2, 0x14);
	GET_STACK(int, timeLeft, 0xC);

	GET(FakeSuperWeaponTypeClass*, pType, EBX);

	// use per-SW charge-to-drain ratio.
	const double ratio = pType->_GetExtData()->GetChargeToDrainRatio();
	const double percentage = (std::fabs(rechargeTime2 * ratio) > 0.001)  ?
		1.0 - (rechargeTime1 * ratio - timeLeft) / (rechargeTime2 * ratio) : 0.0;

	R->EAX(int(percentage * 54.0));
	return 0x6CC053;
}

DEFINE_HOOK(0x6CC053, SuperClass_GetCameoChargeStage_FixFullyCharged, 5)
{
	R->EAX<int>(R->EAX<int>() > 54 ? 54 : R->EAX<int>());
	return 0x6CC066;
}

// a ChargeDrain SW expired - fire it to trigger status update
DEFINE_HOOK(0x6CBD86, SuperClass_Progress_Charged, 7)
{
	GET(SuperClass* const, pThis, ESI);
	SWTypeExtData::Deactivate(pThis, CellStruct::Empty, true);
	return 0;
}

// SW was lost (source went away)
DEFINE_HOOK(0x6CB7B0, SuperClass_Lose, 6)
{
	GET(SuperClass* const, pThis, ECX);
	auto ret = false;

	if (pThis->Granted)
	{
		pThis->IsCharged = false;
		pThis->Granted = false;

		if (SuperClass::ShowTimers->Remove(pThis))
		{
			std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
			[](SuperClass* a, SuperClass* b) {
				const auto aExt = SWTypeExtContainer::Instance.Find(a->Type);
				const auto bExt = SWTypeExtContainer::Instance.Find(b->Type);
				return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
			});
		}

		// changed
		if (pThis->Type->UseChargeDrain && pThis->ChargeDrainState == ChargeDrainState::Draining)
		{
			SWTypeExtData::Deactivate(pThis, CellStruct::Empty, false);
			pThis->ChargeDrainState = ChargeDrainState::Charging;
		}

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB810;
}

// activate or deactivate the SW
// ForceCharged on IDB
DEFINE_HOOK(0x6CB920, SuperClass_ClickFire, 5)
{
	GET(SuperClass* const, pThis, ECX);
	GET_STACK(bool const, isPlayer, 0x4);
	GET_STACK(CellStruct const* const, pCell, 0x8);

	retfunc<bool> ret(R, 0x6CBC9C);

	auto const pType = pThis->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);
	auto const pOwner = pThis->Owner;

	if (!pType->UseChargeDrain)
	{
		//change done
		if ((pThis->RechargeTimer.StartTime < 0
			|| !pThis->Granted
			|| !pThis->IsCharged)
			&& !pType->PostClick)
		{
			return ret(false);
		}

		// auto-abort if no money
		if (pOwner->CanTransactMoney(pExt->Money_Amount))
		{
			// can this super weapon fire now?
			if (auto const pNewType = pExt->GetNewSWType())
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
			}else
			if (pType->ManualControl)
			{
				// set recharge timer, then pause
				const auto time = pThis->GetRechargeTime();
				pThis->CameoChargeState = -1;
				pThis->RechargeTimer.Start(time);
				pThis->RechargeTimer.Pause();

			}else
			if (!pType->PreClick && !pType->PostClick)
			{
				pThis->StopPreclickAnim(isPlayer);
			}

			return ret(false);
		}
		else if (pOwner->IsCurrentPlayer())
		{
			VoxClass::PlayIndex(pExt->EVA_InsufficientFunds);
			pExt->PrintMessage(pExt->Message_InsufficientFunds, pOwner);
			return ret(false);
		}
	}

	if (pThis->ChargeDrainState == ChargeDrainState::Draining)
	{
		// deactivate for human players
		pThis->ChargeDrainState = ChargeDrainState::Ready;
		auto const left = pThis->RechargeTimer.GetTimeLeft();

		auto const duration = int(pThis->GetRechargeTime()
			- (left / pExt->GetChargeToDrainRatio()));
		pThis->RechargeTimer.Start(duration);
		pExt->Deactivate(pThis, *pCell, isPlayer);
		return ret(false);
	}

	if (pThis->ChargeDrainState != ChargeDrainState::Ready)
	{
		return ret(false);
	}

	// activate for human players
	pThis->ChargeDrainState = ChargeDrainState::Draining;
	auto const left = pThis->RechargeTimer.GetTimeLeft();

	auto const duration = int(
			(pThis->GetRechargeTime() - left)
			* pExt->GetChargeToDrainRatio());
	pThis->RechargeTimer.Start(duration);

	pThis->Launch(*pCell, isPlayer);

	return ret(false);
}

// rewriting OnHold to support ChargeDrain
DEFINE_HOOK(0x6CB4D0, SuperClass_SetOnHold, 6)
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
					SWTypeExtData::Deactivate(pThis, CellStruct::Empty, false);
					const auto nTime = pThis->GetRechargeTime();
					const auto nRation = pThis->RechargeTimer.GetTimeLeft() / SWTypeExtContainer::Instance.Find(pThis->Type)->GetChargeToDrainRatio();
					pThis->RechargeTimer.Start(int(nTime - nRation));
					pThis->RechargeTimer.Pause();
				}

				pThis->ChargeDrainState = ChargeDrainState::None;
			}
			else
			{
				const auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);

				pThis->ChargeDrainState = ChargeDrainState::Charging;

				if (!pExt->SW_InitialReady || HouseExtContainer::Instance.Find(pThis->Owner)
					->GetShotCount(pThis->Type).Count)
				{
					pThis->RechargeTimer.Start(pThis->GetRechargeTime());
				}
				else
				{
					pThis->ChargeDrainState = ChargeDrainState::Ready;
					pThis->ReadinessFrame = Unsorted::CurrentFrame();
					pThis->IsCharged = true;
					//pThis->IsOnHold = false;
				}
			}
		}

		ret = true;
	}

	R->EAX(ret);
	return 0x6CB555;
}

DEFINE_HOOK(0x6CBD6B, SuperClass_Update_DrainMoney, 8)
{
	// draining weapon active. take or give money. stop,
	// if player has insufficient funds.
	GET(SuperClass*, pSuper, ESI);
	GET(int, timeLeft, EAX);

	if (timeLeft > 0 && pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining)
	{
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

		const int money = pData->Money_DrainAmount;

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

	return (timeLeft ? 0x6CBE7C : 0x6CBD73);
}

// clear the chrono placement animation if not ChronoWarp
DEFINE_STRONG_HOOK(0x6CBCDE, SuperClass_Update_Animation, 5)
{
	enum { HideAnim = 0x6CBCE3 , Continue = 0x6CBCFE };

	GET(int , curSW , EAX);

	if (((size_t)curSW) >= (size_t)SuperWeaponTypeClass::Array->Count)
		return HideAnim;

	return SuperWeaponTypeClass::Array->
	Items[curSW]->Type == SuperWeaponType::ChronoWarp ?
	Continue : HideAnim;
}

// used only to find the nuke for ICBM crates. only supports nukes fully.
DEFINE_HOOK(0x6CEEB0, SuperWeaponTypeClass_FindFirstOfAction, 8)
{
	GET(Action, action, ECX);

	SuperWeaponTypeClass* pFound = nullptr;

	// this implementation is as stupid as short sighted, but it should work
	// for the moment. as there are no actions any more, this has to be
	// reworked if powerups are expanded. for now, it only has to find a nuke.
	// Otama : can be use for `TeamClass_IronCurtain` stuffs
	for (auto pType : *SuperWeaponTypeClass::Array)
	{
		if (pType->Action == action)
		{
			pFound = pType;
			break;
		}
		else
		{
			if (auto pNewSWType = SWTypeExtContainer::Instance.Find(pType)->GetNewSWType())
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

DEFINE_HOOK(0x6D49D1, TacticalClass_Draw_TimerVisibility, 5)
{
	enum
	{
		DrawSuspended = 0x6D49D8,
		DrawNormal = 0x6D4A0D,
		DoNotDraw = 0x6D4A71
	};

	GET(SuperClass*, pThis, EDX);

	const auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);

	if (!pExt->IsHouseAffected(pThis->Owner, HouseClass::CurrentPlayer(), pExt->SW_TimerVisibility))
		return DoNotDraw;

	return pThis->IsOnHold ? DrawSuspended : DrawNormal;
}

DEFINE_HOOK(0x6CB70C, SuperClass_Grant_InitialReady, 0xA)
{
	GET(SuperClass*, pSuper, ESI);

	pSuper->CameoChargeState = -1;

	if (pSuper->Type->UseChargeDrain)
		pSuper->ChargeDrainState = ChargeDrainState::Charging;

	auto pHouseExt = HouseExtContainer::Instance.Find(pSuper->Owner);
	const auto pSuperExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

	auto const [frame, count] = pHouseExt->GetShotCount(pSuper->Type);

	const int nCharge = !pSuperExt->SW_InitialReady || count ? pSuper->GetRechargeTime() : 0;

	pSuper->RechargeTimer.Start(nCharge);
	auto nFrame = Unsorted::CurrentFrame();

	if (pSuperExt->SW_VirtualCharge)
	{
		if ((frame & 0x80000000) == 0)
		{
			pSuper->RechargeTimer.StartTime = frame;
			nFrame = frame;
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
			//pSuper->IsOnHold = false;
			pSuper->ReadinessFrame = Unsorted::CurrentFrame();

			if (pSuper->Type->UseChargeDrain)
				pSuper->ChargeDrainState = ChargeDrainState::Ready;
		}
	}

	pHouseExt->UpdateShotCountB(pSuper->Type);

	return 0x6CB750;
}

DEFINE_HOOK(0x5098F0, HouseClass_Update_AI_TryFireSW, 5)
{
	GET(HouseClass*, pThis, ECX);

	//if (!pThis->Supers.IsAllocated && !pThis->Supers.IsInitialized)
	//	return 0x509AE7;

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	const bool humanControlled = pThis->IsControlledByHuman();

	for (const auto& pSuper : pThis->Supers) {
		//Debug::Log("House[%s - %x] Trying To Fire SW[%s - %x]\n" , pThis->get_ID() , pThis, pSuper->Type->ID , pSuper);
		if (pSuper->IsCharged && pSuper->ChargeDrainState != ChargeDrainState::Draining) {
			if (!humanControlled || SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_AutoFire) {
				SWTypeExtData::TryFire(pSuper, false);
			}
		}
	}

	return 0x509AE7;
}

#include <EventClass.h>

DEFINE_HOOK(0x4C78D6, Networking_RespondToEvent_SpecialPlace, 8)
{
	//GET(CellStruct*, pCell, EDX);
	//GET(int, Checksum, EAX);
	GET(EventClass*, pEvent, ESI);
	GET(HouseClass*, pHouse, EDI);

	const auto& specialplace = pEvent->Data.SpecialPlace;
	const auto pSuper = pHouse->Supers.Items[specialplace.ID];
	const auto pExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

	if (pExt->SW_UseAITargeting)
	{
		if (!SWTypeExtData::TryFire(pSuper, 1))
		{
			const auto pHouseID = pHouse->get_ID();

			if(pHouse == HouseClass::CurrentPlayer){
				Debug::Log("[%s - %x] SW [%s - %x] CannotFire\n", pHouseID, pHouse, pSuper->Type->ID, pSuper);
				pExt->PrintMessage(pExt->Message_CannotFire, pHouse);
			} else {
				Debug::Log("[%s - %x] SW [%s - %x] AI CannotFire\n", pHouseID, pHouse,  pSuper->Type->ID, pSuper);
			}
		}
	}
	else
	{
		pHouse->Fire_SW(specialplace.ID, specialplace.Location);
	}

	return 0x4C78F8;
}

DEFINE_HOOK(0x50AF10, HouseClass_UpdateSuperWeaponsOwned, 5)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->IsNeutral())
		return 0x50B1CA;

	SuperExtData::UpdateSuperWeaponStatuses(pThis);

	// now update every super weapon that is valid.
	// if this weapon has not been granted there's no need to update
	for (auto& pSuper : pThis->Supers)
	{
		if (pSuper->Granted)
		{
			auto pType = pSuper->Type;
			auto index = pType->ArrayIndex;
			auto& status = SuperExtContainer::Instance.Find(pSuper)->Statusses;

			auto Update = [&]()
			{
				// only the human player can see the sidebar.
				if (pThis->IsCurrentPlayer())
				{
					if (Unsorted::CurrentSWType == index)
						Unsorted::CurrentSWType = -1;

					MouseClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0));
				}
				pThis->RecheckTechTree = true;
			};

			// is this a super weapon to be updated?
			// sw is bound to a building and no single-shot => create goody otherwise
			if (pSuper->CanHold && !pSuper->OneTime || pThis->Defeated)
			{
				if (!status.Available || pThis->Defeated)
				{
					if ((pSuper->Lose() && HouseClass::CurrentPlayer))
						Update();
				}
				else if (status.Charging && !pSuper->IsPowered())
				{
					if (pSuper->IsOnHold && pSuper->SetOnHold(false))
						Update();
				}
				else if (!status.Charging && !pSuper->IsPowered())
				{
					if (!pSuper->IsOnHold && pSuper->SetOnHold(true))
						Update();
				}
				else if (!status.PowerSourced)
				{
					if (pSuper->IsPowered() && pSuper->SetOnHold(true))
						Update();
				}
				else
				{
					if (status.PowerSourced && pSuper->SetOnHold(false))
						Update();
				}
			}
		}
	}

	return 0x50B1CA;
}

DEFINE_HOOK(0x50B1D0, HouseClass_UpdateSuperWeaponsUnavailable, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->IsNeutral())
		return 0x50B1CA;

	if (!pThis->Defeated && !(pThis->IsObserver()))
	{
		SuperExtData::UpdateSuperWeaponStatuses(pThis);
		const bool IsCurrentPlayer = pThis->IsCurrentPlayer();

		// update all super weapons not repeatedly available
		for (auto& pSuper : pThis->Supers)
		{
			if (!pSuper->Granted || pSuper->OneTime)
			{
				auto index = pSuper->Type->ArrayIndex;
				const auto pExt = SuperExtContainer::Instance.Find(pSuper);
				auto& status = pExt->Statusses;

				if (status.Available)
				{
					pSuper->Grant(false, IsCurrentPlayer, !status.PowerSourced);

					if (IsCurrentPlayer)
					{
						// hide the cameo (only if this is an auto-firing SW)
						if (pExt->Type->SW_ShowCameo || !pExt->Type->SW_AutoFire)
						{
							MouseClass::Instance->AddCameo(AbstractType::Special, index);
							MouseClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0));
						}
					}
				}
			}
		}
	}

	return 0x50B36E;
}

// create a downward pointing missile if the launched one leaves the map.
DEFINE_HOOK(0x46B371, BulletClass_NukeMaker, 5)
{
	GET(BulletClass* const, pThis, EBP);

	SuperWeaponTypeClass* pNukeSW = nullptr;

	if (auto const pNuke = BulletExtContainer::Instance.Find(pThis)->NukeSW)
	{
		pNukeSW = pNuke;
	}
	else if (auto pLinkedNuke = SuperWeaponTypeClass::Array->
		GetItemOrDefault(WarheadTypeExtContainer::Instance.Find(pThis->WH)->NukePayload_LinkedSW))
	{
		pNukeSW = pLinkedNuke;
	}

	if (pNukeSW)
	{
		auto const pSWExt = SWTypeExtContainer::Instance.Find(pNukeSW);

		// get the per-SW nuke payload weapon
		if (WeaponTypeClass const* const pPayload = pSWExt->Nuke_Payload)
		{

			// these are not available during initialisation, so we gotta
			// fall back now if they are invalid.
			auto const damage = NewSWType::GetNewSWType(pSWExt)->GetDamage(pSWExt);
			auto const pWarhead = NewSWType::GetNewSWType(pSWExt)->GetWarhead(pSWExt);

			// put the new values into the registers
			R->Stack(0x30, R->EAX());
			R->ESI(pPayload);
			R->Stack(0x10, 0);
			R->Stack(0x18, pPayload->Speed);
			R->Stack(0x28, pPayload->Projectile);
			R->EAX(pWarhead);
			R->ECX(R->lea_Stack<CoordStruct*>(0x10));
			R->EDX(damage);

			return 0x46B3B7;
		}
		else
		{
			Debug::Log(
				"[%s] has no payload weapon type, or it is invalid.\n",
				pNukeSW->ID);
		}
	}

	return 0;
}

// just puts the launched SW pointer on the downward aiming missile.
DEFINE_HOOK(0x46B423, BulletClass_NukeMaker_PropagateSW, 6)
{
	GET(BulletClass* const, pThis, EBP);
	GET(BulletClass* const, pNuke, EDI);

	auto const pThisExt = BulletExtContainer::Instance.Find(pThis);
	auto const pNukeExt = BulletExtContainer::Instance.Find(pNuke);
	pNukeExt->NukeSW = pThisExt->NukeSW;
	pNuke->Owner = pThis->Owner;
	pNukeExt->Owner = pNukeExt->Owner;

	return 0;
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
DEFINE_HOOK(0x467E59, BulletClass_Update_NukeBall, 5)
{
	// changed the hardcoded way to just do this if the warhead is called NUKE
		// to a more universal approach. every warhead can get this behavior.
	GET(BulletClass* const, pThis, EBP);

	auto const pExt = BulletExtContainer::Instance.Find(pThis);
	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

	enum { Default = 0u, FireNow = 0x467F9Bu, PreImpact = 0x467ED0 };

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
		auto const pSWTypeExt = SWTypeExtContainer::Instance.Find(pExt->NukeSW);

		if (pSWTypeExt->SW_RadarEvent)
		{
			auto const coords = pThis->GetMapCoords();
			RadarEventClass::Create(
				RadarEventType::SuperweaponActivated, coords);
		}

		if (pSWTypeExt->Lighting_Enabled.isset())
			allowFlash = pSWTypeExt->Lighting_Enabled.Get();
	}

	// does this create a flash?
	auto const duration = pWarheadExt->NukeFlashDuration.Get();

	if (allowFlash && duration > 0)
	{
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = duration;

		SWTypeExtData::ChangeLighting(pExt->NukeSW ? pExt->NukeSW : nullptr);
		MapClass::Instance->RedrawSidebar(1);
	}

	if (auto pPreImpact = pWarheadExt->PreImpactAnim.Get())
	{
		R->EDI(pPreImpact);
		return PreImpact;
	}

	return FireNow;
}

//create nuke pointing down to the target
//DEFINE_HOOK(0x46B310, BulletClass_NukeMaker_Handle, 6)
//{
//	GET(BulletClass*, pThis, ECX);
//
//	enum { ret = 0x46B53C };
//
//	const auto pTarget = pThis->Target;
//	if (!pTarget)
//	{
//		Debug::Log("Bullet[%s] Trying to Apply NukeMaker but has invalid target !\n", pThis->Type->ID);
//		return ret;
//	}
//
//	WeaponTypeClass* pPaylod = nullptr;
//	SuperWeaponTypeClass* pNukeSW = nullptr;
//
//	if (auto const pNuke = BulletExtContainer::Instance.Find(pThis)->NukeSW)
//	{
//		pPaylod = SWTypeExtContainer::Instance.Find(pNuke)->Nuke_Payload;
//		pNukeSW = pNuke;
//	}
//	else if (auto pLinkedNuke = SuperWeaponTypeClass::Array->
//		GetItemOrDefault(WarheadTypeExtContainer::Instance.Find(pThis->WH)->NukePayload_LinkedSW))
//	{
//		pPaylod = SWTypeExtContainer::Instance.Find(pLinkedNuke)->Nuke_Payload;
//		pNukeSW = pLinkedNuke;
//	}
//	else
//	{
//		pPaylod = WeaponTypeClass::Find(GameStrings::NukePayload());
//	}
//
//	if (!pPaylod || !pPaylod->Projectile)
//	{
//		Debug::Log("Bullet[%s] Trying to Apply NukeMaker but has invalid Payload Weapon or Payload Weapon Projectile !\n",
//			pThis->Type->ID);
//		return ret;
//	}
//	auto targetcoord = pTarget->GetCoords();
//
//	R->EAX(SW_NuclearMissile::DropNukeAt(pNukeSW, targetcoord, nullptr, pThis->Owner ? pThis->Owner->Owner : HouseExtData::FindFirstCivilianHouse(), pPaylod));
//	return ret;
//}

namespace EMPulseCannonTemp
{
	int weaponIndex = 0;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mi_Missile_EMPPulseBulletWeapon, 0x8)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBP);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFSET(0xF0, -0xA4));
	LEA_STACK(CoordStruct*, pCoord, STACK_OFFSET(0xF0, -0x8C));

	if (pWeapon && pBullet)
	{
		pBullet->SetWeaponType(pWeapon);

		CoordStruct src;
		pThis->GetFLH(&src , EMPulseCannonTemp::weaponIndex, pThis->GetRenderCoords());
		CoordStruct dest = *pCoord;
		auto const pTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->GetCellAt(dest);

		// Draw bullet effect
		Helpers_DP::DrawBulletEffect(pWeapon, src, dest, pThis, pTarget);
		// Draw particle system
		Helpers_DP::AttachedParticleSystem(pWeapon, src, pTarget, pThis, dest);
		// Play report sound
		Helpers_DP::PlayReportSound(pWeapon, src, pThis);
		// Draw weapon anim
		Helpers_DP::DrawWeaponAnim(pWeapon, src, dest, pThis, pTarget);
	}

	return 0;
}

#include <Ext/Anim/Body.h>

DEFINE_HOOK(0x44CE46, BuildingClass_Mi_Missile_EMPulse_Pulsball, 5)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pSWExt = SWTypeExtContainer::Instance.Find(TechnoExtContainer::Instance.Find(pThis)->LinkedSW->Type);
	const auto delay = pSWExt->EMPulse_PulseDelay;

	// also support no pulse ball
	if (auto pPulseBall = pSWExt->EMPulse_PulseBall) {
		CoordStruct flh;
		pThis->GetFLH(&flh , 0, CoordStruct::Empty);
		auto pAnim = GameCreate<AnimClass>(pPulseBall, flh);
		pAnim->Owner = pThis->GetOwningHouse();
		((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
	}

	pThis->MissionStatus = 2;
	R->EAX(delay);
	return 0x44CEC2;
}

// this one setting the building target
// either it is non EMPulse or EMPulse
DEFINE_HOOK(0x44CCE7, BuildingClass_Mi_Missile_GenericSW, 6)
{
	enum { ProcessEMPulse = 0x44CD18, ReturnFromFunc = 0x44D599 };
	GET(BuildingClass* const, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pThis->Type->EMPulseCannon)
	{
		// originally, this part was related to chem missiles
		const auto pTarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->NukeTarget;

		pThis->Fire(MapClass::Instance->GetCellAt(pTarget), 0);
		pThis->QueueMission(Mission::Guard, false);

		R->EAX(1);
		return ReturnFromFunc;
	}

	if (pExt->SuperTarget.IsValid())
	{
		pThis->Owner->EMPTarget = pExt->SuperTarget;
	}

	return ProcessEMPulse;
}

DEFINE_HOOK(0x44CEEC, BuildingClass_Mission_Missile_EMPulseSelectWeapon, 0x6)
{
	enum { SkipGameCode = 0x44CEF8 };

	GET(BuildingClass*, pThis, ESI);

	int weaponIndex = 0;
	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis->Owner);
	const auto pLinked = TechnoExtContainer::Instance.Find(pThis)->LinkedSW;

	if (pHouseExt->EMPulseWeaponIndex >= 0)
	{
		weaponIndex = pHouseExt->EMPulseWeaponIndex;
	}
	else if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->EMPulseCannon_UseWeaponSelection) {
		if (auto const pCell = MapClass::Instance->TryGetCellAt(pThis->Owner->EMPTarget)) {
			AbstractClass* pTarget = pCell;

			if (auto const pObject = pCell->GetContent())
				pTarget = pObject;

			weaponIndex = pThis->SelectWeapon(pTarget);
		}
	}

	auto const pSWExt = SWTypeExtContainer::Instance.Find(pLinked->Type);

	//why this fuckery even exist ,..
	//the SW can be state can be exploited at some point , smh
	if (pSWExt->EMPulse_SuspendOthers) {
		auto iter = pHouseExt->SuspendedEMPulseSWs.get_key_iterator(pLinked);

		if (iter != pHouseExt->SuspendedEMPulseSWs.end()) {
			for (auto const& pSuper : iter->second) {
				pSuper->IsOnHold = false;
			}

			pHouseExt->SuspendedEMPulseSWs.erase(iter);
		}
	}

	EMPulseCannonTemp::weaponIndex = weaponIndex;
	R->EAX(pThis->GetWeapon(weaponIndex));
	return SkipGameCode;
}

#include <Misc/Hooks.Otamaa.h>

CoordStruct* FakeBuildingClass::_GetFLH(CoordStruct* pCrd, int weaponIndex)
{
	CoordStruct coords {};
	MapClass::Instance->GetCellAt(this->Owner->EMPTarget)->GetCellCoords(&coords);
	pCrd = this->GetFLH(&coords, EMPulseCannonTemp::weaponIndex, *pCrd);
	return pCrd;
}
DEFINE_JUMP(CALL6, 0x44D1F9, MiscTools::to_DWORD(&FakeBuildingClass::_GetFLH));

DEFINE_HOOK(0x44C9F3, BuildingClass_Mi_Missile_PsiWarn, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(HouseClass*, pOwner, EBP);
	GET(CellClass*, pCell, EAX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	AnimClass* PsiWarn =
	BulletClass::CreateDamagingBulletAnim(pOwner,
	pCell,
	nullptr,
	SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Nuke_PsiWarning
	);

	R->EDI(PsiWarn);
	return 0x44CA74;
}

// Create bullet pointing up to the sky
DEFINE_HOOK(0x44CA97, BuildingClass_MI_Missile_CreateBullet, 0x6)
{
	enum
	{
		SkipGameCode = 0x44CAF2,
		DeleteBullet = 0x44CC42,
		SetUpNext = 0x44CCA7,
		SetRate = 0x44CCB1
	};

	GET(BuildingClass* const, pThis, ESI);

	auto pTarget = MapClass::Instance->GetCellAt(pThis->Owner->NukeTarget);
	auto pSuper = TechnoExtContainer::Instance.Find(pThis)->LinkedSW;

	if (WeaponTypeClass* pWeapon = pSuper->Type->WeaponType)
	{
		//speed harcoded to 255
		if (auto pCreated = pWeapon->Projectile->CreateBullet(pTarget, pThis, pWeapon->Damage, pWeapon->Warhead, 255, pWeapon->Bright || pWeapon->Warhead->Bright))
		{
			BulletExtContainer::Instance.Find(pCreated)->NukeSW = pSuper->Type;
			pCreated->Range = WeaponTypeExtContainer::Instance.Find(pWeapon)->GetProjectileRange();
			pCreated->SetWeaponType(pWeapon);

			if (pThis->PsiWarnAnim)
			{
				pThis->PsiWarnAnim->SetBullet(pCreated);
				pThis->PsiWarnAnim = nullptr;
			}

			//Limbo-in the bullet will remove the `TechnoClass` owner from the bullet !
			//pThis->Limbo();

			CoordStruct nFLH;
			 pThis->GetFLH(&nFLH , 0, CoordStruct::Empty);

			// Otamaa : the original calculation seems causing missile to be invisible
			//auto nCos = 0.00004793836;
			//auto nCos = Math::cos(1.570748388432313); // Accuracy is different from the game
			//auto nSin = 0.99999999885;
			//auto nSin = Math::sin(1.570748388432313);// Accuracy is different from the game

			const auto nMult = pCreated->Type->Vertical ? 10.0 : 100.0;
			//const auto nX = nCos * nCos * nMult;
			//const auto nY = nCos * nSin * nMult;
			//const auto nZ = nSin * nMult;

			if (!pCreated->MoveTo(nFLH, { 0.0, 0.0 , nMult }))
				return DeleteBullet;

			if (auto const pAnimType = SWTypeExtContainer::Instance.Find(pSuper->Type)->Nuke_TakeOff.Get(RulesClass::Instance->NukeTakeOff))
			{
				auto pAnim = GameCreate<AnimClass>(pAnimType, nFLH);
				if (!pAnim->ZAdjust)
					pAnim->ZAdjust = -100;

				pAnim->SetHouse(pThis->GetOwningHouse());
				((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
			}

			return SetUpNext;
		}
	}

	return SetRate;

}

// this is a complete rewrite of LightningStorm::Start.
DEFINE_HOOK(0x539EB0, LightningStorm_Start, 5)
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
DEFINE_HOOK(0x53A6CF, LightningStorm_Update, 7)
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
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				LightningStorm::BoltsPresent->RemoveAt<true>(i);
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
			if (pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2)
			{
				auto const crdStrike = pAnim->GetCoords();
				LightningStorm::Strike2(crdStrike);
				LightningStorm::CloudsManifesting->RemoveAt<true>(i);
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
				if (pAnim->Animation.Value >= pAnimImage->Frames - 1)
				{
					LightningStorm::CloudsPresent->RemoveAt<true>(i);
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
				const auto& nRodTypes = pExt->Weather_LightningRodTypes;

				if (pCellBld && pCellBld->Type->LightningRod)
				{
					if (nRodTypes.empty() || nRodTypes.Contains(pCellBld->Type))
						return ret;
				}

				// if a lightning rod is next to this, hit that instead. naive.
				if (auto const pObj = pCell->FindTechnoNearestTo(
					Point2D::Empty, false, pCellBld))
				{
					if (auto const pBld = cast_to<BuildingClass*>(pObj))
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
DEFINE_HOOK(0x53A140, LightningStorm_Strike, 7)
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

			if(auto const pAnimType = itClouds.at(ScenarioClass::Instance->Random.RandomFromMax(itClouds.size() - 1))) {
				if(pAnimType->GetImage()) {
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
DEFINE_HOOK(0x53A300, LightningStorm_Strike2, 5)
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
	auto const coords = pCell->GetCoordsWithBridge();

	if (coords.IsValid())
	{
		// create a bolt animation
		if (auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			if(auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1)))
			{
				if(pAnimType->GetImage()) {
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
			VocClass::PlayIndexAtPos(it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1)), coords, nullptr);
		}

		auto debris = false;
		auto const pBld = pCell->GetBuilding();

		auto const empty = Point2D::Empty;
		auto const pObj = pCell->FindTechnoNearestTo(empty, false, nullptr);
		auto const isInfantry = cast_to<InfantryClass*>(pObj) != nullptr;

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

			MapClass::DamageArea(
				coords, damage, nullptr, pWarhead, true, pSuper->Owner);

			// fancy stuff if damage is dealt
			if(auto const pAnimType = MapClass::SelectDamageAnimation(
				damage, pWarhead, pCell->LandType, coords)) {
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

				for (int i = 0; i < count; ++i) {
					if(auto const pAnimType = it.at(ScenarioClass::Instance->Random.RandomFromMax(it.size() - 1))){
						auto pAnim = GameCreate<AnimClass>(pAnimType, coords);
						pAnim->SetHouse(pSuper->Owner);
					}
				}
			}
		}
	}

	return 0x53A69A;
}

// completely replace the PsyDom::Fire() method.
DEFINE_HOOK(0x53B080, PsyDom_Fire, 5)
{
	if (SuperClass* pSuper = SW_PsychicDominator::CurrentPsyDom)
	{
		const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
		HouseClass* pFirer = PsyDom::Owner;
		CellStruct cell = PsyDom::Coords;
		auto pNewData = pData->GetNewSWType();
		CellClass* pTarget = MapClass::Instance->GetCellAt(cell);
		CoordStruct coords = pTarget->GetCoords();

		// blast!
		if (pData->Dominator_Ripple)
		{
			auto pBlast = GameCreate<IonBlastClass>(coords);
			pBlast->DisableIonBeam = TRUE;
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
			auto pCreated = GameCreate<AnimClass>(pAnimType, animCoords);
			pCreated->SetHouse(PsyDom::Owner);
			PsyDom::Anim = pCreated;
		}

		// kill
		auto damage = pNewData->GetDamage(pData);
		auto pWarhead = pNewData->GetWarhead(pData);

		if (pWarhead && damage != 0)
		{

			//this update every frame , so getting the firer here , seems degreading the performance ,..
			MapClass::Instance->DamageArea(coords, damage, pNewData->GetFirer(pSuper, cell, false), pWarhead, true, pFirer);
		}

		// capture
		if (pData->Dominator_Capture)
		{
			// every techno in this area shall be one with Yuri.
			auto const [widthORange, Height] = pNewData->GetRange(pData);
			Helpers::Alex::DistinctCollector<TechnoClass*> items;
			Helpers::Alex::for_each_in_rect_or_spread<TechnoClass>(cell, widthORange, Height, items);
			items.apply_function_for_each([pData, pFirer](TechnoClass* pTechno)
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
				if (TechnoExtContainer::Instance.Find(pTechno)->Is_DriverKilled)
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
				if (TechnoExtData::IsPsionicsImmune(pTechno) && !pData->Dominator_CaptureImmuneToPsionics)
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
					pTechno->MindControlRingAnim->TimeToDie = true;
					pTechno->MindControlRingAnim->UnInit();
					pTechno->MindControlRingAnim = nullptr;
				}

				// create a permanent capture anim
				if (AnimTypeClass* pAnimType = pData->Dominator_ControlAnim.Get(RulesClass::Instance->PermaControlledAnimationType))
				{
					CoordStruct animCoords = pTechno->GetCoords();
					bool Isbuilding = false;

					if(pTechno->WhatAmI() != BuildingClass::AbsID)
						animCoords.Z += pType->MindControlRingOffset;
					else
					{
						Isbuilding = true;
						animCoords.Z += ((BuildingClass*)pTechno)->Type->Height;
					}

					pTechno->MindControlRingAnim = GameCreate<AnimClass>(pAnimType, animCoords);
					pTechno->MindControlRingAnim->SetOwnerObject(pTechno);

					if(Isbuilding)
						pTechno->MindControlRingAnim->ZAdjust = -1024;
				}

				// add to the other newly captured minions.
				if (FootClass* pFoot = flag_cast_to<FootClass*>(pTechno))
				{
					// the AI sends all new minions to hunt
					const auto nMission = pFoot->GetTechnoType()->ResourceGatherer ? Mission::Harvest :
						!PsyDom::Owner->IsControlledByHuman() ? Mission::Hunt : Mission::Guard;

					pFoot->QueueMission(nMission, false);
				}

				return true;
			});
		}

		// skip everything
		return 0x53B3EC;
	}
	return 0;
}

// replace entire function
DEFINE_HOOK(0x53C280, ScenarioClass_UpdateLighting, 5)
{
	const auto lighting = SWTypeExtData::GetLightingColor();

	if (lighting.HasValue)
	{
		// something changed the lighting
		ScenarioClass::Instance->AmbientTarget = lighting.Ambient;
		ScenarioClass::Instance->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, true);
	}
	else
	{
		// default lighting
		ScenarioClass::Instance->AmbientTarget = ScenarioClass::Instance->AmbientOriginal;
		ScenarioClass::Instance->RecalcLighting(-1, -1, -1, false);
	}

	return 0x53C441;
}

DEFINE_HOOK(0x555E50, LightConvertClass_CTOR_Lighting, 5)
{
	GET(LightConvertClass*, pThis, ESI);

	auto lighting = SWTypeExtData::GetLightingColor();

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

DEFINE_HOOK(0x4555D5, BuildingClass_IsPowerOnline_KeepOnline, 5)
{
	GET(BuildingClass*, pThis, ESI);
	bool Contains = false;

	if (auto pOwner = pThis->GetOwningHouse())
	{
		for (auto const& pBatt : HouseExtContainer::Instance.Find(pOwner)->Batteries)
		{
			if (SWTypeExtContainer::Instance.Find(pBatt->Type)->Battery_KeepOnline.Contains(pThis->Type))
			{
				Contains = true;
				break;
			}
		}
	}

	R->EDI(Contains ? 0 : 2);
	return  Contains ? 0x4555DA : 0x0;
}

DEFINE_HOOK(0x508E66, HouseClass_UpdateRadar_Battery, 8)
{
	GET(HouseClass*, pThis, ECX);
	return !HouseExtContainer::Instance.Find(pThis)->Batteries.empty()
		? 0x508E87 : 0x508F2F;
}

DEFINE_HOOK(0x44019D, BuildingClass_Update_Battery, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Owner && !pThis->IsOverpowered) {
		for (auto const& pBatt : HouseExtContainer::Instance.Find(pThis->Owner)->Batteries) {
			if (SWTypeExtContainer::Instance.Find(pBatt->Type)->Battery_Overpower.Contains(pThis->Type)) {
				pThis->IsOverpowered = true;
				break;
			}
		}
	}

	return 0x0;
}

#include <Ext/HouseType/Body.h>
#include <Misc/Ares/Hooks/Header.h>

ConvertClass* SWConvert = nullptr;
BSurface* CameoPCXSurface = nullptr;

DEFINE_HOOK(0x6A9948, StripClass_Draw_SuperWeapon, 6)
{
	GET(SuperWeaponTypeClass*, pSuper, EAX);

	if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPalette)
		SWConvert = pManager->GetConvert<PaletteManager::Mode::Default>();

	return 0x0;
}

DEFINE_HOOK(0x6A9A2A, StripClass_Draw_Main, 6)
{
	GET_STACK(TechnoTypeClass*, pTechno, 0x6C);

	ConvertClass* pResult = nullptr;
	if (pTechno) {
		if(auto pPal = TechnoTypeExtContainer::Instance.TryFind(pTechno)->CameoPal) {
			pResult = pPal->GetConvert<PaletteManager::Mode::Default>();
		}
	}
	else
		pResult = SWConvert;

	R->EDX(pResult ? pResult : FileSystem::CAMEO_PAL());
	return 0x6A9A30;
}

DEFINE_HOOK(0x6A9952, StripClass_Draw_SuperWeapon_PCX, 6)
{
	GET(SuperWeaponTypeClass*, pSuper, EAX);
	CameoPCXSurface = SWTypeExtContainer::Instance.Find(pSuper)->SidebarPCX.GetSurface();
	return 0x0;
}

DEFINE_HOOK(0x6A980A, StripClass_Draw_TechnoType_PCX, 8)
{
	GET(TechnoTypeClass*, pType, EBX);

	CameoPCXSurface = TechnoTypeExt_ExtData::GetPCXSurface(pType , HouseClass::CurrentPlayer);

	return 0;
}

DEFINE_HOOK(0x6A99F3, StripClass_Draw_SkipSHPForPCX, 6)
{
	if(CameoPCXSurface)
		return 0x6A9A43;

	GET_STACK(SHPStruct const*, pCameo, STACK_OFFS(0x48C, 0x444));

	if (pCameo)
	{
		auto pCameoRef = pCameo->AsReference();
		char pFilename[0x20];
		strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
		_strlwr_s(pFilename);

		if (!_stricmp(pCameoRef->Filename, GameStrings::XXICON_SHP())
			&& (strstr(pFilename, ".pcx")))
		{
			BSurface* pCXSurf = nullptr;

			if (PCX::Instance->LoadFile(pFilename))
				pCXSurf = PCX::Instance->GetSurface(pFilename);

			if (pCXSurf)
			{
				GET(int, destX, ESI);
				GET(int, destY, EBP);

				RectangleStruct bounds { destX, destY, 60, 48 };
				PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, pCXSurf);

				return 0x6A9A43; //skip drawing shp cameo
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A9A43, StripClass_Draw_DrawPCX, 6)
{
	if (CameoPCXSurface)
	{
		GET(int, TLX, ESI);
		GET(int, TLY, EBP);
		RectangleStruct bounds { TLX, TLY, 60, 48 };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCXSurface, Drawing::ColorStructToWordRGB(Drawing::DefaultColors[6]));
		CameoPCXSurface = nullptr;
	}

	return 0;
}

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(0x712045, TechnoTypeClass_GetCameo, 5)
{
	GET(TechnoTypeClass*, pThis, ECX);

	R->EAX(TechnoTypeExt_ExtData::CameoIsElite(pThis, HouseClass::CurrentPlayer)
		? pThis->AltCameo : pThis->Cameo);
	return 0x7120C6;
}

int FakeBuildingClass::_Mission_Missile() {

	if (!TechnoExtContainer::Instance.Find(this)->LinkedSW && this->MissionStatus < 3) {
		Debug::Log("Building[%s] with Mission::Missile Missing Important Linked SW data !\n", this->get_ID());
	} else if (TechnoExtContainer::Instance.Find(this)->LinkedSW && this->MissionStatus >= 3) {
		TechnoExtContainer::Instance.Find(this)->LinkedSW = nullptr;
	}

	return this->BuildingClass::Mission_Missile();
}
DEFINE_JUMP(VTABLE, 0x7E410C, MiscTools::to_DWORD(&FakeBuildingClass::_Mission_Missile));