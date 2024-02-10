
#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <SlaveManagerClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Particle/Body.h>

#include <Utilities/Helpers.h>

#include <Locomotor/HoverLocomotionClass.h>

#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <WWKeyboardClass.h>
#include <MPGameModeClass.h>
#include <LoadOptionsClass.h>
#include <VersionHelpers.h>
#include <FPSCounter.h>
#include <EventClass.h>
#include <dxcore.h>
#include <TerrainTypeClass.h>
#include <Notifications.h>

#include <strsafe.h>


#include "AresChecksummer.h"
#include "Classes/Dialogs.h"

#include "Header.h"

#include <Ares_TechnoExt.h>

#ifndef aaa
DEFINE_DISABLE_HOOK(0x6873ab, INIClass_ReadScenario_EarlyLoadRules_ares)
DEFINE_DISABLE_HOOK(0x6d4684, TacticalClass_Draw_FlyingStrings_ares)
DEFINE_DISABLE_HOOK(0x7258d0, AnnounceInvalidPointer_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x533058, CommandClassCallback_Register_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x679CAF, RulesData_LoadAfterTypeData_ares)
DEFINE_DISABLE_HOOK(0x667a1d, RulesClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x667a30, RulesClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x668bf0, RulesClass_Addition_ares)
DEFINE_DISABLE_HOOK(0x674730, RulesClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x675205, RulesClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x675210, RulesClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x678841, RulesClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x679a15, RulesData_LoadBeforeTypeData_ares)
#endif

DEFINE_OVERRIDE_HOOK(0x52C5E0, Ares_NOLOGO, 0x7)
{
	return Phobos::Otamaa::NoLogo ? 0x52C5F3 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x62A020, ParasiteClass_Update, 0xA)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExtContainer::Instance.Find(pOwner)->idxSlot_Parasite));
	return 0x62A02A;
}

DEFINE_OVERRIDE_HOOK(0x62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExtContainer::Instance.Find(pOwner)->idxSlot_Parasite));
	return 0x62A7BA;
}

DEFINE_OVERRIDE_HOOK(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	if (pThis->Owner == HouseClass::CurrentPlayer() &&
		pThis->Object &&
		pThis->Object->WhatAmI() == BuildingClass::AbsID
		)
	{
		pThis->Object->RemoveSidebarObject();
	}

	return 0;
}

DEFINE_DISABLE_HOOK(0x565215, MapClass_CTOR_NoInit_Crates_ares)//, 0x6, 56522D)
DEFINE_JUMP(LJMP, 0x565215, 0x56522D);


DEFINE_HOOK(0x5F6500, AbstractClass_Distance2DSquared_1, 8)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pThat, 0x4);

	int nResult = 0;
	if (pThat)
	{
		const auto nThisCoord = pThis->GetCoords();
		const auto nThatCoord = pThat->GetCoords();
		nResult = (int)nThisCoord.DistanceFromXY(nThatCoord);
	}

	R->EAX(nResult);
	return 0x5F655D;
}

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto const nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);
	R->EAX((int)nThisCoord.DistanceFromXY(*pThatCoord));
	return 0x5F659B;
}

//DEFINE_HOOK(0x6E2290, ActionClass_PlayAnimAt, 0x6)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pOwner, 0x4);
//	//GET_STACK(TechnoClass*, pInvoker, 0x8);
//	//GET_STACK(TriggerClass*, pTrigger, 0xC);
//	//GET_STACK(CellStruct*, pCell, 0x10);
//
//	auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Waypoint);
//	auto nCoord = CellClass::Cell2Coord(nCell);
//	nCoord.Z = MapClass::Instance->GetZPos(&nCoord);
//	auto pCellTarget = MapClass::Instance->GetCellAt(nCell);
//	nCoord = pCellTarget->GetCoordsWithBridge();
//
//	if (AnimTypeClass* AnimType = AnimTypeClass::Array->GetItemOrDefault(pThis->Value))
//	{
//		if (AnimClass* pAnim = GameCreate<AnimClass>(AnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
//		{
//			pAnim->IsPlaying = true;
//			AnimExtData::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
//		}
//	}
//
//	R->EAX(1);
//	return 0x6E2387;
//}

DEFINE_DISABLE_HOOK(0x6AD0ED, Game_AllowSinglePlay_ares)//, 0x5, 6AD16C);
DEFINE_JUMP(LJMP, 0x6AD0ED, 0x6AD16C);

DEFINE_OVERRIDE_HOOK(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
{
	REF_STACK(RectangleStruct const, bounds, STACK_OFFS(0x7C, 0x10));
	//0x89C568
	REF_STACK(unsigned char const*, pBuffer, STACK_OFFS(0x7C, 0x6C));

	auto const width = static_cast<size_t>(std::clamp<short>(
		static_cast<short>(bounds.Width), 0, std::numeric_limits<short>::max()));

	// buffer overrun is now not as forgiving as it was before
	auto& Buffer = StaticVars::ShpCompression1Buffer;

	if (Buffer.size() < width)
	{
		Buffer.insert(Buffer.end(), width - Buffer.size(), 0u);
	}

	pBuffer = Buffer.data();

	return 0x437CD4;
}

namespace ShakeScreenHandle
{
	void ShakeScreen(TechnoClass* pThis, int nValToCalc, int nRules)
	{
		if (pThis->IsOnMyView())
		{
			auto nFirst = GeneralUtils::GetValue(nValToCalc);
			auto nSec = nFirst - GeneralUtils::GetValue(nRules) + 4;
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, nSec >> 1);
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, nSec);
		}
	}
}

//handle everything ourself
DEFINE_OVERRIDE_HOOK(0x441C21, BuildingClass_Destroyed_Shake, 0x6)
{
	GET(BuildingClass* const, pBld, ESI);

	if (!pBld || !pBld->Type || !RulesClass::Instance->ShakeScreen)
		return 0x441C39;

	GET(int, BuildingCost, EAX);

	if (!BuildingCost)
		return 0x441C39;

	if (!TechnoTypeExtContainer::Instance.Find(pBld->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pBld, BuildingCost, RulesClass::Instance->ShakeScreen);

	return 0x441C39; //return 0 causing crash
}

DEFINE_OVERRIDE_HOOK(0x7387D1, UnitClass_Destroyed_Shake, 0x6)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen)
		return 0x738801;

	if (!pUnit->Type->Strength)
		return 0x738801;

	if (!TechnoTypeExtContainer::Instance.Find(pUnit->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pUnit, pUnit->Type->Strength, RulesClass::Instance->ShakeScreen);

	return 0x738801;
}

// replaced entire function. error was using delete[] instead of delete.
// it potentially crashed when any of the files were present in the
// game directory.
DEFINE_OVERRIDE_HOOK(0x5F77F0, ObjectTypeClass_UnloadPipsSHP, 0x5)
{
	for (int i = 0; i < (int)TechnoTypeClass::ShapesIsAllocated.size(); ++i)
	{
		if (TechnoTypeClass::ShapesIsAllocated[i] && FileSystem::ShapesAllocated[i])
		{
			GameDelete<true, false>(FileSystem::ShapesAllocated[i]);
			TechnoTypeClass::ShapesIsAllocated[i] = false;
		}
	}

	return 0x5F78FB;
}

// naive way to fix negative indexes to be generated. proper way would be to replace
// the entire function, and the function consuming the indexes. it is not yet known
// whether the out of bounds read causes desync errors. this function appears to
// have been inlined prominently in 585F40
DEFINE_OVERRIDE_HOOK(0x56BC54, ThreatPosedEstimates_GetIndex, 0x5)
{
	GET(const CellStruct*, pCell, ECX);

	int index = -1;
	if (pCell->X >= 0 && pCell->Y >= 0 && pCell->X < 512 && pCell->Y < 512)
	{
		index = pCell->X / 4 + 130 * pCell->Y / 4 + 131;
	}

	R->EAX(index);
	return 0x56BC7D;
}

// don't set the focus when selling (true selling, thus no focus set atm)
DEFINE_OVERRIDE_HOOK(0x4C6DDB, Networking_RespondToEvent_Selling, 0x8)
{
	GET(TechnoClass* const, pTechno, EDI);
	GET(AbstractClass* const, pFocus, EAX);

	if (pTechno->CurrentMission != Mission::Selling || pTechno->Focus)
	{
		pTechno->SetFocus(pFocus);
	}

	return 0x4C6DE3;
}

// #895374: skip the code that removes the crates (size 7)
DEFINE_DISABLE_HOOK(0x483BF1, CellClass_Load_Crates_ares)//, 0x7, 483BFE)
DEFINE_JUMP(LJMP, 0x483BF1, 0x483BFE);

DEFINE_OVERRIDE_HOOK(0x699C1C, Game_ParsePKTs_ClearFile, 0x7)
{
	LEA_STACK(CCINIClass*, pINI, 0x24);
	pINI->Clear(nullptr, nullptr);
	return 0;
}

// Guard command failure
DEFINE_OVERRIDE_HOOK(0x730DB0, GuardCommandClass_Execute, 0xA)
{
	GET(TechnoClass*, T, ESI);
	return (T->Owner != HouseClass::CurrentPlayer() || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
		;
}

/* #367 - do we need to draw a link to this victim */
DEFINE_OVERRIDE_HOOK(0x472198, CaptureManagerClass_DrawLinks, 0x6)
{
	enum { Draw_Maybe = 0, Draw_Yes = 0x4721E6, Draw_No = 0x472287 };

	GET(CaptureManagerClass*, Controlled, EDI);
	//GET(TechnoClass *, Item, ECX);

	if (FootClass* F = generic_cast<FootClass*>(Controlled->Owner))
	{
		if (F->ParasiteImUsing && F->InLimbo)
		{
			return Draw_No;
		}
	}

	return Draw_Maybe;
}

DEFINE_OVERRIDE_HOOK(0x551A30, LayerClass_YSortReorder, 0x5)
{
	GET(LayerClass*, pThis, ECX);

	auto const nCount = pThis->Count;
	auto nBegin = &pThis->Items[nCount / 15 * (Unsorted::CurrentFrame % 15)];
	auto nEnd = (Unsorted::CurrentFrame % 15 >= 14) ? (&pThis->Items[nCount]) : (&nBegin[nCount / 15 + nCount / 15 / 4]);
	std::sort(nBegin, nEnd, [](const ObjectClass* A, const ObjectClass* B)
	{
		return A->GetYSort() < B->GetYSort();
	});

	return 0x551A84;
}

//DEFINE_HOOK(0x5F65F0, ObjectClass_UnUnit_nullptr, 0x6)
//{
//	GET(ObjectClass*, pThis, ECX);
//
//	if (!pThis)
//		Debug::FatalError("ObjectClass UnInit called from %x\n",R->Stack<DWORD>(0x0));
//
//	return 0x0;
//}

DEFINE_OVERRIDE_HOOK(0x5F6612, ObjectClass_UnInit_SkipInvalidation, 0x9)
{
	GET(ObjectClass*, pThis, ESI);

	if (!pThis->Limbo())
		pThis->AnnounceExpiredPointer();

	return 0x5F6625;
}

//speeds up preview drawing by insane amounts
DEFINE_OVERRIDE_HOOK(0x5FED00, OverlayTypeClass_GetRadarColor, 0x6)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct*, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

// only eject the parasite if the unit leaves the battlefield,
// not just when it goes out of sight.
DEFINE_OVERRIDE_HOOK(0x62A283, ParasiteClass_PointerGotInvalid_Cloak, 0x9)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(void*, ptr, EAX);
	GET_STACK(bool, remove, 0x2C);

	// remove only if pointer really got removed
	return (remove && pThis->Victim == ptr) ? 0x62A28C : 0x62A484;
}

/* #746 - don't set parasite eject point to cell center, but set it to fall and explode like a bomb */
DEFINE_OVERRIDE_HOOK(0x62A2F8, ParasiteClass_PointerGotInvalid, 0x6)
{
	GET(ParasiteClass*, Parasite, ESI);
	GET(CoordStruct*, XYZ, EAX);

	auto Owner = Parasite->Owner;
	auto const pWhat = Owner->WhatAmI();

	bool allowed = false;
	if (pWhat == UnitClass::AbsID)
	{
		allowed = !Owner->GetTechnoType()->Naval;
	}
	else if (pWhat == InfantryClass::AbsID)
	{
		allowed = true;
	}

	if (allowed)
	{
		if (Owner->GetHeight() > 200)
		{
			*XYZ = Owner->Location;
			Owner->IsFallingDown = Owner->IsABomb = true;
		}
		else if (Owner->GetHeight() < 0) //inside ground
			*XYZ = CoordStruct::Empty;
	}

	CoordStruct result = *XYZ;

	if (result == CoordStruct::Empty || CellClass::Coord2Cell(result) == CellStruct::Empty)
	{
		Debug::Log("Parasite[%x : %s] With Invalid Location ! , Removing ! \n", Parasite, Parasite->Owner->get_ID());
		TechnoExtData::HandleRemove(Parasite->Owner, nullptr, false, false);
		Parasite->Victim = nullptr;
		return 0x62A47B; //pop the registers
	}

	return 0;
}

// bugfix #187: Westwood idiocy
DEFINE_HOOK(0x5F6960, ObjectClass_Getcell, 0xA)
{
	GET(ObjectClass*, pThis, ECX);
	R->EAX(MapClass::Instance->GetCellAt(pThis->Location));
	return 0x5F69B6;
}

DEFINE_DISABLE_HOOK(0x615BD3, Handle_Static_Messages_LoopingMovie_ares)//, 0x5, 615BE0)
DEFINE_JUMP(LJMP, 0x615BD3, 0x615BE0);
DEFINE_DISABLE_HOOK(0x78997B, sub_789960_RemoveWOLResolutionCheck_ares)//, 0x5, 789A58)
DEFINE_JUMP(LJMP, 0x78997B, 0x789A58);
DEFINE_DISABLE_HOOK(0x4BA61B, DSurface_CTOR_SkipVRAM_ares)//, 0x6, 4BA623)
DEFINE_JUMP(LJMP, 0x4BA61B, 0x4BA623);

DEFINE_OVERRIDE_HOOK(0x74A884, VoxelAnimClass_UpdateBounce_Damage, 0x6)
{
	GET(VoxelAnimClass*, pThis, EBX);

	const auto pType = pThis->Type;
	const auto nRadius = pType->DamageRadius;

	if (nRadius < 0 || !pType->Damage || !pType->Warhead)
		return 0x74A934;

	const auto nCoord = pThis->Bounce.GetCoords();
	const auto pCell = MapClass::Instance->GetCellAt(nCoord);
	const auto pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);

	for (NextObject j(pCell->GetContent()); j; ++j)
	{
		const auto pObj = *j;

		if (!pObj->IsAlive || pObj->InLimbo || pObj->Health <= 0)
			continue;

		const auto nLoc = pObj->Location;
		const auto nDist = abs(nLoc.X - nCoord.X) + abs(nLoc.Y - nCoord.Y);

		if (nDist < nRadius)
		{
			pObj->ReceiveDamage(&pType->Damage, Game::AdjustHeight(nDist), pType->Warhead, pInvoker, false, false, pInvoker ? pInvoker->Owner : nullptr);
		}
	}

	return 0x74A934;
}

DEFINE_OVERRIDE_HOOK(0x545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 0x7)
{
	if (R->EAX() == -1)
	{
		// all theaters except snow have this set, so I'll assume that this was tripped by snow.
		// don't like it? put the damned tag in the INI.
		R->EAX(71);
	}
	return 0;
}

// skip the entire method, we handle it ourselves
DEFINE_DISABLE_HOOK(0x53AF40, PsyDom_Update_ares)//, 6, 53B060)
DEFINE_JUMP(LJMP, 0x53AF40, 0x53B060);

DEFINE_HOOK(0x65EA43, SendReinforcement_Opentopped, 0x6)
{
	GET(AircraftClass*, pPlane, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pPlane->Type->OpenTopped)
		pPlane->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pPlane;

	return 0x0;
}

// TODO : remove this
// issue #1282: remap wall using its owner's colors
//DEFINE_OVERRIDE_HOOK(0x47F9A4, CellClass_DrawOverlay_WallRemap, 0x6)
//{
//	GET(CellClass*, pCell, ESI);
//
//	const int idx = pCell->WallOwnerIndex;
//
//	if (idx >= 0)
//	{
//		R->EDX(HouseClass::Array->Items[idx));
//		return 0x47F9AA;
//	}
//
//	return 0;
//}

