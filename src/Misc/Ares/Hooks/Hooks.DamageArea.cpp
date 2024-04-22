#include <Ext/WarheadType/Body.h>

#include <Utilities/Helpers.h>
#include <Constructable.h>

#include <Misc/PhobosGlobal.h>

// hook up the area damage delivery with chain reactions
DEFINE_HOOK(0x48964F, DamageArea_CellChainReaction, 5)
{
	GET(CellClass*, pCell, EBX);
	pCell->ChainReaction();
	return 0;
}

DEFINE_HOOK(0x4892BE, DamageArea_NullDamage, 0x6)
{
	enum
	{
		DeleteDamageAreaVector = 0x48A4B7,
		ContinueFunction = 0x4892DD,
	};

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	GET(int, Damage, ESI);

	if (!pWarhead
		|| ((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x20) != 0)
		|| !Damage && !WarheadTypeExtContainer::Instance.Find(pWarhead)->AllowZeroDamage)
		return DeleteDamageAreaVector;

	R->ESI(pWarhead);
	return ContinueFunction;
}

static CellSpreadEnumerator CellSpreadenumerator {};

// create enumerator
DEFINE_HOOK(0x4895B8, DamageArea_CellSpread1, 0x6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, 0xE0 - 0xB4);
	GET(int, spread, EAX);

	pIter = nullptr;

	if (spread < 0)
		return 0x4899DA;

	//to avoid unnessesary allocation check
	//simplify the assembly result
	//pIter = DLLCreate<CellSpreadEnumerator>(spread);
	pIter = &CellSpreadenumerator;
	pIter->setSpread(spread);

	return *pIter ? 0x4895C3 : 0x4899DA;
}

// apply the current value
DEFINE_HOOK(0x4895C7, DamageArea_CellSpread2, 0x8)
{
	GET_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));

	R->DX((*pIter)->X);
	R->AX((*pIter)->Y);

	return 0x4895D7;
}

// advance and delete if done
DEFINE_HOOK(0x4899BE, DamageArea_CellSpread3, 0x8)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	REF_STACK(int, index, STACK_OFFS(0xE0, 0xD0));

	// reproduce skipped instruction
	index++;

	// advance iterator
	if (++*pIter)
	{
		return 0x4895C0;
	}

	// all done. delete and go on
	//DLLDelete<false>(pIter);
	pIter->clear();
	return 0x4899DA;
}

//DEFINE_HOOK(0x4896EC, MapClass_DamageArea_DamageSelf, 0x6) {
//	GET(ObjectClass*, pObj, ECX);
//	GET(TechnoTypeClass*, pType, EAX);
//	GET_BASE(WarheadTypeClass*, pWH, 0xC);
//
//	if (!pType->DamageSelf ) {
//		Debug::Log("Techno[%x - %s] Trying to damage itself with Warhead [%s] , but DamageSelf is off , is this intended ?\n", pObj, pType->ID, pWH->ID);
//		return 0x4896F6;
//	}
//
//	return 0x489702;
//}

DEFINE_HOOK(0x48A2D9, DamageArea_ExplodesThreshold, 6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET_STACK(int, damage, 0x24);

	return pOverlay->Explodes && damage >= RulesExtData::Instance()->OverlayExplodeThreshold
		? 0x48A2E7 : 0x48A433;
}

DEFINE_HOOK(0x489E9F, DamageArea_BridgeAbsoluteDestroyer, 5)
{
	GET(WarheadTypeClass*, pWH, EBX);
	GET(WarheadTypeClass*, pIonCannonWH, EDI);
	R->Stack(0x13, WarheadTypeExtContainer::Instance.Find(pWH)->BridgeAbsoluteDestroyer.Get(pWH == pIonCannonWH));
	return 0x489EA4;
}

DEFINE_HOOK(0x489FD8, DamageArea_BridgeAbsoluteDestroyer2, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A004 : 0x489FE0;
}

DEFINE_HOOK(0x48A15D, DamageArea_BridgeAbsoluteDestroyer3, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A188 : 0x48A165;
}

