#include "DamageArea.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>

#include <Constructable.h>

#include <Misc/PhobosGlobal.h>

#include <AircraftClass.h>
#include <Utilities/Helpers.h>

// #895990: limit the number of times a warhead with
// CellSpread will hit the same object for each hit
#include <AircraftTrackerClass.h>
#include <VeinholeMonsterClass.h>
#include <ParticleSystemClass.h>
#include <OverlayTypeClass.h>
#include <VoxelAnimClass.h>
#include <TacticalClass.h>
#include <AnimClass.h>
#include <SmudgeTypeClass.h>

#include <YRAllocator.h>
#include <CellSpread.h>

static void NOINLINE DestroyBridge(CoordStruct* pCoord, const FakeWarheadTypeClass* const  pWarhead, int damage, CellClass* pCell, CellStruct* pCellStruct)
{
	static COMPILETIMEEVAL reference<int, 0xAA0E28> BridgeSet {};
	static COMPILETIMEEVAL reference<int, 0xABAD30> BridgeMiddle1 {};
	static COMPILETIMEEVAL reference<int, 0xAA1028> BridgeMiddle2 {};
	static COMPILETIMEEVAL reference<int, 0xABAD1C> WoodBridgeSet {};

	if (((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x80u) != 0) && pWarhead->WallAbsoluteDestroyer)
	{
		const bool v98 = pWarhead->_GetExtData()->BridgeAbsoluteDestroyer.Get(pWarhead == RulesClass::Instance->IonCannonWarhead);

		int v64 = pCell->IsoTileTypeIndex - BridgeSet + 1;
		CellClass* v104 = pCell->GetBridgeOwner();
		int v67 = 0;
		int v68 = 0;

		if (v104 && (v104->OverlayTypeIndex == 24 || v104->OverlayTypeIndex == 25))
		{
			v67 = BridgeMiddle1;
		}
		else
		{
			v67 = BridgeMiddle1;

			if (v64 != BridgeMiddle1 && v64 != BridgeMiddle1 + 1 && v64 != BridgeMiddle1 + 2 && v64 != BridgeMiddle1 + 3)
			{
				v68 = BridgeMiddle2;
				if (v64 != BridgeMiddle2 && v64 != BridgeMiddle2 + 1 && v64 != BridgeMiddle2 + 2 && v64 != BridgeMiddle2 + 3)
				{
				LABEL_167:
					int v73 = pCell->IsoTileTypeIndex - WoodBridgeSet + 1;
					if (v104 && (v104->OverlayTypeIndex == 237 || v104->OverlayTypeIndex == 238)
					  || v73 == v67
					  || v73 == v67 + 1
					  || v73 == v67 + 2
					  || v73 == v67 + 3
					  || v73 == v68
					  || v73 == v68 + 1
					  || v73 == v68 + 2
					  || v73 == v68 + 3)
					{
						if ((pCell->UINTFlags & 0x100) == 0
						  || (pCoord->Z <= Unsorted::BridgeHeight + Unsorted::LevelHeight * (pCell->Level + 1))
						  && (pCoord->Z > Unsorted::BridgeHeight + Unsorted::LevelHeight * (pCell->Level - 2)))
						{
							if (pWarhead->WallAbsoluteDestroyer)
							{
								if (v98 || ScenarioClass::Instance->Random.RandomRanged(1, RulesClass::Instance->BridgeStrength) < damage)
								{
									int v78 = 3;
									if (MapClass::Instance->findsoemthing_587180(pCellStruct))
									{
										TechnoClass::ClearWhoTargetingThis(pCell);
									}
									else
									{
										while (v98 && v78 > 0)
										{
											--v78;
											if (MapClass::Instance->findsoemthing_587180(pCellStruct))
											{
												TechnoClass::ClearWhoTargetingThis(pCell);
											}
										}
									}

									const auto point = TacticalClass::Instance->CoordsToScreen(pCoord);
									TacticalClass::Instance->RegisterDirtyArea({ point.X - 96 , point.Y - 96 , 192, 192 }, false);
								}
							}
						}
					}

					if (pCell->OverlayTypeIndex >= 74 && pCell->OverlayTypeIndex <= 99)
					{
						if (v98 || ScenarioClass::Instance->Random.RandomRanged(1, RulesClass::Instance->BridgeStrength) < damage)
						{
							if (MapClass::Instance->checkcells_57BAA0(pCellStruct))
							{
								TechnoClass::ClearWhoTargetingThis(pCell);
							}
						}
					}

					if (pCell->OverlayTypeIndex >= 205 && pCell->OverlayTypeIndex <= 230)
					{
						if (v98 || ScenarioClass::Instance->Random.RandomRanged(1, RulesClass::Instance->BridgeStrength) < damage)
						{
							if (MapClass::Instance->checkcells_57CCF0(pCellStruct))
							{
								TechnoClass::ClearWhoTargetingThis(pCell);
							}
						}
					}

					return;
				}

			LABEL_152:
				if ((pCell->UINTFlags & 0x100) == 0
					|| (pCoord->Z <= Unsorted::BridgeHeight + Unsorted::LevelHeight * (pCell->Level + 1))
					&& (pCoord->Z > Unsorted::BridgeHeight + Unsorted::LevelHeight * (pCell->Level - 2)))
				{
					if (pWarhead->WallAbsoluteDestroyer)
					{
						if (v98 || ScenarioClass::Instance->Random.RandomRanged(1, RulesClass::Instance->BridgeStrength))
						{
							int v72 = 3;
							if (MapClass::Instance->findsoemthing_587180(pCellStruct))
							{
								TechnoClass::ClearWhoTargetingThis(pCell);
							}
							else
							{
								while (v98 && v72 > 0)
								{
									--v72;
									if (MapClass::Instance->findsoemthing_587180(pCellStruct))
									{
										TechnoClass::ClearWhoTargetingThis(pCell);
									}
								}
							}

							const auto point = TacticalClass::Instance->CoordsToScreen(pCoord);
							TacticalClass::Instance->RegisterDirtyArea({ point.X - 128 , point.Y - 128 , 256, 256 }, false);
						}
						v67 = BridgeMiddle1;
						v68 = BridgeMiddle2;
					}
				}
				goto LABEL_167;
			}
		}
		v68 = BridgeMiddle2;
		goto LABEL_152;
	}
}

