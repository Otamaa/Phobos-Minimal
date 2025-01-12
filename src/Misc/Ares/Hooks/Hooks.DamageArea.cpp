#include <Ext/WarheadType/Body.h>

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

static HelperedVector<ObjectClass*> Targets {};
static HelperedVector<DamageGroup*> Handled {};

void NOINLINE DestroyBridge(CoordStruct* pCoord , FakeWarheadTypeClass* pWarhead , int damage , CellClass* pCell, CellStruct* pCellStruct)
{
	static constexpr reference<int, 0xAA0E28> BridgeSet {};
	static constexpr reference<int, 0xABAD30> BridgeMiddle1 {};
	static constexpr reference<int, 0xAA1028> BridgeMiddle2 {};
	static constexpr reference<int, 0xABAD1C> WoodBridgeSet {};

	if (((ScenarioClass::Instance->SpecialFlags.RawFlags & 0x80u) != 0) && pWarhead->WallAbsoluteDestroyer)
	{
		const bool v98 = pWarhead->_GetExtData()->BridgeAbsoluteDestroyer.Get(pWarhead == RulesClass::Instance->IonCannonWarhead);

		auto v63 = pCell->UINTFlags;
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

static DamageAreaResult __fastcall DamageArea(CoordStruct* pCoord,
		int damage,
		TechnoClass* pSource,
		FakeWarheadTypeClass* pWarhead,
		bool affectTiberium,
		HouseClass* pHouse)
{
	if (!pWarhead)
	{
		return DamageAreaResult::Missed;
	}

	const auto pWHExt = pWarhead->_GetExtData();
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

	if (int(spread + 0.99) >= 0) {
		//obtain Object within the spread distance
		for (CellSpreadEnumerator it(short(spread + 0.99), short(0)); it; ++it)
		{
			auto cellhere = (cell + (*it));
			auto pCurCell = MapClass::Instance->GetCellAt(cellhere);
			auto cur_cellCoord = pCurCell->GetCoords();

			if (pCurCell->OverlayTypeIndex > -1)
			{
				auto pOvelay = OverlayTypeClass::Array->Items[pCurCell->OverlayTypeIndex];
				if (pOvelay->ChainReaction && (!pOvelay->Tiberium || pWarhead->Tiberium) && affectTiberium)
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
					if (VeinholeMonsterClass* veinhole = VeinholeMonsterClass::GetVeinholeMonsterAt(&cellhere))
					{
						if (!veinhole->InLimbo && veinhole->IsAlive && ((int)veinhole->MonsterCell.DistanceFrom(pCell->MapCoords) <= 0))
						{
							int nDamage = damage;
							if (veinhole->ReceiveDamage(&nDamage,
								(int)cur_cellCoord.DistanceFrom(CellClass::Cell2Coord(veinhole->MonsterCell)),
								pWarhead,
								pSource,
								false,
								false,
								pSource && !pHouse ? pSource->Owner : pHouse
							) == DamageState::NowDead)
							{
								Debug::Log("Veinhole at [%d %d] Destroyed!\n", veinhole->MonsterCell.X, veinhole->MonsterCell.Y);

								if (pCurCell->OverlayTypeIndex == -1)
								{
									TechnoClass::ClearWhoTargetingThis(pCurCell);
								}
							}
						}
					}
				}
			}

			for (NextObject next(alt ? pCurCell->AltObject : pCurCell->FirstObject); next; next++)
			{
				auto pCur = *next;

				if (pCur == pSource && !pWHExt->AllowDamageOnSelf && !isCrushWarhead)
					continue;

				if (!pCur->IsAlive)
					continue;

				const auto what = pCur->WhatAmI();
				auto pTechno = flag_cast_to<TechnoClass*, false>(pCur);

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
						if (!(pCoord->Z - cur_cellCoord.Z <= Unsorted::LevelHeight  * 2))
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

	if (pWHExt->CellSpread_MaxAffect > 0)
	{
		Targets.clear();
		Handled.clear();

		const auto g_end = groupvec.begin() + groupvec.size();

		for (auto g_begin = groupvec.begin(); g_begin != g_end; ++g_begin)
		{

			DamageGroup* group = *g_begin;
			// group could have been cleared by previous iteration.
			// only handle if has not been handled already.
			if (group && Targets.push_back_unique(group->Target))  {

				Handled.clear();

				// collect all slots containing damage groups for this target
				std::for_each(g_begin, g_end, [group](DamageGroup* item) {
					if (item && item->Target == group->Target) {
					   Handled.push_back(item);
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

					std::for_each(Handled.begin() + pWHExt->CellSpread_MaxAffect, Handled.end(), [](DamageGroup* ppItem) {
						 ppItem->Target = nullptr;
					});
				}
			}
		}

		// move all the empty ones to the back, then remove them
		groupvec.remove_all_if([](DamageGroup* pGroup){
			if (!pGroup->Target) {
				GameDelete<false, false>(pGroup);
				pGroup = nullptr;
				return true;
			}

			return false;
		});
	}

	bool AnythingHit = false;

	for(auto it= groupvec.begin(); it != groupvec.end(); ++it) {

		 auto pGroup = *it;
		 auto curDistance = pGroup->Distance;
		 auto pObj = pGroup->Target;

		 if (pObj->IsAlive
		 && (pObj->WhatAmI() != BuildingClass::AbsID || !((BuildingClass*)pObj)->Type->InvisibleInGame)
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
				 pObj->ReceiveDamage(&ddd, curDistance, pWarhead, pSource, false, false, pHouse);
				 AnythingHit = true; // is this function succeed hit any item ?
			 }
		 }

		 GameDelete<false, false>(std::exchange(pGroup, nullptr));
	};

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
			const int cell_radius = 3;
			for (int x = -cell_radius; x <= cell_radius; x++)
			{
				for (int y = -cell_radius; y <= cell_radius; y++)
				{
					int xpos = cell.X + x;
					int ypos = cell.Y + y;

					auto object = MapClass::Instance->GetCellAt(CellStruct(xpos, ypos))->Cell_Occupier(alt);

					while (object)
					{
						if (FootClass* techno = flag_cast_to<FootClass*>(object))
						{
							if (xpos == cell.X && ypos == cell.Y && pSource)
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

	DestroyBridge(pCoord, pWarhead, damage,  pCell, &cell);

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
		MapClass::DamageArea(pCoord, RulesClass::Instance->AmmoCrateDamage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);

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

DEFINE_JUMP(LJMP, 0x489280, MiscTools::to_DWORD(DamageArea));