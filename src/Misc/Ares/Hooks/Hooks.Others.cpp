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

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

#include "Header.h"
#include <Ares_TechnoExt.h>

DEFINE_DISABLE_HOOK(0x6d4684, TacticalClass_Draw_FlyingStrings_ares)
DEFINE_DISABLE_HOOK(0x7258d0, AnnounceInvalidPointer_ares)
DEFINE_DISABLE_HOOK(0x533058, CommandClassCallback_Register_ares)

DEFINE_OVERRIDE_HOOK(0x52C5E0 , Ares_NOLOGO , 0x7)
{
	return Phobos::Otamaa::NoLogo ? 0x52C5F3 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x62A020, ParasiteClass_Update, 0xA)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExt::ExtMap.Find(pOwner)->idxSlot_Parasite));
	return 0x62A02A;
}

DEFINE_OVERRIDE_HOOK(0x62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExt::ExtMap.Find(pOwner)->idxSlot_Parasite));
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


DEFINE_HOOK(0x5F6500, AbstractClass_Distance2DSquared_1, 0)
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

DEFINE_OVERRIDE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 0)
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
//			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
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
	auto& Buffer = PhobosGlobal::Instance()->ShpCompression1Buffer;
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

	if (!TechnoTypeExt::ExtMap.Find(pBld->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pBld, BuildingCost, RulesClass::Instance->ShakeScreen);

	return 0x441C39; //return 0 causing crash
}

DEFINE_OVERRIDE_HOOK(0x7387D1, UnitClass_Destroyed_Shake, 0x56)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen)
		return 0x738801;

	if (!pUnit->Type->Strength)
		return 0x738801;

	if (!TechnoTypeExt::ExtMap.Find(pUnit->Type)->DontShake.Get())
		ShakeScreenHandle::ShakeScreen(pUnit, pUnit->Type->Strength, RulesClass::Instance->ShakeScreen);

	return 0x738801;
}

// replaces entire function (without the pip distortion bug)
DEFINE_OVERRIDE_HOOK(0x4748A0, INIClass_GetPipIdx, 0x7)
{
	GET(INIClass*, pINI, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, fallback, 0xC);

	if (pINI->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer) > 0)
	{
		int nbuffer;
		if (Parser<int>::TryParse(Phobos::readBuffer, &nbuffer))
		{
			R->EAX(nbuffer);
			return 0x474907;
		}
		else
		{
			// find the pip value with the name specified
			for (const auto& data : TechnoTypeClass::PipsTypeName)
			{
				if (data == Phobos::readBuffer)
				{
					//Debug::Log("[%s]%s=%s ([%d] from [%s]) \n", pSection, pKey, Phobos::readBuffer, it->Value, it->Name);
					R->EAX(data.Value);
					return 0x474907;
				}
			}
		}

		Debug::INIParseFailed(pSection, pKey, Phobos::readBuffer, "Expect valid pip");
	}

	R->EAX(fallback);
	return 0x474907;
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