static void NOINLINE Spawn_Flames_And_Smudges(const CellStruct& cell, double scorchChance, double craterChance, double cellAnimChance, NullableVector<AnimTypeClass*>& cellAnims)
{
	CoordStruct cell_coord = CellClass::Cell2Coord(cell);
	cell_coord.Z = MapClass::Instance->GetCellFloorHeight(cell_coord);

	if (ScenarioClass::Instance->Random.ProbabilityOf(scorchChance))
	{
		SmudgeTypeClass::CreateRandomSmudgeFromTypeList(cell_coord, 100, 100, false);
	}
	else if (ScenarioClass::Instance->Random.ProbabilityOf(craterChance))
	{
		SmudgeTypeClass::CreateRandomSmudge(cell_coord, 100, 100, false);
	}

	if (ScenarioClass::Instance->Random.ProbabilityOf(cellAnimChance))
	{
		if (auto pAnims = cellAnims.GetElements(RulesClass::Instance->OnFire))
		{
			if (AnimTypeClass* pType = pAnims.size() == 1 ? pAnims[0] : pAnims[ScenarioClass::Instance->Random.RandomRanged(0, pAnims.size() - 1)])
			{
				GameCreate<AnimClass>(pType, cell_coord);
			}
		}
	}
}

static void NOINLINE Damage_Overlay(CellClass* pCurCell, 
	CellStruct center,
	WarheadTypeClass* pWarhead,
	int distance, 
	int damage, 
	TechnoClass* pSource,
	HouseClass* pHouse,
	bool do_chain_reaction
)
{
	if (pCurCell->OverlayTypeIndex > -1)
	{
		auto pOvelay = OverlayTypeClass::Array->Items[pCurCell->OverlayTypeIndex];
		if (pOvelay->ChainReaction && (!pOvelay->Tiberium || pWarhead->Tiberium) && do_chain_reaction)
		{// hook up the area damage delivery with chain reactions
			pCurCell->ChainReaction();
			pCurCell->ReduceTiberium(damage / 10);
		}

		if (pOvelay->Wall)
		{
			if (pWarhead->WallAbsoluteDestroyer)
			{
				pCurCell->ReduceWall();
			}
			else if (pWarhead->Wall || (pWarhead->Wood && pOvelay->Armor == Armor::Wood))
			{
				pCurCell->ReduceWall(damage);
			}
		}

		if (pCurCell->OverlayTypeIndex == -1)
		{
			TechnoClass::ClearWhoTargetingThis(pCurCell);
		}

		if (pOvelay->IsVeinholeMonster)
		{
			if (VeinholeMonsterClass* veinhole = VeinholeMonsterClass::GetVeinholeMonsterAt(&pCurCell->MapCoords))
			{
				if (!veinhole->InLimbo && veinhole->IsAlive && ((int)distance <= 0))
				{
					int nDamage = damage;
					if (veinhole->ReceiveDamage(&nDamage,
						(int)center.DistanceFrom(veinhole->MonsterCell),
						const_cast<WarheadTypeClass*>(pWarhead),
						pSource,
						false,
						false,
						pSource && !pHouse ? pSource->Owner : pHouse
					) == DamageState::NowDead)
					{
						Debug::LogInfo("Veinhole at [%d %d] Destroyed!", veinhole->MonsterCell.X, veinhole->MonsterCell.Y);

						if (pCurCell->OverlayTypeIndex == -1) {
							TechnoClass::ClearWhoTargetingThis(pCurCell);
						}
					}
				}
			}
		}
	}
}

