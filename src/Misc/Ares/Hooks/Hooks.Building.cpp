#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Cast.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <numeric>
#include "Header.h"

#include <Misc/PhobosGlobal.h>
#include <Misc/Hooks.Otamaa.h>
#include <Misc/DamageArea.h>

#include <RadarEventClass.h>

ASMJIT_PATCH(0x446EE2, BuildingClass_Place_InitialPayload, 6)
{
	GET(BuildingClass* const, pThis, EBP);
	TechnoExtContainer::Instance.Find(pThis)->CreateInitialPayload();
	return 0;
}

ASMJIT_PATCH(0x44D760, BuildingClass_Destroyed_UnitLost, 7)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(ObjectClass*, pKiller, 0x4);

	const auto pType = pThis->Type;
	auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);

	if (pTechnoExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| !pThis->Owner
		|| !pThis->Owner->ControlledByCurrentPlayer())
	{
		return 0x44D7C9;
	}

	VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pType)->EVA_UnitLost);

	if (pKiller)
	{
		CoordStruct nDest = pThis->GetDestination();
		RadarEventClass::Create(CellClass::Coord2Cell(nDest));
	}

	return 0x44D7C9;
}

ASMJIT_PATCH(0x451330, BuildingClass_GetCrewCount, 0xA)
{
	GET(BuildingClass*, pThis, ECX);

	int count = 0;

	if (!pThis->NoCrew && pThis->Type->Crewed)
	{
		auto pHouse = pThis->Owner;

		// get the divisor
		int divisor = HouseExtData::GetSurvivorDivisor(pHouse);

		if (divisor > 0)
		{
			// if captured, less survivors
			if (pThis->HasBeenCaptured)
			{
				divisor *= 2;
			}

			// value divided by "cost per survivor"
			// clamp between 1 and 5
			count = std::clamp(pThis->Type->GetRefund(pHouse, 0) / divisor, 1, 5);
		}
	}

	R->EAX(count);
	return 0x4513CD;
}

ASMJIT_PATCH(0x44EB10, BuildingClass_GetCrew, 9)
{
	GET(BuildingClass*, pThis, ECX);

	// YR defaults to 25 for buildings producing buildings
	R->EAX(TechnoExt_ExtData::GetBuildingCrew(pThis, TechnoTypeExtContainer::Instance.Find(pThis->Type)->
		Crew_EngineerChance.Get((pThis->Type->Factory == BuildingTypeClass::AbsID) ? 25 : 0)));

	return 0x44EB5B;
}

#include <Ext/SWType/Body.h>

// Calculate the mask once at initialization (assuming you know ColorStruct at startup)
constexpr WORD BuildPcxMask() {
    return (0xFFu >> ColorStruct::BlueShiftRight << ColorStruct::BlueShiftLeft)
         | (0xFFu >> ColorStruct::RedShiftRight << ColorStruct::RedShiftLeft);
}

static bool InitEd = false;
// Global static instance:
static AresPcxBlit<WORD> GlobalPcxBlitter(0u ,0, 0, 0);

ASMJIT_PATCH(0x43E7B0, BuildingClass_DrawVisible, 5)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);

	auto pType = pThis->Type;

	if (!pThis->IsSelected || !HouseClass::CurrentPlayer)
		return 0x43E8F2;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

	// helpers (with support for the new spy effect)
	const bool bAllied = pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer);
	const bool IsObserver = HouseClass::CurrentPlayer->IsObserver();
	const bool bReveal = pTypeExt->SpyEffect_RevealProduction && pThis->DisplayProductionTo.Contains(HouseClass::CurrentPlayer);

	// show building or house state
	if (bAllied || IsObserver || bReveal)
	{
		Point2D DrawExtraLoc = { pLocation->X , pLocation->Y };
		pThis->DrawExtraInfo(&DrawExtraLoc, pLocation, pBounds);

		// display production cameo
		if (IsObserver || bReveal)
		{
			const auto pFactory = pThis->Owner->IsControlledByHuman() ?
				pThis->Owner->GetPrimaryFactory(pType->Factory, pType->Naval, BuildCat::DontCare)
				: pThis->Factory;

			if (pFactory && pFactory->Object)
			{
				auto pProdType = TechnoExtContainer::Instance.Find(pFactory->Object)->Type;
				//const int nTotal = pFactory->CountTotal(pProdType);
				Point2D DrawCameoLoc = { pLocation->X , pLocation->Y + 45 };
				const auto pProdTypeExt = TechnoTypeExtContainer::Instance.Find(pProdType);
				RectangleStruct cameoRect {};

				// support for pcx cameos
				if (auto pPCX = TechnoTypeExt_ExtData::GetPCXSurface(pProdType, pThis->Owner))
				{
					const int cameoWidth = 60;
					const int cameoHeight = 48;

					RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
					RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
					RectangleStruct destRect = { DrawCameoLoc.X - cameoWidth / 2, DrawCameoLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

					if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
					{
						cameoRect = destRect;
						if(!InitEd) {
							GlobalPcxBlitter = AresPcxBlit<WORD>(BuildPcxMask() ,60, 48, 2);
							InitEd = true;
						}

						Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds, &GlobalPcxBlitter, 0, 3, 1000, 0);

					}
				}
				else
				{
					// old shp cameos, fixed palette
					if (auto pCameo = pProdType->GetCameo())
					{
						cameoRect = { DrawCameoLoc.X, DrawCameoLoc.Y, pCameo->Width, pCameo->Height };

						ConvertClass* pPal = FileSystem::CAMEO_PAL();
						if (auto pManager = pProdTypeExt->CameoPal.GetConvert())
							pPal = pManager;

						DSurface::Temp->DrawSHP(pPal, pCameo, 0, &DrawCameoLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
					}
				}

				int prog = pFactory->GetProgress();
				{
					Point2D textLoc = { cameoRect.X + cameoRect.Width / 2, cameoRect.Y };
					const auto percent = int(((double)prog / 54.0) * 100.0);
					const auto text_ = std::to_wstring(percent);
					RectangleStruct nTextDimension {};
					COMPILETIMEEVAL TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | TextPrintType::Background | TextPrintType::Center;
					Drawing::GetTextDimensions(&nTextDimension, text_.c_str(), textLoc, printType, 4, 2);
					auto nIntersect = RectangleStruct::Intersect(nTextDimension, *pBounds, nullptr, nullptr);
					const COLORREF foreColor = pThis->Owner->Color.ToInit();
					DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
					DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)foreColor);
					DSurface::Temp->DrawText_Old(text_.c_str(), pBounds, &textLoc, (DWORD)foreColor, 0, (DWORD)printType);
				}

			}
			else if (pType->SuperWeapon != -1)
			{
				SuperClass* const pSuper = pThis->Owner->Supers.Items[pType->SuperWeapon];

				if (pSuper->RechargeTimer.TimeLeft > 0 && SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_ShowCameo)
				{
					RectangleStruct cameoRect {};
					Point2D DrawCameoLoc = { pLocation->X , pLocation->Y + 45 };

					// support for pcx cameos
					if (auto pPCX = SWTypeExtContainer::Instance.Find(pSuper->Type)->SidebarPCX.GetSurface())
					{
						const int cameoWidth = 60;
						const int cameoHeight = 48;

						RectangleStruct cameoBounds = { 0, 0, pPCX->Width, pPCX->Height };
						RectangleStruct DefcameoBounds = { 0, 0, cameoWidth, cameoHeight };
						RectangleStruct destRect = { DrawCameoLoc.X - cameoWidth / 2, DrawCameoLoc.Y - cameoHeight / 2, cameoWidth , cameoHeight };

						if (Game::func_007BBE20(&destRect, pBounds, &DefcameoBounds, &cameoBounds))
						{
							cameoRect = destRect;
							if(!InitEd) {
								GlobalPcxBlitter = AresPcxBlit<WORD>(BuildPcxMask() ,60, 48, 2);
								InitEd = true;
							}

							Buffer_To_Surface_wrapper(DSurface::Temp, &destRect, pPCX, &DefcameoBounds, &GlobalPcxBlitter, 0, 3, 1000, 0);
						}

					}
					else
					{
						// old shp cameos, fixed palette
						if (auto pCameo = pSuper->Type->SidebarImage)
						{
							cameoRect = { DrawCameoLoc.X, DrawCameoLoc.Y, pCameo->Width, pCameo->Height };

							ConvertClass* pPal = FileSystem::CAMEO_PAL();
							if (auto pManager = SWTypeExtContainer::Instance.Find(pSuper->Type)->SidebarPalette.GetConvert())
								pPal = pManager;

							DSurface::Temp->DrawSHP(pPal, pCameo, 0, &DrawCameoLoc, pBounds, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, nullptr, 0, 0, 0);
						}
					}
				}
			}
		}
	}

	return 0x43E8F2;
}

ASMJIT_PATCH(0x452218, BuildingClass_Enable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);
	TechnoExt_ExtData::UpdateFactoryQueues(pThis);
	return 0;
}

ASMJIT_PATCH(0x442D1B, BuildingClass_Init_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (!pThis->Owner)
		return 0x0;

	if (HouseTypeExtContainer::Instance.Find(pThis->Owner->Type)->VeteranBuildings.Contains(pThis->Type))
	{
		pThis->Veterancy.Veterancy = 1.0f;
	}

	if (pThis->Type->Trainable && HouseExtContainer::Instance.Find(pThis->Owner)->Is_ConstructionYardSpied)
		pThis->Veterancy.Veterancy = 1.0f;


	HouseExtData::ApplyAcademy(pThis->Owner, pThis, AbstractType::Building);

	return 0;
}

ASMJIT_PATCH(0x43FE8E, BuildingClass_Update_Reload, 6)
{
	GET(BuildingClass*, B, ESI);

	if (!B->Type->Hospital && !B->Type->Armory)
	{ // TODO: rethink this
		B->Reload();
	}
	return 0x43FEBE;
}

ASMJIT_PATCH(0x440C08, BuildingClass_Put_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x440C2C;
}

ASMJIT_PATCH(0x456370, BuildingClass_UnmarkBaseSpace_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x456394;
}

