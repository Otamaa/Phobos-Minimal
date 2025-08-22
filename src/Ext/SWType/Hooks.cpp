
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

#include <Misc/DamageArea.h>
#include <Commands/Harmless.h>

#include <FPSCounter.h>
#include <EventClass.h>

#pragma endregion

//ASMJIT_PATCH_AGAIN(0x55B6F8, LogicClass_Update, 0xC) //_End
std::chrono::high_resolution_clock::time_point lastFrameTime;

//separate the function
ASMJIT_PATCH(0x55B719, LogicClass_Update_late, 0x5)
{
	HarmlessCommandClass::AI();
	SWFirerClass::Update();
	SWStateMachine::UpdateAll();
	HouseExtData::UpdateAutoDeathObjects();
	HouseExtData::UpdateTransportReloaders();

	for (auto pHouse : *HouseClass::Array) {
		AresHouseExt::UpdateTogglePower(pHouse);
	}

	//remove all invalid teams
	for (auto& [h, teams] : HouseExtContainer::HousesTeams) {
		teams.remove_all_if([](TeamClass* pTeam) {
			return !pTeam->Type;
		});
	}

	return 0x0;
}

ASMJIT_PATCH(0x55AFB3, LogicClass_Update, 0x6) //_Early
{
	lastFrameTime = std::chrono::high_resolution_clock::now();

	HarmlessCommandClass::AI();
	SWFirerClass::Update();
	SWStateMachine::UpdateAll();
	HouseExtData::UpdateAutoDeathObjects();
	HouseExtData::UpdateTransportReloaders();

	for (auto pHouse : *HouseClass::Array) {
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

	//remove all invalid teams
	for (auto&[h, teams] : HouseExtContainer::HousesTeams) {
		teams.remove_all_if([](TeamClass* pTeam) {
			return !pTeam->Type;
		});
	}

	return 0x0;
}//

#include <Ext/Tactical/Body.h>

void FakeTacticalClass::__DrawAllTacticalText(wchar_t* text)
{
	const int AdvCommBarHeight = 32;

	int offset = AdvCommBarHeight;

	auto DrawText_Helper = [](const wchar_t* string, int& offset, int color)
		{
			auto wanted = Drawing::GetTextDimensions(string);

			auto h = DSurface::Composite->Get_Height();
			RectangleStruct rect = { 0, h - wanted.Height - offset, wanted.Width, wanted.Height };

			DSurface::Composite->Fill_Rect(rect, COLOR_BLACK);
			DSurface::Composite->DrawText_Old(string, 0, rect.Y, color);

			offset += wanted.Height;
		};

	if (!AresGlobalData::ModNote.Label)
	{
		AresGlobalData::ModNote = "TXT_RELEASE_NOTE";
	}

	if (!AresGlobalData::ModNote.empty())
	{
		DrawText_Helper(AresGlobalData::ModNote, offset, COLOR_RED);
	}

	switch (RulesExtData::Instance()->FPSCounter)
	{
	case FPSCounterMode::disabled: {
		break;
	}
	case FPSCounterMode::Full: {
		auto currentFrameTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> frameDuration = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
		fmt::basic_memory_buffer<wchar_t> buffer;
		fmt::format_to(std::back_inserter(buffer), L"FPS: {} | {:.3f} ms | Avg: {}", FPSCounter::CurrentFrameRate(), frameDuration.count(), (unsigned int)FPSCounter::GetAverageFrameRate());
		buffer.push_back(L'\0');
		DrawText_Helper(buffer.data(), offset, COLOR_WHITE);
		break;
	}
	case FPSCounterMode::FPSOnly: {
		fmt::basic_memory_buffer<wchar_t> buffer;
		fmt::format_to(std::back_inserter(buffer), L"FPS: {}", FPSCounter::CurrentFrameRate());
		buffer.push_back(L'\0');
		DrawText_Helper(buffer.data(), offset, COLOR_WHITE);
		break;
	}
	case FPSCounterMode::FPSandAVG: {
		fmt::basic_memory_buffer<wchar_t> buffer;
		fmt::format_to(std::back_inserter(buffer), L"FPS: {} | Avg: {}", FPSCounter::CurrentFrameRate(), (unsigned int)FPSCounter::GetAverageFrameRate());
		buffer.push_back(L'\0');
		DrawText_Helper(buffer.data(), offset, COLOR_WHITE);
		break;
	}
	}

	this->DrawAllTacticalText(text);
}

ASMJIT_PATCH(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(FakeSuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	//Debug::LogInfo("[%s - %x] Lauch [%s - %x] ", pSuper->Owner->get_ID() , pSuper->Owner, pSuper->Type->ID, pSuper);
	if ( SWTypeExtData::Activate(pSuper, *pCell, isPlayer) ) {
		pSuper->_GetTypeExtData()->FireSuperWeapon(pSuper, pSuper->Owner, pCell, isPlayer);
	}

	//Debug::LogInfo("Lauch [%x][%s] %s failed ", pSuper, pSuper->Owner->get_ID(), pSuper->Type->ID);
	return 0x6CDE40;
}

ASMJIT_PATCH(0x6CEA92, SuperWeaponType_LoadFromINI_ParseAction, 0x6)
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
ASMJIT_PATCH(0x6CBEF4, SuperClass_AnimStage_UseWeeds, 0x6)
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

ASMJIT_PATCH(0x6CBD2C, SuperClass_AI_UseWeeds, 0x6)
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
ASMJIT_PATCH(0x6CC1E6, SuperClass_SetSWCharge_UseWeeds, 0x5)
{
	enum { Skip = 0x6CC251 };

	GET(FakeSuperClass*, pSuper, EDI);
	return pSuper->_GetTypeExtData()->UseWeeds ? Skip : 0;
}

ASMJIT_PATCH(0x6CEC19, SuperWeaponType_LoadFromINI_ParseType, 0x6)
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

#include <Commands/DistributionMode.h>

ASMJIT_PATCH(0x6DBE74, Tactical_SuperLinesCircles_ShowDesignatorRange, 0x7)
{
	DistributionMode::DrawRadialIndicator();

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

ASMJIT_PATCH(0x41F0F1, AITriggerClass_IC_Ready, 0xA)
{
	enum { advance = 0x41F0FD, breakloop = 0x41F10D };
	GET(FakeSuperClass*, pSuper, EDI);

	return (pSuper->Type->Type == SuperWeaponType::IronCurtain
		&& pSuper->_GetTypeExtData()->IsAvailable(pSuper->Owner))
		? breakloop : advance;
}

// these thing picking first SW type with Chronosphere breaking the AI , bruh
// should check if the SW itself avaible before deciding it!
ASMJIT_PATCH(0x6EFF05, TeamClass_ChronosphereTeam_PickSuper_IsAvail_A, 0x9)
{
	GET(FakeSuperClass*, pSuper, EAX);
	GET(HouseClass*, pOwner, EBP);

	return pSuper->_GetTypeExtData()->IsAvailable(pOwner) ?
		0x0//allow
		: 0x6EFF1C;//advance
}

ASMJIT_PATCH(0x6F01BA, TeamClass_ChronosphereTeam_PickSuper_IsAvail_B, 0x9)
{
	GET(FakeSuperClass*, pSuper, EAX);
	GET(HouseClass*, pOwner, EDI);

	return pSuper->_GetTypeExtData()->IsAvailable(pOwner) ?
		0x0//allow
		: 0x6F01D3;//advance
}

ASMJIT_PATCH(0x41F180, AITriggerTypeClass_Chrono, 0x5)
{
	//GET(AITriggerTypeClass*, pThis, ECX);
	GET_STACK(HouseClass*, pOwner, 0x4);
	//GET_STACK(HouseClass*, pEnemy, 0x8);

	if (!pOwner || pOwner->Supers.Count <= 0) {
		R->EAX(false);
		return 0x41F1BA;
	}

	//Debug::LogInfo("AITrigger[%s] With Owner[%s] Enemy[%s].", pThis->ID, pOwner->get_ID(), pEnemy->get_ID());
	auto iter = pOwner->Supers.find_if([pOwner](SuperClass* pItem) {
		return (pItem->Type->Type == SuperWeaponType::ChronoSphere
			&& SWTypeExtContainer::Instance.Find(pItem->Type)->IsAvailable(pOwner)) && pItem->Granted;
	});

	if (iter == pOwner->Supers.end()) {
		R->EAX(false);
		return 0x41F1BA;
	}
	auto pSuper = *iter;
	auto v8 = pSuper->RechargeTimer.StartTime;
	auto v9 = pSuper->RechargeTimer.TimeLeft;
	const auto rechargePercent = (1.0 - RulesClass::Instance->AIMinorSuperReadyPercent);

	if (v8 == -1) {
		const auto result1 = rechargePercent >= (v9 / (double)pSuper->GetRechargeTime());
		R->EAX(result1);
		return 0x41F1BA;
	}

	const auto chargeTime = Unsorted::CurrentFrame - v8;
	if (chargeTime < v9) {
		v9 = (v9 - chargeTime);
		const auto result2 = rechargePercent >= (v9 / (double)pSuper->GetRechargeTime());
		R->EAX(result2);
		return 0x41F1BA;
	}

	const auto result3 = rechargePercent >= (0 / (double)pSuper->GetRechargeTime());
	R->EAX(result3);
	return 0x41F1BA;
}

#include <Ext/Team/Body.h>

ASMJIT_PATCH(0x6EFC70, TeamClass_IronCurtain, 5)
{
	GET(TeamClass*, pThis, ECX);
	GET_STACK(ScriptActionNode*, pTeamMission, 0x4);
	//GET_STACK(bool, barg3, 0x8);

	//auto pTeamExt = TeamExtContainer::Instance.Find(pThis);
	const auto pLeader = pThis->FetchLeader();

	if (!pLeader)
	{
		pThis->StepCompleted = true;
		return 0x6EFE4F;
	}
	const auto pOwner = pThis->OwnerHouse;

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
		auto nCoord = pThis->Zone->GetCoords();
		pOwner->Fire_SW(obtain->Type->ArrayIndex, CellClass::Coord2Cell(nCoord));
		pThis->StepCompleted = true;
		return 0x6EFE4F;
	}

	if(!found) {
		pThis->StepCompleted = true;
	}
	return 0x6EFE4F;
}

ASMJIT_PATCH(0x6CEF84, SuperWeaponTypeClass_GetAction, 7)
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
ASMJIT_PATCH(0x6CEE96, SuperWeaponTypeClass_GetTypeIndex, 5)
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

// decoupling sw anims from types
ASMJIT_PATCH(0x4463F0, BuildingClass_Place_SuperWeaponAnimsA, 6)
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

ASMJIT_PATCH(0x450F9E, BuildingClass_ProcessAnims_SuperWeaponsA, 6)
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
ASMJIT_PATCH(0x4468F4, BuildingClass_Place_AnnounceSW, 6)
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
ASMJIT_PATCH(0x6CBDD7, SuperClass_AI_AnnounceReady, 6)
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
ASMJIT_PATCH(0x6CC0EA, SuperClass_ForceCharged_AnnounceQuantity, 9)
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

//ASMJIT_PATCH(0x6CBDB7  , SuperClass_Upadate_ChargeDrainSWReady , 0x6)
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
ASMJIT_PATCH(0x50CFAA, HouseClass_PickOffensiveSWTarget, 0xA)
{
	// reset weight
	R->ESI(0);

	// mark as ineligible
	R->Stack8(0x13, 0);

	return 0x50CFC9;
}

ASMJIT_PATCH(0x457630, BuildingClass_SWAvailable, 9)
{
	GET(BuildingClass*, pThis, ECX);

	auto nSuper = pThis->Type->SuperWeapon2;
	if (nSuper >= 0 && !HouseExtData::IsSuperAvail(nSuper, pThis->Owner))
		nSuper = -1;

	R->EAX(nSuper);
	return 0x457688;
}

ASMJIT_PATCH(0x457690, BuildingClass_SW2Available, 9)
{
	GET(BuildingClass*, pThis, ECX);

	auto nSuper = pThis->Type->SuperWeapon2;
	if (nSuper >= 0 && !HouseExtData::IsSuperAvail(nSuper, pThis->Owner))
		nSuper = -1;

	R->EAX(nSuper);
	return 0x4576E8;
}

ASMJIT_PATCH(0x43BE50, BuildingClass_DTOR_HasAnySW, 6)
{
	GET(BuildingClass*, pThis, ESI);

	return (BuildingExtData::GetFirstSuperWeaponIndex(pThis) != -1)
		? 0x43BEEAu : 0x43BEF5u;
}

ASMJIT_PATCH(0x449716, BuildingClass_Mi_Guard_HasFirstSW, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->FirstActiveSWIdx() != -1 ? 0x4497AFu : 0x449762u;
}

ASMJIT_PATCH(0x4FAE72, HouseClass_SWFire_PreDependent, 6)
{
	GET(HouseClass*, pThis, EBX);

	// find the predependent SW. decouple this from the chronosphere.
	// don't use a fixed SW type but the very one acutually fired last.
	R->ESI(pThis->Supers.GetItemOrDefault(HouseExtContainer::Instance.Find(pThis)->SWLastIndex));

	return 0x4FAE7B;
}

ASMJIT_PATCH(0x6CC2B0, SuperClass_NameReadiness, 5)
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

ASMJIT_PATCH(0x4F9004 ,HouseClass_Update_TrySWFire, 7)
{
	enum { UpdateAIExpert = 0x4F9015 , Continue = 0x4F9038};

	GET(FakeHouseClass*, pThis, ESI);

	//Debug::Log("House[%s - %x , calling %s\n" , pThis->get_ID() , pThis ,__FUNCTION__);
	const bool IsHuman = R->AL(); // HumanControlled
	if(!IsHuman) {

		if(pThis->Type->MultiplayPassive)
			return Continue;

		if(RulesExtData::Instance()->AISuperWeaponDelay.isset()){
			const int delay = RulesExtData::Instance()->AISuperWeaponDelay.Get();
			auto const pExt = pThis->_GetExtData();
			const bool hasTimeLeft = pExt->AISuperWeaponDelayTimer.HasTimeLeft();

			if (delay > 0) {

				if(hasTimeLeft){
					return UpdateAIExpert;
				}

				pExt->AISuperWeaponDelayTimer.Start(delay);
			}

			if (!SessionClass::IsCampaign() || pThis->IQLevel2 >= RulesClass::Instance->SuperWeapons)
				pThis->_AITryFireSW();
		}

		return UpdateAIExpert;

	} else {
		pThis->_AITryFireSW();
	}

	return Continue;
}

ASMJIT_PATCH(0x6CBF5B, SuperClass_GetCameoChargeStage_ChargeDrainRatio, 9)
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

ASMJIT_PATCH(0x6CC053, SuperClass_GetCameoChargeStage_FixFullyCharged, 5)
{
	R->EAX<int>(R->EAX<int>() > 54 ? 54 : R->EAX<int>());
	return 0x6CC066;
}

// a ChargeDrain SW expired - fire it to trigger status update
ASMJIT_PATCH(0x6CBD86, SuperClass_Progress_Charged, 7)
{
	GET(SuperClass* const, pThis, ESI);
	SWTypeExtData::Deactivate(pThis, CellStruct::Empty, true);
	return 0;
}

// SW was lost (source went away)
ASMJIT_PATCH(0x6CB7B0, SuperClass_Lose, 6)
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
// this weird return , idk
ASMJIT_PATCH(0x6CB920, SuperClass_ClickFire, 5)
{
	GET(SuperClass* const, pThis, ECX);
	GET_STACK(bool const, isPlayer, 0x4);
	GET_STACK(CellStruct const* const, pCell, 0x8);

	retfunc<bool> ret(R, 0x6CBC9C);

	auto const pType = pThis->Type;
	auto const pExt = SWTypeExtContainer::Instance.Find(pType);
	auto const pOwner = pThis->Owner;
	auto const pHouseExt = HouseExtContainer::Instance.Find(pOwner);

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


		// auto-abort if no resources
		if (!pOwner->CanTransactMoney(pExt->Money_Amount)) {
			if (pOwner->IsCurrentPlayer()) {
				pExt->UneableToTransactMoney(pOwner);
			}
			return ret(false);
		}

		if (!pHouseExt->CanTransactBattlePoints(pExt->BattlePoints_Amount)) {
			if (pOwner->IsCurrentPlayer()) {
				pExt->UneableToTransactBattlePoints(pOwner);
			}
			return ret(false);
		}

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
		}
		else if (pType->ManualControl)
		{
			// set recharge timer, then pause
			const auto time = pThis->GetRechargeTime();
			pThis->CameoChargeState = -1;
			pThis->RechargeTimer.Start(time);
			pThis->RechargeTimer.Pause();

		} else if (!pType->PreClick && !pType->PostClick)
		{
			pThis->StopPreclickAnim(isPlayer);
		}

	} else {

		if (pThis->ChargeDrainState == ChargeDrainState::Draining)
		{
			// deactivate for human players
			pThis->ChargeDrainState = ChargeDrainState::Ready;
			auto const left = pThis->RechargeTimer.GetTimeLeft();

			auto const duration = int(pThis->GetRechargeTime()
				- (left / pExt->GetChargeToDrainRatio()));
			pThis->RechargeTimer.Start(duration);
			pExt->Deactivate(pThis, *pCell, isPlayer);

		}
		else if (pThis->ChargeDrainState == ChargeDrainState::Ready)
		{
			// activate for human players
			pThis->ChargeDrainState = ChargeDrainState::Draining;
			auto const left = pThis->RechargeTimer.GetTimeLeft();

			auto const duration = int(
					(pThis->GetRechargeTime() - left)
					* pExt->GetChargeToDrainRatio());
			pThis->RechargeTimer.Start(duration);

			pThis->Launch(*pCell, isPlayer);
		}
	}

	return ret(false);
}