static PhobosMap<BuildingClass*, double> MergedDamage {};
static DynamicVectorClass<ObjectClass*, DllAllocator<ObjectClass*>> Targets;
static DynamicVectorClass<DamageGroup*, DllAllocator<DamageGroup*>> Handled;

// this function is landmines , hooking it breaking other

DamageAreaResult __fastcall DamageArea::Apply(CoordStruct* pCoord,
		int damage,
		TechnoClass* pSource,
		WarheadTypeClass* pWarhead,
		bool affectTiberium,
		HouseClass* pHouse)
{
	//JMP_STD(0x489280);

	if (!pWarhead)
	{
		return DamageAreaResult::Missed;
	}

	if (VTable::Get(pWarhead) != WarheadTypeClass::vtable)
		Debug::FatalErrorAndExit("!");

	const auto pWHExt = ((FakeWarheadTypeClass*)pWarhead)->_GetExtData();
	CellStruct cell = CellClass::Coord2Cell(*pCoord);

	if (!pWHExt->ShakeIsLocal || TacticalClass::Instance->IsCoordsToClientVisible(*pCoord))
	{

		if (pWarhead->ShakeXhi || pWarhead->ShakeXlo)
			GeneralUtils::CalculateShakeVal(
			GScreenClass::Instance->ScreenShakeX,
			Random2Class::NonCriticalRandomNumber->RandomRanged(pWarhead->ShakeXhi, pWarhead->ShakeXlo), pWHExt->Shake_UseAlternativeCalculation);

		if (pWarhead->ShakeYhi || pWarhead->ShakeYlo)
			GeneralUtils::CalculateShakeVal(
			GScreenClass::Instance->ScreenShakeY,
			Random2Class::NonCriticalRandomNumber->RandomRanged(pWarhead->ShakeYhi, pWarhead->ShakeYlo), pWHExt->Shake_UseAlternativeCalculation);
	}

	auto const pDecidedOwner = !pHouse && pSource ? pSource->Owner : pHouse;

	for (const auto& Lauch : pWHExt->Launchs)
	{
		if (Lauch.LaunchWhat)
		{
			Helpers::Otamaa::LauchSW(Lauch, pDecidedOwner, *pCoord, pSource);
		}
	}

	if (PhobosGlobal::Instance()->DetonateDamageArea)
		pWHExt->Detonate(pSource, pDecidedOwner, nullptr, *pCoord, damage);

	bool alt = false;
	bool HitICEdTechno = false;
	const auto spread = pWarhead->CellSpread;
	const auto spreadLept = spread * 256.0;
	const auto spread_int = (int)spread;
	const bool isCrushWarhead = RulesClass::Instance->CrushWarhead == pWarhead;

	CellClass* pCell = MapClass::Instance->GetCellAt(cell);
	const bool spreadLow = spread <= 0.5;
	CoordStruct aCoord = CellClass::Cell2Coord(cell);
	const int coord_Z = MapClass::Instance->GetCellFloorHeight(aCoord);
	const int coord_Actual = MapClass::Instance->GetCellFloorHeight(pCoord);
	const bool cell_ContainsBridge = pCell->ContainsBridge();

	if (((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x20) != 0) || (!damage && !pWHExt->AllowZeroDamage))
	{
		return DamageAreaResult::Missed;
	}

	HelperedVector<DamageGroup*> groupvec {};

	int heightAboveGround = pCoord->Z - coord_Z;

	// consider explosions on and over bridges
	if (heightAboveGround > Unsorted::BridgeHeight
		&& cell_ContainsBridge
		&& RulesExtData::Instance()->DamageAirConsiderBridges)
	{
		heightAboveGround -= Unsorted::BridgeHeight;
	}

	// damage units in air if detonation is above a threshold
	if (heightAboveGround > pWHExt->DamageAirThreshold)
	{
		AircraftTrackerClass::Instance->AircraftTrackerClass_logics_412B40(pCell, spread_int);

		if (auto Ent = AircraftTrackerClass::Instance->Get())
		{
			do
			{
				if (Ent->IsAlive && Ent->IsOnMap && Ent->Health > 0)
				{
					const auto len = pCoord->operator-(Ent->Location).Length();

					if (len <= spreadLept)
					{
						if (spreadLow && (int)len < 85 && Ent->IsIronCurtained() && Ent->ProtectType == ProtectTypes::IronCurtain)
						{
							HitICEdTechno = !pWHExt->PenetratesIronCurtain;
						}

						groupvec.push_back(GameCreate<DamageGroup>(Ent, (int)len));
					}
				}

				Ent = AircraftTrackerClass::Instance->Get();
			}
			while (Ent);
		}
	}

	if (pCell->Tile_Is_DestroyableCliff())
	{
		if (ScenarioClass::Instance->Random.PercentChance(RulesClass::Instance->CollapseChance))
		{
			MapClass::Instance->DestroyCliff(pCell);
		}
	}

	if (cell_ContainsBridge && (pCoord->Z > (Unsorted::BridgeHeight / 2 + coord_Actual)))
	{
		alt = true;
	}

	if (int(spread + 0.99) >= 0)
	{
		//obtain Object within the spread distance
		for (CellSpreadEnumerator it(short(spread + 0.99), short(0)); it; ++it)
		{
			auto cellhere = (cell + (*it));

			if (auto pCurCell = MapClass::Instance->TryGetCellAt(cellhere))
			{
				if (MapClass::Instance->CoordinatesLegal(cellhere))
				{
					auto cur_cellCoord = pCurCell->GetCoords();
					auto spawn_distance = cellhere.DistanceFrom(cell);
					Damage_Overlay(pCurCell, cell, pWarhead, spawn_distance, damage, pSource, pHouse, affectTiberium);
			
					auto scorch_chance = std::clamp(Math::PercentAtMax(pWHExt->ScorchChance.Get(), spreadLept, spawn_distance, pWHExt->ScorchPercentAtMax.Get()), 0.0, 1.0);
					auto crater_chance = std::clamp(Math::PercentAtMax(pWHExt->CraterChance.Get(), spreadLept, spawn_distance, pWHExt->CraterPercentAtMax.Get()), 0.0, 1.0);
					auto cellanim_chance = std::clamp(Math::PercentAtMax(pWHExt->CellAnimChance.Get(), spreadLept, spawn_distance, pWHExt->CellAnimPercentAtMax.Get()), 0.0, 1.0);

					Spawn_Flames_And_Smudges(cellhere, scorch_chance, crater_chance, cellanim_chance, pWHExt->CellAnim);

					for (NextObject next(alt ? pCurCell->AltObject : pCurCell->FirstObject); next; next++)
					{
						auto pCur = *next;

						if (pCur == pSource && !pWHExt->AllowDamageOnSelf && !isCrushWarhead)
							continue;

						if (!pCur->IsAlive)
							continue;

						const auto what = pCur->WhatAmI();
						//auto pTechno = flag_cast_to<TechnoClass*, false>(pCur);

						if (what == UnitClass::AbsID && ((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x800) != 0))
						{
							if (RulesClass::Instance->HarvesterUnit.FindItemIndex(((UnitClass*)pSource)->Type) != -1)
							{
								continue;
							}
						}

						auto cur_Group = GameCreate<DamageGroup>(pCur, 0);
						groupvec.push_back(cur_Group);

						if (what == BuildingClass::AbsID)
						{
							if (!it.getCurSpread())
							{
								if (!(pCoord->Z - cur_cellCoord.Z <= Unsorted::LevelHeight * 2))
								{
									cur_Group->Distance = (int)((cur_cellCoord - (*pCoord)).Length()) - Unsorted::LevelHeight;
								}

								if (spreadLow)
								{
									if (pCur->IsIronCurtained()
										&& ((BuildingClass*)pCur)->ProtectType == ProtectTypes::IronCurtain
										&& cur_Group->Distance < 85
										)
									{
										HitICEdTechno = !pWHExt->PenetratesIronCurtain;
									}
								}
							}
							else
							{
								cur_Group->Distance = (int)((cur_cellCoord - (*pCoord)).Length()) - Unsorted::CellHeight;
							}
						}
						else
						{
							cur_cellCoord = pCur->GetTargetCoords();
							cur_Group->Distance = (int)((cur_cellCoord - (*pCoord)).Length()) - Unsorted::CellHeight;
						}
					}

				}
			}
		}
	}

	if (pWHExt->CellSpread_MaxAffect > 0)
	{
		Targets.Clear();
		Handled.Clear();

		const auto g_end = groupvec.begin() + groupvec.size();

		for (auto g_begin = groupvec.begin(); g_begin != g_end; ++g_begin)
		{

			DamageGroup* group = *g_begin;
			// group could have been cleared by previous iteration.
			// only handle if has not been handled already.
			if (group && Targets.AddUnique(group->Target))
			{

				Handled.Clear();

				// collect all slots containing damage groups for this target
				std::for_each(g_begin, g_end, [group](DamageGroup* item)
 {
	 if (item && item->Target == group->Target)
	 {
		 Handled.AddItem(item);
	 }
				});

				// if more than allowed, sort them and remove the ones further away
				if ((int)Handled.size() > pWHExt->CellSpread_MaxAffect)
				{
					Helpers::Alex::selectionsort(
						Handled.begin(), Handled.begin() + pWHExt->CellSpread_MaxAffect, Handled.end(),
						[](DamageGroup* a, DamageGroup* b)
						{
							return a->Distance < b->Distance;
						});

					std::for_each(Handled.begin() + pWHExt->CellSpread_MaxAffect, Handled.end(), [](DamageGroup* ppItem)
 {
	 ppItem->Target = nullptr;
					});
				}
			}
		}

		// move all the empty ones to the back, then remove them
		groupvec.remove_all_if([](DamageGroup* pGroup)
{
	if (!pGroup->Target)
	{
		GameDelete<false, false>(pGroup);
		pGroup = nullptr;
		return true;
	}

	return false;
		});
	}

	bool AnythingHit = false;
	const bool merge_bldngDamage = pWHExt->MergeBuildingDamage.Get(RulesExtData::Instance()->MergeBuildingDamage);
	MergedDamage.clear();

	if (merge_bldngDamage)
	{
		for (auto it = groupvec.begin(); it != groupvec.end(); ++it)
		{
			auto pGroup = *it;
			auto curDistance = pGroup->Distance;
			auto pObj = pGroup->Target;
			if (const auto pBuilding = cast_to<BuildingClass*, false>(pObj))
			{
				MergedDamage[pBuilding] += (1.0 - (1.0 - pWarhead->PercentAtMax) * curDistance / (spreadLept));
				pGroup->Distance = 0;
			}
		}
	}

	for (size_t i = 0; i < groupvec.size(); ++i)
	{

		auto pGroup = groupvec[i];
		if (!pGroup || !pGroup->Target)
			continue;

		auto curDistance = pGroup->Distance;
		auto pObj = pGroup->Target;
		const auto pBuilding = cast_to<BuildingClass*, false>(pObj);

		if (pObj->IsAlive
		&& (!pBuilding || !pBuilding->Type->InvisibleInGame)
		  && (!HitICEdTechno
			  || (pObj->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None
			  && ((TechnoClass*)pObj)->IsIronCurtained()))
		{
			if (pObj->WhatAmI() == AircraftClass::AbsID
			  && pObj->IsInAir())
			{
				curDistance /= 2;
			}

			if (pObj->Health > 0 && pObj->IsOnMap && !pObj->InLimbo && curDistance <= spreadLept)
			{
				int ddd = damage;
				if (pBuilding && merge_bldngDamage)
				{
					auto bld_damage = MergedDamage.get_key_iterator(pBuilding);

					if (bld_damage != MergedDamage.end())
					{
						ddd = int(ddd * bld_damage->second);
						MergedDamage.erase(bld_damage);
					}
				}

				pObj->ReceiveDamage(&ddd, curDistance, const_cast<WarheadTypeClass*>(pWarhead), pSource, false, false, pHouse);
				AnythingHit = true; // is this function succeed hit any item ?
			}
		}
	};

	for (auto it = groupvec.begin(); it != groupvec.end(); ++it)
	{
		GameDelete<false, true>(std::exchange(*it, nullptr));
	}

	groupvec.clear();

	if (HitICEdTechno)
	{
		return DamageAreaResult::Nullified;
	}

	if (pWarhead->Rocker)
	{
		const double rockerSpread = MinImpl(pWHExt->Rocker_AmplitudeOverride.Get(damage) * pWHExt->Rocker_AmplitudeMultiplier, 4.0);

		if (rockerSpread > 0.3)
		{
			const short cell_radius = 3;
			for (short x = -cell_radius; x <= cell_radius; x++)
			{
				for (short y = -cell_radius; y <= cell_radius; y++)
				{
					short xpos = cell.X + x;
					short ypos = cell.Y + y;
					CellStruct cellhere { xpos, ypos };

					if (auto pCellHere = MapClass::Instance->TryGetCellAt(cellhere))
					{
						if(!MapClass::Instance->CoordinatesLegal(cellhere))
							continue;

						auto object = pCellHere->Cell_Occupier(alt);

						while (object)
						{
							if (FootClass* techno = flag_cast_to<FootClass*>(object))
							{
								if (xpos == cell.X && ypos == cell.Y && pSource)
								{
									Coordinate rockercoord = (pSource->GetCoords() - techno->GetCoords());
									Vector3D<double> rockervec = Vector3D<double>((double)rockercoord.X, (double)rockercoord.Y, (double)rockercoord.Z).Normalized() * 10.0f;
									CoordStruct rock_((int)rockervec.X, (int)rockervec.Y, (int)rockervec.Z);
									CoordStruct _result_rock = pCoord->operator+(rock_);

									techno->RockByValue(&_result_rock, (float)rockerSpread);
								}
								else if (pWarhead->CellSpread > 0.0f)
								{
									techno->RockByValue(pCoord, (float)rockerSpread);
								}
							}
							object = object->NextObject;
						}
					}
				}
			}
		}
	}

	DestroyBridge(pCoord, (const FakeWarheadTypeClass*)pWarhead, damage, pCell, &cell);

	if (pCell->OverlayTypeIndex > -1
		&& OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex]->Explodes
		&& damage >= RulesExtData::Instance()->OverlayExplodeThreshold)
	{
		pCell->MarkForRedraw();
		pCell->OverlayTypeIndex = -1;
		pCell->RecalcAttributes(-1);

		MapClass::Instance->ResetZones(pCell->MapCoords);
		MapClass::Instance->RecalculateSubZones(pCell->MapCoords);
		TechnoClass::ClearWhoTargetingThis(pCell);

		if (auto pBarrelExplode = RulesClass::Instance->BarrelExplode)
			GameCreate<AnimClass>(pBarrelExplode, *pCoord);

		//recursive call ..
		DamageArea::Apply(pCoord, RulesClass::Instance->AmmoCrateDamage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);

		for (auto brrelDebris : RulesClass::Instance->BarrelDebris)
		{
			if (ScenarioClass::Instance->Random.RandomFromMax(99) < 15)
			{
				GameCreate<VoxelAnimClass>(brrelDebris, pCoord, nullptr);
				break;
			}
		}

		if (auto barrelParticle = RulesClass::Instance->BarrelParticle)
		{
			if (ScenarioClass::Instance->Random.RandomFromMax(99) < 25)
			{
				GameCreate<ParticleSystemClass>(barrelParticle, *pCoord)
					->SpawnHeldParticle(pCoord, pCoord);
			}
		}
	}

	if (auto pParticle = pWarhead->Particle)
	{
		GameCreate<ParticleSystemClass>(pParticle, *pCoord)
			->SpawnHeldParticle(pCoord, pCoord);
	}


	return DamageAreaResult(AnythingHit == 0);

}

