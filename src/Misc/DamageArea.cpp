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

						if (pCurCell->OverlayTypeIndex == -1)
						{
							TechnoClass::ClearWhoTargetingThis(pCurCell);
						}
					}
				}
			}
		}
	}
}

/*
void __fastcall Explosion_Damage(CoordStruct* coord, int damage, TechnoClass* source, WarheadTypeClass* warhead, bool tibbool, HouseClass* house)
{
	if (!damage || (Scen->Specials.Bitfield & 0x20) != 0 || !warhead)
	{
		return;
	}

	std::vector<DamageGroup*> dmgroups;

	int spreadFactorPixels = static_cast<int>(warhead->SpreadFactor * 256.0);
	bool crush = (warhead == Rule->CrushWarhead);

	CellStruct cellPos;
	cellPos.X = coord->X / 256;
	cellPos.Y = coord->Y / 256;

	CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);
	float spreadFactor = warhead->SpreadFactor;
	bool isSmallSpread = (spreadFactor <= 0.5f);
	bool isAboveGround = false;
	bool hasIronCurtainTarget = false;

	CoordStruct cellCenter;
	cellCenter.X = (cellPos.X << 8) + 128;
	cellCenter.Y = (cellPos.Y << 8) + 128;
	cellCenter.Z = 0;

	// Check aircraft if explosion is above ground
	if (MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &cellCenter) < coord->Z)
	{
		AircraftTrackerClass_logics_412B40(&AircraftTracker, cell, warhead->SpreadFactor);

		for (auto entry = AircraftTrackerClass::Get_Entry(&AircraftTracker); entry; entry = AircraftTrackerClass::Get_Entry(&AircraftTracker))
		{
			if (!entry->f.t.r.m.o.IsActive || !entry->f.t.r.m.o.IsDown || entry->f.t.r.m.o.Strength <= 0)
			{
				continue;
			}

			CoordStruct delta;
			delta.X = coord->X - entry->f.t.r.m.o.Coord.X;
			delta.Y = coord->Y - entry->f.t.r.m.o.Coord.Y;
			delta.Z = coord->Z - entry->f.t.r.m.o.Coord.Z;

			int distance = CoordStruct::Length(&delta);

			if (distance > spreadFactorPixels)
			{
				continue;
			}

			if (isSmallSpread && distance < 85 &&
				entry->f.t.r.m.o.a.vftable->t.r.m.o.mAbstractClass_IsIronCurtained(entry) &&
				!entry->f.t.__ForceShielded)
			{
				hasIronCurtainTarget = true;
			}

			auto dmg = new DamageGroup();
			dmg->Techno = &entry->f.t;
			dmg->distance = distance;
			dmgroups.push_back(dmg);
		}
	}

	// Check if coord is above bridge
	if ((*cell->Bitfield2 & 0x100) != 0 &&
		coord->Z > dword_89E864 / 2 + MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, coord))
	{
		isAboveGround = true;
	}

	// Process cells in spread range
	int cellSpreadCount = dword_7ED3D0[static_cast<int>(warhead->SpreadFactor + 0.99f)];

	for (int cellIdx = 0; cellIdx < cellSpreadCount; ++cellIdx)
	{
		CellStruct targetCellPos;
		targetCellPos.X = cellPos.X + CellSpread::CellArray[cellIdx].X;
		targetCellPos.Y = cellPos.Y + CellSpread::CellArray[cellIdx].Y;

		CellClass* targetCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &targetCellPos);

		// Process overlay
		int overlay = targetCell->Overlay;
		if (overlay != -1)
		{
			auto overlayType = (*(&OverlayTypes + 1))[overlay];

			if (overlayType->ChainReaction && (!overlayType->IsTiberium || warhead->IsTiberiumDestroyer) && tibbool)
			{
				CellClass::Reduce_Tiberium(targetCell, damage / 10);
			}

			if (overlayType->IsWall)
			{
				if (warhead->WallAbsoluteDestroyer)
				{
					CellClass::Reduce_Wall(targetCell, -1);
				}
				else if (warhead->IsWallDestroyer || (warhead->IsWoodDestroyer && overlayType->ot.Armor == 6))
				{
					CellClass::Reduce_Wall(targetCell, damage);
				}
			}

			if (targetCell->Overlay == -1)
			{
				TechnoClass::target_stuff(targetCell);
			}
		}

		// Process occupiers
		ObjectClass* occupier = isAboveGround ? targetCell->AltOccupierPtr : targetCell->OccupierPtr;

		for (; occupier; occupier = occupier->f.t.r.m.o.Next)
		{
			// Skip self-damage check
			if (occupier == source && !source->r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(source)->DamageSelf && !crush)
			{
				continue;
			}

			if (!occupier->f.t.r.m.o.IsActive)
			{
				continue;
			}

			// Skip harvesters if special flag set
			if (occupier->f.t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&occupier->f.t.r.m.o.a) == RTTI_UNIT &&
				(Scen->Specials.Bitfield & 0x800) != 0)
			{
				auto classOf = occupier->f.t.r.m.o.a.vftable->t.r.m.o.Class_Of(&occupier->f.t.r.m.o);
				bool isHarvester = false;

				for (int i = 0; i < Rule->HarvesterUnit.dvc.ActiveCount; ++i)
				{
					if (Rule->HarvesterUnit.dvc.Vector_Item[i] == classOf)
					{
						isHarvester = true;
						break;
					}
				}

				if (isHarvester)
				{
					continue;
				}
			}

			// Create damage group entry
			auto dmg = new DamageGroup();
			dmg->Techno = &occupier->f.t;

			// Calculate distance
			if (occupier->f.t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&occupier->f.t.r.m.o.a) == RTTI_BUILDING)
			{
				if (cellIdx == 0)
				{
					auto centerCoord = targetCell->a.vftable->t.r.m.o.a.Center_Coord(targetCell);

					if (coord->Z - centerCoord->Z <= 2 * dword_89E870)
					{
						dmg->distance = 0;
					}
					else
					{
						CoordStruct delta;
						delta.X = coord->X - centerCoord->X;
						delta.Y = coord->Y - centerCoord->Y;
						delta.Z = coord->Z - centerCoord->Z;
						dmg->distance = static_cast<int>(FastMath::Sqrt(
							delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z)) - 2 * dword_89E870;
					}

					// Check iron curtain
					if (isSmallSpread)
					{
						bool isOnMap = (occupier->f.t.r.m.o.a.TargetBitfield[0] & 1) != 0;
						if (isOnMap &&
							occupier->f.t.r.m.o.a.vftable->t.r.m.o.mAbstractClass_IsIronCurtained(&occupier->f.t) &&
							!occupier->f.t.__ForceShielded &&
							dmg->distance < 85)
						{
							hasIronCurtainTarget = true;
						}
					}
				}
				else
				{
					auto centerCoord = targetCell->a.vftable->t.r.m.o.a.Center_Coord(targetCell);
					CoordStruct delta;
					delta.X = coord->X - centerCoord->X;
					delta.Y = coord->Y - centerCoord->Y;
					delta.Z = coord->Z - centerCoord->Z;
					dmg->distance = static_cast<int>(FastMath::Sqrt(
						delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z));
				}
			}
			else
			{
				CoordStruct headCoord;
				occupier->f.t.r.m.o.a.vftable->t.r.m.o.mTarget_Coord__Head_To_Coord(occupier, &headCoord);
				CoordStruct delta;
				delta.X = coord->X - headCoord.X;
				delta.Y = coord->Y - headCoord.Y;
				delta.Z = coord->Z - headCoord.Z;
				dmg->distance = static_cast<int>(FastMath::Sqrt(
					delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z));
			}

			dmgroups.push_back(dmg);
		}
	}

	// Apply damage to all collected targets
	bool damageApplied = false;

	for (auto dmg : dmgroups)
	{
		auto techno = dmg->Techno;
		int distance = dmg->distance;

		if (!techno->r.m.o.IsActive)
		{
			continue;
		}

		if (techno->t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&techno->t.r.m.o.a) == RTTI_BUILDING &&
			techno->Class->InvisibleInGame)
		{
			continue;
		}

		if (hasIronCurtainTarget)
		{
			bool isOnMap = (techno->t.r.m.o.a.TargetBitfield[0] & 1) != 0;
			if (!techno || !isOnMap || !techno->t.r.m.o.a.vftable->t.r.m.o.mAbstractClass_IsIronCurtained(techno))
			{
				continue;
			}
		}

		if (techno->t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&techno->t.r.m.o.a) == RTTI_AIRCRAFT &&
			techno->t.r.m.o.a.vftable->t.r.m.o.a.In_Air(techno))
		{
			distance /= 2;
		}

		if (techno->t.r.m.o.Strength > 0 && techno->t.r.m.o.IsDown &&
			!techno->t.r.m.o.IsInLimbo && distance <= spreadFactorPixels)
		{
			techno->t.r.m.o.a.vftable->ApplyDamage(techno, damage);
			damageApplied = true;
		}
	}

	// Cleanup damage groups
	for (auto dmg : dmgroups)
	{
		delete dmg;
	}
	dmgroups.clear();

	// Early exit if iron curtain target was hit
	if (hasIronCurtainTarget)
	{
		return;
	}

	// Calculate rock factor
	float rockFactor = damage * 0.01f;
	if (rockFactor >= 4.0f)
	{
		rockFactor = 4.0f;
	}

	// Apply rocker effect
	if (warhead->Rocker && rockFactor > 0.3f)
	{
		for (int x = cellPos.X - 3; x <= cellPos.X + 3; ++x)
		{
			for (int y = cellPos.Y - 3; y <= cellPos.Y + 3; ++y)
			{
				CellStruct rockCellPos = { static_cast<short>(x), static_cast<short>(y) };
				ObjectClass* rockOccupier = isAboveGround
					? MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &rockCellPos)->AltOccupierPtr
					: MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &rockCellPos)->OccupierPtr;

				for (; rockOccupier; rockOccupier = rockOccupier->dword30_pointermaybe)
				{
					auto techno = AbstractClass::As_Techno_0(&rockOccupier->a);
					if (!techno)
					{
						continue;
					}

					if (x == cellPos.X && y == cellPos.Y && source)
					{
						CoordStruct technoCoord = techno->r.m.o.Coord;
						CoordStruct delta;
						delta.X = source->r.m.o.Coord.X - technoCoord.X;
						delta.Y = source->r.m.o.Coord.Y - technoCoord.Y;
						delta.Z = source->r.m.o.Coord.Z - technoCoord.Z;

						float length = FastMath::Sqrt(
							delta.X * delta.X + delta.Y * delta.Y + delta.Z * delta.Z);

						CoordStruct normalized;
						if (length == 0.0f)
						{
							normalized = delta;
						}
						else
						{
							normalized.X = static_cast<int>(delta.X / length);
							normalized.Y = static_cast<int>(delta.Y / length);
							normalized.Z = static_cast<int>(delta.Z / length);
						}

						CoordStruct rockCoord;
						rockCoord.X = technoCoord.X + static_cast<int>(normalized.X * 10.0f);
						rockCoord.Y = technoCoord.Y + static_cast<int>(normalized.Y * 10.0f);
						rockCoord.Z = technoCoord.Z + static_cast<int>(normalized.Z * 10.0f);

						techno->r.m.o.a.vftable->t.msub_70B280_spread(techno, &rockCoord.X, rockFactor, 0);
					}
					else if (warhead->SpreadFactor > 0.0f)
					{
						techno->r.m.o.a.vftable->t.msub_70B280_spread(techno, &coord->X, rockFactor, 0);
					}
				}
			}
		}
	}

	// Bridge damage handling
	bool isIonCannon = (warhead == Rule->IonCannonWarhead);
	CellClass* mainCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

	if ((BYTE1(Scen->Specials.Bitfield) & 0x80u) != 0 && warhead->IsWallDestroyer)
	{
		int bridgeTileOffset = mainCell->TileType - BridgeSet + 1;
		CellClass* bridgeOwnerCell = nullptr;

		if ((*mainCell->Bitfield2 & 0x100) != 0)
		{
			CellStruct bridgeCellPos;
			if ((*mainCell->Bitfield2 & 0x80) == 0)
			{
				bridgeCellPos = mainCell->BridgeOwnerCell->Position;
			}
			else
			{
				bridgeCellPos = mainCell->Position;
			}

			bridgeOwnerCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &bridgeCellPos);

			if (bridgeOwnerCell)
			{
				int bridgeOverlay = bridgeOwnerCell->Overlay;
				if (bridgeOverlay != 24 && bridgeOverlay != 25)
				{
					bridgeOwnerCell = nullptr;
				}
			}
		}

		// Check bridge middle tiles
		bool isBridgeMiddle1 = (bridgeTileOffset == BridgeMiddle1 ||
								bridgeTileOffset == BridgeMiddle1 + 1 ||
								bridgeTileOffset == BridgeMiddle1 + 2 ||
								bridgeTileOffset == BridgeMiddle1 + 3);

		bool isBridgeMiddle2 = (bridgeTileOffset == BridgeMiddle2 ||
								bridgeTileOffset == BridgeMiddle2 + 1 ||
								bridgeTileOffset == BridgeMiddle2 + 2 ||
								bridgeTileOffset == BridgeMiddle2 + 3);

		if (isBridgeMiddle1 || isBridgeMiddle2 || bridgeOwnerCell)
		{
			// Process bridge damage
			if (CanDamageBridge(mainCell, coord))
			{
				if (warhead->IsWallDestroyer)
				{
					if (isIonCannon || Random2Class::operator()(&Scen->RandomNumber, 1, Rule->BridgeStrength) < damage)
					{
						int attempts = 3;
						bool destroyed = MapClass_findsoemthing_587180(&Map.sc.t.sb.p.r.d.m, &cellPos);

						while (!destroyed && isIonCannon && attempts > 0)
						{
							--attempts;
							destroyed = MapClass_findsoemthing_587180(&Map.sc.t.sb.p.r.d.m, &cellPos);
						}

						if (destroyed)
						{
							TechnoClass::target_stuff(mainCell);
						}

						Point2D pixel;
						Tactical::Coord_To_Pixel_Adjusted_To_Viewport(TacticalMap, coord, &pixel);
						RectangleStruct redrawRect;
						redrawRect.X = pixel.X - 128;
						redrawRect.Y = pixel.Y - 128;
						redrawRect.Width = 256;
						redrawRect.Height = 256;
						Tactical::Redraw_Area(TacticalMap, redrawRect, 0);
					}
				}
			}
		}

		// Check wood bridge tiles
		int woodBridgeTileOffset = mainCell->TileType - WoodBridgeSet + 1;

		bool isWoodBridgeMiddle1 = (woodBridgeTileOffset == BridgeMiddle1 ||
									woodBridgeTileOffset == BridgeMiddle1 + 1 ||
									woodBridgeTileOffset == BridgeMiddle1 + 2 ||
									woodBridgeTileOffset == BridgeMiddle1 + 3);

		bool isWoodBridgeMiddle2 = (woodBridgeTileOffset == BridgeMiddle2 ||
									woodBridgeTileOffset == BridgeMiddle2 + 1 ||
									woodBridgeTileOffset == BridgeMiddle2 + 2 ||
									woodBridgeTileOffset == BridgeMiddle2 + 3);

		bool hasWoodBridgeOverlay = bridgeOwnerCell &&
			(bridgeOwnerCell->Overlay == 237 || bridgeOwnerCell->Overlay == 238);

		if (isWoodBridgeMiddle1 || isWoodBridgeMiddle2 || hasWoodBridgeOverlay)
		{
			if (CanDamageBridge(mainCell, coord))
			{
				if (warhead->IsWallDestroyer)
				{
					if (isIonCannon || Random2Class::operator()(&Scen->RandomNumber, 1, Rule->BridgeStrength) < damage)
					{
						int attempts = 3;
						bool destroyed = MapClass_findsoemthing_587180(&Map.sc.t.sb.p.r.d.m, &cellPos);

						while (!destroyed && isIonCannon && attempts > 0)
						{
							--attempts;
							destroyed = MapClass_findsoemthing_587180(&Map.sc.t.sb.p.r.d.m, &cellPos);
						}

						if (destroyed)
						{
							TechnoClass::target_stuff(mainCell);
						}

						Point2D pixel;
						Tactical::Coord_To_Pixel_Adjusted_To_Viewport(TacticalMap, coord, &pixel);
						RectangleStruct redrawRect;
						redrawRect.X = pixel.X - 96;
						redrawRect.Y = pixel.Y - 96;
						redrawRect.Width = 192;
						redrawRect.Height = 192;
						Tactical::Redraw_Area(TacticalMap, redrawRect, 0);
					}
				}
			}
		}

		// Check overlay bridges
		int mainOverlay = mainCell->Overlay;
		if (mainOverlay >= 74 && mainOverlay <= 99)
		{
			if (isIonCannon || Random2Class::operator()(&Scen->RandomNumber, 1, Rule->BridgeStrength) < damage)
			{
				if (MapClass_checkcells_57BAA0(&Map.sc.t.sb.p.r.d.m, &cellPos))
				{
					TechnoClass::target_stuff(mainCell);
				}
			}
		}

		mainOverlay = mainCell->Overlay;
		if (mainOverlay >= 205 && mainOverlay <= 230)
		{
			if (isIonCannon || Random2Class::operator()(&Scen->RandomNumber, 1, Rule->BridgeStrength) < damage)
			{
				if (MapClass_57CCF0(&Map.sc.t.sb.p.r.d.m, &cellPos))
				{
					TechnoClass::target_stuff(mainCell);
				}
			}
		}
	}

	// Exploding barrel handling
	int cellOverlay = mainCell->Overlay;
	if (cellOverlay != -1 && (*(&OverlayTypes + 1))[cellOverlay]->Explodes)
	{
		CellClass_flag_area(mainCell);
		mainCell->Overlay = -1;
		CellClass::Recalc_Attributes(mainCell, -1);
		MapClass_Zone_Reset_56D460(&Map.sc.t.sb.p.r.d.m, &mainCell->Position);
		MapClass_subzone_Zone_Reset_584550(&Map.sc.t.sb.p.r.d.m, &mainCell->Position);
		TechnoClass::target_stuff(mainCell);

		if (auto anim = new AnimClass())
		{
			AnimClass::AnimClass(anim, Rule->BarrelExplode, coord, 0, 1, AnimFlag_400 | AnimFlag_200, 0, 0);
		}

		// Recursive explosion
		Explosion_Damage(coord, Rule->AmmoCrateDamage, nullptr, Rule->C4Warhead, true, house);

		// Spawn debris
		for (int i = 0; i < Rule->BarrelDebris.dvc.ActiveCount; ++i)
		{
			if (Random2Class::operator()(&Scen->RandomNumber, 0, 99) < 15)
			{
				if (auto voxelAnim = new VoxelAnimClass())
				{
					VoxelAnimClass::VoxelAnimClass(voxelAnim, Rule->BarrelDebris.dvc.Vector_Item[i], coord, 0);
				}
				break;
			}
		}

		// Spawn particle system
		if (Random2Class::operator()(&Scen->RandomNumber, 0, 99) < 25)
		{
			auto particleSystem = new ParticleSystemClass();
			if (particleSystem)
			{
				ParticleSystemClass::ParticleSystemClass(particleSystem, Rule->BarrelParticle, coord, 0, 0, &stru_89E830, 0);
			}
			ParticleSystemClass_addtovector(particleSystem, coord, coord);
		}
	}

	// Spawn warhead particle
	if (warhead->Particle)
	{
		auto particleSystem = new ParticleSystemClass();
		if (particleSystem)
		{
			ParticleSystemClass::ParticleSystemClass(particleSystem, warhead->Particle, coord, 0, 0, &stru_89E830, house);
		}
		ParticleSystemClass_addtovector(particleSystem, coord, coord);
	}
}

// Helper function for bridge damage checks
static bool CanDamageBridge(CellClass* cell, CoordStruct* coord)
{
	if ((*cell->Bitfield2 & 0x100) == 0)
	{
		return true;
	}

	int level = cell->Level;
	int z = coord->Z;
	int upperBound = dword_89E864 + dword_89E870 * (level + 1);
	int lowerBound = dword_89E864 + dword_89E870 * (level - 2);

	return (z <= upperBound && z > lowerBound);
}
*/