DEFINE_HOOK(0x48A229, DamageArea_BridgeAbsoluteDestroyer4, 6)
{
	return  R->Stack<bool>(0xF) ? 0x48A250 : 0x48A231;
}

DEFINE_HOOK(0x48A283, DamageArea_BridgeAbsoluteDestroyer5, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A2AA : 0x48A28B;
}

DEFINE_HOOK(0x4893BA, DamageArea_DamageAir, 0x9)
{
	GET(const CoordStruct* const, pCoords, EDI);
	GET(WarheadTypeClass*, pWarhead, ESI);
	GET(int const, heightFloor, EAX);
	GET_STACK(const CellClass*, pCell, STACK_OFFS(0xE0, 0xC0));

	int heightAboveGround = pCoords->Z - heightFloor;

	// consider explosions on and over bridges
	if (heightAboveGround > Unsorted::BridgeHeight
		&& pCell->ContainsBridge()
		&& RulesExtData::Instance()->DamageAirConsiderBridges)
	{
		heightAboveGround -= Unsorted::BridgeHeight;
	}

	// damage units in air if detonation is above a threshold
	auto const pExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	return heightAboveGround > pExt->DamageAirThreshold ? 0x4893C3u : 0x48955Eu;
}

// #895990: limit the number of times a warhead with
// CellSpread will hit the same object for each hit
//#include <AircraftTrackerClass.h>

//void DamageArea(CoordStruct& coord, int damage, TechnoClass* source, WarheadTypeClass* warhead, bool affectTiberium, HouseClass* house) {
//
//	if (!damage || ((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x20) != 0) || !warhead)
//		return;
//
//	static std::vector<DamageGroup> DamageAres {};
//
//	auto spreadLept = warhead->CellSpread * 256.0;
//	const bool isCrushWarhead = RulesClass::Instance->CrushWarhead == warhead;
//	CellStruct cell = CellClass::Coord2Cell(coord);
//	auto pCell = MapClass::Instance->TryGetCellAt(cell);
//	auto spread = warhead->CellSpread;
//	bool OnBridge = false;
//	bool spreadLow = spread < 0.5;
//	CoordStruct Coord = CellClass::Cell2Coord(cell);
//	bool NotAllowed = false;
//	int coord_Z = MapClass::Instance->GetCellFloorHeight(Coord);
//
//	if (coord_Z < coord.Z) {
//		AircraftTrackerClass::Instance->AircraftTrackerClass_logics_412B40(pCell, (int)spread);
//		auto Ent = AircraftTrackerClass::Instance->Get();
//
//		if (Ent) {
//			do{
//				if (Ent->IsAlive && Ent->IsOnMap && Ent->Health > 0) {
//					auto difference = coord - Ent->Location;
//					auto len = difference.Length();
//					if (len < spreadLept) {
//						if (spreadLow && (int)len < 85 && Ent->IsIronCurtained() && Ent->ProtectType == ProtectTypes::IronCurtain) {
//							NotAllowed = true;
//						}
//
//						DamageAres.emplace_back(Ent, len);
//					}
//				}
//
//				Ent = AircraftTrackerClass::Instance->Get();
//			}
//			while (Ent);
//		}
//	}
//
//	if (pCell->ContainsBridge() && coord.Z > (Unsorted::BridgeHeight / 2 + coord_Z)) {
//		OnBridge = true;
//	}
//}

static DynamicVectorClass<ObjectClass*, DllAllocator<ObjectClass*>> Targets {};
static DynamicVectorClass<DamageGroup*, DllAllocator<DamageGroup*>> Handled {};

//DEFINE_HOOK(0x4896D5, MapClass_DamageArea_FirstOccupy, 0x8) {
//	GET(CellClass*, pCell, EBX);
//	GET(ObjectClass*, pOccupy, ESI);
//	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
//
//	if (IS_SAME_STR_("Fire2", pWarhead->ID) && (pOccupy == PhobosGlobal::Instance()->AnimAttachedto)){
//		auto coord = pCell->GetCoords();
//		Debug::Log(__FUNCTION__" Executed with technoCoord (%d %d %d) \n" , coord.X , coord.Y , coord.Z);
//	}
//
//	return pOccupy  ?  0x4896DD : 0x4899BE;
//}