DEFINE_FUNCTION_JUMP(CALL, 0x423EAB, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x424647, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x424ED1, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x425237, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x4387A3, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x469A83, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x481E33, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x481E89, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x48266D, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x482836, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x48A371, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x48A88B, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x4A76AF, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x4B5D28, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x4B5FC7, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x4CD9BB, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x51A6C1, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x51A79E, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x51A7D3, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x53A5D0, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x53B16B, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x53CDB5, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x53CDD4, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6632C7, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6CD90C, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E04DD, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E0545, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E05AD, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E062F, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E0697, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x6E250B, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x71BABF, DamageArea::Apply);
DEFINE_FUNCTION_JUMP(CALL, 0x74A1E1, DamageArea::Apply);

#ifdef _ENABLE

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	GET_BASE(WarheadTypeClass*, pWH, 0x0C);
	if (auto const pWHExt = WarheadTypeExtContainer::Instance.TryFind(pWH))
	{
		GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);
		GET(CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(HouseClass*, pHouse, 0x14);
		if (!pWHExt->ShakeIsLocal || TacticalClass::Instance->IsCoordsToClientVisible(*pCoords))
		{
			if (pWH->ShakeXhi || pWH->ShakeXlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeXhi, pWH->ShakeXlo), pWHExt->Shake_UseAlternativeCalculation);
			if (pWH->ShakeYhi || pWH->ShakeYlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeYhi, pWH->ShakeYlo), pWHExt->Shake_UseAlternativeCalculation);
		}
		auto const pDecidedOwner = !pHouse && pOwner ? pOwner->Owner : pHouse;
		for (const auto& Lauch : pWHExt->Launchs)
		{
			if (Lauch.LaunchWhat)
			{
				Helpers::Otamaa::LauchSW(Lauch, pDecidedOwner, *pCoords, pOwner);
			}
		}
		if (PhobosGlobal::Instance()->DetonateDamageArea)
			pWHExt->Detonate(pOwner, pDecidedOwner, nullptr, *pCoords, Damage);
	}
	return 0;
}