//static PhobosMap<BuildingClass*, double> MergedDamage {};
static DynamicVectorClass<ObjectClass*> Targets;
static DynamicVectorClass<DamageGroup*> Handled;

// inline int Distance_Level_Snap(const Coordinate& coord1, const Coordinate& coord2)
// {
// 	int z1 = coord1.Z;
// 	int z2 = coord2.Z;
// 	if (Math::abs(z2 - z1) < 104)
// 	{
// 		z2 = coord1.Z;
// 	}
// 	return (int)CoordStruct(coord1.X - coord2.X, coord1.Y - coord2.Y, z1 - z2).Length();
// }

// this function is landmines , hooking it breaking other

DamageAreaResult __fastcall DamageArea::Apply(CoordStruct* pCoord,
		int damage,
		TechnoClass* pSource,
		WarheadTypeClass* pWarhead,
		bool affectTiberium,
		HouseClass* pHouse)
{

	JMP_FAST(0x489280);
#ifdef _aaa
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
		int i = 0;
		for (CellSpreadEnumerator it(short(spread + 0.99), short(0)); it; ++it)
		{
			auto cellhere = (cell + (*it));
			const bool IsCenter = i++ == 0;

			if (auto pCurCell = MapClass::Instance->TryGetCellAt(cellhere))
			{
				if (MapClass::Instance->CoordinatesLegal(cellhere))
				{

					auto cur_cellCoord = pCurCell->GetCoords();
					auto spawn_distance = cellhere.DistanceFrom(cell);
					Damage_Overlay(pCurCell, cell, pWarhead, spawn_distance, damage, pSource, pHouse, affectTiberium);

					auto scorch_chance = std::clamp(Math::PercentAtMax(pWHExt->ScorchChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->ScorchPercentAtMax.Get()), 0.0, 1.0);
					auto crater_chance = std::clamp(Math::PercentAtMax(pWHExt->CraterChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->CraterPercentAtMax.Get()), 0.0, 1.0);
					auto cellanim_chance = std::clamp(Math::PercentAtMax(pWHExt->CellAnimChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->CellAnimPercentAtMax.Get()), 0.0, 1.0);

					Spawn_Flames_And_Smudges(cellhere, scorch_chance, crater_chance, cellanim_chance, pWHExt->CellAnim);

					for (NextObject next(alt ? pCurCell->AltObject : pCurCell->FirstObject); next; next++)
					{
						auto pCur = *next;

						if (!pCur->IsAlive || pCur == pSource && !pWHExt->AllowDamageOnSelf && !isCrushWarhead)
							continue;

						const auto what = pCur->WhatAmI();

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

							if (IsCenter && !(pCoord->Z - cur_cellCoord.Z <= Unsorted::CellHeight))
							{
								cur_Group->Distance = (int)(cur_cellCoord.operator-(*pCoord).Length()) - Unsorted::CellHeight;
							}
							else
							{
								cur_Group->Distance = (int)pCoord->operator-(cur_cellCoord).Length();
							}
						}
						else
						{
							cur_Group->Distance = (int)pCoord->operator-(pCur->GetTargetCoords()).Length();
						}

						if (spreadLow && IsCenter)
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
						if (!MapClass::Instance->CoordinatesLegal(cellhere))
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
#endif
}

//DEFINE_FUNCTION_JUMP(CALL, 0x423EAB, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x424647, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x424ED1, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x425237, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x4387A3, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x469A83, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x481E33, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x481E89, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x48266D, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x482836, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x48A371, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x48A88B, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x4A76AF, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x4B5D28, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x4B5FC7, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x4CD9BB, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x51A6C1, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x51A79E, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x51A7D3, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x53A5D0, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x53B16B, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x53CDB5, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x53CDD4, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6632C7, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6CD90C, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E04DD, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E0545, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E05AD, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E062F, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E0697, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x6E250B, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x71BABF, DamageArea::Apply);
//DEFINE_FUNCTION_JUMP(CALL, 0x74A1E1, DamageArea::Apply);

#ifndef _ENABLE

// Obviously, it is unreasonable for a large-scale damage like a nuke to only cause damage to units
// located on or under the bridge that are in the same position as the damage center point
namespace DamageAreaTemp
{
	const CellClass* CheckingCell = nullptr;
	bool CheckingCellAlt = false;
}
bool NOINLINE IsObjectEligible(ObjectClass* pObj, WarheadTypeExtData* pWHExt)
{
	if (!pObj)
		return false;

	return true;
}

ASMJIT_PATCH(0x4896BF, DamageArea_DamageItemsFix1, 0x6)
{
	enum { CheckNextCell = 0x4899BE, CheckThisObject = 0x4896DD };

	// Record the current cell for linked list getting
	GET(CellClass*, pCellHere, EBX);
	GET_STACK(CellClass*, pCellDetonation, 0x20);
	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);

	auto pWHExt = pWarhead->_GetExtData();
	auto spawn_distance = pCellHere->MapCoords.DistanceFrom(pCellDetonation->MapCoords);
	double spreadLept = pWarhead->CellSpread * 256.0;
	auto scorch_chance = std::clamp(Math::PercentAtMax(pWHExt->ScorchChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->ScorchPercentAtMax.Get()), 0.0, 1.0);
	auto crater_chance = std::clamp(Math::PercentAtMax(pWHExt->CraterChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->CraterPercentAtMax.Get()), 0.0, 1.0);
	auto cellanim_chance = std::clamp(Math::PercentAtMax(pWHExt->CellAnimChance.Get(), (int)spreadLept, (int)spawn_distance, pWHExt->CellAnimPercentAtMax.Get()), 0.0, 1.0);

	Spawn_Flames_And_Smudges(pCellHere->MapCoords, scorch_chance, crater_chance, cellanim_chance, pWHExt->CellAnim);

	DamageAreaTemp::CheckingCell = pCellHere;

	if (!IsObjectEligible(pCellHere->FirstObject, pWarhead->_GetExtData()) && !IsObjectEligible(pCellHere->AltObject, pWarhead->_GetExtData()))
		return CheckNextCell;

	// First, check the FirstObject linked list
	auto pObject = pCellHere->FirstObject;
	// Check if there are objects in the linked list
	if (pObject)
	{
		// When it exists, start the vanilla processing
		R->ESI(pObject);
		return CheckThisObject;
	}
	// When it does not exist, check AltObject linked list
	pObject = pCellHere->AltObject;
	// If there is also no object in the linked list, return directly to check the next cell
	if (!pObject)
		return CheckNextCell;
	// If there is an object, record the flag
	DamageAreaTemp::CheckingCellAlt = true;
	// Then return and continue with the original execution
	R->ESI(pObject);
	return CheckThisObject;
}

ASMJIT_PATCH(0x4899B3, DamageArea_DamageItemsFix2, 0x5)
{
	enum { CheckNextCell = 0x4899BE, CheckThisObject = 0x4896DD };
	// When there are no units in the FirstObject linked list, it will not enter this hook
	GET(const ObjectClass*, pObject, ESI);
	// As vanilla, first look at the next object in the linked list
	pObject = pObject->NextObject;
	// Check if there are still objects in the linked list
	if (pObject)
	{
		// When it exists, return to continue the vanilla processing
		R->ESI(pObject);
		return CheckThisObject;
	}
	// When it does not exist, check which linked list it is currently in
	if (DamageAreaTemp::CheckingCellAlt)
	{
		// If it is already in the AltObject linked list, reset the flag and return to check the next cell
		DamageAreaTemp::CheckingCellAlt = false;
		return CheckNextCell;
	}
	// If it is still in the FirstObject linked list, take the first object in the AltObject linked list and continue checking
	pObject = DamageAreaTemp::CheckingCell->AltObject;
	// If there is no object in the AltObject linked list, return directly to check the next cell
	if (!pObject)
		return CheckNextCell;
	// If there is an object, record the flag
	DamageAreaTemp::CheckingCellAlt = true;
	// Then return and continue with the original execution
	R->ESI(pObject);
	return CheckThisObject;
}

ASMJIT_PATCH(0x489286, DamageArea, 0x6)
{
	GET_BASE(WarheadTypeClass*, pWH, 0x0C);
	if (auto const pWHExt = WarheadTypeExtContainer::Instance.TryFind(pWH))
	{
		GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);
		GET(CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(HouseClass*, pHouse, 0x14);

		if(!Phobos::Config::HideShakeEffects){
			if (!pWHExt->ShakeIsLocal || TacticalClass::Instance->IsCoordsToClientVisible(*pCoords))
			{
				if (pWH->ShakeXhi || pWH->ShakeXlo)
					GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeXhi, pWH->ShakeXlo), pWHExt->Shake_UseAlternativeCalculation);
				if (pWH->ShakeYhi || pWH->ShakeYlo)
					GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeYhi, pWH->ShakeYlo), pWHExt->Shake_UseAlternativeCalculation);
			}
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

ASMJIT_PATCH(0x489968, DamageArea_PenetratesIronCurtain, 0x5)
{
	enum { BypassInvulnerability = 0x48996D };
	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
	if (WarheadTypeExtContainer::Instance.Find(pWarhead)->PenetratesIronCurtain)
		return BypassInvulnerability;
	return 0;
}

ASMJIT_PATCH(0x48947F, MapClass_DamageArea_AirDamageSelfFix, 0x6)
{
	enum { DamageSelf = 0x0, Continue = 0x489547 };

	GET(TechnoClass*, pAirTechno, EBX);
	GET_BASE(TechnoClass*, pSourceTechno, 0x8);
	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);

	return (pAirTechno != pSourceTechno) || (pWarhead->_GetExtData()->AllowDamageOnSelf
			|| pAirTechno->GetTechnoType()->DamageSelf) ? DamageSelf : Continue;
}