ASMJIT_PATCH(0x445A72, BuildingClass_Remove_AIBaseNormal, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EAX(TechnoExt_ExtData::IsBaseNormal(pThis));
	return 0x445A94;
}

// replaces the UnitReload handling and makes each docker independent of all
// others. this means planes don't have to wait one more ReloadDelay because
// the first docker triggered repair mission while the other dockers arrive
// too late and need to be put to sleep first.
ASMJIT_PATCH(0x44C844, BuildingClass_MissionRepair_Reload, 6)
{
	GET(BuildingClass* const, pThis, EBP);
	auto const pExt = BuildingExtContainer::Instance.Find(pThis);

	// ensure there are enough slots
	pExt->DockReloadTimers.resize(pThis->RadioLinks.Capacity, -1);

	// update all dockers, check if there's
	// at least one needing more attention
	bool keep_reloading = false;
	for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
	{
		if (auto const pLink = pThis->GetNthLink(i))
		{
			auto const SendCommand = [=](RadioCommand command)
				{
					return pThis->SendCommand(command, pLink) == RadioCommand::AnswerPositive;
				};

			// check if reloaded and repaired already
			auto const pLinkType = pLink->GetTechnoType();
			auto done = SendCommand(RadioCommand::QueryReadiness)
				&& pLink->Health == pLinkType->Strength;

			if (!done)
			{
				// check if docked
				auto const miss = pLink->GetCurrentMission();
				if (miss == Mission::Enter
					|| !SendCommand(RadioCommand::QueryMoving))
				{
					continue;
				}

				keep_reloading = true;

				// make the unit sleep first
				if (miss != Mission::Sleep)
				{
					pLink->QueueMission(Mission::Sleep, false);
					continue;
				}

				// check whether the timer completed
				auto const last_timer = pExt->DockReloadTimers[i];
				if (last_timer > Unsorted::CurrentFrame)
				{
					continue;
				}

				// set the next frame
				auto const pLinkExt = TechnoTypeExtContainer::Instance.Find(pLinkType);
				auto const defaultRate = RulesClass::Instance->ReloadRate;
				auto const rate = pLinkExt->ReloadRate.Get(defaultRate);
				auto const frames = static_cast<int>(rate * 900);
				pExt->DockReloadTimers[i] = Unsorted::CurrentFrame + frames;

				// only reload if the timer was not outdated
				if (last_timer != Unsorted::CurrentFrame)
				{
					continue;
				}

				// reload and repair, return true if both failed
				done = !SendCommand(RadioCommand::RequestReload)
					&& !SendCommand(RadioCommand::RequestRepair);
			}

			if (done)
			{
				pLink->EnterIdleMode(false, 1);
				pLink->ForceMission(Mission::Guard);
				pLink->ProceedToNextPlanningWaypoint();

				pExt->DockReloadTimers[i] = -1;
			}
		}
	}

	if (keep_reloading)
	{
		// update each frame
		R->EAX(1);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
		R->EAX(3);
	}

	return 0x44C968;
}

ASMJIT_PATCH(0x444DBC, BuildingClass_KickOutUnit_Infantry, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0;
}

ASMJIT_PATCH(0x4445F6, BuildingClass_KickOutUnit_Clone_NonNavalUnit, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0x444971;
}

ASMJIT_PATCH(0x44441A, BuildingClass_KickOutUnit_Clone_NavalUnit, 6)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	return 0;
}

ASMJIT_PATCH(0x4444E2, BuildingClass_KickOutUnit_FindAlternateKickout, 6)
{
	GET(BuildingClass*, Src, ESI);
	GET(BuildingClass*, Tst, EBP);
	GET(TechnoClass*, Production, EDI);

	if (Src != Tst
	 && Tst->GetCurrentMission() == Mission::Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && TechnoTypeExtData::CanBeBuiltAt(Production->GetTechnoType(), Tst->Type)
	 && !Tst->Factory)
	{
		return 0x44451F;
	}

	return 0x444508;
}

// copy the remaining EMP duration to the unit when undeploying a building.
ASMJIT_PATCH(0x44A04C, BuildingClass_Destruction_CopyEMPDuration, 6)
{
	GET(TechnoClass*, pBuilding, EBP);
	GET(TechnoClass*, pUnit, EBX);

	// reuse the EMP duration of the deployed/undeployed Techno.
	pUnit->EMPLockRemaining = pBuilding->EMPLockRemaining;
	AresEMPulse::UpdateSparkleAnim(pUnit, pBuilding);

	return 0;
}

ASMJIT_PATCH(0x69281E, DisplayClass_ChooseAction_TogglePower, 0xA)
{
	GET(TechnoClass*, pTarget, ESI);
	REF_STACK(Action, action, STACK_OFFS(0x20, 0x10));

	//bool allowed = false;
	action = Action::NoTogglePower;

	if (auto pBld = cast_to<BuildingClass*>(pTarget))
	{
		auto pOwner = pBld->GetOwningHouse();

		if (pOwner && pOwner->ControlledByCurrentPlayer())
		{
			if (pBld->CanBeSelected()
				&& !pBld->IsStrange()
				&& !pBld->IsBeingWarpedOut()
				&& !pBld->IsUnderEMP()
				&& !BuildingExtContainer::Instance.Find(pBld)->AboutToChronoshift
				)
			{
				if (pBld->Type->CanTogglePower())
					action = Action::TogglePower;
			}
		}
	}

	return 0x69289B;
}

ASMJIT_PATCH(0x474E8E, INIClass_GetMovementZone, 5)
{
	GET_STACK(const char*, Section, 0x2C);
	GET_STACK(const char*, Key, 0x30);
	LEA_STACK(const char*, Value, 0x8);
	Debug::INIParseFailed(Section, Key, Value);
	return 0;
}

ASMJIT_PATCH(0x477007, INIClass_GetSpeedType, 8)
{
	if (R->EAX() == -1)
	{
		GET_STACK(const char*, Section, 0x8C);
		GET_STACK(const char*, Key, 0x90);
		LEA_STACK(const char*, Value, 0x8);
		GET_STACK(DWORD, caller, 0x88);
		/*
			this func is called from TechnoTypeClass::LoadFromINI and UnitTypeClass::LoadFromINI
			UnitTypeClass::CTOR initializes SpeedType to -1
			UnitTypeClass::LoadFromINI overrides it to (this->Crusher ? Track : Wheel) just before reading its SpeedType
			so we should not alert if we're responding to a TType read and our subject is a UnitType, or all VehicleTypes without an explicit ST declaration will get dinged
		*/
		if (caller != 0x7121E5u
			|| !Helpers::Alex::is_any_of(R->EBP<TechnoTypeClass*>()->WhatAmI(),
				AbstractType::UnitType, AbstractType::BuildingType))
		{
			Debug::INIParseFailed(Section, Key, Value);
		}
	}
	return 0;
}

ASMJIT_PATCH(0x441F12, BuildingClass_Destroy_RubbleYell, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->GetCurrentMission() == Mission::Selling)
		return 0x0;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	// If this object has a rubble building set, turn
	if (pTypeExt->RubbleDestroyed || pTypeExt->RubbleDestroyedRemove)
	{
		TechnoExt_ExtData::CreateBuilding(pThis,
			pTypeExt->RubbleDestroyedRemove,
			pTypeExt->RubbleDestroyed,
			pTypeExt->RubbleDestroyedOwner,
			pTypeExt->RubbleDestroyedStrength,
			pTypeExt->RubbleDestroyedAnim
		);
	}

	return 0;
}

DEFINE_FUNCTION_JUMP(VTABLE , 0x7E42C8 , FakeBuildingClass::_OnFinishRepair)

// #664: Advanced Rubble - reconstruction part: Reconstruction
ASMJIT_PATCH(0x519FAF, InfantryClass_UpdatePosition_EngineerRepairsFriendly, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(FakeBuildingClass*, Target, EDI);

	const auto TargetTypeExtData = BuildingTypeExtContainer::Instance.Find(Target->Type);

	if (TargetTypeExtData->RubbleIntact || TargetTypeExtData->RubbleIntactRemove)
	{
		auto pRubble = TechnoExt_ExtData::CreateBuilding(Target,
			TargetTypeExtData->RubbleIntactRemove,
			TargetTypeExtData->RubbleIntact,
			TargetTypeExtData->RubbleIntactOwner,
			TargetTypeExtData->RubbleIntactStrength,
			TargetTypeExtData->RubbleIntactAnim
		);

		//Debug::LogInfo(__FUNCTION__" Called ");
		TechnoExtData::HandleRemove(Target, nullptr, false, false);

		if (pRubble)
		{
			const bool wasSelected = pThis->IsSelected;
			pThis->Limbo();
			CellStruct Cell = pThis->GetMapCoords();
			Target->KickOutUnit(pThis, Cell);
			if (wasSelected)
			{
				pThis->Select();
			}

			return TargetTypeExtData->RubbleIntactConsumeEngineer ? 0x51A010 : 0x51A65D;
		}
		else
		{
			return 0x519FB9;
		}
	}
	else if (TargetTypeExtData->EngineerRepairable.isset())
	{
		return TargetTypeExtData->EngineerRepairable ? 0x0 : 0x519FB9;
	}

	if(!Target->Type->Repairable)
		return 0x519FB9;

	Target->_OnFinishRepairB(pThis);

	//0x51A010 eats the Engineer, 0x51A65D hopefully does not
	return 0x51A010;
}

ASMJIT_PATCH(0x459ed0, BuildingClass_GetUIName, 6)
{
	GET(BuildingClass*, pBld, ECX);

	if (HouseClass::CurrentPlayer)
	{
		const auto pBldOWner = pBld->Owner;
		if (HouseClass::CurrentPlayer->IsObserver()
			|| HouseClass::CurrentPlayer == pBldOWner
			|| HouseClass::CurrentPlayer->IsAlliedWith(pBldOWner)
			|| pBld->DisplayProductionTo.Contains(HouseClass::CurrentPlayer->ArrayIndex))
		{
			R->EAX(pBld->Type->UIName);
			return 0x459ED9;
		}
	}

	auto Type = pBld->Type;
	if (TechnoTypeExtContainer::Instance.Find(pBld->Type)->Fake_Of)
		Type = (BuildingTypeClass*)TechnoTypeExtContainer::Instance.Find(pBld->Type)->Fake_Of.Get();

	R->EAX(Type->UIName);
	return 0x459ED9;
}