// issue 1520: logging stupid shit crashes the game
DEFINE_STRONG_OVERRIDE_HOOK(0x4CA437, FactoryClass_GetCRC, 0x8)
{
	GET(FactoryClass*, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass*>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

DEFINE_OVERRIDE_HOOK(0x47243F, CaptureManagerClass_DecideUnitFate_BuildingFate, 0x6)
{
	GET(TechnoClass*, pVictim, EBX);

	//Neutral techno should not do anything after get freed/captured
	if(pVictim->Owner->IsNeutral()) {
		pVictim->Override_Mission(Mission::Sleep);
		return 0x472604;
	} else {
		if (pVictim->WhatAmI() == BuildingClass::AbsID) {
			// 1. add to team and other fates don't really make sense for buildings
			// 2. BuildingClass::Mission_Hunt() implementation is to do nothing!
			pVictim->QueueMission(Mission::Guard, 0);
			return 0x472604;
		}
	}

	return 0;
}

// PrismSupportModifier repair
DEFINE_OVERRIDE_HOOK(0x671152, RulesClass_Addition_General_PrismSupportModifier, 0x6)
{
	GET(RulesClass*, pThis, ESI);
	REF_STACK(double, param, 0x0);
	param = pThis->PrismSupportModifier / 100.0;
	return 0x67115B;
}

DEFINE_OVERRIDE_HOOK(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode.Status != SpawnNodeStatus::TakeOff
		|| !pOwner
		|| pOwner->WhatAmI() == BuildingClass::AbsID)
		? 0x6B735C : 0x6B72FE;
}

DEFINE_OVERRIDE_HOOK(0x725A1F, AnnounceInvalidPointer_SkipBehind, 0x5)
{
	GET(AnimClass*, pAnim, ESI);
	return pAnim->Type == RulesClass::Instance->Behind ?
		0x725C08 : 0x0;
}

//sub_731D90_FakeOf
DEFINE_OVERRIDE_HOOK(0x731E08, Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;

	for (const auto pObj : ObjectClass::CurrentObjects())
	{
		if (const auto pTechno = generic_cast<const TechnoClass*>(pObj))
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->AttachedToObject;
			if (pTypeExt->Fake_Of.isset())
				pType = pTypeExt->Fake_Of.Get();

			nCost += pType->GetActualCost(pTechno->Owner);
		}
	}

	R->EBX(nCost);
	return 0x731E4D;
}

DEFINE_OVERRIDE_HOOK(0x6DA665, sub_6DA5C0_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

DEFINE_OVERRIDE_HOOK(0x7BB445, XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}

// DEFINE_OVERRIDE_HOOK(0x5FDDA4, OverlayClass_GetTiberiumType_NotReallyTiberiumLog, 0x6)
// {
// 	GET(OverlayTypeClass*, pThis, EAX);
// 	Debug::Log("Overlay %s not really tiberium\n", pThis->ID);
// 	return 0x5FDDC1;
// }

DEFINE_OVERRIDE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
DEFINE_OVERRIDE_HOOK(0x489270, CellChainReact, 5)
{
	GET(CellStruct*, cell, ECX);

	const auto pCell = MapClass::Instance->GetCellAt(cell);

	if (pCell->OverlayTypeIndex <= 0)
		return 0x0;

	TiberiumClass* pTib = TiberiumClass::Array->GetItemOrDefault(pCell->GetContainedTiberiumIndex());

	if (!pTib)
		return 0x0;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->GetItemOrDefault(pCell->OverlayTypeIndex);

	if (!pOverlay || !pOverlay->ChainReaction || pCell->OverlayData <= 1u)
		return 0x0;

	if (ScenarioClass::Instance->Random.RandomFromMax(99) <
		(RulesExtData::Instance()->ChainReact_Multiplier * pCell->OverlayData))
	{
		const bool wasFullGrown = (pCell->OverlayData >= 11);

		unsigned char delta = pCell->OverlayData / 2;
		int damage = pTib->Power * delta;

		// remove some of the tiberium
		pCell->OverlayData -= delta;
		pCell->MarkForRedraw();

		// get the warhead
		auto pExt = TiberiumExtExtContainer::Instance.Find(pTib);
		CoordStruct crd = pCell->GetCoords();

		if (auto pWarhead = pExt->GetExplosionWarhead())
		{
			// create an explosion
			if (auto pType = MapClass::SelectDamageAnimation(4 * damage, pWarhead, pCell->LandType, crd))
			{
				GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, 0);
			}

			// damage the area, without affecting tiberium
			MapClass::DamageArea(crd, damage, nullptr, pWarhead, false, nullptr);
		}

		// spawn some animation on the neighbour cells
		if (auto pType = AnimTypeClass::Find(GameStrings::Anim_INVISO()))
		{
			for (int i = 0; i < 8; ++i)
			{
				auto pNeighbour = pCell->GetNeighbourCell((FacingType)i);

				if (pNeighbour->GetContainedTiberiumIndex() != -1 && pNeighbour->OverlayData > 2u)
				{
					if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesExtData::Instance()->ChainReact_SpreadChance)
					{
						int delay = ScenarioClass::Instance->Random.RandomRanged(RulesExtData::Instance()->ChainReact_MinDelay, RulesExtData::Instance()->ChainReact_MaxDelay);
						crd = pNeighbour->GetCoords();

						GameCreate<AnimClass>(pType, crd, delay, 1, 0x600, 0);
					}
				}
			}
		}

		if (wasFullGrown)
		{
			pTib->RegisterForGrowth(cell);
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x424DD3, AnimClass_ReInit_TiberiumChainReaction_Chance, 6)
{
	GET(TiberiumClass*, pTib, EDI);

	return ScenarioClass::Instance->Random.RandomFromMax(99) < TiberiumExtExtContainer::Instance.Find(pTib)->GetDebrisChance()
		? 0x424DF9 : 0x424E9B;
}

DEFINE_OVERRIDE_HOOK(0x424EC5, AnimClass_ReInit_TiberiumChainReaction_Damage, 6)
{
	GET(TiberiumClass*, pTib, EDI);
	auto pExt = TiberiumExtExtContainer::Instance.Find(pTib);

	R->Stack(0x0, pExt->GetExplosionWarhead());
	R->EDX(pExt->GetExplosionDamage());

	return 0x424ECB;
}

DEFINE_OVERRIDE_HOOK(0x71C7C2, TerrainClass_Update_ForestFire, 6)
{
	GET(TerrainClass*, pThis, ESI);

	const auto& flammability = RulesClass::Instance->TreeFlammability;

	// burn spread probability this frame
	if (flammability > 0.0)
	{
		if (pThis->IsBurning && ScenarioClass::Instance->Random.RandomFromMax(99) == 0)
		{
			const auto pCell = pThis->GetCell();

			// check all neighbour cells that contain terrain objects and
			// roll the dice for each of them.
			for (int i = 0; i < 8; ++i)
			{
				if (auto pTree = pCell->GetNeighbourCell((FacingType)i)->GetTerrain(false))
				{
					if (!pTree->IsBurning && ScenarioClass::Instance->Random.RandomDouble() < flammability)
					{
						pTree->Ignite();
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71C5D2, TerrainClass_Ignite_IsFlammable, 6)
{
	GET(TerrainClass*, pThis, EDI);

	enum { Ignite = 0x71C5F3, CantBurn = 0x71C69D };

	// prevent objects from burning that aren't flammable also
	return (pThis->Type->SpawnsTiberium || !pThis->Type->IsFlammable)
		? CantBurn : Ignite;
}

DEFINE_OVERRIDE_HOOK(0x6AB8BB, SelectClass_ProcessInput_BuildTime, 6)
{
	GET(BuildingTypeClass* const, pBuildingProduct, ESI);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pBuildingProduct);

	bool IsAWall = pBuildingProduct->Wall;
	if (pBuildingProduct->Wall && pTypeExt->BuildTime_Speed.isset())
		IsAWall = false;

	R->AL(IsAWall);
	return 0x6AB8C1;
}

DEFINE_DISABLE_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes_ares);
//DEFINE_OVERRIDE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)
DEFINE_JUMP(LJMP, 0x715857, 0x715876);

DEFINE_OVERRIDE_HOOK(0x711EE0, TechnoTypeClass_GetBuildSpeed, 6)
{
	GET(TechnoTypeClass* const, pThis, ECX);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);
	const auto nSeed = pTypeExt->BuildTime_Speed.Get(RulesClass::Instance->BuildSpeed);
	const auto nCost = pTypeExt->BuildTime_Cost.Get(pThis->Cost);
	R->EAX(int(nSeed * nCost / 1000.0 * 900.0));
	return 0x711EDE;
}

DEFINE_DISABLE_HOOK(0x6BB9DD, WinMain_LogGameClasses_ares)//, 5, 6BBE2B)
DEFINE_JUMP(LJMP, 0x6BB9DD, 0x6BBE2B);

DEFINE_DISABLE_HOOK(0x70CAD8, TechnoClass_DealParticleDamage_DontDestroyCliff_ares)//, 9, 70CB30)
DEFINE_JUMP(LJMP, 0x70CAD8, 0x70CB30);

// bugfix #187: Westwood idiocy
DEFINE_DISABLE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix1_ares) //, 5, 53173A)
DEFINE_JUMP(LJMP, 0x531726, 0x53173A);

// bugfix #187: Westwood idiocy
DEFINE_DISABLE_HOOK(0x53173F, Game_BulkDataInit_MultipleDataInitFix2_ares)//, 5, 531749)
DEFINE_JUMP(LJMP, 0x53173F, 0x531749);

//this hook taking a lot of time , i guess because of UnitTypeClass::InitOneTimeData thing
// DEFINE_OVERRIDE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix, 5)
// {
// 	BuildingTypeClass::InitOneTimeData();
// 	UnitTypeClass::InitOneTimeData();
// 	return 0x531749;
// }

DEFINE_OVERRIDE_HOOK(0x535DB6, SetStructureTabCommandClass_Execute_Power, 6)
{
	GET(BuildingClass*, pBuild, EAX);
	R->EAX(pBuild->FindFactory(false, true));
	return 0x535DC2;
}

DEFINE_OVERRIDE_HOOK(0x535E76, SetDefenseTabCommandClass_Execute_Power, 6)
{
	GET(BuildingClass*, pBuild, EAX);
	R->EAX(pBuild->FindFactory(false, true));
	return 0x535E82;
}

DEFINE_OVERRIDE_HOOK(0x4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 7)
{
	GET_STACK(SHPStruct*, pBackground, 0xAC);
	if (pBackground)
	{
		GameDelete<true, false>(pBackground);
		pBackground = nullptr;
	}

	LEA_STACK(SHPStruct**, pSwipeAnims, 0x290);

	for (auto i = 0; i < 4; ++i)
	{
		if (auto pAnim = pSwipeAnims[i])
		{
			GameDelete<true, false>(pAnim);
			pSwipeAnims[i] = nullptr;
		}
	}

	return 0x4B9445;
}

/* fixes to reorder the savegame */
DEFINE_OVERRIDE_HOOK(0x67D315, SaveGame_EarlySaveSides, 5)
{
	GET(LPSTREAM, pStm, ESI);
	return (Game::Save_Sides(pStm, SideClass::Array) >= 0)
		? 0
		: 0x67E0B8
		;
}

DEFINE_OVERRIDE_HOOK(0x67E09A, SaveGame_LateSkipSides, 5)
{
	GET(int, success, EAX);
	return success >= 0
		? 0x67E0C2
		: 0x67E0B8
		;
}

DEFINE_OVERRIDE_HOOK(0x67E74A, LoadGame_EarlyLoadSides, 5)
{
	GET(LPSTREAM, pStm, ESI);

	int length = 0;
	LPVOID out;
	if (pStm->Read(&length, 4, 0) < 0)
	{
		return 0x67F7A3;
	}
	for (int i = 0; i < length; ++i)
	{
		if ((Imports::OleLoadFromStream.get()(pStm, &IIDs::IUnknown, &out)) < 0)
		{
			return 0x67F7A3;
		}
	}

	return 0;
}

DEFINE_DISABLE_HOOK(0x67F281, LoadGame_LateSkipSides_ares) //, 7, 67F2BF)
DEFINE_JUMP(LJMP, 0x67F281, 0x67F2BF);

// fix for ultra-fast processors overrunning the performance evaluator function
DEFINE_OVERRIDE_HOOK(0x5CB0B1, Game_QueryPerformance, 5)
{
	if (!R->EAX())
	{
		R->EAX(1);
	}

	return 0;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
DEFINE_OVERRIDE_HOOK(0x66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x657CF2, MapClass_MinimapChanged_Lock, 6)
DEFINE_OVERRIDE_HOOK(0x657D3D, MapClass_MinimapChanged_Lock, 6)
{
	RadarClass::RadarEvenSurface->Lock();
	RadarClass::RadarEvenSurface_B->Lock();
	return 0;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x657D35, MapClass_MinimapChanged_Unlock, 7)
DEFINE_OVERRIDE_HOOK(0x657D8A, MapClass_MinimapChanged_Unlock, 7)
{
	RadarClass::RadarEvenSurface->Unlock();
	RadarClass::RadarEvenSurface_B->Unlock();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x65731F, RadarClass_UpdateMinimap_Lock, 6)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_121C->Lock();
	pRadar->unknown_1220->Lock();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x65757C, RadarClass_UpdateMinimap_Unlock, 8)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_1220->Unlock();
	pRadar->unknown_121C->Unlock();

	return R->EAX() ? 0x657584 : 0x6576A5;
}

DEFINE_OVERRIDE_HOOK(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

DEFINE_OVERRIDE_HOOK(0x48248D, CellClass_CrateBeingCollected_MoneyRandom, 6)
{
	GET(int, nCur, EAX);

	const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;

	if (nAdd > 0)
		nCur += ScenarioClass::Instance->Random.RandomFromMax(nAdd);

	R->EDI(nCur);
	return 0x4824A7;
}

DEFINE_OVERRIDE_HOOK(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);

	const auto pTechnoType = pThis->GetTechnoType();
	const bool bAnimAttached = pTechnoType ? pThis->Parachute != 0 : pThis->HasParachute;

	int nFallRate = 1;
	int nMaxFallRate = bAnimAttached ? RulesClass::Instance->ParachuteMaxFallRate : RulesClass::Instance->NoParachuteMaxFallRate;

	if (pTechnoType)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
		nFallRate = (!bAnimAttached ? pExt->FallRate_NoParachute : pExt->FallRate_Parachute).Get();
		auto& nCustomMaxFallRate = (!bAnimAttached ? pExt->FallRate_NoParachuteMax : pExt->FallRate_ParachuteMax);

		if (nCustomMaxFallRate.isset())
			nMaxFallRate = nCustomMaxFallRate;
	}

	if (pThis->FallRate - nFallRate >= nMaxFallRate)
		nMaxFallRate = pThis->FallRate - nFallRate;

	pThis->FallRate = nMaxFallRate;
	return 0x5F3FFD;
}

DEFINE_OVERRIDE_HOOK(0x481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || TechnoExtContainer::Instance.Find(Unit)->AE_Cloak)
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExtContainer::Instance.Find(Unit->GetTechnoType())->CloakAllowed)
		? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
DEFINE_OVERRIDE_HOOK(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	TechnoExtContainer::Instance.Find(Unit)->AE_Cloak = true;
	TechnoExt_ExtData::RecalculateStat(Unit);
	return 0x482956;
}

DEFINE_OVERRIDE_HOOK(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult = Pow_ArmorMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x482E89;
	}
	return 0x482E92;
}