ASMJIT_PATCH(0x4896EC, DamageAread_DamageSelf, 0x6)
{
	enum { DamageSelf = 0x489702, Continue = 0x0 };

	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);
	return pWarhead->_GetExtData()->AllowDamageOnSelf ? DamageSelf : Continue;
}

#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x4899DA, DamageArea_Damage_MaxAffect, 7)
{
	REF_STACK(DynamicVectorClass<DamageGroup*>, groupvec, 0xE0 - 0xA8);
	GET_BASE(HouseClass*, pSrcHouse, 0x14);
	GET_BASE(TechnoClass*, pSrcTechno, 0x8);
	GET_BASE(FakeWarheadTypeClass*, pWarhead, 0xC);
	GET_STACK(const bool, isNullified, STACK_OFFSET(0xE0, -0xC9));
	GET_STACK(bool, hitted, STACK_OFFSET(0xE0, -0xC1));
	GET_STACK(CoordStruct*, pCrd, STACK_OFFSET(0xE0, -0xB8));
	GET_STACK(int, damage, STACK_OFFSET(0xE0, -0xBC));

	auto pWHExt = pWarhead->_GetExtData();
	if(!isNullified && pWHExt->AffectsUnderground){
		const bool cylinder = pWHExt->CellSpread_Cylinder;
		const float spread = pWarhead->CellSpread * (float)Unsorted::LeptonsPerCell;

		for (auto const& pTechno : ScenarioExtData::Instance()->UndergroundTracker) {
			if (pTechno->InWhichLayer() == Layer::Underground // Layer.
				&& pTechno->IsAlive && !pTechno->IsIronCurtained()
				&& !pTechno->IsOnMap // Underground is not on map.
				&& !pTechno->InLimbo)
			{
				double dist = 0.0;
				auto const technoCoords = pTechno->GetCoords();

				if (cylinder)
					dist = CoordStruct{ technoCoords.X - pCrd->X, technoCoords.Y - pCrd->Y, 0 }.Length();
				else
					dist = technoCoords.DistanceFrom(*pCrd);

				if (dist <= spread)
				{
					pTechno->ReceiveDamage(&damage, (int)dist, pWarhead, pSrcTechno, false, false, pSrcHouse);
					hitted = true;
				}
			}
		}

		for (int i = 0; i < groupvec.Count; ++i) {
			if (groupvec.Items[i] && (!groupvec.Items[i]->Target->IsAlive || groupvec.Items[i]->Target->Health <= 0 || !groupvec.Items[i]->Target)) {
				GameDelete(std::exchange(groupvec.Items[i], nullptr));
				groupvec.erase_at(i);
			}
		}
	}


	const int MaxAffect = pWHExt->CellSpread_MaxAffect;

	if (MaxAffect > 0)
	{
		const auto g_end = groupvec.Items + groupvec.Count;

		for (auto g_begin = groupvec.Items; g_begin != g_end; ++g_begin)
		{

			DamageGroup* group = *g_begin;
			// group could have been cleared by previous iteration.
			// only handle if has not been handled already.
			if (group && Targets.insert_unique(group->Target))
			{

				Handled.reset();

				// collect all slots containing damage groups for this target
				std::for_each(g_begin, g_end, [group](DamageGroup* item)
 {
	 if (item && item->Target == group->Target)
	 {
		 Handled.push_back(item);
	 }
				});

				// if more than allowed, sort them and remove the ones further away
				if (Handled.size() > MaxAffect)
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
		groupvec.erase_if([](DamageGroup* pGroup)
 {
	 if (!pGroup->Target || !pGroup->Target->IsAlive) {
		 GameDelete<false, false>(pGroup);
		 return true;
	 }

	 return false;
		});

		Targets.reset();
		Handled.reset();
	}

	if(pWHExt->MergeBuildingDamage.Get(RulesExtData::Instance()->MergeBuildingDamage)){
		// Because during the process of causing damage, fragments may be generated that need to continue causing damage, resulting in nested calls
		// to this function. Therefore, a single global variable cannot be used to store this data.
		std::unordered_map<BuildingClass*, double> MapBuildings;

		{
			const auto cellSpread = int(pWarhead->CellSpread * Unsorted::LeptonsPerCell);
			const auto percentDifference = 1.0 - pWarhead->PercentAtMax; // Vanilla will first multiply the damage and round it up, but we don't need to.

			for (const auto& group : groupvec) {
				if (const auto pBuilding = cast_to<BuildingClass* , true>(group->Target)) {
					if (group->Distance > cellSpread)
						continue;

					// Calculate the distance damage ratio in advance
					const auto multiplier = (cellSpread && percentDifference) ? 1.0 - (percentDifference * group->Distance / cellSpread) : 1.0;
					MapBuildings[pBuilding] += multiplier > 0 ? multiplier : 0;
				}
			}
		}

		for (const auto& group : groupvec) // Causing damage to the building alone and avoiding repeated injuries later.
		{
			if (const auto pBuilding = cast_to<BuildingClass* , true>(group->Target))
			{
				if (pBuilding->IsAlive
					&& !pBuilding->Type->InvisibleInGame
					&& (!isNullified || pBuilding->IsIronCurtained())
					&& pBuilding->Health > 0
					&& pBuilding->IsOnMap
					&& !pBuilding->InLimbo
					&& MapBuildings.contains(pBuilding))
				{
					auto receiveDamage = int(damage * MapBuildings[pBuilding]);
					MapBuildings.erase(pBuilding);

					if (!receiveDamage && damage)
						receiveDamage = Math::signum(damage);

					// Set the distance coefficient to 0
					pBuilding->ReceiveDamage(&receiveDamage, 0, pWarhead, pSrcTechno, false, false, pSrcHouse);
					hitted = true;
				}
			}
		}

		for (int i = 0; i < groupvec.Count; ++i){
			if(groupvec.Items[i] && (!groupvec.Items[i]->Target->IsAlive || groupvec.Items[i]->Target->Health <= 0|| !groupvec.Items[i]->Target)) {
				GameDelete(std::exchange(groupvec.Items[i], nullptr));
				groupvec.erase_at(i);
			}
		}
	}


	if (hitted)
		R->Stack8(STACK_OFFSET(0xE0, -0xC1), true);

	return 0;
}

ASMJIT_PATCH(0x489A1B, DamageArea_DamageBuilding_SkipVanillaBuildingDamage, 0x6)
{
	enum { SkipGameCode = 0x489AC1 };

	GET_BASE(FakeWarheadTypeClass*, pWH, 0x0C);
	return pWH->_GetExtData()->MergeBuildingDamage.Get(RulesExtData::Instance()->MergeBuildingDamage) ?
	SkipGameCode : 0;
}

ASMJIT_PATCH(0x489AD6, DamageArea_Damage_AfterLoop, 6)
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
	//GET_STACK(bool, alt, 0xE0 - 0xC2);
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
					auto _PcellHere = MapClass::Instance->GetCellAt(CellStruct(xpos, ypos));

					for (auto object1 = _PcellHere->FirstObject; object1; object1 = object1->NextObject)
					{
						if (!object1->IsAlive)
							continue;

						if (FootClass* techno = flag_cast_to<FootClass*, false>(object1))
						{
							if (xpos == pCell->X && ypos == pCell->Y && pSource)
							{
								Coordinate rockercoord = (pSource->GetCoords() - techno->GetCoords());
								Vector3D<double> rockervec = Vector3D<double>((double)rockercoord.X, (double)rockercoord.Y, (double)rockercoord.Z).Normalized() * 10.0f;
								CoordStruct rock_((int)rockervec.X, (int)rockervec.Y, (int)rockervec.Z);
								auto _result_rock = pCoord->operator+(rock_);
								techno->RockByValue(&_result_rock, (float)rockerSpread);
							}
							else if (pWarhead->CellSpread > 0.0f)
							{
								techno->RockByValue(pCoord, (float)rockerSpread);
							}
						}
					}

					for (auto object2 = _PcellHere->AltObject; object2; object2 = object2->NextObject)
					{
						if (!object2->IsAlive)
							continue;

						if (FootClass* techno = flag_cast_to<FootClass*, false>(object2))
						{
							if (xpos == pCell->X && ypos == pCell->Y && pSource)
							{
								Coordinate rockercoord = (pSource->GetCoords() - techno->GetCoords());
								Vector3D<double> rockervec = Vector3D<double>((double)rockercoord.X, (double)rockercoord.Y, (double)rockercoord.Z).Normalized() * 10.0f;
								CoordStruct rock_((int)rockervec.X, (int)rockervec.Y, (int)rockervec.Z);
								auto _result_rock = pCoord->operator+(rock_);
								techno->RockByValue(&_result_rock, (float)rockerSpread);
							}
							else if (pWarhead->CellSpread > 0.0f)
							{
								techno->RockByValue(pCoord, (float)rockerSpread);
							}
						}
					}
				}
			}
		}
	}

	return 0x489E87;
}