ASMJIT_PATCH(0x44e2b0, BuildingClass_Mi_Unload_LargeGap, 6)
{
	GET(BuildingClass*, pBld, EBP);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Type);

	if (!pTypeExt->SuperGapRadiusInCells)
		return 0x44E371;

	const bool notCharged = !pBld->GapSuperCharged && pBld->IsPowerOnline();

	if (pBld->GapSuperCharged != notCharged)
	{
		pBld->DestroyGap();
		pBld->HasExtraPowerDrain = notCharged;
		pBld->GapSuperCharged = notCharged;
		pBld->GapRadius = (notCharged ? pTypeExt->GapRadiusInCells : pTypeExt->SuperGapRadiusInCells).Get();
		pBld->Owner->RecheckPower = true;
		pBld->CreateGap();
	}

	return 0x44E371;
}

ASMJIT_PATCH(0x4566d5, BuildingClass_GetRangeOfRadial_LargeGap, 6)
{
	GET(BuildingClass*, pBld, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Type);
	R->EAX((pBld->GapSuperCharged ? pTypeExt->SuperGapRadiusInCells : pTypeExt->GapRadiusInCells).Get());
	return 0x456745;
}


bool Bld_ChangeOwnerAnnounce;
ASMJIT_PATCH(0x448260, BuildingClass_SetOwningHouse_ContextSet, 0x8)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(bool, announce, 0x8);
	// Fix : Suppress capture EVA event if ConsideredVehicle=yes
	announce = announce && !pThis->IsStrange();
	Bld_ChangeOwnerAnnounce = announce;
	return 0x0;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4290, FakeBuildingClass::_SetOwningHouse);

ASMJIT_PATCH(0x448BE3, BuildingClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EDI);
	//GET_STACK(bool const, bAnnounce, 0x58 + 0x8); // this thing already used
	//discarded
	for (auto& anim : pThis->Anims)
	{
		if (anim)
		{
			anim->SetHouse(pNewOwner);
		}
	}

	pThis->TechnoClass::SetOwningHouse(pNewOwner, Bld_ChangeOwnerAnnounce);
	return 0x448BED;
}

ASMJIT_PATCH(0x4483FB, BuildingClass_ChangeOwnership_Tech, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBX);

	if (Bld_ChangeOwnerAnnounce && !pNewOwner->Type->MultiplayPassive) {
		if(pThis->Type->NeedsEngineer) {
			if(pThis->Owner != pNewOwner) {
				const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
				const auto color = HouseClass::CurrentPlayer->ColorSchemeIndex;

				if (pThis->Owner->ControlledByCurrentPlayer())
				{
					VoxClass::PlayIndex(pExt->LostEvaEvent);
					pExt->MessageLost->PrintAsMessage(color);
				}

				if (pNewOwner->ControlledByCurrentPlayer())
				{
					VoxClass::PlayIndex(pThis->Type->CaptureEvaEvent);
					pExt->MessageCapture->PrintAsMessage(color);
				}
			}
		} else {
			auto coord = pThis->GetMapCoords();
			if(RadarEventClass::Create(RadarEventType::BuildingCaptured,coord)){
				if(pNewOwner->ControlledByCurrentPlayer())
					VoxClass::Play(GameStrings::EVA_BuildingCaptured);
			}
		}
	}

	return 0x44848F;
}

/*

		struct ProduceCashDataType
		{
			BuildingTypeClass* OwnerObject { nullptr };
			Valueable<unsigned int> ProduceCashBudget { 0 };
			Valueable<bool> IsStartupCashOneTime { false };
			Valueable<bool> IsResetBudgetOnCapture { false };

			void Read(INI_EX& parser, const char* section)
			{
				ProduceCashBudget.Read(parser, section,  "ProduceCashBudget");
				IsStartupCashOneTime.Read(parser, section, "ProduceCashStartupOneTime");
				IsResetBudgetOnCapture.Read(parser, section, "ProduceCashResetOnCapture");
			}

			bool Load(PhobosStreamReader& stm, bool registerForChange)
			{
				return Serialize(stm);
			}
			bool Save(PhobosStreamWriter& stm) const
			{
				return const_cast<ProduceCashDataType*>(this)->Serialize(stm);
			}

		private:
			template <typename T>
			bool Serialize(T& stm)
			{
				return stm
					.Process(OwnerObject)
					.Process(ProduceCashBudget)
					.Process(IsStartupCashOneTime)
					.Process(IsResetBudgetOnCapture)
					.Success();
			}

		} AdditionalProduceCashData {};


//these based on vinifera source
// additional data , including the upgrades
// the function placement is kind a  weird later since you need to do stuffs for `Upgrades` too
// not just the main building
struct ProduceCashData
{
	static COMPILETIMEEVAL OPTIONALINLINE size_t count = 0x4;
	std::array<int, count> CurrentProduceCashBudget {};
	std::array<bool, count> IsCaptureOneTimeCashGiven {};
	std::array<bool, count> IsBudgetDepleted {};

	ProduceCashData() = delete;

	ProduceCashData(BuildingClass* ownerObject)
	{
		auto FromType = ownerObject->GetTypes();

		for (size_t i = 0; i < count; ++i)
		{

			if (auto pType = FromType[i])
			{
				CurrentProduceCashBudget[i] = BuildingTypeExtContainer::Instance.Find(pType)->AdditionalProduceCashData.ProduceCashBudget;
			}
		}
	}

	~ProduceCashData() = default;

	static void AI(BuildingClass* pBld)
	{
		auto pExt = BuildingExtContainer::Instance.Find(pBld);
		auto additionaldata = &pExt->ProduceCashAdditionalData;
		std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
		{ {
		 { pBld->Type , &pBld->CashProductionTimer },
		 { pBld->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
		 { pBld->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
		 { pBld->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
		} };

		for (size_t i = 0; i < count; ++i)
		{
			if (auto pType = Timers[i].first)
			{
				if (!additionaldata->IsBudgetDepleted[i] && (pType->ProduceCashAmount != 0))
				{
					if (pType->Powered)
					{
						if (Timers[i].second->IsTicking() && !pBld->IsPowerOnline())
							Timers[i].second->Pause();

						if (!Timers[i].second->IsTicking() && pBld->IsPowerOnline())
							Timers[i].second->Resume();
					}

					if (Timers[i].second->IsTicking() && Timers[i].second->Expired())
					{
						if (!pBld->Owner->Type->MultiplayPassive)
						{
							int amount = pType->ProduceCashAmount;

							if (additionaldata->CurrentProduceCashBudget[i] > 0)
							{

								if (additionaldata->CurrentProduceCashBudget[i] != -1)
								{
									additionaldata->CurrentProduceCashBudget[i] -= MaxImpl(0, amount);
								}

								if (additionaldata->CurrentProduceCashBudget[i] <= 0)
								{
									additionaldata->IsBudgetDepleted[i] = true;
									additionaldata->CurrentProduceCashBudget[i] = -1;
								}
							}

							if (!additionaldata->IsBudgetDepleted[i] && amount != 0)
							{
								pBld->Owner->TransactMoney(amount);
							}
						}

						//should reset ?
						//idk , there is only start
						Timers[i].second->Start(pType->ProduceCashDelay + 1);
					}
				}
			}
		}
	}

	static void Captured(BuildingClass* pBld, HouseClass* pNewOwner)
	{
		if (pBld->Owner->Type->MultiplayPassive)
		{
			auto pExt = BuildingExtContainer::Instance.Find(pBld);
			auto additionaldata = &pExt->ProduceCashAdditionalData;
			std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
			{ {
			 { pBld->Type , &pBld->CashProductionTimer },
			 { pBld->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
			 { pBld->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
			 { pBld->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
			} };

			for (size_t i = 0; i < count; ++i)
			{
				if (auto pType = Timers[i].first)
				{
					const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

					if (pType->ProduceCashStartup > 0)
					{
						if (!additionaldata->IsCaptureOneTimeCashGiven[i])
						{
							pNewOwner->TransactMoney(pType->ProduceCashStartup);
						}

						if (pTypeExt->AdditionalProduceCashData.IsStartupCashOneTime)
						{
							additionaldata->IsCaptureOneTimeCashGiven[i] = true;
						}

						Timers[i].second->Start(pType->ProduceCashDelay + 1);
					}

					if (pTypeExt->AdditionalProduceCashData.IsResetBudgetOnCapture)
					{
						if (pTypeExt->AdditionalProduceCashData.ProduceCashBudget > 0)
						{
							additionaldata->CurrentProduceCashBudget[i] = pTypeExt->AdditionalProduceCashData.ProduceCashBudget;
						}
					}
				}
			}
		}
	}

	//handle the Upgrades
	static void Unlimbo(BuildingClass* pBld)
	{

	}

	//handle the main building case
	//TODO : all function is still packed here
	// need to move !
	static void GrandOpening(BuildingClass* pBld)
	{
		auto pExt = BuildingExtContainer::Instance.Find(pBld);
		auto additionaldata = &pExt->ProduceCashAdditionalData;
		std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
		{ {
		 { pBld->Type , &pBld->CashProductionTimer },
		 { pBld->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
		 { pBld->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
		 { pBld->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
		} };

		for (size_t i = 0; i < count; ++i)
		{
			if (auto pType = Timers[i].first)
			{
				if (pType->ProduceCashAmount != 0)
				{
					Timers[i].second->Start(pType->ProduceCashDelay + 1);
				}

				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

				if (pTypeExt->AdditionalProduceCashData.ProduceCashBudget > 0)
				{
					additionaldata->CurrentProduceCashBudget[i] = pTypeExt->AdditionalProduceCashData.ProduceCashBudget;
				}
			}
		}
	}

};
*/