// invalid or not set edge reads array out of bounds
DEFINE_OVERRIDE_HOOK(0x4759D4, INIClass_WriteEdge, 0x7)
{
	GET(int const, index, EAX);

	if (index < 0 || index > 4)
	{
		R->EDX(GameStrings::NoneStrb());
		return 0x4759DB;
	}

	return 0;
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
		if(Owner->GetHeight() > 200){
			*XYZ = Owner->Location;
			Owner->IsFallingDown = Owner->IsABomb = true;
		}else if (Owner->GetHeight() < 0) //inside ground
			*XYZ = CoordStruct::Empty;
	}

	CoordStruct result = *XYZ;

	if (result == CoordStruct::Empty || CellClass::Coord2Cell(result) == CellStruct::Empty) {
		Debug::Log("Parasite[%x : %s] With Invalid Location ! , Removing ! \n", Parasite, Parasite->Owner->get_ID());
		TechnoExt::HandleRemove(Parasite->Owner, nullptr, false, false);
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
	const auto pInvoker = VoxelAnimExt::GetTechnoOwner(pThis);

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
//		R->EDX(HouseClass::Array->GetItem(idx));
//		return 0x47F9AA;
//	}
//
//	return 0;
//}

// issue 1520: logging stupid shit crashes the game
DEFINE_OVERRIDE_HOOK(0x4CA437, FactoryClass_GetCRC, 0x8)
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

	if (pVictim->WhatAmI() == BuildingClass::AbsID)
	{
		// 1. add to team and other fates don't really make sense for buildings
		// 2. BuildingClass::Mission_Hunt() implementation is to do nothing!
		pVictim->QueueMission(Mission::Guard, 0);
		return 0x472604;
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
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->Get();
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
	R->EAX(TechnoTypeExt::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

// DEFINE_OVERRIDE_HOOK(0x7BB445, XSurface_20, 0x6)
// {
// 	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
// }

DEFINE_OVERRIDE_HOOK(0x5FDDA4, OverlayClass_GetTiberiumType_NotReallyTiberiumLog, 0x6)
{
	GET(OverlayTypeClass*, pThis, EAX);
	Debug::Log("Overlay %s not really tiberium\n", pThis->ID);
	return 0x5FDDC1;
}

DEFINE_OVERRIDE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

#include <Misc/AresData.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>


// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
DEFINE_OVERRIDE_HOOK(0x489270, CellChainReact, 5)
{
	GET(CellStruct*, cell, ECX);

	const auto pCell = MapClass::Instance->GetCellAt(*cell);

	if (pCell->OverlayData <= 0)
		return 0x0;

	TiberiumClass* pTib = TiberiumClass::Array->GetItemOrDefault(pCell->GetContainedTiberiumIndex());

	if (!pTib)
		return 0x0;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->GetItemOrDefault(pCell->OverlayTypeIndex);

	if (!pOverlay || !pOverlay->ChainReaction)
		return 0x0;

	CoordStruct crd = pCell->GetCoords();

	if (ScenarioClass::Instance->Random.RandomFromMax(99) <
		(RulesExt::Global()->ChainReact_Multiplier * pCell->OverlayData))
	{
		bool wasFullGrown = (pCell->OverlayData >= 11);

		unsigned char delta = pCell->OverlayData / 2;
		int damage = pTib->Power * delta;

		// remove some of the tiberium
		pCell->OverlayData -= delta;
		pCell->MarkForRedraw();

		// get the warhead
		auto pExt = TiberiumExt::ExtMap.Find(pTib);
		auto pWarhead = pExt->GetExplosionWarhead();

		// create an explosion
		if (auto pType = MapClass::SelectDamageAnimation(4 * damage, pWarhead, pCell->LandType, crd))
		{
			GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, 0);
		}

		// damage the area, without affecting tiberium
		MapClass::DamageArea(crd, damage, nullptr, pWarhead, false, nullptr);

		// spawn some animation on the neighbour cells
		if (auto pType = AnimTypeClass::Find(GameStrings::Anim_INVISO()))
		{
			for (int i = 0; i < 8; ++i)
			{
				auto pNeighbour = pCell->GetNeighbourCell((FacingType)i);

				if (pNeighbour->GetContainedTiberiumIndex() != -1 && pNeighbour->OverlayData > 2)
				{
					if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesExt::Global()->ChainReact_SpreadChance)
					{
						int delay = ScenarioClass::Instance->Random.RandomRanged(RulesExt::Global()->ChainReact_MinDelay, RulesExt::Global()->ChainReact_MaxDelay);
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

	return ScenarioClass::Instance->Random.RandomFromMax(99) < TiberiumExt::ExtMap.Find(pTib)->GetDebrisChance()
		? 0x424DF9 : 0x424E9B;
}

DEFINE_OVERRIDE_HOOK(0x424EC5, AnimClass_ReInit_TiberiumChainReaction_Damage, 6)
{
	GET(TiberiumClass*, pTib, EDI);
	auto pExt = TiberiumExt::ExtMap.Find(pTib);

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
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuildingProduct);

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
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis);
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

DEFINE_DISABLE_HOOK(0x5A5C6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNE_ares) //, 9, 5A5CC8)
DEFINE_JUMP(LJMP, 0x5A5C6A, 0x5A5CC8);

DEFINE_DISABLE_HOOK(0x5A5D6F, MapSeedClass_Generate_PlacePavedRoads_RoadEndSW_ares) //, 9, 5A5DB8)
DEFINE_JUMP(LJMP, 0x5A5D6F, 0x5A5DB8);

DEFINE_DISABLE_HOOK(0x5A5F6A, MapSeedClass_Generate_PlacePavedRoads_RoadEndNW_ares) //, 8, 5A5FF8)
DEFINE_JUMP(LJMP, 0x5A5F6A, 0x5A5FF8);

DEFINE_DISABLE_HOOK(0x5A6464, MapSeedClass_Generate_PlacePavedRoads_RoadEndSE_ares) //, 9, 5A64AD)
DEFINE_JUMP(LJMP, 0x5A6464, 0x5A64AD);

DEFINE_DISABLE_HOOK(0x59000E, RMG_FixPavedRoadEnd_Bridges_North) //, 5, 590087)
DEFINE_JUMP(LJMP, 0x59000E, 0x590087);

DEFINE_DISABLE_HOOK(0x5900F7, RMG_FixPavedRoadEnd_Bridges_South) //, 5, 59015E)
DEFINE_JUMP(LJMP, 0x5900F7, 0x59015E);

DEFINE_DISABLE_HOOK(0x58FCC6, RMG_FixPavedRoadEnd_Bridges_West) //, 5, 58FD2A)
DEFINE_JUMP(LJMP, 0x58FCC6, 0x58FD2A);

DEFINE_DISABLE_HOOK(0x58FBDD, RMG_FixPavedRoadEnd_Bridges_East) //, 5, 58FC55)
DEFINE_JUMP(LJMP, 0x58FBDD, 0x58FC55);

DEFINE_OVERRIDE_HOOK(0x58FA51, RMG_PlaceWEBridge, 6)
{
	LEA_STACK(const RectangleStruct* const, pRect, 0x14);

	//it's a WE bridge
	return (pRect->Width > pRect->Height)
		? 0x58FA73
		: 0;
}

DEFINE_OVERRIDE_HOOK(0x58FE7B, RMG_PlaceNSBridge, 8)
{
	LEA_STACK(const RectangleStruct* const, pRect, 0x14);

	//it's a NS bridge
	return (pRect->Height > pRect->Width)
		? 0x58FE91
		: 0;
}

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

#include <WWKeyboardClass.h>

DEFINE_OVERRIDE_HOOK(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

#include <Utilities/Helpers.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Particle/Body.h>

DEFINE_OVERRIDE_HOOK(0x48248D, CellClass_CrateBeingCollected_MoneyRandom, 6)
{
	GET(int, nCur, EAX);

	const auto nAdd = RulesExt::Global()->RandomCrateMoney;

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
		const auto pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
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
	return (TechnoExt::ExtMap.Find(Unit)->AE_ArmorMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (TechnoExt::ExtMap.Find(Unit)->AE_SpeedMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExt::ExtMap.Find(Unit)->AE_FirePowerMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_OVERRIDE_HOOK(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || TechnoExt::ExtMap.Find(Unit)->AE_Cloak)
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExt::ExtMap.Find(Unit->GetTechnoType())->CloakAllowed)
		? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
DEFINE_OVERRIDE_HOOK(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	TechnoExt::ExtMap.Find(Unit)->AE_Cloak = true;
	TechnoExt_ExtData::RecalculateStat(Unit);
	return 0x482956;
}

DEFINE_OVERRIDE_HOOK(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	if (TechnoExt::ExtMap.Find(Unit)->AE_ArmorMult == 1.0)
	{
		TechnoExt::ExtMap.Find(Unit)->AE_ArmorMult = Pow_ArmorMultiplier;
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

	if (TechnoExt::ExtMap.Find(Unit)->AE_SpeedMult == 1.0)
	{
		TechnoExt::ExtMap.Find(Unit)->AE_SpeedMult = Pow_SpeedMultiplier;
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

	if (TechnoExt::ExtMap.Find(Unit)->AE_FirePowerMult == 1.0)
	{
		TechnoExt::ExtMap.Find(Unit)->AE_FirePowerMult = Pow_FirepowerMultiplier;
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
		TechnoExt::ExtMap.Find(pTarget)->RadarJammer.reset(nullptr);
		AresAE::UpdateTempoal(&TechnoExt::ExtMap.Find(pTarget)->AeData , pTarget);
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
		if (pThis->Type->Trainable && HouseExt::ExtMap.Find(pThis->Owner)->Is_AirfieldSpied)
			pThis->Veterancy.Veterancy = 1.0f;

		HouseExt::ApplyAcademy(pThis->Owner, pThis, AbstractType::Aircraft);
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
	const auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAttack.isset())
		index = pAircraftExt->VoiceAirstrikeAttack.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

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
	const auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	if (pAircraftExt->VoiceAirstrikeAbort.isset())
		index = pAircraftExt->VoiceAirstrikeAbort.Get();

	// get from designator
	if (const auto pOwner = pAirstrike->Owner)
	{
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		if (pOwnerExt->VoiceAirstrikeAbort.isset())
			index = pOwnerExt->VoiceAirstrikeAbort.Get();
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

DEFINE_OVERRIDE_HOOK(0x4A8FF5, MapClass_CanBuildingTypeBePlacedHere_Ignore, 5)
{
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(pBuilding)->IsFromSW ? 0x4A8FFA : 0x0;
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
	return (Target && Target->Owner->IsControlledByHuman_() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

// DEFINE_OVERRIDE_HOOK(0x52E9AA, Frontend_WndProc_Checksum, 5)
// {
// 	if (SessionClass::Instance->GameMode == GameMode::LAN || SessionClass::Instance->GameMode == GameMode::Internet)
// 	{
// 		auto nHashes = HashData::GetINIChecksums();
// 		Debug::Log("Rules checksum: %08X\n", nHashes.Rules);
// 		Debug::Log("Art checksum: %08X\n", nHashes.Art);
// 		Debug::Log("AI checksum: %08X\n", nHashes.AI);
// 	}
//
// 	return 0;
// }

DEFINE_OVERRIDE_HOOK(0x480534, CellClass_AttachesToNeighbourOverlay, 5)
{
	GET(int, idxOverlay, EAX);
	const bool Wall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;
	return Wall ? 0x480549 : 0x480552;
}

DEFINE_OVERRIDE_HOOK(0x4A76ED, DiskLaserClass_Update_Anim, 7)
{
	GET(DiskLaserClass* const, pThis, ESI);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x54, 0x1C));

	auto const pWarhead = pThis->Weapon->Warhead;

	if (RulesExt::Global()->DiskLaserAnimEnabled)
	{
		auto const pType = MapClass::SelectDamageAnimation(
			pThis->Damage, pWarhead, LandType::Clear, coords);

		if (pType) {
			AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, coords),
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

#include <MPGameModeClass.h>

//Wrong register ?
// game crash here , ugh
DEFINE_OVERRIDE_HOOK(0x5D6F61, MPGameModeClass_CreateStartingUnits_BaseCenter, 8)
{
	GET(MPGameModeClass*, pMode, ECX);
	GET(HouseClass*, pHouse, ESI);
	GET(int*, AmountToSpend, EAX);

	*AmountToSpend = R->EBP<int>();
	CellStruct nBase = pHouse->BaseSpawnCell;

	if(!pMode->SpawnBaseUnits(pHouse, AmountToSpend))
		return 0x5D701B;

	pHouse->ConYards.for_each([](BuildingClass* pConyards) {
		pConyards->QueueMission(Mission::Construction, true);
		++Unsorted::ScenarioInit();
		pConyards->EnterIdleMode(false, 1);
		--Unsorted::ScenarioInit();
	});

	if (!nBase.IsValid())
		pHouse->BaseSpawnCell = nBase;

	return 0x5D6F77;
}

DEFINE_OVERRIDE_HOOK(0x687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Debug_bTrackParseErrors = false;
	Phobos::Otamaa::TrackParserErrors = false;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5f5add, ObjectClass_SpawnParachuted_Animation, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (const auto pTechno = generic_cast<TechnoClass*>(pThis))
	{
		auto pType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt->IsBomb)
			pThis->IsABomb = true;

		R->EDX(pTypeExt->ParachuteAnim ? pTypeExt->ParachuteAnim : HouseExt::GetParachuteAnim(pTechno->Owner));
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

	if(pBaseUnit)
		return hasBaseUnit;

	Debug::Log(__FUNCTION__" House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return hasNoBaseUnit;
}

DEFINE_OVERRIDE_HOOK(0x4C850B, Exception_Dialog, 5) {
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

	if (!HouseTypeExt::ExtMap.Find(pHouseType)->StartInMultiplayer_WithConst)
		return 0;

	auto idxParentCountry = HouseClass::FindIndexByName(pHouseType->ParentCountry);
	const auto v7 = HouseExt::FindBuildable(pHouse, idxParentCountry, make_iterator(RulesClass::Instance->BuildConst), 0);

	if (!v7) {
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

	if (!pHouse->IsControlledByHuman_())
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

DEFINE_OVERRIDE_HOOK(0x6BD7E3, Expand_MIX_Reorg, 5)
{
	MixFileClass::Bootstrap();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x52BB64, Expand_MIX_Deorg, 5)
{
	R->AL(1);
	return 0x52BB69;
}

//[01:25:38] SyringeDebugger::HandleException: Ares.dll [0x5d6d9a , MPGameModeClass_CreateStartingUnits_UnitCost , 6]
//[01:25 : 38] SyringeDebugger::HandleException: Ares.dll[0x5d7163, MPGameMode_SpawnStartingUnits_Types, 8]

//StartInMultiplayerUnitCost

void FormulateTypeList(std::vector<TechnoTypeClass*>& types, TechnoTypeClass** items, int count, int houseidx)
{
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

int GetTotalCost(const Nullable<int>& fixed, std::vector<TechnoTypeClass*>& types)
{
	if (GameModeOptionsClass::Instance->UnitCount <= 0)
		return 0;

	int totalCost = 0;
	if (fixed.isset()) {
		totalCost = fixed;
	}else{
		int total_ = 0;
		for (auto& tech : types)
		{
			total_ += tech->GetCost();
		}

		int what = !types.size() ? 1 : types.size();
		totalCost = (total_ + (what >> 1)) / what;

	}

	return totalCost * GameModeOptionsClass::Instance->UnitCount;
}

void GetTypeList()
{
	std::vector<TechnoTypeClass*> types;

	DWORD idx = 0u;
	for (auto pHouse : *HouseClass::Array) {
		if (!pHouse->Type->MultiplayPassive) {

			const auto& data = HouseTypeExt::ExtMap.Find(pHouse->Type)->StartInMultiplayer_Types;
			if (!data.empty()) {
				types.assign(data.begin(), data.end());
			}
			else
			{
				idx |= 1 << pHouse->Type->ArrayIndex;
			}
		}
	}

	FormulateTypeList(types, (TechnoTypeClass**)UnitTypeClass::Array->Items, UnitTypeClass::Array->Count, idx);
	FormulateTypeList(types, (TechnoTypeClass**)InfantryTypeClass::Array->Items, InfantryTypeClass::Array->Count, idx);

	for (auto& base : RulesClass::Instance->BaseUnit) {

	}
}