// hook up the area damage delivery with chain reactions
ASMJIT_PATCH(0x48964F, DamageArea_CellChainReaction, 5)
{
	GET(CellClass*, pCell, EBX);
	pCell->ChainReaction();
	return 0;
}

ASMJIT_PATCH(0x4892BE, DamageArea_NullDamage, 0x6)
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
ASMJIT_PATCH(0x4895B8, DamageArea_CellSpread1, 0x6)
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
ASMJIT_PATCH(0x4895C7, DamageArea_CellSpread2, 0x8)
{
	GET_STACK(CellSpreadEnumerator<std::numeric_limits<short>::max()>*, pIter, STACK_OFFS(0xE0, 0xB4));

	R->DX((*pIter)->X);
	R->AX((*pIter)->Y);

	return 0x4895D7;
}

// advance and delete if done
ASMJIT_PATCH(0x4899BE, DamageArea_CellSpread3, 0x8)
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

//ASMJIT_PATCH(0x4896EC, MapClass_DamageArea_DamageSelf, 0x6) {
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

ASMJIT_PATCH(0x48A2D9, DamageArea_ExplodesThreshold, 6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET_STACK(int, damage, 0x24);

	return pOverlay->Explodes && damage >= RulesExtData::Instance()->OverlayExplodeThreshold
		? 0x48A2E7 : 0x48A433;
}