// support oil derrick logic on building upgrades
ASMJIT_PATCH(0x4409F4, BuildingClass_Put_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingClass*, pToUpgrade, EDI);

	auto pExt = BuildingExtContainer::Instance.Find(pToUpgrade);

	if (auto delay = pThis->Type->ProduceCashDelay)
	{
		switch (pToUpgrade->UpgradeLevel - 1)
		{
		case 0:
			pExt->CashUpgradeTimers[0].Start(delay);
			break;
		case 1:
			pExt->CashUpgradeTimers[1].Start(delay);
			break;
		case 2:
			pExt->CashUpgradeTimers[2].Start(delay);
			break;
		default:
			break;
		}
	}

	// if (auto const pOwner = pThis->Owner)
	// {
	// 	if (pOwner->Type->MultiplayPassive)
	// 		return 0x0;
	//
	// 	if (auto const pInfantrySelfHeal = pThis->Type->InfantryGainSelfHeal)
	// 		pOwner->InfantrySelfHeal += pInfantrySelfHeal;
	//
	// 	if (auto const pUnitSelfHeal = pThis->Type->UnitsGainSelfHeal)
	// 		pOwner->UnitsSelfHeal += pUnitSelfHeal;
	// }

	HouseExtData::UpdateFactoryPlans(pThis);
	return 0;
}

ASMJIT_PATCH(0x4482BD, BuildingClass_ChangeOwnership_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBX);

	int startup = 0;
	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
	{ {
	 { pThis->Type , &pThis->CashProductionTimer },
	 { pThis->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
	 { pThis->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
	 { pThis->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
	 } };

	for (auto& [bld, timer] : Timers)
	{
		if (bld)
		{
			if (bld->ProduceCashStartup || bld->ProduceCashAmount)
			{

				if (!pExt->BeignMCEd)
				{
					pExt->BeignMCEd = false;
					startup += bld->ProduceCashStartup;
				}

				if (bld->ProduceCashDelay)
				{
					timer->Start(bld->ProduceCashDelay + 1);
				}
			}
		}
	}

	if (startup)
	{

		if (!pNewOwner->Type->MultiplayPassive)
		{
			pNewOwner->TransactMoney(startup);
			if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->ProduceCashDisplay)
			{
				TechnoExtContainer::Instance.Find(pThis)->TechnoValueAmount += startup;
			}
		}
	}

	return 0x4482FC;
}

ASMJIT_PATCH(0x43FD2C, BuildingClass_Update_ProduceCash, 6)
{
	GET(BuildingClass*, pThis, ESI);

	int produceAmount = 0;
	auto pExt = BuildingExtContainer::Instance.Find(pThis);
	auto pTExt = TechnoExtContainer::Instance.Find(pThis);

	std::array<std::pair<BuildingTypeClass*, CDTimerClass*>, 4u> Timers
	{ {
	 { pThis->Type , &pThis->CashProductionTimer },
	 { pThis->Upgrades[0] ,&pExt->CashUpgradeTimers[0] },
	 { pThis->Upgrades[1] ,&pExt->CashUpgradeTimers[1] },
	 { pThis->Upgrades[2] ,&pExt->CashUpgradeTimers[2] },
	} };

	for (auto& [pbld, timer] : Timers)
	{
		if (pbld)
		{
			if (pbld->ProduceCashDelay > 0)
			{
				if (timer->HasTimeLeft())
					timer->Resume();

				if (timer->GetTimeLeft() == 1)
				{
					timer->Start(pbld->ProduceCashDelay + 1);
					produceAmount += pbld->ProduceCashAmount;
				}
			}
		}
	}

	if (produceAmount && !pThis->Owner->Type->MultiplayPassive && pThis->IsPowerOnline())
	{

		if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->ProduceCashDisplay)
		{
			pTExt->TechnoValueAmount += produceAmount;
		}

		pThis->Owner->TransactMoney(produceAmount);
	}

	if (pTExt->AirstrikeTargetingMe)
		pThis->Mark(MarkType::Redraw);

	return 0x43FDF1;
}

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
ASMJIT_PATCH(0x4430E8, BuildingClass_Destroyed_SurvivourLog, 0x6)
{
	GET(BuildingClass* const, pThis, EDI);
	GET(InfantryClass* const, pInf, ESI);

	const auto pBldID = pThis ? pThis->Type->Name : GameStrings::NoneStr();
	const auto pInfID = pInf ? pInf->Type->Name : GameStrings::NoneStr();
	const auto pOwnedID = pThis && pThis->Owner && pThis->Owner->Type ? pThis->Owner->Type->ID : GameStrings::NoneStr();

	Debug::LogInfo("[{}][{} - {}] Creating survivor type '{}' ", (void*)pThis, pBldID, pOwnedID, pInfID);
	return 0x443109;
}

/* #183 - cloakable on Buildings and Aircraft */
ASMJIT_PATCH(0x442CE0, BuildingClass_Init_Cloakable, 0x6)
{
	GET(BuildingClass*, Item, ESI);

	if (Item->Type->Cloakable)
	{
		Item->Cloakable = true;
	}

	return 0;
}

// if this is a radar, drop the new owner from the bitfield
ASMJIT_PATCH(0x448D95, BuildingClass_ChangeOwnership_OldSpy2, 0x8)
{
	GET(HouseClass* const, newOwner, EDI);
	GET(BuildingClass*, pThis, ESI);

	if (pThis->DisplayProductionTo.Contains(newOwner))
	{
		pThis->DisplayProductionTo.Remove(newOwner);
	}

	return 0x448DB9;
}


ASMJIT_PATCH(0x455923, BuildingClass_SensorArray_BuildingRedraw, 0x6)
{
	GET(CellClass* const, pCell, ESI);

	// mark detected buildings for redraw
	if (auto pBld = pCell->GetBuilding())
	{
		if (pBld->Owner != HouseClass::CurrentPlayer()
			&& pBld->VisualCharacter(VARIANT_FALSE, nullptr) != VisualType::Normal)
		{
			pBld->NeedsRedraw = true;
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x4557BC, BuildingClass_SensorArray_BuildingRedraw, 0x6)

// capture and mind-control support: deactivate the array for the original
// owner, then activate it a few instructions later for the new owner.
ASMJIT_PATCH(0x448B70, BuildingClass_ChangeOwnership_SensorArrayA, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayDeactivate();
	}

	return 0;
}

ASMJIT_PATCH(0x448C3E, BuildingClass_ChangeOwnership_SensorArrayB, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayActivate();
	}

	return 0;
}

// remove sensor on destruction
ASMJIT_PATCH(0x4416A2, BuildingClass_Destroy_SensorArray, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayDeactivate();
	}

	return 0;
}

// sensor arrays show SensorsSight instead of CloakRadiusInCells
ASMJIT_PATCH(0x4566F9, BuildingClass_GetRangeOfRadial_SensorArray, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (pThis->Type->SensorArray)
	{
		R->EAX(pThis->Type->SensorsSight);
		return 0x45674B;
	}

	return 0x456703;
}

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
// ASMJIT_PATCH(0x4430E8, BuildingClass_Demolish_LogCrash, 0x6)
// {
// 	GET(BuildingClass* const, pThis, EDI);
// 	GET(InfantryClass* const, pInf, ESI);
//
// 	R->EDX(pThis ? pThis->Type->Name : GameStrings::NoneStr());
// 	R->EAX(pInf ? pInf->Type->Name : GameStrings::NoneStr());
//
// 	return 0x4430FA;
// }

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
//BuildingClass_Destroy
DEFINE_JUMP(LJMP, 0x441D25, 0x441D37);

ASMJIT_PATCH(0x451E40, BuildingClass_DestroyNthAnim_Destroy, 0x7)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(int, AnimState, 0x4);

	if (AnimState == -2)
	{
		for (int i = 0; i < 21; ++i)
		{
			if (auto pAnim = std::exchange(pThis->Anims[i], nullptr))
			{
				pAnim->RemainingIterations = 0;
				pAnim->UnInit();
			}
		}
	}
	else
	{
		if (auto pAnim = std::exchange(pThis->Anims[AnimState], nullptr))
		{
			pAnim->RemainingIterations = 0;
			pAnim->UnInit();
		}
	}

	return 0x451E93;
}

ASMJIT_PATCH(0x451A28, BuildingClass_PlayAnim_Destroy, 0x7)
{
	//GET(BuildingClass* const , pThis , ESI);

	GET(AnimClass*, pAnim, ECX);
	//pAnim->Limbo();
	pAnim->UnInit();
	return 0x451A2F;
}

ASMJIT_PATCH(0x458E1E, BuildingClass_GetOccupyRangeBonus_Demacroize, 0xA)
{
	auto v1 = R->EDI<int>();
	if (v1 >= R->EAX<int>())
		v1 = R->EAX<int>();

	R->EAX(v1);
	return 0x458E2D;
}

// restore pip count for tiberium storage (building and house)
ASMJIT_PATCH(0x44D755, BuildingClass_GetPipFillLevel_Tiberium, 0x6)
{
	GET(BuildingClass* const, pThis, ECX);
	GET(BuildingTypeClass* const, pType, ESI);

	double amount = 0.0;
	if (pType->Storage > 0)
	{
		amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetStoragePercentage(pType->Storage);
	}
	else
	{
		amount = HouseExtContainer::Instance.Find(pThis->Owner)->TiberiumStorage.GetStoragePercentage(pThis->Owner->TotalStorage);
	}

	R->EAX(static_cast<int>(pType->GetPipMax() * amount));
	return 0x44D750;
}

// #814: force sidebar repaint for standard spy effects
// ASMJIT_PATCH_AGAIN(0x4574D2, BuildingClass_Infiltrate_Standard, 0x6)
// ASMJIT_PATCH(0x457533, BuildingClass_Infiltrate_Standard, 0x6)
// {
// 	MouseClass::Instance->SidebarNeedsRepaint();
// 	return R->Origin() + 6;
// }

// infantry exiting hospital get their focus reset, but not for armory
ASMJIT_PATCH(0x444D26, BuildingClass_KickOutUnit_ArmoryExitBug, 0x6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	R->AL(pType->Hospital || pType->Armory);
	return 0x444D2C;
}

// BuildingClass_KickOutUnit_PreventClone
DEFINE_JUMP(LJMP, 0x4449DF, 0x444A53);