DEFINE_OVERRIDE_HOOK(0x48303A, CellClass_CrateBeingCollected_Speed2, 6)
{
	GET(FootClass*, Unit, EDI);
	GET_STACK(double, Pow_SpeedMultiplier, 0x20);

	// removed aircraft check
	// these originally not allow those to gain speed mult

	if (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult = Pow_SpeedMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->CL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

DEFINE_OVERRIDE_HOOK(0x483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult = Pow_FirepowerMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483258;
	}
	return 0x483261;
}

// temporal per-slot
DEFINE_OVERRIDE_HOOK(0x71A84E, TemporalClass_UpdateA, 5)
{
	GET(TemporalClass* const, pThis, ESI);

	// it's not guaranteed that there is a target
	if (auto const pTarget = pThis->Target)
	{
		TechnoExtContainer::Instance.Find(pTarget)->RadarJammer.reset(nullptr);
		AresAE::UpdateTempoal(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);
	}

	pThis->WarpRemaining -= pThis->GetWarpPerStep(0);

	R->EAX(pThis->WarpRemaining);
	return 0x71A88D;
}

DEFINE_OVERRIDE_HOOK(0x413FD2, AircraftClass_Init_Academy, 6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Owner)
	{
		if (pThis->Type->Trainable && HouseExtContainer::Instance.Find(pThis->Owner)->Is_AirfieldSpied)
			pThis->Veterancy.Veterancy = 1.0f;

		HouseExtData::ApplyAcademy(pThis->Owner, pThis, AbstractType::Aircraft);
	}

	return 0;
}

// issue #279: per unit AirstrikeAttackVoice and AirstrikeAbortSound
DEFINE_OVERRIDE_HOOK(0x41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
{
	GET(AirstrikeClass*, pAirstrike, EDI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAttackVoice;

	// get from aircraft
	const auto pAircraftExt = TechnoTypeExtContainer::Instance.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAttack.isset())
		index = pAircraftExt->VoiceAirstrikeAttack.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());

		if (pOwnerExt->VoiceAirstrikeAttack.isset())
			index = pOwnerExt->VoiceAirstrikeAttack.Get();
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D970;
}

DEFINE_OVERRIDE_HOOK(0x41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
{
	GET(AirstrikeClass*, pAirstrike, ESI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAbortSound;

	// get from aircraft
	const auto pAircraftExt = TechnoTypeExtContainer::Instance.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAbort.isset())
		index = pAircraftExt->VoiceAirstrikeAbort.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());
		if (pOwnerExt->VoiceAirstrikeAbort.isset())
			index = pOwnerExt->VoiceAirstrikeAbort.Get();
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

DEFINE_OVERRIDE_HOOK(0x4A8FF5, MapClass_CanBuildingTypeBePlacedHere_Ignore, 5)
{
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->IsFromSW ? 0x4A8FFA : 0x0;
}


//DEFINE_SKIP_HOOK(0x71B09C, TemporalClass_Logic_BuildingUnderAttack_NullptrShit, 0x5, 71B0E7);
DEFINE_JUMP(LJMP, 0x71B09C, 0x71B0E7);

DEFINE_OVERRIDE_HOOK(0x6BED08, Game_Terminate_Mouse, 7)
{
	GameDelete<false, false>(R->ECX<void*>());
	return 0x6BED34;
}

DEFINE_DISABLE_HOOK(0x56017A, OptionsDlg_WndProc_RemoveResLimit_ares)//, 0x5, 560183)
DEFINE_JUMP(LJMP, 0x56017A, 0x560183);

DEFINE_DISABLE_HOOK(0x5601E3, OptionsDlg_WndProc_RemoveHiResCheck_ares)//, 0x9, 5601FC)
DEFINE_JUMP(LJMP, 0x5601E3, 0x5601FC);

DEFINE_OVERRIDE_HOOK(0x621B80, DSurface_FillRectWithColor, 5)
{
	GET(RectangleStruct*, rect, ECX);
	GET(Surface*, surface, EDX);

	int surfaceWidth = surface->Get_Width();
	int surfaceHeight = surface->Get_Height();

	//make sure the rectangle to fill is within the surface's boundaries, this should do the trick
	rect->X = (rect->X >= 0) ? rect->X : 0;
	rect->Y = (rect->Y >= 0) ? rect->Y : 0;
	rect->Width = (rect->X + rect->Width <= surfaceWidth) ? rect->Width : surfaceWidth - rect->X;
	rect->Height = (rect->Y + rect->Height <= surfaceHeight) ? rect->Height : surfaceHeight - rect->Y;

	if (rect->Width == 0 || rect->Height == 0)
		return 0x621D26;
	else
		return 0;
}

DEFINE_OVERRIDE_HOOK(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass*, Target, ESI);
	return (Target && Target->Owner->IsControlledByHuman() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

DEFINE_OVERRIDE_HOOK(0x52E9AA, Frontend_WndProc_Checksum, 5)
{
	if (SessionClass::Instance->GameMode == GameMode::LAN || SessionClass::Instance->GameMode == GameMode::Internet)
	{
		auto nHashes = HashData::GetINIChecksums();
		Debug::Log("Rules checksum: %08X\n", nHashes.Rules);
		Debug::Log("Art checksum: %08X\n", nHashes.Art);
		Debug::Log("AI checksum: %08X\n", nHashes.AI);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x480534, CellClass_AttachesToNeighbourOverlay, 5)
{
	GET(int, idxOverlay, EAX);
	const bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->Items[idxOverlay]->Wall;
	return Wall ? 0x480549 : 0x480552;
}

DEFINE_OVERRIDE_HOOK(0x4A76ED, DiskLaserClass_Update_Anim, 7)
{
	GET(DiskLaserClass* const, pThis, ESI);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x54, 0x1C));

	auto const pWarhead = pThis->Weapon->Warhead;

	if (RulesExtData::Instance()->DiskLaserAnimEnabled)
	{
		auto const pType = MapClass::SelectDamageAnimation(
			pThis->Damage, pWarhead, LandType::Clear, coords);

		if (pType)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, coords),
				pThis->Owner ? pThis->Owner->Owner : nullptr,
				pThis->Target ? pThis->Target->Owner : nullptr,
				false,
				pThis->Owner
			);
		}
	}

	MapClass::FlashbangWarheadAt(pThis->Damage, pWarhead, coords);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x48a4f9, SelectDamageAnimation_FixNegatives, 6)
{
	GET(int, Damage, EDI);
	Damage = abs(Damage);
	R->EDI(Damage);
	return Damage ? 0x48A4FF : 0x48A618;
}

//InitGame_Delay
DEFINE_JUMP(LJMP, 0x52CA37, 0x52CA65)

//Wrong register ?
// game crash here , ugh
DEFINE_OVERRIDE_HOOK(0x5D6F61, MPGameModeClass_CreateStartingUnits_BaseCenter, 8)
{
	GET(MPGameModeClass*, pMode, ECX);
	GET(HouseClass*, pHouse, ESI);
	GET(int*, AmountToSpend, EAX);

	*AmountToSpend = R->EBP<int>();
	CellStruct nBase = pHouse->BaseSpawnCell;

	if (!pMode->SpawnBaseUnits(pHouse, AmountToSpend))
		return 0x5D701B;

	pHouse->ConYards.for_each([](BuildingClass* pConyards)
 {
	 pConyards->QueueMission(Mission::Construction, true);
	 ++Unsorted::ScenarioInit();
	 pConyards->EnterIdleMode(false, 1);
	 --Unsorted::ScenarioInit();
	});

	if (!nBase.IsValid())
		pHouse->BaseSpawnCell = nBase;

	return 0x5D6F77;
}

DEFINE_OVERRIDE_HOOK(0x5f5add, ObjectClass_SpawnParachuted_Animation, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (const auto pTechno = generic_cast<TechnoClass*>(pThis))
	{
		auto pType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->IsBomb)
			pThis->IsABomb = true;

		R->EDX(pTypeExt->ParachuteAnim ? pTypeExt->ParachuteAnim : HouseExtData::GetParachuteAnim(pTechno->Owner));
		return 0x5F5AE3;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x5d7337, MPGameMode_SpawnStartingUnits_NoInfantry, 5)
{
	return R->Stack<int>(0x28) ? 0x0 : 0x5D734F;
}