ASMJIT_PATCH(0x489E9F, DamageArea_BridgeAbsoluteDestroyer, 5)
{
	GET(WarheadTypeClass*, pWH, EBX);
	GET(WarheadTypeClass*, pIonCannonWH, EDI);
	R->Stack(0x13, WarheadTypeExtContainer::Instance.Find(pWH)->BridgeAbsoluteDestroyer.Get(pWH == pIonCannonWH));
	return 0x489EA4;
}

ASMJIT_PATCH(0x489FD8, DamageArea_BridgeAbsoluteDestroyer2, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A004 : 0x489FE0;
}

ASMJIT_PATCH(0x48A15D, DamageArea_BridgeAbsoluteDestroyer3, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A188 : 0x48A165;
}

ASMJIT_PATCH(0x48A229, DamageArea_BridgeAbsoluteDestroyer4, 6)
{
	return  R->Stack<bool>(0xF) ? 0x48A250 : 0x48A231;
}

ASMJIT_PATCH(0x48A283, DamageArea_BridgeAbsoluteDestroyer5, 6)
{
	return R->Stack<bool>(0xF) ? 0x48A2AA : 0x48A28B;
}

ASMJIT_PATCH(0x4893BA, DamageArea_DamageAir, 0x9)
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

ASMJIT_PATCH(0x489562, DamageArea_DestroyCliff, 6)
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