ASMJIT_PATCH(0x4586D6, BuildingClass_KillOccupiers, 0x9)
{
	GET(TechnoClass*, pVictim, ECX);
	GET(TechnoClass*, pKiller, EBP);
	pKiller->RegisterDestruction(pVictim);
	return 0x4586DF;
}

// do not crash if the EMP cannon primary has no Report sound
ASMJIT_PATCH(0x44D4CA, BuildingClass_Mi_Missile_NoReport, 0x9)
{
	GET(TechnoTypeClass* const, pType, EAX);
	GET(WeaponTypeClass* const, pWeapon, EBP);

	return !pType->IsGattling && pWeapon->Report.Count ?
		0x44D4D4 : 0x44D51F;
}

// for yet unestablished reasons a unit might not be present.
// maybe something triggered the KickOutHospitalArmory
ASMJIT_PATCH(0x44BB1B, BuildingClass_Mi_Repair_Promote, 0x6)
{
	//GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass* const, pTrainee, EAX);
	return pTrainee ? 0 : 0x44BB3C;
}

// remember that this building ejected its survivors already
ASMJIT_PATCH(0x44A8A2, BuildingClass_Mi_Selling_Crew, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	pThis->NoCrew = BuildingTypeExtContainer::Instance.Find(pThis->Type)->SpawnCrewOnlyOnce;
	return 0;
}

// #1156943, #1156937: replace the engineer check, because they were smart
// enough to use the pointer right before checking whether it's null, and
// even if it isn't, they build a possible infinite loop.
ASMJIT_PATCH(0x44A5F0, BuildingClass_Mi_Selling_EngineerFreeze, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(InfantryTypeClass*, pType, ESI);
	LEA_STACK(bool*, pEngineerSpawned, 0x13);

	if (*pEngineerSpawned && pType && pType->Engineer)
	{
		// randomize until probability is below 0.01%
		// for only the Engineer tag being returned.
		for (int i = 9; i >= 0; --i)
		{
			pType = !i ? nullptr : pThis->GetCrew();

			if (!pType || !pType->Engineer)
			{
				break;
			}
		}
	}

	if (pType && pType->Engineer)
	{
		*pEngineerSpawned = true;
	}

	R->ESI(pType);
	return 0x44A628;
}

// prevent invisible mcvs, which shouldn't happen any more as the sell/move
// hack is fixed. thus this one is a double unnecessity
ASMJIT_PATCH(0x449FF8, BuildingClass_Mi_Selling_PutMcv, 7)
{
	GET(UnitClass* const, pUnit, EBX);
	GET(DirType, facing, EAX);
	REF_STACK(CoordStruct const, Crd, STACK_OFFS(0xD0, 0xB8));

	// set the override for putting, not just for creation as WW did
	++Unsorted::ScenarioInit;
	const auto ret = pUnit->Unlimbo(Crd, facing);
	--Unsorted::ScenarioInit;

	// should never happen, but if anything breaks, it's here
	if (!ret)
	{
		GameDelete<true, false>(pUnit);
		// do not keep the player alive if it couldn't be placed
		//pUnit->UnInit();
	}

	return ret ? 0x44A010u : 0x44A16Bu;
}

// start after free unit checks 0x446B16
//FreeUnit end 0x446EE2

ASMJIT_PATCH(0x45EE30, BuildingTypeClass_GetActualCost_FreeUnitCount, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBX);
	GET(HouseClass*, pHouse, EBP);

	if (!pThis->FreeUnit)
		return 0x45EE58;

	const int FreeUnitamount = BuildingTypeExtContainer::Instance.Find(pThis)->FreeUnit_Count;
	if (!FreeUnitamount)
		return 0x45EE58;

	R->EAX(pThis->FreeUnit->GetActualCost(pHouse) * FreeUnitamount);
	return 0x45EE45;
}

ASMJIT_PATCH(0x45EDA3, BuildingClass_GetCost_FreeUnitCount, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBX);

	if (!pThis->FreeUnit)
		return 0x45EDC8;

	const int FreeUnitamount = BuildingTypeExtContainer::Instance.Find(pThis)->FreeUnit_Count;
	if (!FreeUnitamount)
		return 0x45EDC8;

	R->EAX(pThis->FreeUnit->GetCost() * FreeUnitamount);
	return 0x45EDB7;
}

void SetFreeUnitMission(UnitClass* pUnit)
{
	Mission nMissions;

	//Initial DriverKilled
	if (TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
	{
		nMissions = Mission::Sleep;
	}
	else
	{

		if (pUnit->Type->Harvester ||
			pUnit->Type->Weeder ||
			pUnit->Type->ResourceGatherer)
		{
			nMissions = Mission::Harvest;
		}
		else
		{
			nMissions = !pUnit->Owner->IsControlledByHuman()
				? Mission::Hunt : Mission::Area_Guard;
		}
	}

	pUnit->QueueMission(nMissions, false);
	pUnit->NextMission();
}

void SpawnFreeUnits(BuildingClass* pBuilding, int count)
{

	if (count <= 0)
		return;

	std::vector<bool> placements(count, false);

	const auto pBldLoc = pBuilding->GetCoords();
	const auto pBldCell = CellClass::Coord2Cell(pBldLoc);
	const auto placement_first = pBldCell + CellSpread::AdjacentCell[(size_t)FacingType::South];

	for (size_t i = 0; i < placements.size(); ++i)
	{
		if (auto pUnit = (UnitClass*)pBuilding->Type->FreeUnit->CreateObject(pBuilding->Owner))
		{
			if (pUnit->Unlimbo(CellClass::Cell2Coord(placement_first), DirType::West))
			{
				SetFreeUnitMission(pUnit);
				placements[i] = true;
				continue;
			}

			// weeee
			for (int a = 0; a < 2; ++a)
			{
				const auto pBldLoc_Cell = CellClass::Coord2Cell(pBuilding->Location);
				auto zone = MapClass::Instance->GetMovementZoneType(pBldLoc_Cell, pUnit->Type->MovementZone, false);
				auto nearbyLoc = MapClass::Instance->NearByLocation(pBldLoc_Cell,
				pUnit->Type->SpeedType,
				zone,
				pUnit->Type->MovementZone,
				false, 1, 1, !a, true, false, false, CellStruct::Empty, false, false);

				if (nearbyLoc.IsValid())
				{
					if (pUnit->Unlimbo(CellClass::Cell2Coord(nearbyLoc), DirType::SouthWest))
					{
						SetFreeUnitMission(pUnit);
						placements[i] = true;
						break; // berak from 2nd loop
					}
				}
			}

			if (!placements[i])
			{
				GameDelete<true, false>(pUnit);
				pBuilding->Owner->TransactMoney(pBuilding->Type->FreeUnit->GetRefund(pBuilding->Owner, true));
			}
		}
	}
}

ASMJIT_PATCH(0x446B16, BuildingClass_Place_FreeUnits, 0x7)
{
	GET(BuildingClass* const, pThis, EBP);
	SpawnFreeUnits(pThis, BuildingTypeExtContainer::Instance.Find(pThis->Type)->FreeUnit_Count);
	return 0x446EE2;
}

// also consider NeedsEngineer when activating animations
// if the status changes, animations might start to play that aren't
// supposed to play because the building requires an Engineer which
// didn't capture the building yet.
ASMJIT_PATCH(0x4467D6, BuildingClass_Place_NeedsEngineer, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	R->AL(pThis->Type->Powered || pThis->Type->NeedsEngineer && !pThis->HasEngineer);
	return 0x4467DC;
}

ASMJIT_PATCH(0x454BF7, BuildingClass_UpdatePowered_NeedsEngineer, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->CL(pThis->Type->Powered || pThis->Type->NeedsEngineer && !pThis->HasEngineer);
	return 0x454BFD;
}

ASMJIT_PATCH(0x451A54, BuildingClass_PlayAnim_NeedsEngineer, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->CL(pThis->Type->Powered || pThis->Type->NeedsEngineer && !pThis->HasEngineer);
	return 0x451A5A;
}

//BuildingClass_Put_DontSpawnSpotlight
DEFINE_JUMP(LJMP, 0x441163, 0x441196);
//BuildingClass_ProcessAnims_SuperWeaponsB
DEFINE_JUMP(LJMP, 0x451132, 0x451145);
//BuildingClass_Place_SuperWeaponAnimsB
DEFINE_JUMP(LJMP, 0x44656D, 0x446580);

// EMP'd power plants don't produce power
ASMJIT_PATCH(0x44E855, BuildingClass_PowerProduced_EMP, 0x6)
{
	GET(BuildingClass* const, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}

// removing hardcoded references to GAWALL and NAWALL as part of #709
ASMJIT_PATCH(0x440709, BuildingClass_Unlimbo_RemoveHarcodedWall, 0x6)
{
	GET(CellClass* const, Cell, EDI);
	const int idxOverlay = Cell->OverlayTypeIndex;
	return idxOverlay != -1 && OverlayTypeClass::Array->Items[idxOverlay]->Wall ? 0x44071A : 0x440725;
}

ASMJIT_PATCH(0x45E416, BuildingTypeClass_CTOR_Initialize, 0x6)
{
	GET(BuildingTypeClass*, pThis, ESI);

	pThis->BuildingAnimFrame[3].dwUnknown = 0;
	pThis->BuildingAnimFrame[3].FrameCount = 1;
	pThis->BuildingAnimFrame[3].FrameDuration = 0;
	pThis->double_1728 = 1.0;
	pThis->VoxelBarrelOffsetToPitchPivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToRotatePivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToBuildingPivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToBarrelEnd = CoordStruct::Empty;
	return 0x0;
}

ASMJIT_PATCH(0x4456E5, BuildingClass_UpdateConstructionOptions_ExcludeDisabled, 0x6)
{
	GET(BuildingClass* const, pBld, ECX);

	// add the EMP check to the limbo check
	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
		0x44583E : 0x4456F3;
}

//void AddPassengers(std::vector<TechnoClass*>& colle , TechnoClass* Vic)
//{
//	for(auto nPass = Vic->Passengers.GetFirstPassenger();
//		nPass;
//		nPass = (FootClass*)nPass->NextObject)
//	{
//		if (TechnoTypeExtContainer::Instance.Find(nPass->GetTechnoType())->CanBeReversed) {
//			colle.push_back(nPass);
//		}
//
//		AddPassengers(colle, nPass);
//	}
//}

// https://bugs.launchpad.net/ares/+bug/1925359
//bool ReverseEngineer(BuildingClass* pBuilding , TechnoClass* Victim) {
//	auto pReverseData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
//
//	if (!pReverseData->ReverseEngineersVictims || !pBuilding->Owner) {
//		return false;
//	}
//
//	std::vector<TechnoClass*> Victims;
//
//	if(TechnoTypeExtContainer::Instance.Find(Victim->GetTechnoType())->CanBeReversed)
//		Victims.push_back(Victim);
//
//	AddPassengers(Victims,Victim);
//	HouseClass* Owner = pBuilding->Owner;
//	auto& nVec = ReverseEngineeredTechnoType(Owner);
//
//	for(auto nColle : Victims) {
//
//		const auto VictimType = nColle->GetTechnoType();
//		const auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(VictimType);
//		const auto pVictimAs = pVictimTypeExt->ReversedAs.Get(VictimType);
//
//		if(std::find_if(std::begin(nVec), std::end(nVec), [&](TechnoTypeClass* pTech) { return pTech == pVictimAs; }) == std::end(nVec)) {
//			if (!(AresData::PrereqValidate(Owner, pVictimAs, false, true) == CanBuildResult::Buildable)) {
//
//				nVec.push_back(pVictimAs);
//
//				if (AresData::RequirementsMet(Owner, pVictimAs) >= 2) {
//
//					Owner->RecheckTechTree = true;
//
//					if (nColle->Owner && nColle->Owner->ControlledByCurrentPlayer()) {
//						VoxClass::Play(nColle->WhatAmI() == InfantryClass::AbsID ? "EVA_ReverseEngineeredInfantry" : "EVA_ReverseEngineeredVehicle");
//						VoxClass::Play(GameStrings::EVA_NewTechAcquired());
//					}
//
//					if (auto FirstTag = pBuilding->AttachedTag) {
//						FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, pBuilding, CellStruct::Empty, false, nColle);
//
//						if (auto pSecondTag = pBuilding->AttachedTag)
//							FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, pBuilding, CellStruct::Empty, false, nullptr);
//					}
//				}
//			}
//		}
//	}
//
//	return true;
//}


// ASMJIT_PATCH(0x73A1BC, UnitClass_UpdatePosition_EnteredGrinder, 0x7)
// {
// 	GET(UnitClass* const, Vehicle, EBP);
// 	GET(BuildingClass* const, Grinder, EBX);
//
// 	//ReverseEngineer(Grinder, Vehicle);
//
//
// 	return 0;
// }

// ASMJIT_PATCH(0x5198AD, InfantryClass_UpdatePosition_EnteredGrinder, 0x6)
// {
// 	GET(InfantryClass* const, Infantry, ESI);
// 	GET(BuildingClass* const, Grinder, EBX);
//
// 	//ReverseEngineer(Grinder, Infantry);
//
// 	return 0;
// }

ASMJIT_PATCH(0x7004AD, TechnoClass_GetActionOnObject_Saboteur, 0x6)
{
	// this is known to be InfantryClass, and Infiltrate is yes
	GET(InfantryClass* const, pThis, ESI);
	GET(ObjectClass* const, pObject, EDI);

	bool infiltratable = false;
	if (const auto pBldObject = cast_to<BuildingClass*>(pObject))
	{
		infiltratable = TechnoExt_ExtData::GetiInfiltrateActionResult(pThis, pBldObject) != Action::None;
	}

	return infiltratable ? 0x700531u : 0x700536u;
}

ASMJIT_PATCH(0x51EE6B, InfantryClass_GetActionOnObject_Saboteur, 6)
{
	enum
	{
		infiltratable = 0x51EEEDu, Notinfiltratable = 0x51F04Eu
	};

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);

	if (auto pBldObject = cast_to<BuildingClass*>(pObject))
	{
		if (pThis->Owner && !pThis->Owner->IsAlliedWith(pBldObject))
		{
			const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBldObject->Type);

			switch (TechnoExt_ExtData::GetiInfiltrateActionResult(pThis, pBldObject))
			{
			case Action::Move:
				MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Spy, Action::Capture, 0);
				break;
			case Action::NoMove:
				MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Sabotage, Action::Capture, 0);
				break;
			case Action::None:
				return Notinfiltratable;
			}

			return infiltratable;
		}
	}

	return Notinfiltratable;
}