DEFINE_OVERRIDE_HOOK(0x5D705E, MPGameMode_SpawnBaseUnit_BaseUnit, 6)
{
	enum { hasBaseUnit = 0x5D7064, hasNoBaseUnit = 0x5D70DB };

	GET(HouseClass*, pHouse, EDI);
	GET(UnitTypeClass*, pBaseUnit, EAX);
	R->ESI(pBaseUnit);

	if (pBaseUnit)
		return hasBaseUnit;

	Debug::Log(__FUNCTION__" House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return hasNoBaseUnit;
}

DEFINE_OVERRIDE_HOOK(0x4C850B, Exception_Dialog, 5)
{
	Debug::FreeMouse();
	return 0;
}

DEFINE_DISABLE_HOOK(0x4E3792, HTExt_Unlimit1_ares)//, 0){ return 0x4E37AD; }
DEFINE_JUMP(LJMP, 0x4E3792, 0x4E37AD);

DEFINE_DISABLE_HOOK(0x4E3A9C, HTExt_Unlimit2_ares)//, 0){ return 0x4E3AA1; }
DEFINE_JUMP(LJMP, 0x4E3A9C, 0x4E3AA1);

DEFINE_DISABLE_HOOK(0x4E3F31, HTExt_Unlimit3_ares)//, 0){ return 0x4E3F4C; }
DEFINE_JUMP(LJMP, 0x4E3F31, 0x4E3F4C);

DEFINE_DISABLE_HOOK(0x4E412C, HTExt_Unlimit4_ares)//, 0){ return 0x4E4147; }
DEFINE_JUMP(LJMP, 0x4E412C, 0x4E4147);

DEFINE_DISABLE_HOOK(0x4E41A7, HTExt_Unlimit5_ares)//, 0){ return 0x4E41C3; }
DEFINE_JUMP(LJMP, 0x4E41A7, 0x4E41C3);

DEFINE_OVERRIDE_HOOK(0x5d7048, MPGameMode_SpawnBaseUnit_BuildConst, 5)
{
	GET_STACK(HouseClass*, pHouse, 0x18);

	auto pHouseType = pHouse->Type;

	if (!HouseTypeExtContainer::Instance.Find(pHouseType)->StartInMultiplayer_WithConst)
		return 0;

	const auto v7 = HouseExtData::FindBuildable(
		pHouse,
		pHouseType->FindParentCountryIndex(),
		make_iterator(RulesClass::Instance->BuildConst),
		0
	);

	if (!v7)
	{
		Debug::Log(__FUNCTION__" House of country [%s] cannot build anything from [General]BuildConst=.\n", pHouse->Type->ID);
		return 0x5D70DB;
	}

	const auto pBld = (BuildingClass*)v7->CreateObject(pHouse);

	if (!pBld)
		return 0x5D70DB;

	pBld->ForceMission(Mission::Guard);

	if (v7->GetFoundationWidth() > 2 || v7->GetFoundationHeight(0) > 2)
	{
		--pHouse->BaseSpawnCell.X;
		--pHouse->BaseSpawnCell.Y;
	}

	if (!pHouse->IsControlledByHuman())
	{
		pHouse->Func_505180();
		CellStruct base = pHouse->GetBaseCenter();

		pHouse->Base.Center = base;
		pHouse->Base.BaseNodes.Items->MapCoords = base;
		pHouse->Production = 1;
		pHouse->AITriggersActive = true;
	}

	R->EAX(pBld);
	R->EDI(pHouse);
	return 0x5D707E;
}

DEFINE_DISABLE_HOOK(0x6BD7E3, Expand_MIX_Reorg_ares)
DEFINE_STRONG_HOOK(0x6BD7D5, Expand_MIX_Reorg, 7)
{
	MixFileClass::Bootstrap();
	R->EAX(YRMemory::Allocate(sizeof(MixFileClass)));
	return 0x6BD7DF;
}


DEFINE_DISABLE_HOOK(0x52bb64, Expand_MIX_Deorg_ares)
DEFINE_JUMP(LJMP, 0x52BB64, 0x52BB95) //Expand_MIX_Deorg

DEFINE_OVERRIDE_HOOK(0x5FDDA4, IsOverlayIdxTiberium_Log, 6)
{
	GET(OverlayTypeClass*, pThis, EAX);
	Debug::Log(*reinterpret_cast<const char**>(0x833490), pThis->ID);
	return 0x5FDDC1;
}

DEFINE_OVERRIDE_HOOK(0x7cd819, ExeRun, 5)
{
	Game::Savegame_Magic = AresGlobalData::InternalVersion;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x74fdc0, GetModuleVersion, 5)
{
	R->EAX("Ares r21.352.1218");
	return 0x74FEEF;
}

DEFINE_OVERRIDE_HOOK(0x74fae0, GetModuleInternalVersion, 5)
{
	R->EAX("1.001/Ares 3.0p1");
	return 0x74FC7B;
}

DEFINE_OVERRIDE_HOOK(0x732d47, TacticalClass_CollectSelectedIDs, 5)
{
	auto pNames = R->EBX<DynamicVectorClass<const char*>*>();

	auto Add = [pNames](TechnoTypeClass* pType)
		{
			const char* id = TechnoTypeExtContainer::Instance.Find(pType)->GetSelectionGroupID();

			if (pNames->none_of([id](const char* pID)
				{
					return !CRT::strcmpi(pID, id);
				}))
			{
				pNames->AddItem(id);
			}
		};

	bool useDeploy = RulesExtData::Instance()->TypeSelectUseDeploy;

	for (auto pObject : ObjectClass::CurrentObjects())
	{
		// add this object's id used for grouping
		if (TechnoTypeClass* pType = pObject->GetTechnoType())
		{
			Add(pType);

			// optionally do the same the original game does, but support the new grouping feature.
			if (useDeploy)
			{
				if (pType->DeploysInto)
				{
					Add(pType->DeploysInto);
				}
				if (pType->UndeploysInto)
				{
					Add(pType->UndeploysInto);
				}
			}
		}
	}

	R->EAX(pNames);
	return 0x732FD9;
}

DEFINE_OVERRIDE_HOOK(0x534f89, Game_ReloadNeutralMIX_NewLine, 5)
{
	Debug::Log("LOADED NEUTRAL.MIX\n");
	return 0x534F96;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x4ABD9D, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
DEFINE_OVERRIDE_HOOK_AGAIN(0x4ABE58, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
DEFINE_OVERRIDE_HOOK(0x4ABD6C, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

//this is still 0.A code , need check the new one ,..
DEFINE_OVERRIDE_HOOK(0x537BC0, Game_MakeScreenshot, 6)
{
	RECT Viewport = {};
	if (Imports::GetWindowRect.get()(Game::hWnd, &Viewport))
	{
		POINT TL = { Viewport.left, Viewport.top }, BR = { Viewport.right, Viewport.bottom };
		if (Imports::ClientToScreen.get()(Game::hWnd, &TL) && Imports::ClientToScreen.get()(Game::hWnd, &BR))
		{
			RectangleStruct ClipRect = { TL.x, TL.y, Viewport.right + 1, Viewport.bottom + 1 };

			DSurface* Surface = DSurface::Primary;

			int width = Surface->Get_Width();
			int height = Surface->Get_Height();

			size_t arrayLen = width * height;

			if (width < ClipRect.Width)
			{
				ClipRect.Width = width;
			}

			if (height < ClipRect.Height)
			{
				ClipRect.Height = height;
			}

			WWMouseClass::Instance->HideCursor();

			if (WORD* buffer = reinterpret_cast<WORD*>(Surface->Lock(0, 0)))
			{
				char fName[0x80];

				SYSTEMTIME time;
				GetLocalTime(&time);

				IMPL_SNPRNINTF(fName, sizeof(fName), "SCRN.%04u%02u%02u-%02u%02u%02u-%05u.BMP",
					time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

				CCFileClass ScreenShot { "\0" };
				ScreenShot.OpenEx(fName, FileAccessMode::Write);

#pragma pack(push, 1)
				struct bmpfile_full_header
				{
					unsigned char magic[2];
					DWORD filesz;
					WORD creator1;
					WORD creator2;
					DWORD bmp_offset;
					DWORD header_sz;
					DWORD width;
					DWORD height;
					WORD nplanes;
					WORD bitspp;
					DWORD compress_type;
					DWORD bmp_bytesz;
					DWORD hres;
					DWORD vres;
					DWORD ncolors;
					DWORD nimpcolors;
					DWORD R; //
					DWORD G; //
					DWORD B; //
				} h;
#pragma pack(pop)

				h.magic[0] = 'B';
				h.magic[1] = 'M';

				h.creator1 = h.creator2 = 0;

				h.header_sz = 40;
				h.width = width;
				h.height = -height; // magic! no need to reverse rows this way
				h.nplanes = 1;
				h.bitspp = 16;
				h.compress_type = BI_BITFIELDS;
				h.bmp_bytesz = arrayLen * 2;
				h.hres = 4000;
				h.vres = 4000;
				h.ncolors = h.nimpcolors = 0;

				h.R = 0xF800;
				h.G = 0x07E0;
				h.B = 0x001F; // look familiar?

				h.bmp_offset = sizeof(h);
				h.filesz = h.bmp_offset + h.bmp_bytesz;

				ScreenShot.WriteBytes(&h, sizeof(h));
				std::unique_ptr<WORD[]> pixelData(new WORD[arrayLen]);
				WORD* pixels = pixelData.get();
				int pitch = Surface->VideoSurfaceDescription->lPitch;
				for (int r = 0; r < height; ++r)
				{
					memcpy(pixels, reinterpret_cast<void*>(buffer), width * 2);
					pixels += width;
					buffer += pitch / 2; // /2 because buffer is a WORD * and pitch is in bytes
				}

				ScreenShot.WriteBytes(pixelData.get(), arrayLen * 2);
				ScreenShot.Close();
				Debug::Log("Wrote screenshot to file %s\n", fName);
				Surface->Unlock();
			}

			WWMouseClass::Instance->ShowCursor();
		}
	}

	return 0x537DC9;
}

DEFINE_OVERRIDE_HOOK(0x5d7163, MPGameMode_SpawnStartingUnits_Types, 8)
{
	LEA_STACK(DynamicVectorClass<TechnoTypeClass*>*, pInfVec, 0x18);
	LEA_STACK(DynamicVectorClass<TechnoTypeClass*>*, pUnitVec, 0x30);
	GET_STACK(HouseClass*, pHouse, 0x4C);

	const auto pTypeExt = HouseTypeExtContainer::Instance.Find(pHouse->Type);

	if (!pTypeExt->StartInMultiplayer_Types.HasValue())
		return !UnitTypeClass::Array->Count ? 0x5D721A : 0x5D716B; //restore overriden instruction

	if (pTypeExt->StartInMultiplayer_Types.empty())
		return 0x5D743E;

	for (auto& start : pTypeExt->StartInMultiplayer_Types)
	{
		const auto what = start->WhatAmI();

		if (what == UnitTypeClass::AbsID)
			pUnitVec->AddItem(start);
		else
			pInfVec->AddItem(start);
	}

	return 0x5D72AB;
}

//[01:25:38] SyringeDebugger::HandleException: Ares.dll [0x5d6d9a , MPGameModeClass_CreateStartingUnits_UnitCost , 6]
//StartInMultiplayerUnitCost

void FormulateTypeList(std::vector<TechnoTypeClass*>& types, TechnoTypeClass** items, int count, int houseidx)
{
	if (!count)
		return;

	const auto end = items + count;
	for (auto find = items; find != end; ++find)
	{
		if ((*find)->AllowedToStartInMultiplayer)
		{
			if ((*find)->InOwners(houseidx) && ((*find))->TechLevel <= Game::TechLevel())
			{
				types.push_back(*find);
			}
		}
	}
}

std::vector<TechnoTypeClass*> GetTypeList()
{
	DWORD avaibleHouses = 0u;
	std::vector<TechnoTypeClass*> types;
	types.reserve(InfantryTypeClass::Array->Count + UnitTypeClass::Array->Count);

	for (auto pHouse : *HouseClass::Array)
	{
		if (!pHouse->Type->MultiplayPassive)
		{

			const auto& data = HouseTypeExtContainer::Instance.Find(pHouse->Type)->StartInMultiplayer_Types;
			if (data.HasValue())
			{
				types.insert(types.end(), data.begin(), data.end());
			}
			else
			{
				avaibleHouses |= 1 << pHouse->Type->ArrayIndex;
			}
		}
	}

	FormulateTypeList(types, (TechnoTypeClass**)UnitTypeClass::Array->Items, UnitTypeClass::Array->Count, avaibleHouses);
	FormulateTypeList(types, (TechnoTypeClass**)InfantryTypeClass::Array->Items, InfantryTypeClass::Array->Count, avaibleHouses);

	//remove any `BaseUnit` included 
	//base unit given for free then ?
	auto Iter = std::remove_if(types.begin(), types.end(), [](TechnoTypeClass* pItem)
 {
	 for (int i = 0; i < RulesClass::Instance->BaseUnit.Count; ++i)
	 {
		 if (pItem == (RulesClass::Instance->BaseUnit.Items[i]))
		 {
			 return true;
		 }
	 }

	 return false;
	});

	//idk these part 
	//but lets put it here 
	//need someone to test this to make sure if the calculation were correct :s
	//-Otamaa
	if (Iter != types.end())
		types.erase(Iter, types.end());

	std::sort(types.begin(), types.end());
	types.erase(std::unique(types.begin(), types.end()), types.end());
	return types;
}

int GetTotalCost(const Nullable<int>& fixed)
{
	if (GameModeOptionsClass::Instance->UnitCount <= 0)
		return 0;

	int totalCost = 0;
	if (fixed.isset())
	{
		totalCost = fixed;
	}
	else
	{

		auto types = GetTypeList();
		int total_ = 0;

		for (auto& tech : types)
		{
			total_ += tech->GetCost();
		}

		const int what = !types.size() ? 1 : types.size();
		totalCost = (total_ + (what >> 1)) / what;
		Debug::Log("Unit cost of %d derived from %u units totalling %d credits.\n", totalCost, what, total_);
	}

	return totalCost * GameModeOptionsClass::Instance->UnitCount;
}

DEFINE_OVERRIDE_HOOK(0x5d6d9a, MPGameModeClass_CreateStartingUnits_UnitCost, 6)
{
	R->EBP(GetTotalCost(RulesExtData::Instance()->StartInMultiplayerUnitCost));
	return 0x5D6ED6;
}

UniqueGamePtrB<MixFileClass> aresMIX;

DEFINE_OVERRIDE_HOOK(0x53029e, Load_Bootstrap_AresMIX, 5)
{
	aresMIX.reset(GameCreate<MixFileClass>("ares.mix"));
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6BE9BD, Game_ProgramEnd_ClearResource, 6)
{
	aresMIX.reset(nullptr);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x531413, Game_Start, 5)
{
	int topActive = 500;

	DSurface::Hidden->DrawText_Old(L"Ares is active.", 10, topActive, COLOR_GREEN);
	DSurface::Hidden->DrawText_Old(L"Ares is © The Ares Contributors 2007 - 2021.", 10, 520, COLOR_GREEN);

	wchar_t wVersion[256];
	wsprintfW(wVersion, L"%hs", L"Ares version: 3.0p1");

	DSurface::Hidden->DrawText_Old(wVersion, 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x532017, DlgProc_MainMenu_Version, 5)
{
	GET(HWND, hWnd, ESI);

	// account for longer version numbers
	const int MinimumWidth = 168;

	RECT Rect;
	if (GetWindowRect(hWnd, &Rect))
	{
		int width = Rect.right - Rect.left;

		if (width < MinimumWidth)
		{
			// extend to the left by the difference
			Rect.left -= (MinimumWidth - width);

			// if moved out of screen, move right by this amount
			if (Rect.left < 0)
			{
				Rect.right += -Rect.left;
				Rect.left = 0;
			}

			MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, FALSE);
		}
	}

	return 0;
}

void LoadGlobalConfig()
{
	CCFileClass IniFile { "Ares.ini" };
	if (!IniFile.Exists() || !IniFile.Open(FileAccessMode::Read))
	{
		Debug::Log("Failed to Open file Ares.ini \n");
		return;
	}

	CCINIClass Ini {};
	Ini.ReadCCFile(&IniFile);

	if (Ini.ReadString("Graphics.Advanced", "DirectX.Force", Phobos::readDefval, Phobos::readBuffer))
	{
		if (IS_SAME_STR_(Phobos::readBuffer, "hardware"))
		{
			AresGlobalData::GFX_DX_Force = 0x01l; //HW
		}
		else if (IS_SAME_STR_(Phobos::readBuffer, "emulation"))
		{
			AresGlobalData::GFX_DX_Force = 0x02l; //EM
		}
	}

	if (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0))
	{
		AresGlobalData::GFX_DX_Force = 0;
	}
}

DWORD ProcessCRCStrings(const char* str, int strsize, DWORD initial = -1)
{
	auto bytes = reinterpret_cast<const BYTE*>(str);
	auto ret = ~initial;

	for (int i = strsize; i > 0; --i)
	{
		ret = Checksummer::Table[*bytes++ ^ static_cast<BYTE>(ret)] ^ (ret >> 8);
	}

	return ~ret;
}

void DoSomethingWithThe64Char(AresSafeChecksummer& crc, const char* str, int strsize)
{
	crc.Value = ProcessCRCStrings(str, strsize);
	int remain = 64 - strsize;

	if (remain != 0)
	{
		*crc.Bytes = 0;
		std::memcpy(crc.Bytes, &str[strsize], remain);
		crc.ByteIndex = remain;
	}

	AresSafeChecksummer::Process(crc.Bytes, strsize, crc.Value);
}


DEFINE_OVERRIDE_HOOK(0x5facdf, Options_LoadFromINI, 5)
{
	Phobos::Config::Read();
	return 0x0;
}

#include <Misc/Spawner/Main.h>

DEFINE_OVERRIDE_HOOK(0x6BC0CD, _LoadRA2MD, 5)
{
	LoadGlobalConfig();
	SpawnerMain::LoadConfigurations();
	SpawnerMain::ApplyStaticOptions();

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x69B97D, Game_ProcessRandomPlayers_ObserverColor, 7)
{
	GET(NodeNameType* const, pStartingSpot, ESI);

	// observer uses last color, beyond the actual colors
	pStartingSpot->Color = AresGlobalData::colorCount;

	return 0x69B984;
}

DEFINE_OVERRIDE_HOOK(0x69B949, Game_ProcessRandomPlayers_ColorsA, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B95E;
}

DEFINE_OVERRIDE_HOOK(0x69BA13, Game_ProcessRandomPlayers_ColorsB, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69BA28;
}

DEFINE_OVERRIDE_HOOK(0x69B69B, GameModeClass_PickRandomColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B6AF;
}

DEFINE_OVERRIDE_HOOK(0x69B7FF, Session_SetColor_Unlimited, 6)
{
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, AresGlobalData::colorCount - 1));
	return 0x69B813;
}

DEFINE_OVERRIDE_HOOK(0x60FAD7, Ownerdraw_PostProcessColors, 0xA)
{
	// copy original instruction
	*reinterpret_cast<int*>(0xAC1B90) = 0x443716;

	// update colors
	*reinterpret_cast<int*>(0xAC18A4) = AresGlobalData::uiColorText;
	*reinterpret_cast<int*>(0xAC184C) = AresGlobalData::uiColorCaret;
	*reinterpret_cast<int*>(0xAC4604) = AresGlobalData::uiColorSelection;
	*reinterpret_cast<int*>(0xAC1B98) = AresGlobalData::uiColorBorder1;
	*reinterpret_cast<int*>(0xAC1B94) = AresGlobalData::uiColorBorder2;
	*reinterpret_cast<int*>(0xAC1AF8) = AresGlobalData::uiColorDisabledObserver;
	*reinterpret_cast<int*>(0xAC1CB0) = AresGlobalData::uiColorTextObserver;
	*reinterpret_cast<int*>(0xAC4880) = AresGlobalData::uiColorSelectionObserver;
	*reinterpret_cast<int*>(0xAC1CB4) = AresGlobalData::uiColorDisabled;

	// skip initialization
	//CommonDialogStuff_Color_Shifts_Set_PCXes_Loaded
	bool inited = *reinterpret_cast<bool*>(0xAC48D4);
	return inited ? 0x60FB5D : 0x60FAE3;
}

DEFINE_OVERRIDE_HOOK(0x612DA9, Handle_Button_Messages_Color, 6)
{
	R->EDI(AresGlobalData::uiColorTextButton);
	return 0x612DAF;
}

DEFINE_OVERRIDE_HOOK(0x613072, Handle_Button_Messages_DisabledColor, 7)
{
	R->EDI(AresGlobalData::uiColorDisabledButton);
	return 0x613138;
}

DEFINE_OVERRIDE_HOOK(0x61664C, Handle_Checkbox_Messages_Color, 5)
{
	R->EAX(AresGlobalData::uiColorTextCheckbox);
	return 0x616651;
}

DEFINE_OVERRIDE_HOOK(0x616655, Handle_Checkbox_Messages_Disabled, 5)
{
	R->EAX(AresGlobalData::uiColorDisabledCheckbox);
	return 0x61665A;
}

DEFINE_OVERRIDE_HOOK(0x616AF0, Handle_RadioButton_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextRadio);
	return 0x616AF6;
}

DEFINE_OVERRIDE_HOOK(0x615DF7, Handle_Static_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextLabel);
	return 0x615DFD;
}

DEFINE_OVERRIDE_HOOK(0x615AB7, Handle_Static_Messages_Disabled, 6)
{
	R->ECX(AresGlobalData::uiColorDisabledLabel);
	return 0x615ABD;
}

DEFINE_OVERRIDE_HOOK(0x619A4F, Handle_Listbox_Messages_Color, 6)
{
	R->ESI(AresGlobalData::uiColorTextList);
	return 0x619A55;
}

DEFINE_OVERRIDE_HOOK(0x6198D3, Handle_Listbox_Messages_DisabledA, 6)
{
	R->EBX(AresGlobalData::uiColorDisabledList);
	return 0x6198D9;
}

DEFINE_OVERRIDE_HOOK(0x619A5F, Handle_Listbox_Messages_DisabledB, 6)
{
	R->ESI(AresGlobalData::uiColorDisabledList);
	return 0x619A65;
}

DEFINE_OVERRIDE_HOOK(0x619270, Handle_Listbox_Messages_SelectionA, 5)
{
	R->EAX(AresGlobalData::uiColorSelectionList);
	return 0x619275;
}

DEFINE_OVERRIDE_HOOK(0x619288, Handle_Listbox_Messages_SelectionB, 6)
{
	R->DL(BYTE(AresGlobalData::uiColorSelectionList >> 16));
	return 0x61928E;
}

DEFINE_OVERRIDE_HOOK(0x617A2B, Handle_Combobox_Messages_Color, 6)
{
	R->EBX(AresGlobalData::uiColorTextCombobox);
	return 0x617A31;
}

DEFINE_OVERRIDE_HOOK(0x617A57, Handle_Combobox_Messages_Disabled, 6)
{
	R->EBX(AresGlobalData::uiColorDisabledCombobox);
	return 0x617A5D;
}

DEFINE_OVERRIDE_HOOK(0x60DDA6, Handle_Combobox_Dropdown_Messages_SelectionA, 5)
{
	R->EAX(AresGlobalData::uiColorSelectionCombobox);
	return 0x60DDAB;
}

DEFINE_OVERRIDE_HOOK(0x60DDB6, Handle_Combobox_Dropdown_Messages_SelectionB, 6)
{
	R->DL(BYTE(AresGlobalData::uiColorSelectionCombobox >> 16));
	return 0x60DDBC;
}

DEFINE_OVERRIDE_HOOK(0x61E2A5, Handle_Slider_Messages_Color, 5)
{
	R->EAX(AresGlobalData::uiColorTextSlider);
	return 0x61E2AA;
}

DEFINE_OVERRIDE_HOOK(0x61E2B1, Handle_Slider_Messages_Disabled, 5)
{
	R->EAX(AresGlobalData::uiColorDisabledSlider);
	return 0x61E2B6;
}

DEFINE_OVERRIDE_HOOK(0x61E8A0, Handle_GroupBox_Messages_Color, 6)
{
	R->ECX(AresGlobalData::uiColorTextGroupbox);
	return 0x61E8A6;
}

DEFINE_OVERRIDE_HOOK(0x614FF2, Handle_NewEdit_Messages_Color, 6)
{
	R->EDX(AresGlobalData::uiColorTextEdit);
	return 0x614FF8;
}