// Cylinder CellSpread
ASMJIT_PATCH(0x489430, MapClass_DamageArea_Cylinder_1, 0x7)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET_STACK(int, nVictimCrdZ, STACK_OFFSET(0xE0, -0x5C));

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4894C1, MapClass_DamageArea_Cylinder_2, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ESI);

	//auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x48979C, MapClass_DamageArea_Cylinder_3, 0x8)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4897C3, MapClass_DamageArea_Cylinder_4, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x48985A, MapClass_DamageArea_Cylinder_5, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	if (pWH->_GetExtData()->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

ASMJIT_PATCH(0x4898BF, MapClass_DamageArea_Cylinder_6, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ECX);

	if (pWH->_GetExtData()->CellSpread_Cylinder) {
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

// AffectsInAir and AffectsGround
ASMJIT_PATCH(0x489416, MapClass_DamageArea_CheckHeight_AircraftTarcker, 0x6)
{
	enum { SkipThisObject = 0x489547 };

	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, EBX);

	auto pWHExt = pWH->_GetExtData();

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsGround && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

ASMJIT_PATCH(0x489710, MapClass_DamageArea_CheckHeight_2, 0x7)
{
	enum { SkipThisObject = 0x4899B3 };

	GET_BASE(FakeWarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, ESI);

	auto pWHExt = pWH->_GetExtData();

	if (pWHExt->AffectsInAir && pWHExt->AffectsGround)
		return 0;

	if (!pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsGround && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

#endif

<<<<<<< HEAD
//DamageState __fastcall TT_ReceiveDamage(TechnoClass* pThis, discard_t,
//	int* Damage,
//	int DistanceToEpicenter,
//	WarheadTypeClass* WH,
//	TechnoClass* Attacker,
//	bool IgnoreDefenses,
//	bool PreventsPassengerEscape,
//	HouseClass* SourceHouse)
//{
//	return pThis->ReceiveDamage(Damage, DistanceToEpicenter, WH, Attacker, IgnoreDefenses, PreventsPassengerEscape, SourceHouse);
//}
//
//DEFINE_FUNCTION_JUMP(CALL6, 0x489AB6, TT_ReceiveDamage);
=======
// DamageState __fastcall TT_ReceiveDamage(TechnoClass* pThis, discard_t,
// 	int* Damage,
// 	int DistanceToEpicenter,
// 	WarheadTypeClass* WH,
// 	TechnoClass* Attacker,
// 	bool IgnoreDefenses,
// 	bool PreventsPassengerEscape,
// 	HouseClass* SourceHouse)
// {
// 	return pThis->ReceiveDamage(Damage, DistanceToEpicenter, WH, Attacker, IgnoreDefenses, PreventsPassengerEscape, SourceHouse);
// }

// DEFINE_FUNCTION_JUMP(CALL6, 0x489AB6, TT_ReceiveDamage);
>>>>>>> origin/Adjusment_for_scorpionModCrash