ASMJIT_PATCH(0x51E635, InfantryClass_GetActionOnObject_EngineerOverFriendlyBuilding, 5)
{
	enum
	{
		DontRepair = 0x51E63A,
		DoRepair = 0x51E659,
		SkipAll = 0x51E458,
		ReturnValue = 0x51F17E
	};

	GET(TechnoClass* const, pTarget, ESI);
	GET(InfantryClass* const, pThis, EDI);

	auto pBuilding = cast_to<BuildingClass*>(pTarget);

	if(pBuilding){
		const auto pData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		if ((pData->RubbleIntact || pData->RubbleIntactRemove) && pTarget->Owner->IsAlliedWith(pThis))
		{
			MouseCursorFuncs::SetMouseCursorAction(90u, Action::GRepair, false);
			R->EAX(Action::GRepair);
			return SkipAll;
		}
	}

	if(((R->EAX<DWORD>() & 0x4000) != 0))
	{
		if (pBuilding)
		{
			const bool canBeGrinded = pBuilding->Type->Grinding && BuildingExtData::CanGrindTechno(pBuilding, pThis);
			Action ret = canBeGrinded ? Action::Repair : Action::NoGRepair;

			if(ret == Action::NoGRepair &&
				(pBuilding->Type->InfantryAbsorb
				|| BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->TunnelType != -1
				|| pBuilding->Type->Hospital && pThis->GetHealthPercentage() < RulesClass::Instance->ConditionGreen
				|| pBuilding->Type->Armory && pThis->Type->Trainable
			  )){
				ret = pThis->SendCommand(RadioCommand::QueryCanEnter, pTarget) == RadioCommand::AnswerPositive ?
					Action::Enter : Action::NoEnter;
			}

			R->EBP(ret);
			return ReturnValue;
		}

		return DontRepair;
	}

	return DoRepair;
}

ASMJIT_PATCH(0x51FA82, InfantryClass_GetActionOnCell_EngineerRepairable, 6)
{
	GET(BuildingTypeClass* const, pBuildingType, EBP);
	R->AL(BuildingTypeExtContainer::Instance.Find(pBuildingType)
		->EngineerRepairable.Get(pBuildingType->Repairable));
	return 0x51FA88;
}

ASMJIT_PATCH(0x51E4ED, InfantryClass_GetActionOnObject_EngineerRepairable, 6)
{
	GET(BuildingClass* const, pBuilding, ESI);
	R->CL(BuildingTypeExtContainer::Instance.Find(pBuilding->Type)
		->EngineerRepairable.Get(pBuilding->Type->Repairable));
	return 0x51E4F3;
}

ASMJIT_PATCH(0x51B2CB, InfantryClass_SetTarget_Saboteur, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(ObjectClass* const, pTarget, EDI);

	if (const auto pBldObject = cast_to<BuildingClass*>(pTarget))
	{
		const auto nResult = TechnoExt_ExtData::GetiInfiltrateActionResult(pThis, pBldObject);

		if (nResult == Action::Move || nResult == Action::NoMove || nResult == Action::Enter)
			pThis->SetDestination(pTarget, true);
	}

	return 0x51B33F;
}

ASMJIT_PATCH(0x51A521, InfantryClass_UpdatePosition_ApplyC4, 0xA)
{
	enum { RetFail = 0x51A59D, RetSucceeded = 0x51A65D };

	GET(InfantryClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	if (!TechnoExt_ExtData::ApplyC4ToBuilding(pThis, pBuilding, false))
		return RetFail;

	return RetSucceeded;
}

/* #633 - spy building infiltration */
// wrapper around the entire function
ASMJIT_PATCH(0x4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass*, EnteredBuilding, ECX);
	GET_STACK(HouseClass*, Enterer, 0x4);

	TechnoExt_ExtData::InfiltratedBy(EnteredBuilding, Enterer);
	return 0x4575A2;
}

#include <Ext/Infantry/Body.h>

// This function is called when an infantry infiltrates a structure.
void WhenInfiltratesInto(FakeInfantryClass* pSpy, BuildingClass* pBuilding)
{
	// Weapon / warhead detonation upon infiltration.
	// Note that, despite the detonate or area damage func are called right before the normal infiltration effects,
	// the actual damage is resolved some time later, at least later than the normal infiltration effects resolve.
	// So there is no need to worry if the building or the spy itself will be killed before the infiltration.
	auto const rank = pSpy->Veterancy.GetRemainingLevel();

	if (auto pWeapon = pSpy->_GetTypeExtData()->WhenInfiltrate_Weapon.GetFromSpecificRank(rank))
	{
		WeaponTypeExtData::DetonateAt(pWeapon, pBuilding->GetCoords(), pSpy, pWeapon->Damage, false, pSpy->Owner);
	}
	else
	{

		const int damage = pSpy->_GetTypeExtData()->WhenInfiltrate_Damage.GetFromSpecificRank(rank);

		if (damage != 0)
		{

			auto pWarhead = pSpy->_GetTypeExtData()->WhenInfiltrate_Warhead.GetFromSpecificRank(rank);

			if (!pWarhead)
				pWarhead = RulesClass::Instance->C4Warhead;

			if (pWarhead)
			{
				if (pSpy->_GetTypeExtData()->WhenInfiltrate_Warhead_Full)
					WarheadTypeExtData::DetonateAt(pWarhead, pBuilding->GetCoords(), pSpy, damage, pSpy->Owner);
				else
				{
					auto coord = pBuilding->GetCoords();
					DamageArea::Apply(&coord, damage, pSpy, pWarhead, true, pSpy->Owner);
				}
			}
		}
	}
}

