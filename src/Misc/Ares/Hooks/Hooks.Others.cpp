
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

#include "Header.h"

#include <Misc/Spawner/Main.h>

DEFINE_HOOK(0x52C5E0, Ares_NOLOGO, 0x7)
{
	if(SpawnerMain::Configs::Enabled)
		return 0x52C5F8; //skip showing looading screen

	return Phobos::Otamaa::NoLogo ? 0x52C5F3 : 0x0;
}

DEFINE_HOOK(0x62A020, ParasiteClass_Update, 0xA)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExtContainer::Instance.Find(pOwner)->idxSlot_Parasite));
	return 0x62A02A;
}

DEFINE_HOOK(0x62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass*, pOwner, ECX);
	R->EAX(pOwner->GetWeapon(TechnoExtContainer::Instance.Find(pOwner)->idxSlot_Parasite));
	return 0x62A7BA;
}

DEFINE_HOOK(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
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

constexpr int cell_Distance_Squared(CoordStruct& our_coord  , CoordStruct& their_coord)
{
    int our_cell_x = our_coord.X / Unsorted::LeptonsPerCell;
    int their_cell_x = their_coord.X / Unsorted::LeptonsPerCell;
    int our_cell_y = our_coord.Y / Unsorted::LeptonsPerCell;
    int their_cell_y = their_coord.Y / Unsorted::LeptonsPerCell;

    int x_distance = our_cell_x - their_cell_x;
    int y_distance = our_cell_y - their_cell_y;
    return x_distance * x_distance + y_distance * y_distance;
}

DEFINE_HOOK(0x5F6500, AbstractClass_Distance2DSquared_1, 8)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pThat, 0x4);

	int nResult = 0;
	if (pThat)
	{
		auto nThisCoord = pThis->GetCoords();
		auto nThatCoord = pThat->GetCoords();
		nResult = //(int)nThisCoord.DistanceFromXY(nThatCoord)
		cell_Distance_Squared(nThisCoord, nThatCoord);
		;
	}

	R->EAX(nResult);
	return 0x5F655D;
}

DEFINE_HOOK(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);
	R->EAX(
		//(int)nThisCoord.DistanceFromXY(*pThatCoord)
		cell_Distance_Squared(nThisCoord, *pThatCoord)
		);
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

DEFINE_JUMP(LJMP, 0x6AD0ED, 0x6AD16C);

DEFINE_HOOK(0x437CCC, BSurface_DrawSHPFrame1_Buffer, 0x8)
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

DEFINE_HOOK(0x7387D1, UnitClass_Destroyed_Shake, 0x6)
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
DEFINE_HOOK(0x5F77F0, ObjectTypeClass_UnloadPipsSHP, 0x5)
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
DEFINE_HOOK(0x56BC54, ThreatPosedEstimates_GetIndex, 0x5)
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
DEFINE_HOOK(0x4C6DDB, Networking_RespondToEvent_Selling, 0x8)
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

DEFINE_HOOK(0x699C1C, Game_ParsePKTs_ClearFile, 0x7)
{
	LEA_STACK(CCINIClass*, pINI, 0x24);
	pINI->Clear(nullptr, nullptr);
	return 0;
}

// Guard command failure
DEFINE_HOOK(0x730DB0, GuardCommandClass_Execute, 0xA)
{
	GET(TechnoClass*, T, ESI);
	return (T->Owner != HouseClass::CurrentPlayer() || !T->IsControllable())
		? 0x730E62
		: 0x730DBE
		;
}

/* #367 - do we need to draw a link to this victim */
DEFINE_HOOK(0x472198, CaptureManagerClass_DrawLinks, 0x6)
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

DEFINE_HOOK(0x551A30, LayerClass_YSortReorder, 0x5)
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

DEFINE_HOOK(0x5F6612, ObjectClass_UnInit_SkipInvalidation, 0x9)
{
	GET(ObjectClass*, pThis, ESI);

	if (!pThis->Limbo())
		pThis->AnnounceExpiredPointer();

	return 0x5F6625;
}

//speeds up preview drawing by insane amounts
DEFINE_HOOK(0x5FED00, OverlayTypeClass_GetRadarColor, 0x6)
{
	GET(OverlayTypeClass*, ovType, ECX);
	GET_STACK(ColorStruct*, color, 0x04);
	*color = ovType->RadarColor;
	R->EAX<ColorStruct*>(color);
	return 0x5FEDDA;
}