// reset the colors
DEFINE_OVERRIDE_HOOK(0x4E43C0, Game_InitDropdownColors, 5)
{
	// mark all colors as unused (+1 for the  observer)
	for (auto i = 0; i < AresGlobalData::colorCount + 1; ++i)
	{
		AresGlobalData::Colors[i].selectedIndex = -1;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x69A310, SessionClass_GetPlayerColorScheme, 7)
{
	GET_STACK(int const, idx, 0x4);
	GET_STACK(DWORD, caller, 0x0);

	int ret = 0;

	// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
	// What we want to do is to restore vanilla from Ares hook, and immediately return arg
	// So if spawner feeds us a number, it will be used to look up color scheme directly	
	// Original Author : Morton
	if (Phobos::UI::UnlimitedColor && idx != -2) {
		ret = idx << 1;

	} else {

		// get the slot
		AresGlobalData::ColorData* slot = nullptr;
		if (idx == -2 || idx == AresGlobalData::colorCount)
		{
			// observer color
			slot = &AresGlobalData::Colors[0];
		}
		else if (idx < AresGlobalData::colorCount)
		{
			// house color
			slot = &AresGlobalData::Colors[idx + 1];
		}

		// retrieve the color scheme index

		if (slot)
		{
			if (slot->colorSchemeIndex == -1) {

				slot->colorSchemeIndex = ColorScheme::FindIndex(slot->colorScheme);

				if (slot->colorSchemeIndex == -1) {
					Debug::Log("Color scheme \"%s\" not found.\n", slot->colorScheme);
					slot->colorSchemeIndex = 4;
				}
			}

			ret = slot->colorSchemeIndex;
		}
	}

	ret += 1;

	if ((size_t)ret >= (size_t)ColorScheme::Array->Count)
		Debug::FatalErrorAndExit("Address[%x] Trying To get Player Color[idx %d , %d(%d)] that more than ColorScheme Array Count [%d]!\n", caller, idx , ret, ret - 1, ColorScheme::Array->Count);

	R->EAX(ret);
	return 0x69A334;
}

// return the tool tip describing this color
DEFINE_OVERRIDE_HOOK(0x4E42A0, GameSetup_GetColorTooltip, 5)
{
	GET(int const, idxColor, ECX);

	if (idxColor == -2)
	{
		return 0x4E42A5;// random
	}
	else if (idxColor > AresGlobalData::colorCount)
	{
		return 0x4E43B7;
	}

	R->EAX(AresGlobalData::Colors[(idxColor + 1) % (AresGlobalData::colorCount + 1)].sttToolTipSublineText);
	return 0x4E43B9;
}

// handle adding colors to combo box
DEFINE_OVERRIDE_HOOK(0x4E46BB, hWnd_PopulateWithColors, 7)
{
	GET(HWND const, hWnd, ESI);
	GET_STACK(int const, idxPlayer, 0x14);

	// add all colors
	auto curSel = 0;
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto const& Color = AresGlobalData::Colors[i + 1];
		auto const isCurrent = Color.selectedIndex == idxPlayer;

		if (isCurrent || Color.selectedIndex == -1)
		{
			int idx = SendMessageA(hWnd, WW_CB_ADDITEM, 0, 0x822B78);
			SendMessageA(hWnd, WW_SETCOLOR, idx, Color.colorRGB);
			SendMessageA(hWnd, CB_SETITEMDATA, idx, i);

			if (isCurrent)
			{
				curSel = idx;
			}
		}
	}

	SendMessageA(hWnd, CB_SETCURSEL, curSel, 0);
	SendMessageA(hWnd, 0x4F1, 0, 0);

	return 0x4E4749;
}

// update the color in the combo drop-down lists
DEFINE_OVERRIDE_HOOK(0x4E4A41, hWnd_SetPlayerColor_A, 7)
{
	GET(int const, idxPlayer, EAX);

	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto& Color = AresGlobalData::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4A6D;
}

DEFINE_OVERRIDE_HOOK(0x4E4B47, hWnd_SetPlayerColor_B, 7)
{
	GET(int const, idxColor, EBP);
	GET(int const, idxPlayer, ESI);

	AresGlobalData::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4B4E;
}

DEFINE_OVERRIDE_HOOK(0x4E4556, hWnd_GetSlotColorIndex, 7)
{
	GET(int const, idxPlayer, ECX);

	auto ret = -1;
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto const& Color = AresGlobalData::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			ret = i + 1;
			break;
		}
	}

	R->EAX(ret);
	return 0x4E4570;
}

DEFINE_OVERRIDE_HOOK(0x4E4580, hWnd_IsAvailableColor, 5)
{
	GET(int const, idxColor, ECX);
	R->AL(AresGlobalData::Colors[idxColor + 1].selectedIndex == -1);
	return 0x4E4592;
}

DEFINE_OVERRIDE_HOOK(0x4E4C9D, hWnd_UpdatePlayerColors_A, 7)
{
	GET(int const, idxPlayer, EAX);

	// check players and reset used color for this player
	for (auto i = 0; i < AresGlobalData::colorCount; ++i)
	{
		auto& Color = AresGlobalData::Colors[i + 1];
		if (Color.selectedIndex == idxPlayer)
		{
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4CC9;
}

DEFINE_OVERRIDE_HOOK(0x4E4D67, hWnd_UpdatePlayerColors_B, 7)
{
	GET(int const, idxColor, EAX);
	GET(int const, idxPlayer, ESI);

	// reserve the color for a player. skip the observer
	AresGlobalData::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4D6E;
}

DEFINE_DISABLE_HOOK(0x69A310, SessionClass_GetPlayerColorScheme_ares)

DEFINE_OVERRIDE_HOOK(0x67D04E, Game_Save_SavegameInformation, 7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x4A4, 0x3F4));

	// remember the Ares version and a mod id
	Info.Version = AresGlobalData::version;
	Info.InternalVersion = AresGlobalData::InternalVersion + PHOBOSSAVEGAME_ID;
	CRT::sprintf(Info.ExecutableName.data(), "GAMEMD.EXE + Ares/3.0p1");

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x559F31, LoadOptionsClass_GetFileInfo, 9)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));

	// compare equal if same mod and same Ares version (or compatible)
	auto same = (Info.Version == (AresGlobalData::version)
		&& (Info.InternalVersion - PHOBOSSAVEGAME_ID ) == AresGlobalData::InternalVersion);

	R->ECX(&Info);
	return same ? 0x559F60u : 0x559F48u;
}

DEFINE_OVERRIDE_HOOK(0x67CEFE, Game_Save_FixLog, 7)
{
	GET(const char*, pFilename, EDI);
	GET(const wchar_t*, pSaveName, ESI);

	Debug::Log("\nSAVING GAME [%s - %ls]\n", pFilename, pSaveName);

	return 0x67CF0D;
}

DEFINE_DISABLE_HOOK(0x685659, Scenario_ClearClasses_ares)

DEFINE_OVERRIDE_HOOK(0x6d4b25, TacticalClass_Draw_TheDarkSideOfTheMoon, 6)
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

	if (RulesExtData::Instance()->FPSCounter)
	{
		wchar_t buffer[0x100];
		swprintf_s(buffer, L"FPS: %-4u Avg: %.2f", FPSCounter::CurrentFrameRate(), FPSCounter::GetAverageFrameRate());

		DrawText_Helper(buffer, offset, COLOR_WHITE);
	}

	return 0;
}

DEFINE_STRONG_OVERRIDE_HOOK(0x7C89D4, DDRAW_Create, 6)
{
	R->Stack<DWORD>(0x4, AresGlobalData::GFX_DX_Force);
	return 0;
}

DEFINE_STRONG_OVERRIDE_HOOK_AGAIN(0x4A4AC0, Debug_Log, 1)
DEFINE_STRONG_OVERRIDE_HOOK(0x4068E0, Debug_Log, 1)
{
	LEA_STACK(va_list const, args, 0x8);
	GET_STACK(const char* const, pFormat, 0x4);

	Debug::LogWithVArgs(pFormat, args);

	return 0x4A4AF9; // changed to co-op with YDE
}

#pragma region ErrorHandlings

DEFINE_STRONG_OVERRIDE_HOOK(0x64CCBF, DoList_ReplaceReconMessage, 6)
{
	// mimic an increment because decrement happens in the middle of function cleanup and can't be erased nicely
	++Unsorted::SystemResponseMessages;

	Debug::Log("Reconnection error detected!\n");
	if (MessageBoxW(Game::hWnd, L"Yuri's Revenge has detected a desynchronization!\n"
		L"Would you like to create a full error report for the developers?\n"
		L"Be advised that reports from at least two players are needed.", L"Reconnection Error!", MB_YESNO | MB_ICONERROR) == IDYES)
	{
		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		Debug::DumpStack(R, 8084);

		std::wstring path = Dialogs::PrepareSnapshotDirectory();

		if (Debug::LogEnabled)
		{
			Debug::Log("Copying debug log\n");
			std::wstring logCopy = path + L"\\debug.log";
			CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
		}

		Debug::Log("Making a memory snapshot\n");
		Debug::FullDump(std::move(path));

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);
		Debug::FatalError("A desynchronization has occurred.\r\n"
			"%s"
			"A crash dump should have been created in your game's \\debug subfolder.\r\n"
			"Please submit that to the developers along with SYNC*.txt, debug.txt and syringe.log."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
		);
	}

	return 0x64CD11;
}

#pragma warning(push)
#pragma warning(disable: 4646) // this function does not return, though it isn't declared VOID
#pragma warning(disable: 4477)
#pragma warning(disable: 4715)
#define EXCEPTION_STACK_COLUMNS 8 // Number of columns in stack dump.
#define EXCEPTION_STACK_DEPTH_MAX 1024