ASMJIT_PATCH(0x519FF8, InfantryClass_UpdatePosition_Saboteur, 6)
{
	enum
	{
		SkipInfiltrate = 0x51A03E,
		Infiltrate_Vanilla = 0x51A002,
		InfiltrateSucceded = 0x51A010,
	};

	GET(FakeInfantryClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	const auto nResult = TechnoExt_ExtData::GetiInfiltrateActionResult(pThis, pBuilding);

	if (nResult == Action::Move) // this one will Infiltrate instead
	{
		const auto pHouse = pThis->Owner;
		if (!pThis->Type->Agent || pHouse->IsAlliedWith(pBuilding))
			return SkipInfiltrate;

		WhenInfiltratesInto(pThis, pBuilding);

		if (pThis->IsAlive)
		{
			if (pBuilding->IsAlive)
			{
				pBuilding->Infiltrate(pHouse);
				return InfiltrateSucceded;
			}

			return SkipInfiltrate;
		}

		return 0x51A034;
	}
	else
		if (nResult == Action::NoMove)
		{
			if (!TechnoExt_ExtData::ApplyC4ToBuilding(pThis, pBuilding, true))
				return SkipInfiltrate;

			if (auto const pTag = pBuilding->AttachedTag)
			{
				pTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, CellStruct::Empty, false, nullptr);
			}

			return InfiltrateSucceded;
		}

	return SkipInfiltrate;

}

ASMJIT_PATCH(0x7376D9, UnitClass_ReceivedRadioCommand_DockUnload_Facing, 5)
{
	GET(UnitClass* const, pUnit, ESI);
	GET(DirStruct* const, nCurrentFacing, EAX);

	const auto nDecidedFacing = TechnoExt_ExtData::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing)
		return 0x73771B;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73770C;
}

ASMJIT_PATCH(0x73DF66, UnitClass_Mi_Unload_DockUnload_Facing, 5)
{
	GET(UnitClass* const, pUnit, ESI);
	GET(DirStruct* const, nCurrentFacing, EAX);

	const auto nDecidedFacing = TechnoExt_ExtData::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing || pUnit->IsRotating)
		return 0x73DFBD;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73DFB0;
}

ASMJIT_PATCH(0x43CA80, BuildingClass_ReceivedRadioCommand_DockUnloadCell, 7)
{
	GET(CellStruct* const, pCell, EAX);
	GET(BuildingClass* const, pThis, ESI);

	const auto nBuff = TechnoExt_ExtData::UnloadCell(pThis);
	R->DX(pCell->X + nBuff.X);
	R->AX(pCell->Y + nBuff.Y);

	return 0x43CA8D;
}

ASMJIT_PATCH(0x73E013, UnitClass_Mi_Unload_DockUnloadCell1, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E05F;
}

ASMJIT_PATCH(0x73E17F, UnitClass_Mi_Unload_DockUnloadCell2, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E1CB;
}

ASMJIT_PATCH(0x73E2BF, UnitClass_Mi_Unload_DockUnloadCell3, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E30B;
}

ASMJIT_PATCH(0x741BDB, UnitClass_SetDestination_DockUnloadCell, 7)
{
	GET(UnitClass* const, pThis, EBP);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x741C28;
}

//CanBuildResult __fastcall StripClass_DrawIt_HouseClass_CanBuild(HouseClass* pThis , DWORD ,TechnoTypeClass* pProduct, bool buildLimitOnly, bool allowIfInProduction)
//{
//	auto nResult = pThis->CanBuild(pProduct , buildLimitOnly , allowIfInProduction);
//
//	if(nResult == CanBuildResult::Unbuildable)
//		nResult = CanBuildResult::TemporarilyUnbuildable;
//
//	return nResult;
//}
//
//DEFINE_JUMP(CALL , 0x6AA781 , GET_OFFSET(StripClass_DrawIt_HouseClass_CanBuild))
//DEFINE_JUMP(CALL , 0x6A97D2, GET_OFFSET(StripClass_DrawIt_HouseClass_CanBuild))

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
ASMJIT_PATCH(0x709B4E, TechnoClass_DrawPipscale_SkipSkipTiberium, 6)
{
	GET(TechnoClass* const, pThis, EBP);

	bool showTiberium = true;
	if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
	{
		if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
		{
			// show only if this refinery uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			showTiberium = TechnoTypeExtContainer::Instance.Find(pBld->Type)->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}

ASMJIT_PATCH(0x44F7A0, BuildingClass_UpdateDisplayTo, 6)
{
	GET(BuildingClass*, B, ECX);
	TechnoExt_ExtData::UpdateDisplayTo(B);
	return 0x44F813;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
ASMJIT_PATCH(0x44161C, BuildingClass_Destroy_OldSpy1, 6)
{
	GET(BuildingClass*, B, ESI);
	B->DisplayProductionTo.Clear();
	TechnoExt_ExtData::UpdateDisplayTo(B);
	return 0x4416A2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
ASMJIT_PATCH(0x448312, BuildingClass_ChangeOwnership_OldSpy1, 0xA)
{
	GET(HouseClass*, newOwner, EBX);
	GET(BuildingClass*, B, ESI);

	if (B->DisplayProductionTo.Contains(newOwner))
	{
		B->DisplayProductionTo.Remove(newOwner);
		TechnoExt_ExtData::UpdateDisplayTo(B);
	}

	return 0x4483A0;
}

ASMJIT_PATCH(0x455DA0, BuildingClass_IsFactory_CloningFacility, 6)
{
	GET(BuildingClass*, pThis, ECX);

	const auto what = pThis->Type->Factory;

	if (what == AircraftTypeClass::AbsID
		|| BuildingTypeExtContainer::Instance.Find(pThis->Type)->CloningFacility)
		return 0x455DCD;

	return 0x0;
}

ASMJIT_PATCH(0x4444B3, BuildingClass_KickOutUnit_NoAlternateKickout, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->Factory == AbstractType::None
		|| BuildingTypeExtContainer::Instance.Find(pThis->Type)->CloningFacility.Get()
		? 0x4452C5 : 0x0;
}

ASMJIT_PATCH(0x446366, BuildingClass_Place_Academy, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->Academy)
	{
		HouseExtData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	return 0x446382;
}

ASMJIT_PATCH(0x445905, BuildingClass_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap &&
		BuildingTypeExtContainer::Instance.Find(pThis->Type)->Academy)
	{
		HouseExtData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	R->ECX(pThis->Type);
	return 0x445946;
}

ASMJIT_PATCH(0x448AB2, BuildingClass_ChangeOwnership_UnregisterFunction, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->Academy)
	{
		HouseExtData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	//if (pThis->Type->OrePurifier && pThis->Owner)
	//	--pThis->Owner->NumOrePurifiers;

	return 0x448AC8;
}

ASMJIT_PATCH(0x4491D5, BuildingClass_ChangeOwnership_RegisterFunction, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->IsAcademy()
		 && pThis->Owner)
	{
		HouseExtData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	//if (pThis->Type->OrePurifier && pThis->Owner)
	//	++pThis->Owner->NumOrePurifiers;

	return 0x4491F1;
}

ASMJIT_PATCH(0x44D8A1, BuildingClass_UnloadPassengers_Unload, 6)
{
	GET(BuildingClass*, B, EBP);
	TechnoExt_ExtData::KickOutHospitalArmory(B);
	return 0;
}

ASMJIT_PATCH(0x446AAF, BuildingClass_Place_SkipFreeUnits, 6)
{
	// allow free units and non-separate aircraft to be created
	// only once.
	GET(BuildingClass*, pBld, EBP);

	auto pBldExt = TechnoExtContainer::Instance.Find(pBld);

	// skip handling free units
	if (pBldExt->FreeUnitDone)
		return 0x446FB6;

	pBldExt->FreeUnitDone = true;
	return 0;
}

// #665: Raidable Buildings - prevent raided buildings from being sold while raided
ASMJIT_PATCH(0x4494D2, BuildingClass_IsSellable, 6)
{
	enum { Sellable = 0x449532, Unsellable = 0x449536, Undecided = 0 };
	GET(BuildingClass*, pThis, ESI);

	// enemy shouldn't be able to sell "borrowed" buildings
	return BuildingExtContainer::Instance.Find(pThis)->OwnerBeforeRaid ? Unsellable : Undecided;
}

ASMJIT_PATCH(0x449518, BuildingClass_IsSellable_FirestormWall, 6)
{
	enum { CheckHouseFireWallActive = 0x449522, ReturnFalse = 0x449536 };
	//GET(BuildingClass*, pThis, ESI);

	GET(BuildingTypeClass*, pType, ECX);
	return BuildingTypeExtContainer::Instance.Find(pType)->Firestorm_Wall ? CheckHouseFireWallActive : ReturnFalse;
}

ASMJIT_PATCH(0x44E550, BuildingClass_Mi_Open_GateDown, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->ECX(BuildingTypeExtContainer::Instance.Find(pThis->Type)->GateDownSound
		.Get(RulesClass::Instance->GateDown));
	return 0x44E556;
}

ASMJIT_PATCH(0x44E61E, BuildingClass_Mi_Open_GateUp, 6)
{
	GET(DWORD, offset, ESI);
	const auto pThis = reinterpret_cast<BuildingClass*>(offset - 0x9C);
	R->ECX(BuildingTypeExtContainer::Instance.Find(pThis->Type)->GateUpSound
		.Get(RulesClass::Instance->GateUp));
	return 0x44E624;
}

ASMJIT_PATCH(0x4509B4, BuildingClass_UpdateRepair_Funds, 7)
{
	GET(BuildingClass*, pThis, ESI);
	return !pThis->Owner->IsControlledByHuman() || RulesExtData::Instance()->RepairStopOnInsufficientFunds
		? 0x0 : 0x4509BB;
}

ASMJIT_PATCH(0x4521C8, BuildingClass_Disable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);

	auto const pType = pThis->Type;
	if (pType->Factory != AbstractType::None)
	{
		pThis->Owner->Update_FactoriesQueues(
		pType->Factory, pType->Naval, BuildCat::DontCare);
	}
	return 0;
}

ASMJIT_PATCH(0x4566B0, BuildingClass_GetRangeOfRadial_Radius, 6)
{
	enum
	{
		SetVal = 0x45674E
		, Nothing = 0x0
	};

	GET(BuildingClass*, pThis, ECX);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pExt->RadialIndicatorRadius.isset())
		return Nothing;

	R->EAX(pExt->RadialIndicatorRadius.Get());
	return SetVal;
}

ASMJIT_PATCH(0x456768, BuildingClass_DrawRadialIndicator_Always, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return pExt->AlwayDrawRadialIndicator.Get(pThis->HasPower) ?
		0x456776 : 0x456962;
}


ASMJIT_PATCH(0x4581CD, BuildingClass_HandleOccupants, 6) //UnloadOccupants_AllOccupantsHaveLeft
{
	GET(BuildingClass*, pBld, ESI);
	TechnoExt_ExtData::EvalRaidStatus(pBld);
	return 0;
}ASMJIT_PATCH_AGAIN(0x458729, BuildingClass_HandleOccupants, 6) //KillOccupiers_AllOccupantsKilled
ASMJIT_PATCH_AGAIN(0x4586CA, BuildingClass_HandleOccupants, 6) //KillOccupiers_EachOccupierKilled

ASMJIT_PATCH(0x441f2c, BuildingClass_Destroy_KickOutOfRubble, 5)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->RubbleDestroyed || pTypeExt->RubbleIntact)
		TechnoExt_ExtData::KickOutOfRubble(pThis);

	return 0x0;
}

ASMJIT_PATCH(0x465A48, BuildingTypeClass_GetBuildup_BuildupTime, 5)
{
	GET(BuildingTypeClass*, pThis, ESI);
	BuildingTypeExtData::UpdateBuildupFrames(pThis);
	return 0x465AAE;
}

ASMJIT_PATCH(0x45EAA5, BuildingTypeClass_LoadArt_BuildupTime, 6)
{
	GET(BuildingTypeClass*, pThis, ESI);
	BuildingTypeExtData::UpdateBuildupFrames(pThis);
	return 0x45EB3A;
}

ASMJIT_PATCH(0x45F2B4, BuildingTypeClass_Load2DArt_BuildupTime, 5)
{
	GET(BuildingTypeClass*, pThis, EBP);
	BuildingTypeExtData::UpdateBuildupFrames(pThis);
	return 0x45F310;
}

ASMJIT_PATCH(0x447a63, BuildingClass_QueueImageAnim_Sell, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_BASE(int, frames, 0x8);

	if (pThis->CurrentMission == Mission::Selling)
	{
		R->EAX(BuildingTypeExtContainer::Instance.Find(pThis->Type)->SellFrames);
	}
	else
	{
		R->EAX(frames);
	}

	R->EDX(pThis->Type);
	return 0x447A6C;
}

ASMJIT_PATCH(0x459C03, BuildingClass_CanBeSelectedNow_MassSelectable, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->MassSelectable.Get(pThis->Type->IsUndeployable()))
	{
		return 0x459C14;
	}

	R->EAX(false);
	return 0x459C12;
}