DEFINE_HOOK(0x489968, Explosion_Damage_PenetratesIronCurtain, 0x5)
{
	enum { BypassInvulnerability = 0x48996D };
	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	if (WarheadTypeExtContainer::Instance.Find(pWarhead)->PenetratesIronCurtain)
		return BypassInvulnerability;
	return 0;
}

DEFINE_HOOK(0x4899DA, DamageArea_Damage_MaxAffect, 7)
{
	REF_STACK(DynamicVectorClass<DamageGroup*>, groupvec, 0xE0 - 0xA8);
	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);

	auto pWHExt = pWarhead->_GetExtData();
	const int MaxAffect = pWHExt->CellSpread_MaxAffect;

	if (MaxAffect > 0)
	{
		const auto g_end = groupvec.Items + groupvec.Count;

		for (auto g_begin = groupvec.Items; g_begin != g_end; ++g_begin)
		{

			DamageGroup* group = *g_begin;
			// group could have been cleared by previous iteration.
			// only handle if has not been handled already.
			if (group && Targets.AddUnique(group->Target))
			{

				Handled.Reset();

				// collect all slots containing damage groups for this target
				std::for_each(g_begin, g_end, [group](DamageGroup* item)
 {
	 if (item && item->Target == group->Target)
	 {
		 Handled.AddItem(item);
	 }
				});

				// if more than allowed, sort them and remove the ones further away
				if (Handled.Count > MaxAffect)
				{
					Helpers::Alex::selectionsort(
						Handled.begin(), Handled.begin() + MaxAffect, Handled.end(),
						[](DamageGroup* a, DamageGroup* b)
 {
	 return a->Distance < b->Distance;
						});

					std::for_each(Handled.begin() + MaxAffect, Handled.end(), [](DamageGroup* ppItem)
 {
	 ppItem->Target = nullptr;
					});
				}
			}
		}

		// move all the empty ones to the back, then remove them
		groupvec.remove_if([](DamageGroup* pGroup)
 {
	 if (!pGroup->Target)
	 {
		 GameDelete<false, false>(pGroup);
		 return true;
	 }

	 return false;
		});

		Targets.Count = 0;
		Handled.Count = 0;
	}

	return 0;
}
DEFINE_HOOK(0x489AD6, DamageArea_Damage_AfterLoop, 6)
{
	REF_STACK(DynamicVectorClass<DamageGroup*>, groupvec, 0xE0 - 0xA8);
	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);
	GET_STACK(bool, Something, 0x17);
	GET_STACK(int, idamage, 0x24);
	//GET_STACK(int, distance, 0x68);
	GET_BASE(TechnoClass*, pSource, 0x8);
	//GET_BASE(HouseClass*, pHouse, 0x14);

	auto pWHExt = pWarhead->_GetExtData();

	for (int i = 0; i < groupvec.Count; ++i)
	{/*
		auto pTarget = groupvec.Items[i];
		auto curDistance = pTarget->Distance;
		auto pObj = pTarget->Target;
		R->Stack(0x37, (pObj->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None);

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
				if (pSource && (!pSource->Health || !pSource->IsAlive) && !pSource->Owner)
					pSource = nullptr;

				int Damage = idamage;
				pObj->ReceiveDamage(&Damage, curDistance, pWarhead, pSource, false, false, pHouse);
				R->Stack(0x1F, 1);
			}
		}
		*/
		GameDelete(std::exchange(groupvec.Items[i], nullptr));
	}


	groupvec.Count = 0;
	if (groupvec.IsAllocated)
	{
		GameDeleteArray(groupvec.Items, groupvec.Capacity);
		groupvec.IsAllocated = false;
	}
	groupvec.Items = nullptr;

	if (Something)
	{
		return 0x489B3B;
	}

	//dont do any calculation when it is not even a rocker
	R->EBX(pWarhead);
	GET_STACK(bool, alt, 0xE0 - 0xC2);
	LEA_STACK(CellStruct*, pCell, 0xE0 - 0xC8);
	LEA_STACK(CoordStruct*, pCoord, 0xE0 - 0xB8);

	if (pWarhead->Rocker)
	{

		const double rockerSpread = MinImpl(pWHExt->Rocker_AmplitudeOverride.Get(idamage) * pWHExt->Rocker_AmplitudeMultiplier, 4.0);

		if (rockerSpread > 0.3)
		{
			const short cell_radius = 3;
			for (short x = -cell_radius; x <= cell_radius; x++)
			{
				for (short y = -cell_radius; y <= cell_radius; y++)
				{
					short xpos = pCell->X + x;
					short ypos = pCell->Y + y;

					auto object = MapClass::Instance->GetCellAt(CellStruct(xpos, ypos))->Cell_Occupier(alt);

					while (object)
					{
						if (FootClass* techno = flag_cast_to<FootClass*>(object))
						{
							if (xpos == pCell->X && ypos == pCell->Y && pSource)
							{
								Coordinate rockercoord = (pSource->GetCoords() - techno->GetCoords());
								Vector3D<double> rockervec = Vector3D<double>((double)rockercoord.X, (double)rockercoord.Y, (double)rockercoord.Z).Normalized() * 10.0f;
								CoordStruct rock_((int)rockervec.X, (int)rockervec.Y, (int)rockervec.Z);
								techno->RockByValue(&pCoord->operator+(rock_), (float)rockerSpread);
							}
							else if (pWarhead->CellSpread > 0.0f)
							{
								techno->RockByValue(pCoord, (float)rockerSpread);
							}
						}
						object = object->NextObject;
					}
				}
			}
		}
	}

	return 0x489E87;
}
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