[[noreturn]] LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS const pExs)
{
	Debug::FreeMouse();
	Debug::Log("Exception handler fired!\n");
	Debug::Log("Exception %X at %p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
	SetWindowTextW(Game::hWnd, L"Fatal Error - Yuri's Revenge");

	switch (pExs->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	case EXCEPTION_INVALID_DISPOSITION:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_SINGLE_STEP:
	case EXCEPTION_STACK_OVERFLOW:
	case 0xE06D7363: // exception thrown and not caught
	{
		std::wstring path = Dialogs::PrepareSnapshotDirectory();

		if (Debug::LogEnabled)
		{
			std::wstring logCopy = path + L"\\debug.log";
			CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
		}

		std::wstring except_file = path + L"\\except.txt";

		if (FILE* except = _wfsopen(except_file.c_str(), L"w", _SH_DENYNO))
		{
			constexpr auto const pDelim = "------------------------------------------------------------------------------------\n";
			fprintf(except, "Internal Error encountered!\n");
			fprintf(except, pDelim);
			fprintf(except, "Ares version: 21.352.1218 With Phobos %s", PRODUCT_VERSION); //TODO
			fprintf(except, "\n");
			fprintf(except, pDelim);

			fprintf(except, "\n");

			int i = 0;
			for (auto const& [str, data] : Patch::ModuleDatas)
			{
				fprintf(except, "Module [(%d) %s: Base address = %p]\n", i++, str.c_str(), data.BaseAddr);
			}

			fprintf(except, "\n");
			switch (pExs->ExceptionRecord->ExceptionCode)
			{
			case EXCEPTION_STACK_OVERFLOW:
				fprintf(except, "Exception is stack overflow! (0x%08X) at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			case EXCEPTION_ACCESS_VIOLATION:
			{
				std::string VioType;
				switch (pExs->ExceptionRecord->ExceptionInformation[0])
				{
				case 0: // Read violation
					VioType = ("Access address: 0x%08X was read from.\n");
					break;
				case 1: // Write violation
					VioType = ("Access address: 0x%08X was written to.\n");
					break;
				case 2: // Execute violation
					VioType = ("Access address: 0x%08X was written to.\n");
					break;
				case 8: // User-mode data execution prevention (DEP).
					VioType = ("Access address: 0x%08X DEP violation.\n");
					break;
				default: // Unknown
					VioType = ("Access address: 0x%08X Unknown violation.\n");
					break;
				};
				std::string type = "Exception is access violation (0x%08X) at %08p ";
				type += VioType;
				fprintf(except, type.c_str(), pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress, pExs->ExceptionRecord->ExceptionInformation[1]);
			}
			break;
			case EXCEPTION_IN_PAGE_ERROR:
				fprintf(except, "Exception is page fault (0x%08X) at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			default:
				fprintf(except, "Exception code is 0x%08X at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			};

			PCONTEXT pCtxt = pExs->ContextRecord;
			fprintf(except, "Bytes at CS:EIP (0x%08X)  : ", pCtxt->Eip);
			uint8_t* eip_pointer = reinterpret_cast<uint8_t*>(pCtxt->Eip);

			for (int i = 32; i > 0; --i)
			{
				if (IsBadReadPtr(eip_pointer, sizeof(uint8_t)))
				{
					fprintf(except, "?? ");
				}
				else
				{
					fprintf(except, "%02X ", (uintptr_t)*eip_pointer);
				}
				++eip_pointer;
			}

			fprintf(except, "\n\nRegisters:\n");
			fprintf(except, "EIP: %08X\tESP: %08X\tEBP: %08X\t\n", pCtxt->Eip, pCtxt->Esp, pCtxt->Ebp);
			fprintf(except, "EAX: %08X\tEBX: %08X\tECX: %08X\n", pCtxt->Eax, pCtxt->Ebx, pCtxt->Ecx);
			fprintf(except, "EDX: %08X\tESI: %08X\tEDI: %08X\n", pCtxt->Edx, pCtxt->Esi, pCtxt->Edi);
			fprintf(except, "CS:  %04x\tSS:  %04x\tDS:  %04x\n", pCtxt->SegCs, pCtxt->SegSs, pCtxt->SegDs);
			fprintf(except, "ES:  %04x\tFS:  %04x\tGS:  %04x\n", pCtxt->SegEs, pCtxt->SegFs, pCtxt->SegGs);
			fprintf(except, "\n");

			fprintf(except, "EFlags: %08X\n", pCtxt->EFlags);

			fprintf(except, "\n");

			fprintf(except, "Floating point status:\n");
			fprintf(except, "Control word:\t%08x\n", pCtxt->FloatSave.ControlWord);
			fprintf(except, "Status word:\t%08x\n", pCtxt->FloatSave.StatusWord);
			fprintf(except, "Tag word:\t%08x\n", pCtxt->FloatSave.TagWord);
			fprintf(except, "Error Offset:\t%08x\n", pCtxt->FloatSave.ErrorOffset);
			fprintf(except, "Error Selector:\t%08x\n", pCtxt->FloatSave.ErrorSelector);
			fprintf(except, "Data Offset:\t%08x\n", pCtxt->FloatSave.DataOffset);
			fprintf(except, "Data Selector:\t%08x\n", pCtxt->FloatSave.DataSelector);
			fprintf(except, "Cr0NpxState:\t%08x\n", pCtxt->FloatSave.Spare0);

			fprintf(except, "\n");

			fprintf(except, "Floating point Registers:\n");

			for (int i = 0; i < EXCEPTION_STACK_COLUMNS; ++i)
			{
				fprintf(except, "ST%d : ", i);

				for (int j = 0; j < 10; ++j)
				{
					fprintf(except, "%02X", pCtxt->FloatSave.RegisterArea[i * 10 + j]);
				}

				fprintf(except, "   %+#.17e\n", *reinterpret_cast<double*>(&pCtxt->FloatSave.RegisterArea[i * 10]));
			}

			if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
			{
				fprintf(except, "\n");
				fprintf(except, "MMX Registers:\n");

				fprintf(except, "MMX0:	%016llX\tMMX1:	%016llX\tMMX2:	%016llX\tMMX3:	%016llX\n",
					pCtxt->ExtendedRegisters[0],
					pCtxt->ExtendedRegisters[1],
					pCtxt->ExtendedRegisters[2],
					pCtxt->ExtendedRegisters[3]
				);

				fprintf(except, "MMX4:	%016llX\tMMX5:	%016llX\tMMX6:	%016llX\tMMX7:	%016llX\n",
					pCtxt->ExtendedRegisters[4],
					pCtxt->ExtendedRegisters[5],
					pCtxt->ExtendedRegisters[6],
					pCtxt->ExtendedRegisters[7]
				);
			}

			fprintf(except, "\n");

			fprintf(except, "Debug Registers:\n");
			fprintf(except, "Dr0: %016llX\tDr1: %016llX\tDr2: %016llX\tDr3: %016llX\n",
				pCtxt->Dr0,
				pCtxt->Dr1,
				pCtxt->Dr2,
				pCtxt->Dr3
			);

			fprintf(except, "Dr4: OBSOLETE\tDr5: OBSOLETE\tDr6: %08X\tDr7: %08X\n",
				pCtxt->Dr6,
				pCtxt->Dr7
			);

			fprintf(except, "\nStack dump (depth : %d):\n", EXCEPTION_STACK_DEPTH_MAX);
			DWORD* ptr = reinterpret_cast<DWORD*>(pCtxt->Esp);
			for (int i = 0; i < EXCEPTION_STACK_DEPTH_MAX; ++i)
			{
				const char* suffix = "";
				if (*ptr >= 0x401000 && *ptr <= 0xB79BE4)
					suffix = "GameMemory!";

				fprintf(except, "%08p: %08X %s\n", ptr, *ptr, suffix);
				++ptr;
			}

			fclose(except);
			Debug::Log("Exception data has been saved to file:\n%ls\n", except_file.c_str());
		}

		if (MessageBoxW(Game::hWnd, L"Yuri's Revenge has encountered a fatal error!\nWould you like to create a full crash report for the developers?", L"Fatal Error!", MB_YESNO | MB_ICONERROR) == IDYES)
		{
			HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
			SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
			SetCursor(loadCursor);
			Debug::Log("Making a memory dump\n");

			MINIDUMP_EXCEPTION_INFORMATION expParam;
			expParam.ThreadId = GetCurrentThreadId();
			expParam.ExceptionPointers = pExs;
			expParam.ClientPointers = FALSE;

			Dialogs::FullDump(std::move(path), &expParam);

			loadCursor = LoadCursor(nullptr, IDC_ARROW);
			SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
			SetCursor(loadCursor);
			Debug::FatalError("The cause of this error could not be determined.\r\n"
				"%s"
				"A crash dump should have been created in your game's \\debug subfolder.\r\n"
				"You can submit that to the developers (along with debug.txt and syringe.log)."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
			);
		}
		break;
	}
	case ERROR_MOD_NOT_FOUND:
	case ERROR_PROC_NOT_FOUND:
		Debug::Log("Massive failure: Procedure or module not found!\n");
		break;
	default:
		Debug::Log("Massive failure: reason unknown, have fun figuring it out\n");
		Debug::DumpObj(reinterpret_cast<byte*>(pExs->ExceptionRecord), sizeof(*(pExs->ExceptionRecord)));
		//			return EXCEPTION_CONTINUE_SEARCH;
		break;
	}

	Debug::Log("Exiting...\n");
	Phobos::ExeTerminate();
	ExitProcess(pExs->ExceptionRecord->ExceptionCode);
};

DEFINE_STRONG_OVERRIDE_HOOK(0x4C8FE0, Exception_Handler, 9)
{
	//GET(int, code, ECX);
	GET(LPEXCEPTION_POINTERS, pExs, EDX);
	if (!Phobos::Otamaa::ExeTerminated)
	{ //dont fire exception twices ,..
//i dont know how handle recursive exception
		ExceptionHandler(pExs);
		__debugbreak();
	}
}

#pragma warning(pop)
template<typename T>
void WriteLog(const T* it, int idx, DWORD checksum, FILE* F)
{
	fprintf(F, "#%05d:\t%08X", idx, checksum);
}

template<>
void WriteLog(const AbstractClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);
	auto abs = it->WhatAmI();
	fprintf(F, "; Abs: %u (%s)", abs, AbstractClass::GetAbstractClassName(abs));
}

template<>
void WriteLog(const ObjectClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);

	const char* typeID = GameStrings::NoneStr();
	int typeIndex = -1;
	if (auto pType = it->GetType())
	{
		typeID = pType->ID;
		typeIndex = pType->GetArrayIndex();
	}

	CoordStruct crd = it->GetCoords();
	CellStruct cell = CellClass::Coord2Cell(crd);

	fprintf(F, "; Type: %d (%s); Coords: %d,%d,%d (%d,%d); Health: %d ; InLimbo: %u",
		typeIndex, typeID, crd.X, crd.Y, crd.Z, cell.X, cell.Y, it->Health, it->InLimbo);
}

template<>
void WriteLog(const MissionClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<ObjectClass>(it, idx, checksum, F);
	const auto Cur = it->GetCurrentMission();
	fprintf(F, "; Mission: %s (%d); StartTime: %d",
		MissionClass::MissionToString(Cur), Cur, it->CurrentMissionStartTime);
}

template<>
void WriteLog(const TechnoClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<MissionClass>(it, idx, checksum, F);

	const char* targetID = GameStrings::NoneStr();
	int targetIndex = -1;
	CoordStruct targetCrd = { -1, -1, -1 };
	if (auto pTarget = it->Target)
	{
		targetID = AbstractClass::GetAbstractClassName(pTarget->WhatAmI());
		targetIndex = pTarget->GetArrayIndex();
		targetCrd = pTarget->GetCoords();
	}

	fprintf(F, "; Facing: %d; Facing2: %d; Target: %s (%d; %d,%d)",
		it->PrimaryFacing.Current().Getvalue8(), it->SecondaryFacing.Current().Getvalue8(), targetID, targetIndex, targetCrd.X, targetCrd.Y);
}

template<>
void WriteLog(const FootClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);

	const char* destID = GameStrings::NoneStr();
	int destIndex = -1;
	CoordStruct destCrd = { -1, -1, -1 };
	if (auto pDest = it->Destination)
	{
		destID = AbstractClass::GetAbstractClassName(pDest->WhatAmI());
		destIndex = pDest->GetArrayIndex();
		destCrd = pDest->GetCoords();
	}

	fprintf(F, "; Destination: %s (%d; %d,%d); SpeedPercentage %d ; Height %d",
		destID, destIndex, destCrd.X, destCrd.Y
		, Game::F2I(it->SpeedPercentage * 256)
		, it->GetHeight()
	);
}

template<>
void WriteLog(const UnitClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);

	const auto& Loco = it->Locomotor;
	auto accum = Loco->Get_Speed_Accum();
	auto index = Loco->Get_Track_Index();
	auto number = Loco->Get_Track_Number();

	fprintf(F, "; SpeedAccum %d; TrackNumber: %d; TrackIndex: %d", accum, number, index);
}

template<>
void WriteLog(const InfantryClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const AircraftClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
}

//static std::array<std::string, 3> whos{
//	{
//		"GACNST" , "NACNST" , "YACNST"
//	}
//};
//
//static std::map <const BuildingClass*, std::vector<unsigned int>> CRCRecords {};

void PrintBld(const BuildingClass* pThis, FILE* stream)
{
	/*
	const auto iter = CRCRecords.find(pThis);

	if (iter == CRCRecords.end())
		return;

	for (int i = 0; i < (int)iter->second.size(); ++i) {
		fprintf(stream, "LasrRecorded CRCs [%d] [%08X]\n",i , iter->second.data() + i);
	}
	
	for (auto const& who : whos)
	{
		if (IS_SAME_STR_(pThis->Type->ID, who.c_str()))
		{
			auto printTimer = [=](const CDTimerClass& timer)
				{
					fprintf(stream, "%d %d\n", timer.TimeLeft, timer.StartTime);
				};

			//ObjectClass]
			fprintf(stream, "\n");
			fprintf(stream, "%d\n", pThis->unknown_24);
			fprintf(stream, "%d\n", pThis->unknown_28);
			fprintf(stream, "%d\n", pThis->FallRate);
			fprintf(stream, "%p\n", pThis->NextObject);
			//AmbientSoundController
			//CustomSoundController
			fprintf(stream, "%d\n", pThis->CustomSound);
			fprintf(stream, "%d\n", pThis->BombVisible);
			fprintf(stream, "%d\n", pThis->Health);
			fprintf(stream, "%d\n", pThis->EstimatedHealth);
			fprintf(stream, "%d\n", pThis->IsOnMap);
			fprintf(stream, "%d\n", pThis->NeedsRedraw);
			fprintf(stream, "%d\n", pThis->InLimbo);
			fprintf(stream, "%d\n", pThis->InOpenToppedTransport);
			fprintf(stream, "%d\n", pThis->IsSelected);

			fprintf(stream, "%p\n", pThis->Parachute);
			fprintf(stream, "%d\n", pThis->OnBridge);
			fprintf(stream, "%d\n", pThis->IsFallingDown);
			fprintf(stream, "%d\n", pThis->WasFallingDown);
			fprintf(stream, "%d\n", pThis->IsABomb);
			fprintf(stream, "%d\n", pThis->IsAlive);
			fprintf(stream, "%d\n", (int)pThis->LastLayer);
			fprintf(stream, "%d\n", pThis->IsInLogic);
			fprintf(stream, "%d\n", pThis->IsVisible);
			fprintf(stream, "%d %d %d\n", pThis->Location.X, pThis->Location.Y, pThis->Location.Z);
			fprintf(stream, "%p\n", pThis->LineTrailer);
			fprintf(stream, "%d\n", pThis->IsSelected);


			//MissionClass
			fprintf(stream, "%d\n", pThis->CurrentMission);
			fprintf(stream, "%d\n", pThis->SuspendedMission);
			fprintf(stream, "%d\n", pThis->QueuedMission);
			fprintf(stream, "%d\n", pThis->unknown_bool_B8);
			fprintf(stream, "%d\n", pThis->MissionStatus);
			fprintf(stream, "%d\n", pThis->CurrentMissionStartTime);
			fprintf(stream, "%d\n", pThis->unknown_C4);
			printTimer(pThis->UpdateTimer);


			//RadioClass
			fprintf(stream, "%d %d %d\n", (int)pThis->LastCommands[0], (int)pThis->LastCommands[1], (int)pThis->LastCommands[2]);
			fprintf(stream, "%d %d %d\n", pThis->RadioLinks.Capacity, pThis->RadioLinks.IsInitialized, pThis->RadioLinks.IsAllocated);

			fprintf(stream, "%d %d\n", pThis->Flashing.DurationRemaining, pThis->Flashing.FlashingNow);
			fprintf(stream, "%d %d %d\n", pThis->Animation.Step, pThis->Animation.Value, pThis->Animation.Timer.Duration); // how the unit animates
			fprintf(stream, "%d\n", pThis->Passengers.NumPassengers);
			fprintf(stream, "%p\n", pThis->Transporter); // unit carrying me
			fprintf(stream, "%d\n", pThis->__LastGuardAreaTargetingFrame_120);
			fprintf(stream, "%d\n", pThis->CurrentTurretNumber); // for IFV/gattling/charge turrets
			fprintf(stream, "%d\n", pThis->__TurretWeapon2_128);
			fprintf(stream, "%p\n", pThis->BehindAnim);
			fprintf(stream, "%p\n", pThis->DeployAnim);
			fprintf(stream, "%d\n", pThis->InAir);
			fprintf(stream, "%d\n", pThis->CurrentWeaponNumber); // for IFV/gattling 138
			fprintf(stream, "%d\n", pThis->CurrentRanking); // only used for promotion detection
			fprintf(stream, "%d\n", pThis->CurrentGattlingStage);
			fprintf(stream, "%d\n", pThis->GattlingValue); // sum of RateUps and RateDowns
			fprintf(stream, "%d\n", pThis->TurretAnimFrame);
			fprintf(stream, "%p\n", pThis->InitialOwner); // only set in ctor
			fprintf(stream, "%fl\n", pThis->Veterancy.Veterancy);
			fprintf(stream, "%d\n", pThis->align_154);
			fprintf(stream, "%fl\n", pThis->ArmorMultiplier);
			fprintf(stream, "%fl\n", pThis->FirepowerMultiplier);
			printTimer(pThis->IdleActionTimer); // MOO
			printTimer(pThis->RadarFlashTimer);
			printTimer(pThis->TargetingTimer); //Duration = 45 on init!
			printTimer(pThis->IronCurtainTimer);
			printTimer(pThis->IronTintTimer); // how often to alternate the effect color
			fprintf(stream, "%d\n", pThis->IronTintStage); // ^
			printTimer(pThis->AirstrikeTimer);
			printTimer(pThis->AirstrikeTintTimer); // tracks alternation of the effect color
			fprintf(stream, "%d\n", pThis->AirstrikeTintStage); //  ^
			fprintf(stream, "%d\n", pThis->ProtectType);	//0 or 1, NOT a bool - is this under ForceShield as opposed to IC?
			fprintf(stream, "%d\n", pThis->Deactivated); //Robot Tanks without power for instance
			fprintf(stream, "%p\n", pThis->DrainTarget); // eg Disk -> PowerPlant, this points to PowerPlant
			fprintf(stream, "%p\n", pThis->DrainingMe);  // eg Disk -> PowerPlant, this points to Disk
			fprintf(stream, "%p\n", pThis->DrainAnim);
			fprintf(stream, "%d\n", pThis->Disguised);
			fprintf(stream, "%d\n", pThis->DisguiseCreationFrame);
			printTimer(pThis->InfantryBlinkTimer); // Rules->InfantryBlinkDisguiseTime , detects mirage firing per description
			printTimer(pThis->DisguiseBlinkTimer); // disguise disruption timer
			fprintf(stream, "%d\n", pThis->UnlimboingInfantry); //1F8
			printTimer(pThis->ReloadTimer);
			fprintf(stream, "%d\n", pThis->unknown_208);
			fprintf(stream, "%d\n", pThis->unknown_20C);

			// WARNING! this is actually an index of HouseTypeClass es, but it's being changed to fix typical WW bugs.
			//DECLARE_PROPERTY(IndexBitfield<HouseClass*>, DisplayProductionTo); // each bit corresponds to one player on the map, telling us whether that player has (1) or hasn't (0) spied this building, and the game should display what's being produced inside it to that player. The bits are arranged by player ID, i.e. bit 0 refers to house #0 in HouseClass::Array, 1 to 1, etc.; query like ((1 << somePlayer->ArrayIndex) & someFactory->DisplayProductionToHouses) != 0

			fprintf(stream, "%d\n", pThis->Group);
			fprintf(stream, "%p\n", pThis->Focus);
			fprintf(stream, "%p\n", pThis->Owner);
			fprintf(stream, "%d\n", pThis->CloakState);
			//DECLARE_PROPERTY(StageClass, CloakProgress); // phase from [opaque] -> [fading] -> [transparent] , [General]CloakingStages= long
			printTimer(pThis->CloakDelayTimer); // delay before cloaking again
			fprintf(stream, "%fl\n", pThis->WarpFactor); // don't ask! set to 0 in CTOR, never modified, only used as ((this->Fetch_ID) + this->WarpFactor) % 400 for something in cloak ripple
			fprintf(stream, "%d\n", pThis->unknown_bool_250);
			//CoordStruct      LastSightCoords;
			fprintf(stream, "%d\n", pThis->LastSightRange);
			fprintf(stream, "%d\n", pThis->LastSightHeight);
			fprintf(stream, "%d\n", pThis->GapSuperCharged); // GapGenerator, when SuperGapRadiusInCells != GapRadiusInCells, you can deploy the gap to boost radius
			fprintf(stream, "%d\n", pThis->GeneratingGap); // is currently generating gap
			fprintf(stream, "%d\n", pThis->GapRadius);
			fprintf(stream, "%d\n", pThis->BeingWarpedOut); // is being warped by CLEG used , for 70C5B0
			fprintf(stream, "%d\n", pThis->WarpingOut); // phasing in after chrono-jump used , for 70C5C0
			fprintf(stream, "%d\n", pThis->unknown_bool_272);
			fprintf(stream, "%d\n", pThis->unused_273);
			fprintf(stream, "%p\n", pThis->TemporalImUsing); // CLEG attacking Power Plant : CLEG's this
			fprintf(stream, "%p\n", pThis->TemporalTargetingMe); 	// CLEG attacking Power Plant : PowerPlant's this
			fprintf(stream, "%d\n", pThis->IsImmobilized); // by chrono aftereffects ,27C
			fprintf(stream, "%d\n", pThis->unknown_280);
			fprintf(stream, "%d\n", pThis->ChronoLockRemaining); // 284 countdown after chronosphere warps things around
			//CoordStruct      ChronoDestCoords; // teleport loco and chsphere set this
			fprintf(stream, "%p\n", pThis->Airstrike); //Boris
			fprintf(stream, "%d\n", pThis->Berzerk);
			fprintf(stream, "%d\n", pThis->BerzerkDurationLeft);
			fprintf(stream, "%d\n", pThis->SprayOffsetIndex); // hardcoded array of xyz offsets for sprayattack, 0 - 7, see 6FE0AD
			fprintf(stream, "%d\n", pThis->Uncrushable); // DeployedCrushable fiddles this, otherwise all 0
			fprintf(stream, "%p\n", pThis->DirectRockerLinkedUnit);
			fprintf(stream, "%p\n", pThis->LocomotorTarget); // mag->LocoTarget = victim
			fprintf(stream, "%p\n", pThis->LocomotorSource); // victim->LocoSource = mag
			fprintf(stream, "%p\n", pThis->Target); //if attacking ,tarcom
			fprintf(stream, "%p\n", pThis->LastTarget); //suspendedtarcom
			fprintf(stream, "%p\n", pThis->CaptureManager); //for Yuris
			fprintf(stream, "%p\n", pThis->MindControlledBy);
			fprintf(stream, "%d\n", pThis->MindControlledByAUnit);
			fprintf(stream, "%p\n", pThis->MindControlRingAnim);
			fprintf(stream, "%p\n", pThis->MindControlledByHouse); //used for a TAction
			fprintf(stream, "%p\n", pThis->SpawnManager);
			fprintf(stream, "%p\n", pThis->SpawnOwner); // on DMISL , points to DRED and such
			fprintf(stream, "%p\n", pThis->SlaveManager);
			fprintf(stream, "%p\n", pThis->SlaveOwner); // on SLAV, points to YAREFN
			fprintf(stream, "%p\n", pThis->OriginallyOwnedByHouse); //used for mind control

			//units point to the Building bunkering them, building points to Foot contained within
			fprintf(stream, "%p\n", pThis->BunkerLinkedItem);

			fprintf(stream, "%fl\n", pThis->PitchAngle); // not exactly, and it doesn't affect the drawing, only internal state of a dropship
			printTimer(pThis->DiskLaserTimer);
			fprintf(stream, "%d\n", pThis->ROF);
			fprintf(stream, "%d\n", pThis->Ammo);
			fprintf(stream, "%d\n", pThis->Value); //,PurchasePrice set to actual cost when this gets queued in factory, updated only in building's 42C


			fprintf(stream, "%p\n", pThis->FireParticleSystem);
			fprintf(stream, "%p\n", pThis->SparkParticleSystem);
			fprintf(stream, "%p\n", pThis->NaturalParticleSystem);
			fprintf(stream, "%p\n", pThis->DamageParticleSystem);
			fprintf(stream, "%p\n", pThis->RailgunParticleSystem);
			fprintf(stream, "%p\n", pThis->unk1ParticleSystem);
			fprintf(stream, "%p\n", pThis->unk2ParticleSystem);
			fprintf(stream, "%p\n", pThis->FiringParticleSystem);

			fprintf(stream, "%p\n", pThis->Wave); //Beams


			// rocking effect
			fprintf(stream, "%fl\n", pThis->AngleRotatedSideways); // in this frame, in radians - if abs() exceeds pi/2, it dies
			fprintf(stream, "%fl\n", pThis->AngleRotatedForwards); // same

			// set these and leave the previous two alone!
			// if these are set, the unit will roll up to pi/4, by this step each frame, and balance back
			fprintf(stream, "%fl\n", pThis->RockingSidewaysPerFrame); // left to right - positive pushes left side up
			fprintf(stream, "%fl\n", pThis->RockingForwardsPerFrame); // back to front - positive pushes ass up

			fprintf(stream, "%d\n", pThis->HijackerInfantryType); // mutant hijacker

			//DECLARE_PROPERTY(StorageClass, Tiberium);
			fprintf(stream, "%d\n", pThis->unknown_34C);

			//DECLARE_PROPERTY(DoorClass, UnloadTimer); // times the deploy, unload, etc. cycles ,DoorClass

			//DECLARE_PROPERTY(FacingClass, BarrelFacing);
			//DECLARE_PROPERTY(FacingClass, PrimaryFacing); //Facing
			//DECLARE_PROPERTY(FacingClass, SecondaryFacing); // TurretFacing
			fprintf(stream, "%d\n", pThis->CurrentBurstIndex);
			printTimer(pThis->TargetLaserTimer);
			fprintf(stream, "%d\n", pThis->weapon_sound_randomnumber_3C8);
			fprintf(stream, "%d\n", pThis->__shipsink_3CA);
			fprintf(stream, "%d\n", pThis->CountedAsOwned); // is this techno contained in OwningPlayer->Owned... counts?
			fprintf(stream, "%d\n", pThis->IsSinking);
			fprintf(stream, "%d\n", pThis->WasSinkingAlready); // if(IsSinking && !WasSinkingAlready) { play SinkingSound; WasSinkingAlready = 1; }
			fprintf(stream, "%d\n", pThis->__ProtectMe_3CF);
			fprintf(stream, "%d\n", pThis->IsUseless); //3D0
			fprintf(stream, "%d\n", pThis->IsTickedOff); //HasBeenAttacked //3D1
			fprintf(stream, "%d\n", pThis->Cloakable); //3D2
			fprintf(stream, "%d\n", pThis->IsPrimaryFactory); //3D3 IsLoaner
			//BYTE			 IsALoaner; // 3D4
			//BYTE			 IsLocked; // 3D5
			fprintf(stream, "%d\n", pThis->Spawned); // 3D6
			fprintf(stream, "%d\n", pThis->IsInPlayfield); // 3D7
			//DECLARE_PROPERTY(RecoilData, TurretRecoil);
			//DECLARE_PROPERTY(RecoilData, BarrelRecoil);
			fprintf(stream, "%d\n", pThis->IsTethered); //418
			fprintf(stream, "%d\n", pThis->RADIO_26_27_419);
			fprintf(stream, "%d\n", pThis->IsOwnedByCurrentPlayer);
			fprintf(stream, "%d\n", pThis->DiscoveredByCurrentPlayer);
			fprintf(stream, "%d\n", pThis->DiscoveredByComputer);
			fprintf(stream, "%d\n", pThis->unknown_bool_41D);
			fprintf(stream, "%d\n", pThis->unknown_bool_41E);
			fprintf(stream, "%d\n", pThis->unknown_bool_41F);
			fprintf(stream, "%d\n", pThis->SightIncrease); // used for LeptonsPerSightIncrease
			fprintf(stream, "%d\n", pThis->RecruitableA); // these two are like Lenny and Carl, weird purpose and never seen separate
			fprintf(stream, "%d\n", pThis->RecruitableB); // they're usually set on preplaced objects in maps
			fprintf(stream, "%d\n", pThis->IsRadarTracked);
			fprintf(stream, "%d\n", pThis->IsOnCarryall);
			fprintf(stream, "%d\n", pThis->IsCrashing);
			fprintf(stream, "%d\n", pThis->WasCrashingAlready);
			fprintf(stream, "%d\n", pThis->IsBeingManipulated);
			fprintf(stream, "%p\n", pThis->BeingManipulatedBy); // set when something is being molested by a locomotor such as magnetron
			// the pointee will be marked as the killer of whatever the victim falls onto
			fprintf(stream, "%p\n", pThis->ChronoWarpedByHouse);
			fprintf(stream, "%d\n", pThis->_Mission_Patrol_430);
			fprintf(stream, "%d\n", pThis->IsMouseHovering);
			fprintf(stream, "%d\n", pThis->parasitecontrol_byte432);
			//	BYTE			 byte_433;
			fprintf(stream, "%p\n", pThis->OldTeam);
			fprintf(stream, "%d\n", pThis->CountedAsOwnedSpecial); // for absorbers, infantry uses this to manually control OwnedInfantry count
			fprintf(stream, "%d\n", pThis->Absorbed); // in UnitAbsorb/InfantryAbsorb or smth, lousy memory
			fprintf(stream, "%d\n", pThis->forceattackforcemovefirendlytarget_bool_43A);
			fprintf(stream, "%d\n", pThis->__RadialFireCounter_43C);
			//DECLARE_PROPERTY(DynamicVectorClass<int>, CurrentTargetThreatValues);
			//DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, CurrentTargets);

			// if DistributedFire=yes, this is used to determine which possible targets should be ignored in the latest threat scan
			//DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, AttackedTargets);

			//DECLARE_PROPERTY(AudioController, Audio3);

			fprintf(stream, "%d\n", pThis->__IsTurretTurning_49C); // Turret is moving?
			fprintf(stream, "%d\n", pThis->TurretIsRotating);

			//DECLARE_PROPERTY(AudioController, Audio4);

			fprintf(stream, "%d\n", pThis->GattlingAudioPlayed); //4B8
			fprintf(stream, "%d\n", pThis->unknown_4BC);

			//DECLARE_PROPERTY(AudioController, Audio5);

			fprintf(stream, "%d\n", pThis->gattlingsound_4D4);
			fprintf(stream, "%d\n", pThis->unknown_4D8);

			//DECLARE_PROPERTY(AudioController, Audio6);

			fprintf(stream, "%d\n", pThis->QueuedVoiceIndex);
			fprintf(stream, "%d\n", pThis->__LastVoicePlayed); //4F4
			fprintf(stream, "%d\n", pThis->deploy_bool_4F8);
			fprintf(stream, "%d\n", pThis->__creationframe_4FC);	//gets initialized with the current Frame, but this is NOT a CDTimerClass!
			fprintf(stream, "%p\n", pThis->LinkedBuilding); // 500 BuildingClass*
			fprintf(stream, "%d\n", pThis->EMPLockRemaining);
			fprintf(stream, "%d\n", pThis->ThreatPosed); // calculated to include cargo etc
			fprintf(stream, "%d\n", pThis->ShouldLoseTargetNow); //the rest is padded for sure
			fprintf(stream, "%p\n", pThis->FiringRadBeam);
			fprintf(stream, "%p\n", pThis->PlanningToken);
			fprintf(stream, "%p\n", pThis->Disguise);
			fprintf(stream, "%p\n", pThis->DisguisedAsHouse);


			//BuildingClass
			fprintf(stream, "%p\n", pThis->Type);
			fprintf(stream, "%p\n", pThis->Factory);
			printTimer(pThis->GoingToBlowTimer);  // used for warhead DelayKill and also C4
			fprintf(stream, "%d\n", pThis->BState);
			fprintf(stream, "%d\n", pThis->QueueBState);
			fprintf(stream, "%d\n", pThis->OwnerCountryIndex);
			fprintf(stream, "%p\n", pThis->C4AppliedBy);
			fprintf(stream, "%d\n", pThis->LastStrength); //544
			fprintf(stream, "%p\n", pThis->FirestormAnim); //pointer
			fprintf(stream, "%p\n", pThis->PsiWarnAnim); //pointer
			printTimer(pThis->PlacementDelay); //550

			//AnimClass* Anims[0x15];
			//bool AnimStates[0x15]; // one flag for each of the above anims (whether the anim was enabled when power went offline?)

			//PROTECTED_PROPERTY(BYTE, align_5C5[3]);

			//DWORD DamageFireAnims1; //0x5C8
			//AnimClass* DamageFireAnims[0x8];
			fprintf(stream, "%d\n", pThis->RequiresDamageFires); // if set, ::Update spawns damage fire anims and zeroes it

			//5E8 - 5F8 ????????
			//BuildingTypeClass* Upgrades[0x3];

			fprintf(stream, "%d\n", pThis->FiringSWType); // type # of sw being launched
			fprintf(stream, "%d\n", pThis->upgrade_5FC);
			fprintf(stream, "%p\n", pThis->Spotlight);
			//RateTimer GateTimer;
			fprintf(stream, "%p\n", pThis->LightSource); // tiled light , LightIntensity > 0
			fprintf(stream, "%d\n", pThis->LaserFenceFrame); // 0-7 for active directionals, 8/12 for offline ones, check ntfnce.shp or whatever
			fprintf(stream, "%d\n", pThis->FirestormWallFrame); // anim data for firestorm active animations
			//StageClass RepairProgress; // for hospital, armory, unitrepair etc
			fprintf(stream, "%d %d %d %d\n", pThis->unknown_rect_63C.X, pThis->unknown_rect_63C.Y, pThis->unknown_rect_63C.Width, pThis->unknown_rect_63C.Height);
			fprintf(stream, "%d %d %d\n", pThis->unknown_coord_64C.X, pThis->unknown_coord_64C.Y, pThis->unknown_coord_64C.Z);
			fprintf(stream, "%d\n", pThis->unknown_int_658);
			fprintf(stream, "%d\n", pThis->unknown_65C);
			fprintf(stream, "%d\n", pThis->HasPower);
			fprintf(stream, "%d\n", pThis->IsOverpowered);

			// each powered unit controller building gets this set on power activation and unset on power outage
			fprintf(stream, "%d\n", pThis->RegisteredAsPoweredUnitSource);

			fprintf(stream, "%d\n", pThis->SupportingPrisms);
			fprintf(stream, "%d\n", pThis->HasExtraPowerBonus);
			fprintf(stream, "%d\n", pThis->HasExtraPowerDrain);
			//DynamicVectorClass<InfantryClass*> Overpowerers;
			//DynamicVectorClass<InfantryClass*> Occupants;
			fprintf(stream, "%d\n", pThis->FiringOccupantIndex); // which occupant should get XP, which weapon should be fired (see 6FF074)

			//AudioController Audio7;
			//AudioController Audio8;

			fprintf(stream, "%d\n", pThis->WasOnline); // the the last state when Update()ing. if this changed since the last Update(), UpdatePowered is called.
			fprintf(stream, "%d\n", pThis->ShowRealName); // is also NOMINAL under [Structures]
			fprintf(stream, "%d\n", pThis->BeingProduced); // is also AI_REBUILDABLE under [Structures]
			fprintf(stream, "%d\n", pThis->ShouldRebuild);// is also AI_REPAIRABLE under [Structures]
			fprintf(stream, "%d\n", pThis->HasEngineer); // used to pass the NeedsEngineer check
			printTimer(pThis->CashProductionTimer);
			fprintf(stream, "%d\n", pThis->IsAllowedToSell); //6DC bool AI_Sellable; AI_SELLABLE under [Structures]
			fprintf(stream, "%d\n", pThis->IsReadyToCommence); //6DD
			fprintf(stream, "%d\n", pThis->NeedsRepairs); // AI handholder for repair logic,
			fprintf(stream, "%d\n", pThis->IsGoingToBlow); // used for warhead DelayKill and also C4
			fprintf(stream, "%d\n", pThis->NoCrew);
			fprintf(stream, "%d\n", pThis->IsCharging); //6E1
			fprintf(stream, "%d\n", pThis->IsCharged);	//6E2
			fprintf(stream, "%d\n", pThis->HasBeenCaptured); // has this building changed ownership at least once? affects crew and repair.
			fprintf(stream, "%d\n", pThis->ActuallyPlacedOnMap);
			fprintf(stream, "%d\n", pThis->unknown_bool_6E5);
			fprintf(stream, "%d\n", pThis->IsDamaged); // AI handholder for repair logic,
			fprintf(stream, "%d\n", pThis->IsFogged);
			fprintf(stream, "%d\n", pThis->IsBeingRepaired); // show animooted repair wrench
			fprintf(stream, "%d\n", pThis->HasBuildup);
			fprintf(stream, "%d\n", pThis->StuffEnabled); // status set by EnableStuff() and DisableStuff()
			fprintf(stream, "%d\n", pThis->HasCloakingData); // some fugly buffers
			fprintf(stream, "%d\n", pThis->CloakRadius); // from Type->CloakRadiusInCells
			fprintf(stream, "%d\n", pThis->Translucency);
			fprintf(stream, "%d\n", pThis->StorageFilledSlots); // the old "silo needed" logic
			fprintf(stream, "%p\n", pThis->SecretProduction); // randomly assigned secret lab bonus, used if SecretInfantry, SecretUnit, and SecretBuilding are null
			fprintf(stream, "%d %d %d\n", pThis->ColorAdd.R, pThis->ColorAdd.G, pThis->ColorAdd.B);
			fprintf(stream, "%d\n", pThis->IsAirstrikeTargetingMe); //6FC
			fprintf(stream, "%d\n", pThis->unknown_short_700);
			fprintf(stream, "%d\n", pThis->UpgradeLevel); // as defined by Type->UpgradesToLevel=
			fprintf(stream, "%d\n", pThis->GateStage);
			fprintf(stream, "%d\n", pThis->PrismStage);
			fprintf(stream, "%d %d %d\n", pThis->PrismTargetCoords.X, pThis->PrismTargetCoords.Y, pThis->PrismTargetCoords.Z);
			fprintf(stream, "%d\n", pThis->DelayBeforeFiring); //714

			fprintf(stream, "%d\n", pThis->BunkerState); // used in UpdateBunker and friends 0x718
		}
	}
	*/
}

template<>
void WriteLog(const BuildingClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);
	PrintBld(it, F);
}

