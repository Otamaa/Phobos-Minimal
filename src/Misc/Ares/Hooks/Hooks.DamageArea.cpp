#include <Ext/WarheadType/Body.h>

#include <Utilities/Helpers.h>

// hook up the area damage delivery with chain reactions
DEFINE_OVERRIDE_HOOK(0x48964F, DamageArea_CellChainReaction, 5)
{
	GET(CellClass*, pCell, EBX);
	pCell->ChainReaction();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4892BE, DamageArea_NullDamage, 0x6)
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

// create enumerator
DEFINE_OVERRIDE_HOOK(0x4895B8, DamageArea_CellSpread1, 0x6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	GET(int, spread, EAX);

	pIter = nullptr;

	if (spread < 0)
		return 0x4899DA;

	pIter = new CellSpreadEnumerator(spread);

	return *pIter ? 0x4895C3 : 0x4899DA;
}

// apply the current value
DEFINE_OVERRIDE_HOOK(0x4895C7, DamageArea_CellSpread2, 0x8)
{
	GET_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));

	auto& offset = **pIter;
	R->DX(offset.X);
	R->AX(offset.Y);

	return 0x4895D7;
}

// advance and delete if done
DEFINE_OVERRIDE_HOOK(0x4899BE, DamageArea_CellSpread3, 0x8)
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
	delete pIter;

	return 0x4899DA;
}

DEFINE_OVERRIDE_HOOK(0x48A2D9, DamageArea_ExplodesThreshold, 6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET_STACK(int, damage, 0x24);

	return pOverlay->Explodes && damage >= RulesExtData::Instance()->OverlayExplodeThreshold
		? 0x48A2E7 : 0x48A433;
}

DEFINE_OVERRIDE_HOOK(0x489E9F, DamageArea_BridgeAbsoluteDestroyer, 5)
{
	GET(WarheadTypeClass*, pWH, EBX);
	GET(WarheadTypeClass*, pIonCannonWH, EDI);
	R->Stack(0x13, WarheadTypeExtContainer::Instance.Find(pWH)->BridgeAbsoluteDestroyer.Get(pWH == pIonCannonWH));
	return 0x489EA4;
}

DEFINE_OVERRIDE_HOOK(0x489FD8, DamageArea_BridgeAbsoluteDestroyer2, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A004 : 0x489FE0;
}

DEFINE_OVERRIDE_HOOK(0x48A15D, DamageArea_BridgeAbsoluteDestroyer3, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A188 : 0x48A165;
}

DEFINE_OVERRIDE_HOOK(0x48A229, DamageArea_BridgeAbsoluteDestroyer4, 6)
{
	return  R->Stack<bool>(0xF) ? 0x48A250 : 0x48A231;
}

DEFINE_OVERRIDE_HOOK(0x48A283, DamageArea_BridgeAbsoluteDestroyer5, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A2AA : 0x48A28B;
}

DEFINE_OVERRIDE_HOOK(0x4893BA, DamageArea_DamageAir, 9)
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
	auto const damageAir = heightAboveGround > pExt->DamageAirThreshold;

	return damageAir ? 0x4893C3u : 0x48955Eu;
}

#include <Lib/SimpleVector.h>

// #895990: limit the number of times a warhead with
// CellSpread will hit the same object for each hit
DEFINE_OVERRIDE_HOOK(0x4899DA, DamageArea_Damage_MaxAffect, 7)
{
 	struct DamageGroup
 	{
 		ObjectClass* Target;
 		int Distance;
 	};

 	REF_STACK(DynamicVectorClass<DamageGroup*>, groups, STACK_OFFS(0xE0, 0xA8));
 	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

 	auto pExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
 	const int MaxAffect = pExt->CellSpread_MaxAffect;

 	if (MaxAffect < 0)
 	{
 		return 0;
 	}

	constexpr size_t const DefaultSize = 1000;
	SimpleDynVecClass<ObjectClass*> handled(DefaultSize);
	SimpleDynVecClass<DamageGroup**> target(DefaultSize);

 	for (auto& group : groups) {
 		// group could have been cleared by previous iteration.
 		// only handle if has not been handled already.
 		if (group && handled.Add_Unique(group->Target)) {
 			target.Set_Count(0);

 			// collect all slots containing damage groups for this target
 			std::for_each(&group, groups.end(), [group, &target](DamageGroup*& item) {
 			 if (item && item->Target == group->Target) {
 					target.Add(&item);
 				}
 			});

 			// if more than allowed, sort them and remove the ones further away
 			if (target.Count() > MaxAffect) {
 				Helpers::Alex::selectionsort(
 					target.begin(), target.begin() + MaxAffect, target.end(),
 					[](DamageGroup** a, DamageGroup** b) {
 						return (*a)->Distance < (*b)->Distance;
 					});

				std::for_each(target.begin() + MaxAffect, target.end(), [](DamageGroup** ppItem) {
					GameDelete(*ppItem);
					*ppItem = nullptr;
				});
 			}
 		}
 	}

 	// move all the empty ones to the back, then remove them
 	auto const end = std::remove_if(groups.begin(), groups.end(), [](DamageGroup* pGroup) {
 		return pGroup == nullptr;
 	});

 	groups.Count = std::distance(groups.begin(), end);

 	return 0;
}

DEFINE_OVERRIDE_HOOK(0x489562, DamageArea_DestroyCliff, 9)
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
