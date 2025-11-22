
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
#include <Misc/DamageArea.h>

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

#include "Header.h"

#include <Misc/Spawner/Main.h>

ASMJIT_PATCH(0x52C5E0, Ares_NOLOGO, 0x7)
{
	if(SpawnerMain::Configs::Enabled)
		return 0x52C5F8; //skip showing looading screen

	return Phobos::Otamaa::NoLogo ? 0x52C5F3 : 0x0;
}

ASMJIT_PATCH(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
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

DEFINE_JUMP(LJMP, 0x565215, 0x56522D);

//ASMJIT_PATCH(0x6E2290, ActionClass_PlayAnimAt, 0x6)
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

DEFINE_JUMP(LJMP, 0x6AD0ED, 0x6AD16C);

ASMJIT_PATCH(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
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

ASMJIT_PATCH(0x7387D1, UnitClass_Destroyed_Shake, 0x6)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen || Phobos::Config::HideShakeEffects)
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
ASMJIT_PATCH(0x5F77F0, ObjectTypeClass_UnloadPipsSHP, 0x5)
{
	for (int i = 0; i < (int)TechnoTypeClass::ShapesIsAllocated.size(); ++i)
	{
		if (TechnoTypeClass::ShapesIsAllocated[i] && FileSystem::ShapesAllocated[i])
		{
			GameDelete<true, false>(std::exchange(FileSystem::ShapesAllocated[i] , nullptr));
			TechnoTypeClass::ShapesIsAllocated[i] = false;
		}
	}

	return 0x5F78FB;
}

// naive way to fix negative indexes to be generated. proper way would be to replace
// the entire function, and the function consuming the indexes. it is not yet known
// whether the out of bounds read causes desync errors. this function appears to
// have been inlined prominently in 585F40
ASMJIT_PATCH(0x56BC54, ThreatPosedEstimates_GetIndex, 0x5)
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
ASMJIT_PATCH(0x4C6DDB, Networking_RespondToEvent_Selling, 0x8)
{
	GET(TechnoClass* const, pTechno, EDI);
	GET(AbstractClass* const, pFocus, EAX);

	if (pTechno->CurrentMission != Mission::Selling || pTechno->ArchiveTarget)
	{
		pTechno->SetArchiveTarget(pFocus);
	}

	return 0x4C6DE3;
}

// #895374: skip the code that removes the crates (size 7)
DEFINE_JUMP(LJMP, 0x483BF1, 0x483BFE);

ASMJIT_PATCH(0x699C1C, Game_ParsePKTs_ClearFile, 0x7)
{
	LEA_STACK(CCINIClass*, pINI, 0x24);
	pINI->Clear(nullptr, nullptr);
	return 0;
}

// Guard command failure
ASMJIT_PATCH(0x730DB0, GuardCommandClass_Execute, 0xA)
{
	GET(TechnoClass*, T, ESI);
	return (T->Owner != HouseClass::CurrentPlayer() || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
		;
}

/* #367 - do we need to draw a link to this victim */
ASMJIT_PATCH(0x472198, CaptureManagerClass_DrawLinks, 0x6)
{
	enum { Draw_Maybe = 0, Draw_Yes = 0x4721E6, Draw_No = 0x472287 };

	GET(CaptureManagerClass*, Controlled, EDI);
	//GET(TechnoClass *, Item, ECX);

	if (FootClass* F = flag_cast_to<FootClass*>(Controlled->Owner))
	{
		if (F->ParasiteImUsing && F->InLimbo)
		{
			return Draw_No;
		}
	}

	return Draw_Maybe;
}

ASMJIT_PATCH(0x551A30, LayerClass_YSortReorder, 0x5)
{
	GET(LayerClass*, pThis, ECX);

	auto const nCount = pThis->Count;
	auto nBegin = &pThis->Items[nCount / 15 * (Unsorted::CurrentFrame % 15)];
	auto nEnd = (Unsorted::CurrentFrame % 15 >= 14) ? (&pThis->Items[nCount]) : (&nBegin[nCount / 15 + nCount / 15 / 4]);
	std::sort(nBegin, nEnd, [](const ObjectClass* A, const ObjectClass* B) {
		return A->GetYSort() < B->GetYSort();
	});

	return 0x551A84;
}

//ASMJIT_PATCH(0x5F65F0, ObjectClass_UnUnit_nullptr, 0x6)
//{
//	GET(ObjectClass*, pThis, ECX);
//
//	if (!pThis)
//		Debug::FatalError("ObjectClass UnInit called from %x",R->Stack<DWORD>(0x0));
//
//	return 0x0;
//}

static void __fastcall AnnounceInvalidatePointerWrapper(ObjectClass* pObject , bool removed)
{
	if (!pObject->Limbo())
		pObject->AnnounceExpiredPointer(removed);
}

DEFINE_FUNCTION_JUMP(CALL , 0x5F6616, AnnounceInvalidatePointerWrapper)
// ASMJIT_PATCH(0x5F6612, ObjectClass_UnInit_SkipInvalidation, 0x9)
// {
// 	GET(ObjectClass*, pThis, ESI);
//
// 	if (!pThis->Limbo())
// 		pThis->AnnounceExpiredPointer();
//
// 	return 0x5F6625;
// }

//speeds up preview drawing by insane amounts
ASMJIT_PATCH(0x5FED00, OverlayTypeClass_GetRadarColor, 0x6)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct*, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

struct FakeObjectClass : public ObjectClass
{
public:

	CellClass* _GetCell() const{
		return MapClass::Instance->GetCellAt(this->Location);
	}
};
// bugfix #187: Westwood idiocy
// ASMJIT_PATCH(0x5F6960, ObjectClass_Getcell, 0xA)
// {
// 	GET(ObjectClass*, pThis, ECX);
// 	R->EAX(MapClass::Instance->GetCellAt(pThis->Location));
// 	return 0x5F69B6;
// }

DEFINE_FUNCTION_JUMP(VTABLE,0x7E2460, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7E3510, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7E3C8C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7E4078, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7E48A0, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7E8E50, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EB214, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EC414, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EDE7C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EF21C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EF590, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EFB10, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7EFD58, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F06C4, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F34B8, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F4B1C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F53E8, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F5E2C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F64D4, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F6864, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE,0x7F6DB0, FakeObjectClass::_GetCell);

//Handle_Static_Messages_LoopingMovie
DEFINE_JUMP(LJMP, 0x615BD3, 0x615BE0);
//sub_789960_RemoveWOLResolutionCheck
DEFINE_JUMP(LJMP, 0x78997B, 0x789A58);
//DSurface_CTOR_SkipVRAM
DEFINE_JUMP(LJMP, 0x4BA61B, 0x4BA623);

ASMJIT_PATCH(0x74A884, VoxelAnimClass_UpdateBounce_Damage, 0x6)
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

ASMJIT_PATCH(0x545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 0x7)
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
// PsyDom_Update
DEFINE_JUMP(LJMP, 0x53AF40, 0x53B060);

ASMJIT_PATCH(0x65EA43, SendReinforcement_Opentopped, 0x6)
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
//ASMJIT_PATCH(0x47F9A4, CellClass_DrawOverlay_WallRemap, 0x6)
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
ASMJIT_PATCH(0x4CA437, FactoryClass_GetCRC, 0x8)
{
	GET(FactoryClass*, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass*>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

ASMJIT_PATCH(0x47243F, CaptureManagerClass_DecideUnitFate_BuildingFate, 0x6)
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
ASMJIT_PATCH(0x671152, RulesClass_Addition_General_PrismSupportModifier, 0x6)
{
	GET(RulesClass*, pThis, ESI);
	REF_STACK(double, param, 0x0);
	param = pThis->PrismSupportModifier / 100.0;
	return 0x67115B;
}

ASMJIT_PATCH(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode.Status != SpawnNodeStatus::TakeOff
		|| !pOwner
		|| pOwner->WhatAmI() == BuildingClass::AbsID)
		? 0x6B735C : 0x6B72FE;
}

ASMJIT_PATCH(0x725A1F, AnnounceInvalidPointer_SkipBehind, 0x5)
{
	GET(AnimClass*, pAnim, ESI);
	return pAnim->Type == RulesClass::Instance->Behind ?
		0x725C08 : 0x0;
}

//sub_731D90_FakeOf
ASMJIT_PATCH(0x731E08, Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;

	for (const auto pObj : ObjectClass::CurrentObjects())
	{
		if (const auto pTechno = flag_cast_to<const TechnoClass*>(pObj))
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->This();
			if (pTypeExt->Fake_Of)
				pType = pTypeExt->Fake_Of;

			nCost += pType->GetActualCost(pTechno->Owner);
		}
	}

	R->EBX(nCost);
	return 0x731E4D;
}

ASMJIT_PATCH(0x6DA665, sub_6DA5C0_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

ASMJIT_PATCH(0x7BB445, XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}

ASMJIT_PATCH(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

#include <Ext/Cell/Body.h>

// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
ASMJIT_PATCH(0x489270, CellChainReact, 5)
{
	GET(CellStruct*, cell, ECX);

	const auto pCell = (FakeCellClass*)MapClass::Instance->GetCellAt(cell);
	TiberiumClass* pTib = TiberiumClass::Array->get_or_default(pCell->_GetTiberiumType());

	if (!pTib)
		return 0x0;

	OverlayTypeClass* pOverlay = OverlayTypeClass::Array->get_or_default(pCell->OverlayTypeIndex);

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
		auto pExt = TiberiumExtContainer::Instance.Find(pTib);
		CoordStruct crd = pCell->GetCoords();

		if (auto pWarhead = pExt->GetExplosionWarhead())
		{
			// create an explosion
			if (auto pType = MapClass::SelectDamageAnimation(4 * damage, pWarhead, pCell->LandType, crd))
			{
				GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, 0);
			}

			// damage the area, without affecting tiberium
			DamageArea::Apply(&crd, damage, nullptr, pWarhead, false, nullptr);
		}

		// spawn some animation on the neighbour cells
		if (auto pType = AnimTypeClass::Find(GameStrings::Anim_INVISO()))
		{
			for (int i = 0; i < 8; ++i)
			{
				auto pNeighbour = (FakeCellClass*)pCell->GetNeighbourCell((FacingType)i);

				if (pNeighbour->_GetTiberiumType() != -1 && pNeighbour->OverlayData > 2u)
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

// ASMJIT_PATCH(0x424DD3, AnimClass_ReInit_TiberiumChainReaction_Chance, 6)
// {
// 	GET(TiberiumClass*, pTib, EDI);
//
// 	return ScenarioClass::Instance->Random.RandomFromMax(99) < TiberiumExtContainer::Instance.Find(pTib)->
// 		? 0x424DF9 : 0x424E9B;
// }

// ASMJIT_PATCH(0x424EC5, AnimClass_ReInit_TiberiumChainReaction_Damage, 6)
// {
// 	GET(TiberiumClass*, pTib, EDI);
// 	auto pExt = TiberiumExtContainer::Instance.Find(pTib);
//
// 	R->Stack(0x0, pExt->GetExplosionWarhead());
// 	R->EDX(pExt->GetExplosionDamage());
//
// 	return 0x424ECB;
// }

ASMJIT_PATCH(0x6AB8BB, SelectClass_ProcessInput_BuildTime, 6)
{
	GET(BuildingTypeClass* const, pBuildingProduct, ESI);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pBuildingProduct);

	bool IsAWall = pBuildingProduct->Wall;
	if (pBuildingProduct->Wall && pTypeExt->BuildTime_Speed.isset())
		IsAWall = false;

	R->AL(IsAWall);
	return 0x6AB8C1;
}

//DEFINE_SKIP_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes, 5, 715876)
DEFINE_JUMP(LJMP, 0x715857, 0x715876);

ASMJIT_PATCH(0x711EE0, TechnoTypeClass_GetBuildSpeed, 6)
{
	GET(TechnoTypeClass* const, pThis, ECX);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);
	const auto nSeed = pTypeExt->BuildTime_Speed.Get(RulesClass::Instance->BuildSpeed);
	const auto nCost = pTypeExt->BuildTime_Cost.Get(pThis->Cost);
	R->EAX(int(nSeed * nCost / 1000.0 * 900.0));
	return 0x711EDE;
}

//WinMain_LogGameClasses
DEFINE_JUMP(LJMP, 0x6BB9DD, 0x6BBE2B);

//TechnoClass_DealParticleDamage_DontDestroyCliff
DEFINE_JUMP(LJMP, 0x70CAD8, 0x70CB30);

// bugfix #187: Westwood idiocy
// Game_BulkDataInit_MultipleDataInitFix1
DEFINE_JUMP(LJMP, 0x531726, 0x53173A);

// bugfix #187: Westwood idiocy
//Game_BulkDataInit_MultipleDataInitFix2
DEFINE_JUMP(LJMP, 0x53173F, 0x531749);

//this hook taking a lot of time , i guess because of UnitTypeClass::InitOneTimeData thing
// ASMJIT_PATCH(0x531726, Game_BulkDataInit_MultipleDataInitFix, 5)
// {
// 	BuildingTypeClass::InitOneTimeData();
// 	UnitTypeClass::InitOneTimeData();
// 	return 0x531749;
// }

DEFINE_PATCH(0x535DB9, 0x01);

//ASMJIT_PATCH(0x535DB6, SetStructureTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535DC2;
//}

DEFINE_PATCH(0x535E79, 0x01);

//ASMJIT_PATCH(0x535E76, SetDefenseTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535E82;
//}

ASMJIT_PATCH(0x4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 7)
{
	GET_STACK(SHPStruct*, pBackground, 0xAC);

	if (pBackground) {
		GameDelete<true, false>(std::exchange(pBackground , nullptr));
	}

	LEA_STACK(SHPStruct**, pSwipeAnims, 0x290);

	for (auto i = 0; i < 4; ++i) {
		if (auto pAnim = pSwipeAnims[i]) {
			GameDelete<true, false>(std::exchange(pAnim , nullptr));
		}
	}

	return 0x4B9445;
}

// ASMJIT_PATCH(0x67E74A, LoadGame_EarlyLoadSides, 5)
// {
// 	GET(LPSTREAM, pStm, ESI);
//
//	
// 	int length = 0;
// 	LPVOID out;
// 	if (pStm->Read(&length, 4, 0) < 0)
// 	{
// 		return 0x67F7A3;
// 	}
// 	for (int i = 0; i < length; ++i)
// 	{
// 		if ((Imports::OleLoadFromStream.invoke()(pStm, &IIDs::IUnknown, &out)) < 0)
// 		{
// 			return 0x67F7A3;
// 		}
// 	}
//
// 	return 0;
// }

//LoadGame_LateSkipSides
DEFINE_JUMP(LJMP, 0x67F281, 0x67F2BF);

// fix for ultra-fast processors overrunning the performance evaluator function
ASMJIT_PATCH(0x5CB0B1, Game_QueryPerformance, 5)
{
	if (!R->EAX())
	{
		R->EAX(1);
	}

	return 0;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
ASMJIT_PATCH(0x66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}


ASMJIT_PATCH(0x657D3D, MapClass_MinimapChanged_Lock, 6)
{
	RadarClass::RadarEvenSurface->Lock();
	RadarClass::RadarEvenSurface_B->Lock();
	return 0;
}ASMJIT_PATCH_AGAIN(0x657CF2, MapClass_MinimapChanged_Lock, 6)


ASMJIT_PATCH(0x657D8A, MapClass_MinimapChanged_Unlock, 7)
{
	RadarClass::RadarEvenSurface->Unlock();
	RadarClass::RadarEvenSurface_B->Unlock();
	return 0;
}ASMJIT_PATCH_AGAIN(0x657D35, MapClass_MinimapChanged_Unlock, 7)

ASMJIT_PATCH(0x65731F, RadarClass_UpdateMinimap_Lock, 6)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_121C->Lock();
	pRadar->unknown_1220->Lock();
	return 0;
}

ASMJIT_PATCH(0x65757C, RadarClass_UpdateMinimap_Unlock, 8)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_1220->Unlock();
	pRadar->unknown_121C->Unlock();

	return R->EAX() ? 0x657584 : 0x6576A5;
}

ASMJIT_PATCH(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);
	GET(Layer , curLayer , EBP);

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

	if(curLayer != pThis->InWhichLayer()) {
		DisplayClass::Instance->SubmitObject(pThis);
	}

	if(!pThis->IsFallingDown){

		if(pThis->IsABomb && pThis->Health > 0) {

			if (pTechnoType)
			{
				auto const pTechno = static_cast<TechnoClass*>(pThis);

				auto pCell = pTechno->GetCell();
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

				if (!pCell || !pCell->IsClearToMove(pTechnoType->SpeedType, true, true, ZoneType::None, pTechnoType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
					return 0;


				double ratio = pCell->Tile_Is_Water() && !pTechno->OnBridge ?
						  pExt->FallingDownDamage_Water.Get(pExt->FallingDownDamage.Get())
						: pExt->FallingDownDamage.Get();

				int damage = 0;

				if (ratio < 0.0)
					damage = int(pThis->Health * Math::abs(ratio));
				else if (ratio >= 0.0 && ratio <= 1.0)
					damage = int(pThis->GetTechnoType()->Strength * ratio);
				else
					damage = int(ratio);

				pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

				if (pThis->Health > 0 && pThis->IsAlive)
				{
					pThis->IsABomb = false;

					if (pThis->WhatAmI() == AbstractType::Infantry)
					{
						auto pInf = static_cast<InfantryClass*>(pTechno);
						const bool isWater = pCell->Tile_Is_Water();

						if (isWater && pInf->SequenceAnim != DoType::Swim)
							pInf->PlayAnim(DoType::Swim, true, false);
						else if (!isWater && pInf->SequenceAnim != DoType::Guard)
							pInf->PlayAnim(DoType::Guard, true, false);
					}

					return 0x5F413F;

				} else {
					pTechno->UpdatePosition(PCPType::During);
					return 0x5F413F;
				}

			} else {
				int _str = pThis->Health;
				pThis->ReceiveDamage(&_str, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
			}
		}

		//iam confused with the code ..
		//probably calculating the desired destination ..
		return 0x5F405B;
	}

	return 0x5F413F;
}

ASMJIT_PATCH(0x5F5965, ObjectClass_SpawnParachuted_Track, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if ((pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None) {
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = true;
	}
	return 0;
}

ASMJIT_PATCH(0x5F4160, ObjectClass_DropAsBomb_Track, 0x6)
{
	GET(ObjectClass*, pThis, ECX);

	if ((pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None) {
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = true;
	}

	return 0;
}

ASMJIT_PATCH(0x5F3F86, ObjectClass_Update_Track, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if ((pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None) {
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = false;
	}

	return 0;
}

ASMJIT_PATCH(0x413FD2, AircraftClass_Init_Academy, 6)
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
ASMJIT_PATCH(0x41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
{
	GET(AirstrikeClass*, pAirstrike, EDI);
	GET(TechnoClass*, pTarget, ESI);

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

	VocClass::SafeImmedietelyPlayAt(index, &pAirstrike->FirstObject->Location, nullptr);
	pAirstrike->Target = pTarget;

	if(pTarget){
		const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
		pTargetExt->AirstrikeTargetingMe = pAirstrike;
		pTarget->StartAirstrikeTimer(100000);

		if(auto pBld = cast_to<BuildingClass* , false>(pTarget)){
			pBld->IsAirstrikeTargetingMe = true;
			pBld->Mark(MarkType::Redraw);
		}
	}

	//return 0x41D970;
	return 0x41DA0B;
}

ASMJIT_PATCH(0x41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
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

	VocClass::SafeImmedietelyPlayAt(index, &pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

ASMJIT_PATCH(0x6BED08, Game_Terminate_Mouse, 7)
{
	GameDelete<true, false>(R->ECX<SHPStruct*>());
	return 0x6BED34;
}

ASMJIT_PATCH(0x621B80, DSurface_FillRecWithColor, 5)
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

ASMJIT_PATCH(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass*, Target, ESI);
	return (Target && Target->Owner->IsControlledByHuman() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

ASMJIT_PATCH(0x4A76ED, DiskLaserClass_Update_Anim, 7)
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
				pThis->Owner, false, false
			);
		}
	}

	MapClass::FlashbangWarheadAt(pThis->Damage, pWarhead, coords);

	return 0;
}

// ASMJIT_PATCH(0x48a4f9, SelectDamageAnimation_FixNegatives, 6)
// {
// 	GET(int, Damage, EDI);
// 	Damage = abs(Damage);
// 	R->EDI(Damage);
// 	return Damage ? 0x48A4FF : 0x48A618;
// }

//InitGame_Delay
DEFINE_JUMP(LJMP, 0x52CA37, 0x52CA65)

#include <CD.h>

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E04> const Language{};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E00> const LangMD{};
static COMPILETIMEEVAL constant_ptr<const char, 0x840D5C> const LANGMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x840D4C> const LANGUAGE_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884DF8> const RA2MD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884DFC> const RA2 {};
static COMPILETIMEEVAL constant_ptr<const char, 0x82667C> const RA2MD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826674> const RA2_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E48> const CACHEMD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E4C> const CACHE {};
static COMPILETIMEEVAL constant_ptr<const char, 0x82665C> const CACHEMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826650> const CACHE_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E50> const LOCALMD {};
static COMPILETIMEEVAL reference<MixFileClass*, 0x884E54> const LOCAL {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826644> const LOCALMD_MIX {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826638> const LOCAL_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E38> const CONQMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x826838> const CONQMD_MIX {};

bool NOINLINE  __fastcall MixFilesBoostrap() {
	int disk = CD::Disk();
	CD::SetReqCD(-2);

	auto pKey = MixFileClass::Key();

	if (SpawnerMain::Configs::Enabled)
	{
		for (auto& preloadMix : SpawnerMain::GetGameConfigs()->PreloadMixes)
		{
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(preloadMix.c_str(), pKey));
			Debug::LogInfo("Loading Preloaded Mix Name : {} ", preloadMix.c_str());
		}
	}

	for (int i = 99; i >= 0; --i)
	{
		char buffer[256];
		_snprintf(buffer, sizeof(buffer) - 1, GameStrings::EXPANDMD02d(), i);
		RawFileClass _raw(buffer);
		if (_raw.Exists())
		{
			Debug::LogInfo("Loading {}", buffer);
			MixFileClass::Array->push_back(GameCreate<MixFileClass>(buffer, pKey));
		}
	}

	Debug::LogInfo("Loading {}", RA2MD_MIX());
	RA2MD = GameCreate<MixFileClass>(RA2MD_MIX(), pKey);

	if (!RA2MD())
		return false;

	Debug::LogInfo("Loading {}", RA2_MIX());
	RA2 = GameCreate<MixFileClass>(RA2_MIX, pKey);

	if (!RA2())
		return false;

	Debug::LogInfo("Loading {}", CACHEMD_MIX());
	CACHEMD = GameCreate<MixFileClass>(CACHEMD_MIX(), pKey);

	if (!CACHEMD())
		return false;

	Debug::LogInfo("Loading {}", CACHE_MIX());
	CACHE = GameCreate<MixFileClass>(CACHE_MIX(), pKey);

	if (!CACHE())
		return false;

	Debug::LogInfo("Loading {}", LOCALMD_MIX());
	LOCALMD = GameCreate<MixFileClass>(LOCALMD_MIX(), pKey);

	if (!LOCALMD())
		return false;

	Debug::LogInfo("Loading {}", LOCAL_MIX());
	LOCAL = GameCreate<MixFileClass>(LOCAL_MIX, pKey);

	if (!LOCAL())
		return false;

	if (SpawnerMain::Configs::Enabled)
	{
		for (auto& postloadMix : SpawnerMain::GetGameConfigs()->PostloadMixes)
		{
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(postloadMix.c_str(), pKey));
			Debug::LogInfo("Loading Postload Mix Name : {} ", postloadMix.c_str());
		}
	}

	CD::SetReqCD(disk);
	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5301A0, MixFilesBoostrap);

ASMJIT_PATCH(0x6BD7D5, Expand_MIX_Reorg, 7)
{
	StaticVars::aresMIX.reset(GameCreate<MixFileClass>("ares.mix", MixFileClass::Key()));
	if(SpawnerMain::Configs::Enabled) {
		SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>("cncnet.mix", MixFileClass::Key()));
	}

	MixFilesBoostrap();

	Language = GameCreate<MixFileClass>(LANGUAGE_MIX(), MixFileClass::Key());
	LangMD = GameCreate<MixFileClass>(LANGMD_MIX(), MixFileClass::Key());

	return 0x6BD835;
}

DEFINE_JUMP(LJMP, 0x52BB64, 0x52BB95) //Expand_MIX_Deorg

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E58> const NTRLMD {};
static COMPILETIMEEVAL constant_ptr<const char, 0x827DA0> const NTRLMD_MIX {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E5C> const NEUTRAL {};
static COMPILETIMEEVAL constant_ptr<const char, 0x827D80> const NEUTRAL_MIX {};

void NOINLINE __fastcall Release_Neutral()
{
	if (NEUTRAL())
	{
		Debug::LogInfo("Releasing {} ", NEUTRAL_MIX());
		GameDelete<true,false>(NEUTRAL());
		NEUTRAL = nullptr;
	}

	if (NTRLMD())
	{
		Debug::LogInfo("Releasing {} ", NTRLMD_MIX());
		GameDelete<true, false>(NTRLMD());
		NTRLMD = nullptr;
	}
}

bool NOINLINE __fastcall Prep_Neutral()
{
	Release_Neutral();

	if (!Phobos::Otamaa::ExeTerminated)
	{
		Debug::LogInfo("Loading {} ", NTRLMD_MIX());
		NTRLMD = GameCreate<MixFileClass>(NTRLMD_MIX(), MixFileClass::Key());

		if (!NTRLMD())
			return false;

		Debug::LogInfo("Loading {} ", NEUTRAL_MIX());
		NEUTRAL = GameCreate<MixFileClass>(NEUTRAL_MIX(), MixFileClass::Key());

		if (!NEUTRAL())
			return false;
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x534E50, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72AB0A, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72DDBD, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E071, Prep_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E462, Prep_Neutral);

DEFINE_FUNCTION_JUMP(LJMP, 0x534DE0, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72ACFA, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72DFA0, Release_Neutral);
DEFINE_FUNCTION_JUMP(CALL, 0x72E060, Release_Neutral);

//TODO these , aaaa lot
// static inline bool LoadSecondaryMixFiles()
// {
// 	CDFileClass _cd(CONQMD_MIX());
// 	if(_cd.Exists()) {
// 		CONQMD = GameCreate<MixFileClass>(CONQMD_MIX() , MixFileClass::Key());
// 	}

// 	if(!CONQMD())
// 		return false;
// }

// ASMJIT_PATCH(0x530460, InitSecondaryMixfiles, 0x6){
// 	R->EAX(LoadSecondaryMixFiles());
// 	return 0x5304FE;
// }

ASMJIT_PATCH(0x6BE9BD, Game_ProgramEnd_ClearResource, 6)
{
	StaticVars::aresMIX.reset(nullptr);
	if(SpawnerMain::Configs::Enabled) {
		for (auto& Spawner_Mix : SpawnerMain::LoadedMixFiles)
			GameDelete<true>(std::exchange(Spawner_Mix, nullptr));
	}
	return 0;
}

ASMJIT_PATCH(0x531413, Game_Start, 5)
{
	int topActive = 500;
	DSurface::Hidden->DrawText_Old(L"Ares is active.", 10, topActive, COLOR_GREEN);
	DSurface::Hidden->DrawText_Old(L"Ares is © The Ares Contributors 2007 - 2021.", 10, 520, COLOR_GREEN);
	DSurface::Hidden->DrawText_Old(L"Ares version: 3.0p1 Backport", 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}

//
typedef BOOL(__stdcall* FP_MoveWindow)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
static COMPILETIMEEVAL referencefunc<FP_MoveWindow, 0x7E1398> const Game_MoveWindow {};

ASMJIT_PATCH(0x532017, DlgProc_MainMenu_Version, 5)
{
	GET(HWND, hWnd, ESI);

	// account for longer version numbers
	const int MinimumWidth = 168;

	RECT Rect;
	if (Imports::GetWindowRect.invoke()(hWnd, &Rect))
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

			Game_MoveWindow.invoke()(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, FALSE);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x5FACDF, Options_LoadFromINI, 5)
{
	Phobos::Config::Read_RA2MD();
	Phobos::Config::Read_UIMD();
	return 0x0;
}

ASMJIT_PATCH(0x52D21F, Game_InitRules, 0x6)
{
	Phobos::Config::Read_RULESMD();
	return 0x0;
}

ASMJIT_PATCH(0x6BC0CD, _LoadRA2MD, 5)
{
	StaticVars::LoadGlobalsConfig();
	SpawnerMain::LoadConfigurations();
	SpawnerMain::ApplyStaticOptions();

	return 0;
}

ASMJIT_PATCH(0x7C89D4, DDRAW_Create, 6)
{
	R->Stack<DWORD>(0x4, AresGlobalData::GFX_DX_Force);
	return 0;
}