template<>
void WriteLog(const AbstractTypeClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);
	fprintf(F, "; ID: %s; Name: %s", it->ID, it->Name);
}

template<>
void WriteLog(const HouseClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);

	fprintf(F, "; Player Name : %s (%d - %s); IsHumanPlayer: %u; ColorScheme: %s (%d); Edge: %d; StartingAllies: %u; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d",
		it->PlainName ? it->PlainName : NONE_STR,
		it->ArrayIndex, HouseTypeClass::Array->Items[it->Type->ArrayIndex]->Name,
		it->IsHumanPlayer, ColorScheme::Array->Items[it->ColorSchemeIndex]->ID, it->ColorSchemeIndex,
		(int)it->Edge, it->StartingAllies.data, it->StartingCell.X, it->StartingCell.Y, it->Visionary,
		it->MapIsClear, it->Available_Money());

	if (!it->IsNeutral() && !it->IsControlledByHuman())
	{
		fprintf(F, "\nLogging AI BaseNodes : \n");

		const auto& b = it->Base.BaseNodes;
		for (int j = 0; j < b.Count; ++j)
		{
			const auto& n = b[j];
			auto idx = n.BuildingTypeIndex;
			if (idx >= 0)
			{
				auto lbl = BuildingTypeClass::Array->Items[idx]->ID;
				fprintf(F, "\tNode #%03d: %s @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
					, j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
			}
			else
			{
				fprintf(F, "\tNode #%03d: Special %d @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
					, j, idx, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
			}
		}
		fprintf(F, "\n");
	}

}

// calls WriteLog and appends a newline
template<typename T>
void WriteLogLine(const T* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog(it, idx, checksum, F);
	fprintf(F, "\n");
}

template<typename T>
void LogItem(const T* it, int idx, FILE* F)
{
	if (it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2)
	{
		DWORD Checksum(0);
		SafeChecksummer Ch;
		it->ComputeCRC(Ch);
		Checksum = Ch.CRC;
		WriteLogLine(it, idx, Checksum, F);
	}
}

template<typename T>
void VectorLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Label)
	{
		fprintf(F, "Checksums for [%s] (%d) :\n", Label, Array ? Array->Count : -1);
	}
	if (Array)
	{
		for (auto i = 0; i < Array->Count; ++i)
		{
			auto it = Array->Items[i];
			LogItem(it, i, F);
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

template<typename T>
void HouseLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Array)
	{
		for (auto j = 0; j < HouseClass::Array->Count; ++j)
		{
			auto pHouse = HouseClass::Array->Items[j];
			fprintf(F, "-------------------- %s (%d) %s -------------------\n", pHouse->Type->Name, j, Label ? Label : "");

			for (auto i = 0; i < Array->Count; ++i)
			{
				auto it = Array->Items[i];

				if (it->Owner == pHouse)
				{
					LogItem(it, i, F);
				}
			}
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

#include <Phobos.version.h>
static constexpr reference<DynamicVectorClass<ObjectClass*>*, 0x87F778u> const Logics {};

bool LogFrame(const char* LogFilename, EventClass* OffendingEvent = nullptr)
{
	FILE* LogFile = nullptr;
	if (!fopen_s(&LogFile, LogFilename, "wt") && LogFile)
	{
		std::setvbuf(LogFile, nullptr, _IOFBF, 1024 * 1024); // 1024 kb buffer - should be sufficient for whole log

		fprintf(LogFile, "YR synchronization log\n");
		fprintf(LogFile, "With Ares [21.352.1218] and Phobos %s\n", PRODUCT_VERSION);

		fprintf(LogFile, "\n");

		int i = 0;
		for (auto const& [str, data] : Patch::ModuleDatas) {
			fprintf(LogFile,"Module [(%d) %s: Base address = %08X]\n", i++, str.c_str(), data.BaseAddr);
		}

		fprintf(LogFile, "\n");

		for (size_t ixF = 0; ixF < EventClass::LatestFramesCRC.c_size(); ++ixF)
		{
			fprintf(LogFile, "LastFrame CRC[%02d] = %08X\n", ixF, EventClass::LatestFramesCRC[ixF]);
		}

		fprintf(LogFile, "My Random Number: %08X\n", ScenarioClass::Instance->Random.Random());
		fprintf(LogFile, "My Frame: %08X\n", Unsorted::CurrentFrame());
		fprintf(LogFile, "Average FPS: %d\n", Game::F2I(FPSCounter::GetAverageFrameRate()));
		fprintf(LogFile, "Max MaxAhead: %d\n", Game::Network::MaxMaxAhead());
		fprintf(LogFile, "Latency setting: %d\n", Game::Network::LatencyFudge());
		fprintf(LogFile, "Game speed setting: %d\n", GameOptionsClass::Instance->GameSpeed);
		fprintf(LogFile, "FrameSendRate: %d\n", Game::Network::FrameSendRate());
		fprintf(LogFile, "Mod is %s (%s) with %X\n", AresGlobalData::ModName, AresGlobalData::ModVersion, AresGlobalData::ModIdentifier);

		if (HouseClass::CurrentPlayer())
			fprintf(LogFile, "Player Name: %s\n", HouseClass::CurrentPlayer->PlainName);

		const auto nHashes = HashData::GetINIChecksums();

		fprintf(LogFile, "Rules checksum: %08X\n", nHashes.Rules);
		fprintf(LogFile, "Art checksum: %08X\n", nHashes.Art);
		fprintf(LogFile, "AI checksum: %08X\n", nHashes.AI);

		if (OffendingEvent)
		{
			fprintf(LogFile, "\nOffending event:\n");
			fprintf(LogFile, "Type:         %X\n", OffendingEvent->Type);
			fprintf(LogFile, "Frame:        %X\n", OffendingEvent->Frame);
			fprintf(LogFile, "ID:           %X\n", OffendingEvent->HouseIndex);
			fprintf(LogFile, "CRC:          %X(%d)\n", OffendingEvent->Data.FrameInfo.CRC, OffendingEvent->Data.FrameInfo.CRC);
			fprintf(LogFile, "CommandCount: %hu\n", OffendingEvent->Data.FrameInfo.CommandCount);
			fprintf(LogFile, "Delay:        %hhu\n", OffendingEvent->Data.FrameInfo.Delay);
			fprintf(LogFile, "\n\n");
		}

		fprintf(LogFile, "\n**Types**\n");
		HouseLogger(InfantryClass::Array(), LogFile, "Infantry");
		HouseLogger(UnitClass::Array(), LogFile, "Units");
		HouseLogger(AircraftClass::Array(), LogFile, "Aircraft");
		HouseLogger(BuildingClass::Array(), LogFile, "Buildings");

		fprintf(LogFile, "\n**Checksums**\n");
		VectorLogger(HouseClass::Array(), LogFile, "Houses");
		VectorLogger(InfantryClass::Array(), LogFile, "Infantry");
		VectorLogger(UnitClass::Array(), LogFile, "Units");
		VectorLogger(AircraftClass::Array(), LogFile, "Aircraft");
		VectorLogger(BuildingClass::Array(), LogFile, "Buildings");

		fprintf(LogFile, "\n");
		VectorLogger(&ObjectClass::CurrentObjects(), LogFile, "Current Objects");
		VectorLogger(&LogicClass::Instance(), LogFile, "Logics");

		fprintf(LogFile, "\n**Checksums for Map Layers**\n");
		for (size_t ixL = 0; ixL < MapClass::ObjectsInLayers.c_size(); ++ixL)
		{
			fprintf(LogFile, "Checksums for Layer %d\n", ixL);
			VectorLogger(&(MapClass::ObjectsInLayers[ixL]), LogFile);
		}

		fprintf(LogFile, "\n**Checksums for Logics**\n");
		VectorLogger(&LogicClass::Instance(), LogFile);

		fprintf(LogFile, "\n**Checksums for Abstracts**\n");
		VectorLogger(AbstractClass::Array(), LogFile, "Abstracts");
		VectorLogger(AbstractTypeClass::Array(), LogFile, "AbstractTypes");

		fclose(LogFile);
		return true;
	}
	else
	{
		Debug::Log("Failed to open file for sync log. Error code %X.\n", errno);
		return false;
	}
}

DEFINE_STRONG_OVERRIDE_HOOK(0x64DEA0, Multiplay_LogToSYNC_NOMPDEBUG, 6)
{
	GET(EventClass*, OffendingEvent, ECX);

	char LogFilename[0x40];
	IMPL_SNPRNINTF(LogFilename, sizeof(LogFilename), "SYNC%01d.TXT", HouseClass::CurrentPlayer->ArrayIndex);

	LogFrame(LogFilename, OffendingEvent);

	return 0x64DF3D;
}

DEFINE_STRONG_OVERRIDE_HOOK(0x6516F0, Multiplay_LogToSync_MPDEBUG, 6)
{
	GET(int, SlotNumber, ECX);
	GET(EventClass*, OffendingEvent, EDX);

	char LogFilename[0x40];
	IMPL_SNPRNINTF(LogFilename, sizeof(LogFilename), "SYNC%01d_%03d.TXT", HouseClass::CurrentPlayer->ArrayIndex, SlotNumber);

	LogFrame(LogFilename, OffendingEvent);

	return 0x651781;
}
#pragma endregion

void NOINLINE BuildingClass_CalculateCRC(const BuildingClass* pThis, CRCEngine* pEngine)
{
	pThis->AbstractClass::Compute_CRC_Impl(pEngine);
	std::vector<unsigned int> record {};

	record.reserve(200);
	record.push_back((unsigned int)pEngine->CRC);

	if (auto pNext = pThis->NextObject) {
		pEngine->Compute_int(pNext->Fetch_ID()); record.push_back((unsigned int)pEngine->CRC);
	}

	if (auto pTag = pThis->AttachedTag) { pEngine->Compute_int(pTag->Fetch_ID());
		record.push_back((unsigned int)pEngine->CRC);
	}

	pEngine->Compute_int(pThis->Health); record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsOnMap); record.push_back((unsigned int)pEngine->CRC);
	if (SessionClass::Instance->GameMode == GameMode::Campaign 
		|| SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		pEngine->Compute_bool(pThis->NeedsRedraw); record.push_back((unsigned int)pEngine->CRC);
		pEngine->Compute_bool(pThis->IsSelected); record.push_back((unsigned int)pEngine->CRC);
	}
	pEngine->Compute_bool(pThis->InLimbo); record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->HasParachute); record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->OnBridge);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsFallingDown);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsABomb);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsAlive);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->Location.X);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->Location.Y);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->Location.Z);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->CurrentMission);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->SuspendedMission);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->QueuedMission);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->MissionStatus);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->UpdateTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->CurrentMissionStartTime);		record.push_back((unsigned int)pEngine->CRC);

	pEngine->Compute_int(pThis->RadioLinks.Capacity);		record.push_back((unsigned int)pEngine->CRC);
	for (int i = 0; i < pThis->RadioLinks.Capacity; ++i) {
		if (auto v4 = pThis->RadioLinks.Items[i]) {
			pEngine->Compute_int(v4->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
			pEngine->Compute_int((int)v4->WhatAmI());		record.push_back((unsigned int)pEngine->CRC);
		}
	}

	pEngine->Compute_int(pThis->Group);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_double(pThis->ArmorMultiplier);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_double(pThis->FirepowerMultiplier);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->IdleActionTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->DisplayProductionTo.data);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->CloakState);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->CloakDelayTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->WarpFactor);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->unknown_bool_250);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->LastSightRange);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->PitchAngle);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->DiskLaserTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->Ammo);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->ReloadTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->Value);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->AngleRotatedSideways);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->AngleRotatedForwards);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->RockingSidewaysPerFrame);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_float(pThis->RockingForwardsPerFrame);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsSinking);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->__ProtectMe_3CF);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsUseless);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsTickedOff);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->Cloakable);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsPrimaryFactory);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->Spawned);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsInPlayfield);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsTethered);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->RADIO_26_27_419);		record.push_back((unsigned int)pEngine->CRC);
	if (SessionClass::Instance->GameMode == GameMode::Campaign
		|| SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		pEngine->Compute_bool(pThis->IsOwnedByCurrentPlayer);		record.push_back((unsigned int)pEngine->CRC);
		pEngine->Compute_bool(pThis->DiscoveredByCurrentPlayer);		record.push_back((unsigned int)pEngine->CRC);
		pEngine->Compute_bool(pThis->DiscoveredByComputer);		record.push_back((unsigned int)pEngine->CRC);
	}
	pEngine->Compute_bool(pThis->unknown_bool_41D);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->EMPLockRemaining);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->unknown_bool_41E);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->unknown_bool_41F);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_uchar(pThis->SightIncrease);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->CurrentTargets.Count);		record.push_back((unsigned int)pEngine->CRC);
	for (int i = 0; i < pThis->CurrentTargets.Count; ++i) {
		pEngine->Compute_int(pThis->CurrentTargets.Items[i]->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
		pEngine->Compute_int(pThis->CurrentTargetThreatValues.Items[i]);		record.push_back((unsigned int)pEngine->CRC);
	}

	pEngine->Compute_int(pThis->AttackedTargets.Count);		record.push_back((unsigned int)pEngine->CRC);
	for (int j = 0; j < pThis->AttackedTargets.Count; ++j) {
		pEngine->Compute_int(pThis->AttackedTargets.Items[j]->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
	}

	pEngine->Compute_int(pThis->GoingToBlowTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->CashProductionTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->BState);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->QueueBState);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->OwnerCountryIndex);		record.push_back((unsigned int)pEngine->CRC);
	if (auto WhomToRepay  = pThis->C4AppliedBy) {
		pEngine->Compute_int(WhomToRepay->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
	}

	pEngine->Compute_int(pThis->LastStrength);		record.push_back((unsigned int)pEngine->CRC);

	if (auto FirestormAnim = pThis->FirestormAnim) {
		pEngine->Compute_int(FirestormAnim->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
	}

	pEngine->Compute_int(pThis->PlacementDelay.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->upgrade_5FC);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->LaserFenceFrame);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int(pThis->FirestormWallFrame);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->HasPower);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->StuffEnabled);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->BeingProduced);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->ShouldRebuild);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsAllowedToSell);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsReadyToCommence);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsBeingRepaired);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->NeedsRepairs);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsGoingToBlow);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->NoCrew);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsCharging);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsCharged);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->HasBeenCaptured);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->ActuallyPlacedOnMap);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->HasEngineer);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->unknown_bool_6E5);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_bool(pThis->IsDamaged);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_int((int)pThis->unknown_short_700);		record.push_back((unsigned int)pEngine->CRC);
	pEngine->Compute_uchar(pThis->UpgradeLevel);		record.push_back((unsigned int)pEngine->CRC);

	//CRCRecords[pThis] = std::move(record);
}

//DEFINE_HOOK(0x454260, BuildingClass_CalculateCRC_Handle, 0x6)
//{
//	GET(const BuildingClass*, pThis, ECX);
//	GET_STACK(CRCEngine*, pCRCEngine, 0x4);
//
//	for (const auto& who : whos) {
//		if (IS_SAME_STR_(who.c_str(), pThis->Type->ID)) {
//			BuildingClass_CalculateCRC(pThis, pCRCEngine);
//			return 0x454498;
//		}
//	}
//
//	return 0x0;
//}