DEFINE_HOOK(0x4899DA, DamageArea_Damage_MaxAffect, 7)
{
	REF_STACK(DamageGroup**, items, 0x3C);
	REF_STACK(int, count, 0x48);
	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	const int MaxAffect = WarheadTypeExtContainer::Instance.Find(pWarhead)->CellSpread_MaxAffect;

	if (MaxAffect > 0) {

		Targets.Reset();
		Handled.Reset();

		const auto g_end = items + count;

		for (auto g_begin = items; g_begin != g_end; ++g_begin) {

			DamageGroup* group = *g_begin;
			// group could have been cleared by previous iteration.
			// only handle if has not been handled already.
			if (group && Targets.AddUnique(group->Target)) {

				Handled.Reset();

				// collect all slots containing damage groups for this target
				std::for_each(g_begin, g_end, [group](DamageGroup* item) {
					if (item && item->Target == group->Target) {
						Handled.AddItem(item);
					}
				});

				// if more than allowed, sort them and remove the ones further away
				if (Handled.Count > MaxAffect) {
					Helpers::Alex::selectionsort(
						Handled.begin(), Handled.begin() + MaxAffect, Handled.end(),
						[](DamageGroup* a, DamageGroup* b) {
							return a->Distance < b->Distance;
						});

					std::for_each(Handled.begin() + MaxAffect, Handled.end(), [](DamageGroup* ppItem) {
						ppItem->Target = nullptr;
					});
				}
			}
		}

		// move all the empty ones to the back, then remove them
		auto const end = std::remove_if(items, &items[count], [](DamageGroup* pGroup)  {
			if (!pGroup->Target)
			{
				GameDelete<false, false>(pGroup);
				return true;
			}

		   return false;
		});

		count = int(std::distance(items, end));
	}

	GET_STACK(bool, Something, 0x17);
	GET_STACK(int, idamage, 0x24);
	GET_STACK(int, distance, 0x68);
	GET_BASE(TechnoClass*, pSource, 0x8);
	GET_BASE(HouseClass*, pHouse, 0x14);

	//if (IS_SAME_STR_("Fire2", pWarhead->ID) && PhobosGlobal::Instance()->AnimAttachedto)
	//	Debug::Log(__FUNCTION__" Executed at [%d] \n", count);

	for (int i = 0; i < count; ++i)
	{
		auto pTarget = *(items + i);
		auto curDistance = pTarget->Distance;
		auto pObj = pTarget->Target;

		if (pObj->IsAlive
		&& (pObj->WhatAmI() != BuildingClass::AbsID || !((BuildingClass*)pObj)->Type->InvisibleInGame)
		  && (!Something
			  || (pObj->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None
			  && ((TechnoClass*)pObj)->IsIronCurtained()))
		{
			if (pObj->WhatAmI() == AircraftClass::AbsID
			  && pObj->IsInAir())
			{
				curDistance /= 2;
			}

			if (pObj->Health > 0 && pObj->IsOnMap && !pObj->InLimbo && curDistance <= distance)
			{
				int Damage = idamage;
				pObj->ReceiveDamage(&Damage, curDistance, pWarhead, pSource, false, false, pHouse);
				R->Stack(0x1F, 1);
			}
		}
	}

	R->ECX(count);
	return 0x489AD6;
}

DEFINE_HOOK(0x489562, DamageArea_DestroyCliff, 9)
{
	GET(CellClass* const, pCell, EAX);

	if (pCell->Tile_Is_DestroyableCliff())
	{
		if (ScenarioClass::Instance->Random.PercentChance(RulesClass::Instance->CollapseChance))
		{
			MapClass::Instance->DestroyCliff(pCell);
		}
	}

	return 0;
}