ASMJIT_PATCH(0x457D58, BuildingClass_CanBeOccupied_SpecificOccupiers, 6)
{
	enum { AllowOccupy = 0x457DD5, DisallowOccupy = 0x457DA3 };

	GET(BuildingClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);
	BuildingTypeExtData* pBuildTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	//bool can_occupy = false;

	if (!pBuildTypeExt->CanBeOccupiedBy(pInf))
		return DisallowOccupy;

	const int occupantCount = pThis->GetOccupantCount();

	if (occupantCount == pThis->Type->MaxNumberOccupants)
		return DisallowOccupy;

	if (((pThis->Type->TechLevel == -1) && pThis->IsRedHP()) || pInf->IsMindControlled())
		return DisallowOccupy;

	const bool isNeutral = pThis->Owner->IsNeutral();
	const bool sameOwner = (pThis->Owner == pInf->Owner);
	const bool isRaidable = pBuildTypeExt->BunkerRaidable && occupantCount == 0;
	const bool controlledByCurrentPlayer = SessionClass::Instance->GameMode == GameMode::Campaign && pThis->Owner->ControlledByCurrentPlayer() && pInf->Owner->ControlledByCurrentPlayer();

	if (sameOwner || controlledByCurrentPlayer || isNeutral || isRaidable)
	{
		return AllowOccupy;
	}

	return DisallowOccupy;
}

ASMJIT_PATCH(0x52297F, InfantryClass_GarrisonBuilding_OccupierEntered, 5)
{
	GET(InfantryClass*, pInf, ESI);
	GET(BuildingClass*, pBld, EBP);

	TechnoExtContainer::Instance.Find(pInf)->GarrisonedIn = pBld;
	//pInf->Target = nullptr; //reset targeting

	auto buildingExtData = BuildingExtContainer::Instance.Find(pBld);

	// if building and owner are from different players, and the building is not in raided state
	// change the building's owner and mark it as raided
	// but only if that's even necessary - no need to raid urban combat buildings.
	// 27.11.2010 changed to include fix for #1305
	const bool differentOwners = (pBld->Owner != pInf->Owner) && (SessionClass::Instance->GameMode != GameMode::Campaign) || !pBld->Owner->IsHumanPlayer || !pInf->Owner->IsHumanPlayer;
	const bool ucBuilding = ((pBld->Type->TechLevel == -1) && pBld->Owner->IsNeutral());

	if (differentOwners && !buildingExtData->OwnerBeforeRaid && !ucBuilding)
	{
		buildingExtData->OwnerBeforeRaid = pBld->Owner;
		pBld->SetOwningHouse(pInf->Owner);
	}

	return 0;
}

ASMJIT_PATCH(0x455828, BuildingClass_SensorArrayActivate, 8)
{
	GET(BuildingClass*, pBld, ECX);
	auto pExt = BuildingExtContainer::Instance.Find(pBld);

	// only add sensor if ActiveCount was zero
	if (pBld->IsPowerOnline() && !pBld->Deactivated && !pExt->SensorArrayActiveCounter++)
	{
		return 0x455838;
	}

	return 0x45596F;
}

ASMJIT_PATCH(0x4556E1, BuildingClass_SensorArrayDeactivate, 7)
{
	GET(BuildingClass*, pBld, ECX);
	auto pExt = BuildingExtContainer::Instance.Find(pBld);

	// don't do the same work twice
	if (pExt->SensorArrayActiveCounter > 0)
	{
		pExt->SensorArrayActiveCounter = 0;

		// this fixes the issue where the removed area does not match the
		// added area. adding uses SensorsSight, so we remove that one here.
		R->EBP(pBld->Type->SensorsSight);

		return 0x4556E8;
	}

	return 0x45580D;
}

// powered state changed
// ???

ASMJIT_PATCH(0x4549F8, BuildingClass_UpdatePowered_SensorArray, 6)
{
	GET(BuildingClass*, pBld, ESI);
	TechnoExt_ExtData::UpdateSensorArray(pBld);
	return 0;
}ASMJIT_PATCH_AGAIN(0x454B5F, BuildingClass_UpdatePowered_SensorArray, 6)

// something changed to the worse, like toggle power
ASMJIT_PATCH(0x4524A3, BuildingClass_DisableThings, 6)
{
	GET(BuildingClass*, pBld, EDI);
	TechnoExt_ExtData::UpdateSensorArray(pBld);
	return 0;
}

// check every frame
ASMJIT_PATCH(0x43FE69, BuildingClass_Update_SensorArray, 0xA)
{
	GET(FakeBuildingClass*, pThis, ESI);
	TechnoExt_ExtData::UpdateSensorArray(pThis);

	//const auto pTechnoExt = pThis->_GetTechnoExtData();
	const auto pExt = pThis->_GetExtData();
	pExt->DisplayIncomeString();
	pExt->UpdatePoweredKillSpawns();
	pExt->UpdateAutoSellTimer();
	pExt->UpdateSpyEffecAnimDisplay();

	//const auto pFactory = pThis->Factory;

	//if (pFactory && pFactory->Object)
	//{
	//	const auto pTimer = &pFactory->Production.Timer;
	//
	//	if (pTimer->InProgress() &&
	//		(pThis->IsUnderEMP() ||
	//			pThis->Deactivated ||
	//			(!pThis->IsPowerOnline() && pThis->GetPowerDrain() == 0 && pThis->GetPowerOutput() == 0)))
	//		pTimer->TimeLeft++;
	//}

	return 0;
}

ASMJIT_PATCH(0x454BDC, BuildingClass_UpdatePowered_LargeGap, 7)
{
	GET(BuildingTypeClass*, pType, ECX);
	R->EDX(TechnoTypeExtContainer::Instance.Find(pType)->GapRadiusInCells.Get());
	return 0x454BE3;
}

ASMJIT_PATCH(0x43FB6D, BuildingClass_Update_LaserFencePost, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->LaserFencePost)
		pThis->CreateEndPost(true);

	return 0x0;
}

ASMJIT_PATCH(0x445F80, BuildingClass_Place, 5)
{
	GET(BuildingClass*, pThis, ECX);

	if (pThis->Type->SecretLab)
	{
		BuildingExtData::UpdateSecretLab(pThis);
	}

	HouseExtData::UpdateFactoryPlans(pThis);
	return 0;
}

// make temporal weapons play nice with power toggle.
// previously, power state was set to true unconditionally.
ASMJIT_PATCH(0x452287, BuildingClass_GoOnline_TogglePower, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	BuildingExtContainer::Instance.Find(pThis)->TogglePower_HasPower = true;
	return 0;
}

ASMJIT_PATCH(0x452393, BuildingClass_GoOffline_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ESI);
	BuildingExtContainer::Instance.Find(pThis)->TogglePower_HasPower = false;
	return 0;
}

ASMJIT_PATCH(0x452210, BuildingClass_Enable_TogglePower, 7)
{
	GET(BuildingClass* const, pThis, ECX);
	pThis->HasPower = BuildingExtContainer::Instance.Find(pThis)->TogglePower_HasPower;
	return 0x452217;
}