// rewriting OnHold to support ChargeDrain
ASMJIT_PATCH(0x6CB4D0, SuperClass_SetOnHold, 6)
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

ASMJIT_PATCH(0x6CBD6B, SuperClass_Update_DrainMoney, 8)
{
	// draining weapon active. take or give money. stop,
	// if player has insufficient funds.
	GET(SuperClass*, pSuper, ESI);
	GET(int, timeLeft, EAX);

	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

	if(!pData->ApplyDrainMoney(timeLeft , pSuper->Owner))
		return 0x6CBD73;

	if(!pData->ApplyDrainBattlePoint(timeLeft,pSuper->Owner))
		return 0x6CBD73;

	return (timeLeft ? 0x6CBE7C : 0x6CBD73);
}

// clear the chrono placement animation if not ChronoWarp
ASMJIT_PATCH(0x6CBCDE, SuperClass_Update_Animation, 5)
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
ASMJIT_PATCH(0x6CEEB0, SuperWeaponTypeClass_FindFirstOfAction, 8)
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

ASMJIT_PATCH(0x6D49D1, TacticalClass_Draw_TimerVisibility, 5)
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

ASMJIT_PATCH(0x6CB70C, SuperClass_Grant_InitialReady, 0xA)
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
DEFINE_FUNCTION_JUMP(LJMP, 0x5098F0 , FakeHouseClass::_AITryFireSW)