// only eject the parasite if the unit leaves the battlefield,
// not just when it goes out of sight.
DEFINE_HOOK(0x62A283, ParasiteClass_PointerGotInvalid_Cloak, 0x9)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(void*, ptr, EAX);
	GET_STACK(bool, remove, 0x2C);

	// remove only if pointer really got removed
	return (remove && pThis->Victim == ptr) ? 0x62A28C : 0x62A484;
}

/* #746 - don't set parasite eject point to cell center, but set it to fall and explode like a bomb */
DEFINE_HOOK(0x62A2F8, ParasiteClass_PointerGotInvalid, 0x6)
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

//Handle_Static_Messages_LoopingMovie
DEFINE_JUMP(LJMP, 0x615BD3, 0x615BE0);
//sub_789960_RemoveWOLResolutionCheck
DEFINE_JUMP(LJMP, 0x78997B, 0x789A58);
//DSurface_CTOR_SkipVRAM
DEFINE_JUMP(LJMP, 0x4BA61B, 0x4BA623);

DEFINE_HOOK(0x74A884, VoxelAnimClass_UpdateBounce_Damage, 0x6)
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

DEFINE_HOOK(0x545904, IsometricTileTypeClass_CreateFromINIList_MediansFix, 0x7)
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
//DEFINE_HOOK(0x47F9A4, CellClass_DrawOverlay_WallRemap, 0x6)
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
DEFINE_STRONG_HOOK(0x4CA437, FactoryClass_GetCRC, 0x8)
{
	GET(FactoryClass*, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass*>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

DEFINE_HOOK(0x47243F, CaptureManagerClass_DecideUnitFate_BuildingFate, 0x6)
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
DEFINE_HOOK(0x671152, RulesClass_Addition_General_PrismSupportModifier, 0x6)
{
	GET(RulesClass*, pThis, ESI);
	REF_STACK(double, param, 0x0);
	param = pThis->PrismSupportModifier / 100.0;
	return 0x67115B;
}

DEFINE_HOOK(0x6B72F9, SpawnManagerClass_Update_Buildings, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(SpawnNode, nNode, EAX);

	auto const pOwner = pThis->Owner;
	return (nNode.Status != SpawnNodeStatus::TakeOff
		|| !pOwner
		|| pOwner->WhatAmI() == BuildingClass::AbsID)
		? 0x6B735C : 0x6B72FE;
}

DEFINE_HOOK(0x725A1F, AnnounceInvalidPointer_SkipBehind, 0x5)
{
	GET(AnimClass*, pAnim, ESI);
	return pAnim->Type == RulesClass::Instance->Behind ?
		0x725C08 : 0x0;
}

//sub_731D90_FakeOf
DEFINE_HOOK(0x731E08, Select_By_Units_Text_FakeOf, 0x6)
{
	int nCost = 0;

	for (const auto pObj : ObjectClass::CurrentObjects())
	{
		if (const auto pTechno = generic_cast<const TechnoClass*>(pObj))
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

			TechnoTypeClass* pType = pTypeExt->AttachedToObject;
			if (pTypeExt->Fake_Of)
				pType = pTypeExt->Fake_Of;

			nCost += pType->GetActualCost(pTechno->Owner);
		}
	}

	R->EBX(nCost);
	return 0x731E4D;
}

DEFINE_HOOK(0x6DA665, sub_6DA5C0_GroupAs, 0xA)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExtData::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

DEFINE_HOOK(0x7BB445, XSurface_20, 0x6)
{
	return R->EAX<void*>() ? 0x0 : 0x7BB90C;
}

// DEFINE_HOOK(0x5FDDA4, OverlayClass_GetTiberiumType_NotReallyTiberiumLog, 0x6)
// {
// 	GET(OverlayTypeClass*, pThis, EAX);
// 	Debug::Log("Overlay %s not really tiberium\n", pThis->ID);
// 	return 0x5FDDC1;
// }

DEFINE_HOOK(0x716D98, TechnoTypeClass_Load_Palette, 0x5)
{
	GET(TechnoTypeClass*, pThis, EDI);

	pThis->Palette = nullptr;
	return pThis->PaletteFile[0] == 0 ? 0x716DAA : 0x716D9D;
}

// this was only a leftover stub from TS. reimplemented
// using the same mechanism.
DEFINE_HOOK(0x489270, CellChainReact, 5)
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

DEFINE_HOOK(0x424DD3, AnimClass_ReInit_TiberiumChainReaction_Chance, 6)
{
	GET(TiberiumClass*, pTib, EDI);

	return ScenarioClass::Instance->Random.RandomFromMax(99) < TiberiumExtContainer::Instance.Find(pTib)->GetDebrisChance()
		? 0x424DF9 : 0x424E9B;
}

DEFINE_HOOK(0x424EC5, AnimClass_ReInit_TiberiumChainReaction_Damage, 6)
{
	GET(TiberiumClass*, pTib, EDI);
	auto pExt = TiberiumExtContainer::Instance.Find(pTib);

	R->Stack(0x0, pExt->GetExplosionWarhead());
	R->EDX(pExt->GetExplosionDamage());

	return 0x424ECB;
}

DEFINE_HOOK(0x71C5D2, TerrainClass_Ignite_IsFlammable, 6)
{
	GET(TerrainClass*, pThis, EDI);

	enum { Ignite = 0x71C5F3, CantBurn = 0x71C69D };

	// prevent objects from burning that aren't flammable also
	return (pThis->Type->SpawnsTiberium || !pThis->Type->IsFlammable)
		? CantBurn : Ignite;
}

DEFINE_HOOK(0x6AB8BB, SelectClass_ProcessInput_BuildTime, 6)
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

DEFINE_HOOK(0x711EE0, TechnoTypeClass_GetBuildSpeed, 6)
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
// DEFINE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix, 5)
// {
// 	BuildingTypeClass::InitOneTimeData();
// 	UnitTypeClass::InitOneTimeData();
// 	return 0x531749;
// }

DEFINE_PATCH(0x535DB9, 0x01);

//DEFINE_HOOK(0x535DB6, SetStructureTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535DC2;
//}

DEFINE_PATCH(0x535E79, 0x01);

//DEFINE_HOOK(0x535E76, SetDefenseTabCommandClass_Execute_Power, 6)
//{
//	GET(BuildingClass*, pBuild, EAX);
//	R->EAX(pBuild->FindFactory(false, true));
//	return 0x535E82;
//}

DEFINE_HOOK(0x4B93BD, ScenarioClass_GenerateDropshipLoadout_FreeAnims, 7)
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

DEFINE_HOOK(0x67E74A, LoadGame_EarlyLoadSides, 5)
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

//LoadGame_LateSkipSides
DEFINE_JUMP(LJMP, 0x67F281, 0x67F2BF);

// fix for ultra-fast processors overrunning the performance evaluator function
DEFINE_HOOK(0x5CB0B1, Game_QueryPerformance, 5)
{
	if (!R->EAX())
	{
		R->EAX(1);
	}

	return 0;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
DEFINE_HOOK(0x66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}

DEFINE_HOOK_AGAIN(0x657CF2, MapClass_MinimapChanged_Lock, 6)
DEFINE_HOOK(0x657D3D, MapClass_MinimapChanged_Lock, 6)
{
	RadarClass::RadarEvenSurface->Lock();
	RadarClass::RadarEvenSurface_B->Lock();
	return 0;
}

DEFINE_HOOK_AGAIN(0x657D35, MapClass_MinimapChanged_Unlock, 7)
DEFINE_HOOK(0x657D8A, MapClass_MinimapChanged_Unlock, 7)
{
	RadarClass::RadarEvenSurface->Unlock();
	RadarClass::RadarEvenSurface_B->Unlock();
	return 0;
}

DEFINE_HOOK(0x65731F, RadarClass_UpdateMinimap_Lock, 6)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_121C->Lock();
	pRadar->unknown_1220->Lock();
	return 0;
}

DEFINE_HOOK(0x65757C, RadarClass_UpdateMinimap_Unlock, 8)
{
	GET(RadarClass*, pRadar, ESI);
	pRadar->unknown_1220->Unlock();
	pRadar->unknown_121C->Unlock();

	return R->EAX() ? 0x657584 : 0x6576A5;
}

DEFINE_HOOK(0x4B769B, ScenarioClass_GenerateDropshipLoadout, 5)
{
	WWKeyboardClass::Instance->Clear();
	WWMouseClass::Instance->ShowCursor();
	return 0x4B76A0;
}

DEFINE_HOOK(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);
	GET(Layer , curLayer , EAX);

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
					pTechno->UpdatePosition((int)PCPType::During);
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

// temporal per-slot
DEFINE_HOOK(0x71A84E, TemporalClass_UpdateA, 5)
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

DEFINE_HOOK(0x413FD2, AircraftClass_Init_Academy, 6)
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
DEFINE_HOOK(0x41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
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

DEFINE_HOOK(0x41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
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

//DEFINE_SKIP_HOOK(0x71B09C, TemporalClass_Logic_BuildingUnderAttack_NullptrShit, 0x5, 71B0E7);
DEFINE_JUMP(LJMP, 0x71B09C, 0x71B0E7);

DEFINE_HOOK(0x6BED08, Game_Terminate_Mouse, 7)
{
	GameDelete<true, false>(R->ECX<SHPStruct*>());
	return 0x6BED34;
}

DEFINE_HOOK(0x621B80, DSurface_FillRecWithColor, 5)
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

DEFINE_HOOK(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
{
	GET(TechnoClass*, Target, ESI);
	return (Target && Target->Owner->IsControlledByHuman() && Target->WhatAmI() == AbstractType::Building)
		? 0x4ABFCE
		: 0x4AC294
		;
}

DEFINE_HOOK(0x4A76ED, DiskLaserClass_Update_Anim, 7)
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

// DEFINE_HOOK(0x48a4f9, SelectDamageAnimation_FixNegatives, 6)
// {
// 	GET(int, Damage, EDI);
// 	Damage = abs(Damage);
// 	R->EDI(Damage);
// 	return Damage ? 0x48A4FF : 0x48A618;
// }

//InitGame_Delay
DEFINE_JUMP(LJMP, 0x52CA37, 0x52CA65)

DEFINE_HOOK(0x5f5add, ObjectClass_SpawnParachuted_Animation, 6)
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

DEFINE_STRONG_HOOK(0x6BD7D5, Expand_MIX_Reorg, 7)
{
	StaticVars::aresMIX.reset(GameCreate<MixFileClass>("ares.mix"));
	if(SpawnerMain::Configs::Enabled) {
		SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>("cncnet.mix"));
	}

	MixFileClass::Bootstrap();
	R->EAX(YRMemory::Allocate(sizeof(MixFileClass)));
	return 0x6BD7DF;
}

DEFINE_JUMP(LJMP, 0x52BB64, 0x52BB95) //Expand_MIX_Deorg

DEFINE_HOOK(0x5301AC, InitBootstrapMixfiles_CustomMixes_Preload, 0x5)
{
	if(SpawnerMain::Configs::Enabled) {
		for(auto& preloadMix : SpawnerMain::GetGameConfigs()->PreloadMixes) {
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(preloadMix.c_str()));
			Debug::Log("Loading Preloaded Mix Name : %s \n", preloadMix.c_str());
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x53044A, InitBootstrapMixfiles_CustomMixes_Postload, 0x6)
{
	if(SpawnerMain::Configs::Enabled) {
		for(auto& postloadMix : SpawnerMain::GetGameConfigs()->PostloadMixes) {
			SpawnerMain::LoadedMixFiles.push_back(GameCreate<MixFileClass>(postloadMix.c_str()));
			Debug::Log("Loading Postload Mix Name : %s \n", postloadMix.c_str());
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x7cd819, ExeRun, 5)
{
	Game::Savegame_Magic = AresGlobalData::InternalVersion;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	return 0;
}

DEFINE_HOOK(0x6BE9BD, Game_ProgramEnd_ClearResource, 6)
{
	StaticVars::aresMIX.reset(nullptr);
	if(SpawnerMain::Configs::Enabled) {
		for (auto& Spawner_Mix : SpawnerMain::LoadedMixFiles)
			GameDelete<true>(std::exchange(Spawner_Mix, nullptr));
	}
	return 0;
}

DEFINE_HOOK(0x531413, Game_Start, 5)
{
	int topActive = 500;

	DSurface::Hidden->DrawText_Old(L"Ares is active.", 10, topActive, COLOR_GREEN);
	DSurface::Hidden->DrawText_Old(L"Ares is © The Ares Contributors 2007 - 2021.", 10, 520, COLOR_GREEN);

	DSurface::Hidden->DrawText_Old(L"Ares version: 3.0p1 Backport", 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}

DEFINE_HOOK(0x532017, DlgProc_MainMenu_Version, 5)
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

DEFINE_HOOK(0x5facdf, Options_LoadFromINI, 5)
{
	Phobos::Config::Read();
	return 0x0;
}

DEFINE_HOOK(0x6BC0CD, _LoadRA2MD, 5)
{
	StaticVars::LoadGlobalsConfig();
	SpawnerMain::LoadConfigurations();
	SpawnerMain::ApplyStaticOptions();

	return 0;
}

DEFINE_HOOK(0x6d4b25, TacticalClass_Draw_TheDarkSideOfTheMoon, 6)
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

	if (!AresGlobalData::ModNote.Label) {
		AresGlobalData::ModNote = "TXT_RELEASE_NOTE";
	}

	if (!AresGlobalData::ModNote.empty()) {
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

DEFINE_STRONG_HOOK(0x7C89D4, DDRAW_Create, 6)
{
	R->Stack<DWORD>(0x4, AresGlobalData::GFX_DX_Force);
	return 0;
}