// create enumerator
DEFINE_HOOK(0x4895B8, DamageArea_CellSpread1, 0x6)
{
	REF_STACK(CellSpreadEnumerator<std::numeric_limits<short>::max()>*, pIter, 0xE0 - 0xB4);
	GET(int, spread, EAX);

	pIter = nullptr;

	if (spread < 0)
		return 0x4899DA;

	//to avoid unnessesary allocation check
	//simplify the assembly result
	pIter = DLLCreate<CellSpreadEnumerator<std::numeric_limits<short>::max()>>((short)spread);
	pIter->setSpread((short)spread);

	return *pIter ? 0x4895C3 : 0x4899DA;
}

// apply the current value
DEFINE_HOOK(0x4895C7, DamageArea_CellSpread2, 0x8)
{
	GET_STACK(CellSpreadEnumerator<std::numeric_limits<short>::max()>*, pIter, STACK_OFFS(0xE0, 0xB4));

	R->DX((*pIter)->X);
	R->AX((*pIter)->Y);

	return 0x4895D7;
}

// advance and delete if done
DEFINE_HOOK(0x4899BE, DamageArea_CellSpread3, 0x8)
{
	REF_STACK(CellSpreadEnumerator<std::numeric_limits<short>::max()>*, pIter, STACK_OFFS(0xE0, 0xB4));
	REF_STACK(int, index, STACK_OFFS(0xE0, 0xD0));

	// reproduce skipped instruction
	index++;

	// advance iterator
	if (++*pIter)
	{
		return 0x4895C0;
	}

	// all done. delete and go on
	DLLDelete<false>(pIter);
	return 0x4899DA;
}

//DEFINE_HOOK(0x4896EC, MapClass_DamageArea_DamageSelf, 0x6) {
//	GET(ObjectClass*, pObj, ECX);
//	GET(TechnoTypeClass*, pType, EAX);
//	GET_BASE(WarheadTypeClass*, pWH, 0xC);
//
//	if (!pType->DamageSelf ) {
//		Debug::LogInfo("Techno[%x - %s] Trying to damage itself with Warhead [%s] , but DamageSelf is off , is this intended ?", pObj, pType->ID, pWH->ID);
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
#endif