ASMJIT_PATCH(0x4C78D6, Networking_RespondToEvent_SpecialPlace, 8)
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
				Debug::LogInfo("[{} - {}] SW [{} - {}] CannotFire", pHouseID, (void*)pHouse, pSuper->Type->ID, (void*)pSuper);
				pExt->PrintMessage(pExt->Message_CannotFire, pHouse);
			} else {
				Debug::LogInfo("[{} - {}] SW [{} - {}] AI CannotFire", pHouseID, (void*)pHouse,  pSuper->Type->ID, (void*)pSuper);
			}
		}
	}
	else
	{
		pHouse->Fire_SW(specialplace.ID, specialplace.Location);
	}

	return 0x4C78F8;
}

ASMJIT_PATCH(0x50AF10, HouseClass_UpdateSuperWeaponsOwned, 5)
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

ASMJIT_PATCH(0x50B1D0, HouseClass_UpdateSuperWeaponsUnavailable, 6)
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
						if(!pExt->Type->SW_ShowCameo || pExt->Type->SW_AutoFire)
							continue;

						MouseClass::Instance->AddCameo(AbstractType::Special, index);
						MouseClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(SuperClass::AbsID, index, 0));

					}
				}
			}
		}
	}

	return 0x50B36E;
}

ASMJIT_PATCH(0x4555D5, BuildingClass_IsPowerOnline_KeepOnline, 5)
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

// ASMJIT_PATCH(0x508E66, HouseClass_UpdateRadar_Battery, 8)
// {
// 	GET(HouseClass*, pThis, ECX);
//
// 	if(!HouseExtContainer::Instance.Find(pThis)->Batteries.empty())
// 		return 0x508F2A;
//
// 	int power = pThis->PowerOutput;
//     int drain = pThis->PowerDrain;
//
// 	return (power >= drain || !drain || (power > 0 && (double)power / (double)drain >= 1.0)) ?
// 		0x508E87 : 0x508F2F;
// }

ASMJIT_PATCH(0x44019D, BuildingClass_Update_Battery, 6)
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

