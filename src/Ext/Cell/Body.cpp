#include "Body.h"

#include <TiberiumClass.h>

#include <Utilities/Macro.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/Mouse/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include <TacticalClass.h>
#include <IsometricTileTypeClass.h>
#include <BuildingClass.h>
#include <TerrainClass.h>
#include <ScenarioClass.h>

#include <cmath>
#include <algorithm>

ASMJIT_PATCH(0x47DA74, CellClass_RecalcAttributes_TileAnimDrawer, 0x7)
{
	enum { SkipGameCode = 0x47DA7B };

	GET(AnimClass*, pAnim, EAX);

	pAnim->__lighting__celldraw_196 = AnimTypeExtContainer::Instance.Find(pAnim->Type)->TheaterPalette.Get(true);

	return SkipGameCode;
}

int FakeCellClass::_Reduce_Tiberium(int levels)
{
	RectangleStruct dirty = RectangleStruct::Union(this->Overlay_Render_Rect(), this->Overlay_Shadow_Render_Rect());
	dirty.Y -= DSurface::ViewBounds->Y;

	int tibtype = this->GetContainedTiberiumIndex();
	int reducer = levels;

	if (levels > 0 && tibtype != -1)
	{
		TiberiumClass* tiberium = TiberiumClass::Array->Items[tibtype];
		if (this->OverlayData == 11)
		{
			tiberium->RegisterForGrowth(&this->MapCoords);
		}
		if (this->OverlayData + 1 > levels)
		{
			OverlayData -= levels;
			reducer = levels;
		}
		else
		{
			PassabilityType passability = this->Passability;
			this->OverlayTypeIndex = -1;
			reducer = OverlayData;
			this->OverlayData = 0;
			this->RecalcAttributes(-1);

			if (passability != this->Passability)
			{
				MapClass::Instance->ResetZones(this->MapCoords);
				MapClass::Instance->RecalculateSubZones(this->MapCoords);
			}

			RadarClass::Instance->Push_Cell(&this->MapCoords);
			auto pTibExt = TiberiumExtContainer::Instance.Find(tiberium);

			pTibExt->Clear_Tiberium_Spread_State(this->MapCoords);

			for (int facing = 0; facing < 8; facing++) {
				auto adjacent = this->GetAdjacentCell((FacingType)facing);
				if (MapClass::Instance->IsWithinUsableArea(adjacent,false)) {
					if (!pTibExt->SpreadState[TiberiumExtData::Map_Cell_Index(adjacent->MapCoords)]) {
						tiberium->Queue_Spread_At_Cell(&adjacent->MapCoords);
					}
				}
			}
		}
		TacticalClass::Instance->RegisterDirtyArea(dirty, false);
		return reducer;
	}
	return 0;
}

int FakeCellClass::_GetRampLevel(CellStruct* where)
{
	int level = this->GetRampLevel(where);
	if (level > 11)
		return -1;

	return level;
}

TiberiumClass* CellExtData::GetTiberium(CellClass* pCell)
{
	int overlay_ = CellExtData::GetTiberiumType(pCell->OverlayTypeIndex);

	if (overlay_ != -1)
		if (const auto pTiberium = TiberiumClass::Array->get_or_default(overlay_))
			return pTiberium;

	return nullptr;
}

int CellExtData::GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium)
{
		return (pCell->SlopeIndex > 0) ?
			(pCell->SlopeIndex + pTiberium->Image->ArrayIndex + pTiberium->NumImages - 1) : (pTiberium->Image->ArrayIndex + pCell->MapCoords.X * pCell->MapCoords.Y % pTiberium->NumImages)
			;
}

int CellExtData::GetOverlayIndex(CellClass* pCell)
{
	if (pCell->OverlayTypeIndex != -1) {
		if (const auto pTiberium = TiberiumClass::Find(pCell->OverlayTypeIndex)) {
			return GetOverlayIndex(pCell, pTiberium);
		}
	}

	return 0 ;
}

#include <OverlayClass.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Terrain/Body.h>

int FakeCellClass::_GetTiberiumType()
{
    if (this->OverlayTypeIndex == -1) {
        return -1;
    }

	auto pOverlay = OverlayTypeClass::Array->Items[this->OverlayTypeIndex];
    if (!pOverlay->Tiberium || TiberiumClass::Array->Count <= 0) {
        return -1;
    }


	for(auto pTib : *TiberiumClass::Array) {
		const auto v5 = pTib->Image->ArrayIndex;
		if(this->OverlayTypeIndex >= v5 && this->OverlayTypeIndex < (v5 + pTib->NumImages)) {
			return pTib->ArrayIndex;
		}

		int NumImages = pTib->NumImages;
        if (this->OverlayTypeIndex >= NumImages + v5 && this->OverlayTypeIndex < v5 + NumImages + pTib->SlopeFrames ) {
             return pTib->ArrayIndex;
         }
	}

	//Debug::LogInfo("Overlay [%s - %s] not really tiberium[%d]", pOverlay->ID , pOverlay->Name , 0);
	return 0;
}

bool FakeCellClass::_SpreadTiberium(bool force)
{
	int tib_ = -1;
	if (!force && !ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
		return false;

	tib_ = CellExtData::GetTiberiumType(this->OverlayTypeIndex);

	if (!force)
	{

		if (tib_ == -1
			  || this->OverlayData <= tib_ / 2
			  || this->SlopeIndex
			  || TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
			  || this->FirstObject)
		{
			return false;
		}

	}
	else
	{
		if (tib_ == -1)
		{
			tib_ = 0;
		}
	}

	auto pTib = TiberiumClass::Array->Items[tib_];
	auto facing = (BYTE)ScenarioClass::Instance->Random.RandomRangedSpecific(FacingType::Min, FacingType::Max);
	int index = 0;
	CellClass* newcell = nullptr;

	while (true)
	{
		const auto v9 = ((BYTE)index + facing) & 7;

		if (v9 < 8)
		{
			newcell = MapClass::Instance->GetCellAt(CellSpread::AdjacentCell[v9] + this->MapCoords);
		}
		else
		{
			newcell = this;
		}
		if (newcell && newcell->CanTiberiumGerminate(pTib))
		{
			break;
		}

		if (++index >= 8)
		{
			return false;
		}
	}

	return newcell->IncreaseTiberium(tib_, 3);
}

int __fastcall CellExtData::GetTiberiumType(int Overlay)
{

	if (Overlay == -1)
	{
		return -1;
	}

	auto pOverlay = OverlayTypeClass::Array->Items[Overlay];
	if (!pOverlay->Tiberium || TiberiumClass::Array->Count <= 0)
	{
		return -1;
	}


	for (auto pTib : *TiberiumClass::Array)
	{
		const auto v5 = pTib->Image->ArrayIndex;
		if (Overlay >= v5 && Overlay < (v5 + pTib->NumImages))
		{
			return pTib->ArrayIndex;
		}

		int NumImages = pTib->NumImages;
		if (Overlay >= NumImages + v5 && Overlay < v5 + NumImages + pTib->SlopeFrames)
		{
			return pTib->ArrayIndex;
		}
	}

	//Debug::LogInfo("Overlay [%s - %s] not really tiberium[%d]", pOverlay->ID , pOverlay->Name , 0);
	return 0;
}

bool FakeCellClass::_SpreadTiberium_2(TerrainClass* pTerrain, bool force)
{
	if (!pTerrain)
		Debug::FatalErrorAndExit(__FUNCTION__" Need `TerrainClass` !");

	auto pTerrainTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

	size_t tib_ = pTerrainTypeExt->SpawnsTiberium_Type;

	if (tib_ >= (size_t)TiberiumClass::Array->Count)
		tib_ = CellExtData::GetTiberiumType(this->OverlayTypeIndex);

	if (!force)
	{
		if (!ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
		{
			return false;
		}

		if (tib_ >= (size_t)TiberiumClass::Array->Count || (TiberiumClass::Array->Items[tib_]->SlopeFrames <= 0 && this->SlopeIndex))
			return false;

		if (TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
			|| this->FirstObject)
		{
			return false;
		}

	}
	else
	{
		if (tib_ >= (size_t)TiberiumClass::Array->Count)
		{
			tib_ = 0;
		}
	}

	auto pTib = TiberiumClass::Array->Items[tib_];
	auto pTerrainExt = TerrainExtContainer::Instance.Find(pTerrain);
	size_t size = pTerrainExt->AdjacentCells.size();

	for (int i = 0; i < (int)size; i++)
	{
		const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
		CellClass* tgtCell = MapClass::Instance->GetCellAt(this->MapCoords + pTerrainExt->AdjacentCells[(i + rand) % size]);
		int growth = pTerrainTypeExt->GetTiberiumGrowthStage();
		growth -= int(pTerrainTypeExt->SpawnsTiberium_StageFalloff * i);
		growth = std::clamp(growth, 0, pTib->NumFrames - 1);

		if (tgtCell->CanTiberiumGerminate(pTib))
		{
			return tgtCell->IncreaseTiberium(tib_, growth);
		}
	}

	return false;
}

void FakeCellClass::_Invalidate(AbstractClass* ptr, bool removed)
{
	auto pExt = this->_GetExtData();

	if (removed)
	{
		if (ptr == static_cast<void*>(this->AltObject)) {
			//Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalid Alt Obj {}", (void*)this, this->MapCoords.X , this->MapCoords.Y , (void*)this->AltObject);
			this->AltObject = nullptr;
		}

		if (ptr == static_cast<void*>(this->FirstObject)) {
			//Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalid Obj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)this->FirstObject);
			this->FirstObject = nullptr;
		}

		if(pExt) {
			if (ptr == static_cast<void*>(pExt->IncomingUnit)) {
				//Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalid IncomingObj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)pExt->IncomingUnit);
				this->OccupationFlags &= ~0x20;
				pExt->IncomingUnit = nullptr;
			}

			if (ptr == static_cast<void*>(pExt->IncomingUnitAlt)) {
				//Debug::LogInfo("Cell {} - at ( {} . {} ) with Invalid IncomingAltObj {}", (void*)this, this->MapCoords.X, this->MapCoords.Y, (void*)pExt->IncomingUnitAlt);
				this->AltOccupationFlags &= ~0x20;
				pExt->IncomingUnitAlt = nullptr;
			}
		}
	}
}

// ============================ =
// load / save
template <typename T>
void CellExtData::Serialize(T& Stm) {

	Stm
		.Process(this->NewPowerups)
		.Process(this->InfantryCount)
		.Process(this->IncomingUnit)
		.Process(this->IncomingUnitAlt)
		.Process(this->RadSites)
		.Process(this->RadLevels)
		;
}

// =============================
// container

CellExtContainer CellExtContainer::Instance;

// =============================
// container hooks

ASMJIT_PATCH(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);

	if(!Phobos::Otamaa::DoingLoadGame || pItem == CellClass::Instance())
		CellExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x47BB60, CellClass_DTOR, 0x6) {
	GET(CellClass*, pItem, ECX);

	CellExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4F14, FakeCellClass::_Invalidate);

//// =============================
//// MouseClassExt::__NearByLocation implementation
//// Backport of MapClass::NearByLocation (0x56DC20)
//
//// Maximum number of candidate cells to collect
//static constexpr int MaxCandidates = 24;
//
////**
// * Reimplementation of CellClass_can_enter_cell at 0x486FF0
// * Checks if a cell can be entered for burrowing/subterranean purposes.
// */
//static bool CanEnterCell(CellClass* pCell)
//{
//	if (!pCell)
//		return false;
//
//	// Check 1: If not in radar/usable area, cell is "enterable"
//	if (!MapClass::Instance->IsWithinUsableArea(pCell, true))
//		return true;
//
//	// Check 2: TileType and AllowBurrowing
//	int tileType = pCell->IsoTileTypeIndex;
//	if (tileType >= 0 && tileType < IsometricTileTypeClass::Array->Count)
//	{
//		IsometricTileTypeClass* pTile = IsometricTileTypeClass::Array->Items[tileType];
//		if (pTile && !pTile->AllowBurrowing)
//			return false;
//	}
//
//	// Check 3: Ramp (SlopeIndex)
//	if (pCell->SlopeIndex != 0)
//		return false;
//
//	// Check 4: Flags check (Bridge and other flags)
//	if ((pCell->UINTFlags & 0x500) != 0)
//		return false;
//
//	// Check 5: GameActive - if game not running, allow entry
//	if (!Game::IsActive.get())
//		return true;
//
//	// Check 6: Loop through occupiers looking for buildings
//	for (ObjectClass* pObj = pCell->FirstObject; pObj != nullptr; pObj = pObj->NextObject)
//	{
//		if (pObj->WhatAmI() == AbstractType::Building)
//			return false;
//	}
//
//	// Check 7: GameActive again
//	if (!Game::IsActive.get())
//		return true;
//
//	// Check 8: Loop through occupiers looking for terrain
//	for (ObjectClass* pObj = pCell->FirstObject; pObj != nullptr; pObj = pObj->NextObject)
//	{
//		if (pObj->WhatAmI() == AbstractType::Terrain)
//			return false;
//	}
//
//	return true;
//}
//
//// Check if foundation rect is buildable
//static bool IsBuildableRect(MapClass* pMap, const CellStruct& cell, int sizeX, int sizeY)
//{
//	for (int x = 0; x < sizeX; ++x)
//	{
//		for (int y = 0; y < sizeY; ++y)
//		{
//			CellStruct checkCell = {
//				static_cast<short>(cell.X + x),
//				static_cast<short>(cell.Y + y)
//			};
//
//			auto pCheckCell = pMap->TryGetCellAt(checkCell);
//			if (!pCheckCell)
//				return false;
//
//			if (pCheckCell->GetBuilding())
//				return false;
//
//			if (pCheckCell->GetTerrain(false))
//				return false;
//		}
//	}
//	return true;
//}
//
//// Check if a cell is visible on the tactical screen
//static bool IsCellOnScreen(TacticalClass* pTactical, const CellStruct& cell)
//{
//	if (!pTactical)
//		return true;
//
//	CoordStruct worldCoord = {
//		(cell.X << 8) + 128,
//		(cell.Y << 8) + 128,
//		0
//	};
//
//	CellStruct result;
//	pTactical->CoordsToCell(&result, &worldCoord);
//
//	return (result.X == cell.X && result.Y == cell.Y);
//}
//
//CellStruct* MouseClassExt::__NearByLocation(
//	CellStruct* pOutBuffer,
//	const CellStruct* pPosition,
//	SpeedType speed,
//	int zone,
//	MovementZone movementZone,
//	bool alt,
//	int spaceSizeX,
//	int spaceSizeY,
//	bool disallowOverlay,
//	bool checkLevel,
//	bool requireBurrowable,
//	bool allowBridge,
//	const CellStruct* pCloseTo,
//	bool skipFirstCheck,
//	bool checkBuildable)
//{
//	//Debug::Log("MouseClassExt::__NearByLocation called at position (%d, %d)\n", pPosition->X, pPosition->Y);
//
//	const int posX = pPosition->X;
//	const int posY = pPosition->Y;
//
//	// Handle zone type - 0xFFFF means None (-1)
//	if (zone == 0xFFFF)
//		zone = -1;
//
//	// Get the starting cell and its level
//	CellClass* pStartCell = this->GetCellAt(*pPosition);
//	int baseLevel = pStartCell->Level;
//
//	// If alt flag set and cell has bridge, add bridge levels
//	if (alt) {
//		if (pStartCell->ContainsBridge()) {
//			baseLevel += Unsorted::BridgeLevels;
//		}
//	}
//
//	// Calculate maximum search radius (map dimensions, capped at 32)
//	int maxRadius = this->MapSize->Width + this->MapSize->Height;
//	if (maxRadius > 32)
//		maxRadius = 32;
//
//	if (maxRadius <= 0)
//	{
//		Debug::Log("MapClass::NearByLocation: No candidate cells found.\n");
//		*pOutBuffer = CellStruct::Empty;
//		return pOutBuffer;
//	}
//
//	// Storage for candidate cells
//	CellStruct candidates[MaxCandidates];
//	int candidateCount = 0;
//	bool foundVisibleCell = false;
//
//	// Search in expanding squares around the center
//	for (int radius = 0; radius < maxRadius && candidateCount < MaxCandidates && !foundVisibleCell; ++radius)
//	{
//		// Top and bottom edges
//		for (int dx = -radius; dx <= radius && candidateCount < MaxCandidates; ++dx)
//		{
//			// Top edge: (posX + dx, posY - radius)
//			if (!skipFirstCheck)
//			{
//				CellStruct testCell = {
//					static_cast<short>(posX + dx),
//					static_cast<short>(posY - radius)
//				};
//
//				CellClass* pTestCell = this->GetCellAt(testCell);
//
//				if (this->IsWithinUsableArea(pTestCell, true))
//				{
//					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
//					{
//						bool passLevelCheck = true;
//
//						if (checkLevel)
//						{
//							int cellLevel = pTestCell->Level;
//							if (pTestCell->ContainsBridge())
//								cellLevel += Unsorted::BridgeLevels;
//
//							int levelDiff = baseLevel - cellLevel;
//							if (levelDiff < 0)
//								levelDiff = -levelDiff;
//
//							if (levelDiff >= 2)
//								passLevelCheck = false;
//						}
//
//						if (passLevelCheck)
//						{
//							if (!requireBurrowable || CanEnterCell(pTestCell))
//							{
//								if (allowBridge || !pTestCell->ContainsBridge())
//								{
//									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
//									{
//										candidates[candidateCount++] = testCell;
//
//										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
//										{
//											foundVisibleCell = true;
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//
//			if (candidateCount >= MaxCandidates)
//				break;
//
//			if (skipFirstCheck && dx <= -radius)
//				continue;
//
//			// Bottom edge: (posX + dx, posY + radius)
//			{
//				CellStruct testCell = {
//					static_cast<short>(posX + dx),
//					static_cast<short>(posY + radius)
//				};
//
//				CellClass* pTestCell = this->GetCellAt(testCell);
//
//				if (this->IsWithinUsableArea(pTestCell, true))
//				{
//					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
//					{
//						bool passLevelCheck = true;
//						if (checkLevel)
//						{
//							int cellLevel = pTestCell->Level;
//							if (pTestCell->ContainsBridge())
//								cellLevel += Unsorted::BridgeLevels;
//
//							int levelDiff = baseLevel - cellLevel;
//							if (levelDiff < 0)
//								levelDiff = -levelDiff;
//
//							if (levelDiff >= 2)
//								passLevelCheck = false;
//						}
//
//						if (passLevelCheck)
//						{
//							if (!requireBurrowable || CanEnterCell(pTestCell))
//							{
//								if (allowBridge || !pTestCell->ContainsBridge())
//								{
//									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
//									{
//										candidates[candidateCount++] = testCell;
//
//										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
//										{
//											foundVisibleCell = true;
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//
//		if (candidateCount >= MaxCandidates)
//			break;
//
//		// Left and right edges (excluding corners already covered)
//		for (int dy = 1 - radius; dy <= radius - 1 && candidateCount < MaxCandidates; ++dy)
//		{
//			// Left edge: (posX - radius, posY + dy)
//			if (!skipFirstCheck)
//			{
//				CellStruct testCell = {
//					static_cast<short>(posX - radius),
//					static_cast<short>(posY + dy)
//				};
//
//				CellClass* pTestCell = this->GetCellAt(testCell);
//
//				if (this->IsWithinUsableArea(pTestCell, true))
//				{
//					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
//					{
//						bool passLevelCheck = true;
//						if (checkLevel)
//						{
//							int cellLevel = pTestCell->Level;
//							if (pTestCell->ContainsBridge())
//								cellLevel += Unsorted::BridgeLevels;
//
//							int levelDiff = baseLevel - cellLevel;
//							if (levelDiff < 0)
//								levelDiff = -levelDiff;
//
//							if (levelDiff >= 2)
//								passLevelCheck = false;
//						}
//
//						if (passLevelCheck)
//						{
//							if (!requireBurrowable || CanEnterCell(pTestCell))
//							{
//								if (allowBridge || !pTestCell->ContainsBridge())
//								{
//									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
//									{
//										candidates[candidateCount++] = testCell;
//
//										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
//										{
//											foundVisibleCell = true;
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//
//			if (candidateCount >= MaxCandidates)
//				break;
//
//			// Right edge: (posX + radius, posY + dy)
//			{
//				CellStruct testCell = {
//					static_cast<short>(posX + radius),
//					static_cast<short>(posY + dy)
//				};
//
//				CellClass* pTestCell = this->GetCellAt(testCell);
//
//				if (this->IsWithinUsableArea(pTestCell, true))
//				{
//					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
//					{
//						bool passLevelCheck = true;
//						if (checkLevel)
//						{
//							int cellLevel = pTestCell->Level;
//							if (pTestCell->ContainsBridge())
//								cellLevel += Unsorted::BridgeLevels;
//
//							int levelDiff = baseLevel - cellLevel;
//							if (levelDiff < 0)
//								levelDiff = -levelDiff;
//
//							if (levelDiff >= 2)
//								passLevelCheck = false;
//						}
//
//						if (passLevelCheck)
//						{
//							if (!requireBurrowable || CanEnterCell(pTestCell))
//							{
//								if (allowBridge || !pTestCell->ContainsBridge())
//								{
//									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
//									{
//										candidates[candidateCount++] = testCell;
//
//										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
//										{
//											foundVisibleCell = true;
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//
//	// No candidates found - return default cell
//	if (candidateCount <= 0) {
//		Debug::Log("MapClass::NearByLocation: No candidate cells found.\n");
//		*pOutBuffer = CellStruct::Empty;
//		return pOutBuffer;
//	}
//
//	// Phase 2: Separate candidates into visible and off-screen lists
//	CellStruct visibleCells[MaxCandidates];
//	CellStruct offscreenCells[MaxCandidates];
//	int visibleCount = 0;
//	int offscreenCount = 0;
//
//	for (int i = 0; i < candidateCount; ++i)
//	{
//		const CellStruct& cell = candidates[i];
//
//		if (IsCellOnScreen(TacticalClass::Instance, cell))
//		{
//			visibleCells[visibleCount++] = cell;
//		}
//		else
//		{
//			offscreenCells[offscreenCount++] = cell;
//		}
//	}
//
//	// Prefer visible cells
//	CellStruct* selectedList = (visibleCount > 0) ? visibleCells : offscreenCells;
//	int selectedCount = (visibleCount > 0) ? visibleCount : offscreenCount;
//
//	if (selectedCount <= 0)
//	{
//		Debug::Log("MapClass::NearByLocation: No candidate cells found.\n");
//		*pOutBuffer = CellStruct::Empty;
//		return pOutBuffer;
//	}
//
//	// Phase 3: Select final cell
//	if (pCloseTo->X == CellStruct::Empty.X && pCloseTo->Y == CellStruct::Empty.Y)
//	{
//		// Random selection based on current frame
//		int index = Unsorted::CurrentFrame.get() % selectedCount;
//		*pOutBuffer = selectedList[index];
//	}
//	else
//	{
//		// Find closest cell to closeTo point
//		double minDist = 1e30;
//		CellStruct closestCell = selectedList[0];
//
//		for (int i = 0; i < selectedCount; ++i)
//		{
//			const CellStruct& cell = selectedList[i];
//			double dist = cell.operator-(*pCloseTo).Length();
//
//			if (dist < minDist)
//			{
//				minDist = dist;
//				closestCell = cell;
//			}
//		}
//
//		*pOutBuffer = closestCell;
//	}
//
//	return pOutBuffer;
//}
//


// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static constexpr int MaxCandidates = 24;   // 0x18 — hard cap in assembly (cmp ..., 18h)
static constexpr int BridgeLevelAdd = 4;    // assembly: shl edx,2 → always 4, NOT Unsorted::BridgeLevels

// ---------------------------------------------------------------------------
// ResolveCellOrWorking
// Falls back to WorkingCellClassInstance on OOB or null slot.
// Assembly: repeated pattern at e.g. 0x56DD63–0x56DD8C
// VERIFY: MapClass_Array.Vector @ 0x0087F924, WorkingCellClassInstance @ 0xABDC50
// ---------------------------------------------------------------------------
static CellClass* ResolveCellOrWorking(int x, int y)
{
	return MapClass::Instance->GetCellAt(CellStruct(x, y));
}

// ---------------------------------------------------------------------------
// ProjectToTactical
// Converts cell coords → tactical projection → back to cell.
// Assembly: (X<<8)+128, (Y<<8)+128, 0 passed as 3-int struct to Tactical_coordmap_math_6D6410.
// Used both to set foundOnMap flag (phase 1) and to split onMap/offMap (phase 2).
// VERIFY: Tactical_coordmap_math_6D6410 signature and CoordInput layout
// ---------------------------------------------------------------------------
static CellStruct ProjectToTactical(const CellStruct& cell)
{
	CoordStruct coordIn {
		(static_cast<int>(cell.X) << 8) + 128,
		(static_cast<int>(cell.Y) << 8) + 128,
		0
	};
	CellStruct out {};
	// VERIFY: argument order — (TacticalClass*, CellStruct* outCell, CoordInput*)
	TacticalClass::Instance->coordmap_math(&out, &coordIn);
	return out;
}

// ---------------------------------------------------------------------------
// AbsLevelDelta
// Assembly: cdq / xor eax,edx / sub eax,edx = portable abs().
// Bridge contribution is always bit8 of Bitfield2, scaled by 4 (not BridgeLevels).
// VERIFY: CellClass::Level @ +0x11B (byte), Bitfield2 @ +0x140 (dword)
// ---------------------------------------------------------------------------
static int AbsLevelDelta(int baseLevel, const CellClass* pCell)
{
	const int bridgeBonus = ((pCell->UINTFlags >> 8) & 1) * BridgeLevelAdd;
	const int delta = baseLevel - static_cast<int>(pCell->Level) - bridgeBonus;
	return std::abs(delta);
}

// ---------------------------------------------------------------------------
// CheckCell
// Runs the 6-condition gate identical across all four ring arms.
// On pass: stores candidate, updates foundOnMap.
// Returns true if cell was accepted.
//
// IMPORTANT — foundOnMap is set but does NOT break the inner loop here.
// The outer ring loop checks it only after completing the full ring.
// Assembly proof: var_1D5 set inside arm bodies, outer-loop break after both arm loops.
// ---------------------------------------------------------------------------
static bool CheckCell(
	MapClass* pMap,
	const CellStruct pos,
	int              a8,
	int              a9,
	SpeedType        speed,
	int              zone,
	MovementZone        check,
	bool             bool1,
	bool              a10,
	char             a11,        // level-delta filter
	bool             a12,        // CanEnter filter
	char             a13,        // 0 = exclude bridge cells
	char             a16,        // rect passability filter
	int              baseLevel,
	CellStruct* candidates,
	int& candidateCount,
	bool& foundOnMap
)
{
	CellClass* pCell = ResolveCellOrWorking(pos.X, pos.Y);

	if (!pMap->IsWithinUsableArea(pCell, true))
		return false;

	if (!pMap->CanMoveHere(pos, a8, a9, speed, zone, check, -1, bool1, a10))
		return false;

	if (a11 && AbsLevelDelta(baseLevel, pCell) >= 2)
		return false;

	if (a12 && !pCell->CanEnterCell())
		return false;

	// a13 == 0 → bridge cells excluded; a13 != 0 → bridge cells allowed
	// Assembly: jnz short skip_bridge_check / test ah,1 / jnz fail
	if (!a13 && (pCell->ContainsBridge()))
		return false;

	if (a16)
	{
		// VERIFY: MapClass_rect_586780 signature — (MapClass*, Rect*, int house)
		// Rect built from {pos.X, pos.Y, a8, a9} in assembly
		RectangleStruct footprint { pos.X, pos.Y, a8, a9 };
		if (!pMap->IsAreaFree(&footprint, -1))
			return false;
	}

	candidates[candidateCount++] = pos;

	// foundOnMap check — skipped entirely when bool1 is true.
	// Assembly: jnz short loc_56DF25 when bool1 != 0, directly sets flag.
	if (bool1)
	{
		foundOnMap = true;
	}
	else
	{
		const CellStruct projected = ProjectToTactical(pos);
		if (projected.X == pos.X && projected.Y == pos.Y)
			foundOnMap = true;
	}

	return true;
}

// ---------------------------------------------------------------------------
// MapClass::Nearby_Location — 0x0056DC20
//
// a15 (skipFirstCheck) gate — IMPORTANT:
//   Assembly does NOT fully skip top/bottom rows when a15=1.
//   It skips only the TOP arm check, and in the left/right column loop
//   it skips entries where dy <= -radius (trim condition).
//   The bottom arm always runs regardless of a15.
//   This matches the assembly `jnz loc_56DF2A` at 0x56DD31 (top arm only)
//   and `jle loc_56E141` at 0x56DF46 (left column trim).
// ---------------------------------------------------------------------------
CellStruct* MouseClassExt::__NearByLocation(
	CellStruct* pOut,
	const CellStruct* pOrigin,
	SpeedType         speed,
	int               zone,
	MovementZone        check,
	bool              bool1,
	int               a8,
	int               a9,
	bool               a10,
	bool              a11,
	bool              a12,
	bool              a13,
	const CellStruct* pHint,    // a14 — MapClass_Default_Cell → random pick; else → nearest
	bool              a15,      // skipFirstCheck / half-ring mode
	bool              a16
) {
	// Normalise zone sentinel
	if (static_cast<unsigned>(zone) == 0xFFFFu)
		zone = -1;

	const int originX = pOrigin->X;
	const int originY = pOrigin->Y;

	// Base level + optional bridge bonus
	// VERIFY: CellClass::Level @ +0x11B, Bitfield2 @ +0x140
	CellClass* pOriginCell = ResolveCellOrWorking(originX, originY);
	int baseLevel = static_cast<int>(pOriginCell->Level);

	if (bool1 && (pOriginCell->ContainsBridge()))
		baseLevel += BridgeLevelAdd;

	// Max radius — capped at 32
	// VERIFY: MapSize.Width @ +0xF4, MapSize.Height @ +0xF8
	const int maxRadius = std::min(MapSize->Width + MapSize->Height, 32);

	if (maxRadius <= 0) {
		*pOut = CellStruct::Empty;
		return pOut;
	}

	CellStruct candidates[MaxCandidates];
	int        candidateCount = 0;
	bool       foundOnMap = false;

	// -----------------------------------------------------------------------
	// Ring expansion
	// -----------------------------------------------------------------------
	for (int radius = 0; radius < maxRadius; ++radius) {
		// -- Top & bottom row arms --
		// Top arm: guarded by !a15 (assembly: jnz loc_56DF2A at 0x56DD31)
		// Bottom arm: always runs regardless of a15
		for (int dx = -radius; dx <= radius; ++dx) {
			if (candidateCount >= MaxCandidates)
				break;

			// Top arm — skipped entirely when a15 != 0
			if (!a15)
			{
				const CellStruct top { static_cast<short>(originX + dx), static_cast<short>(originY - radius) };
				CheckCell(this, top, a8, a9, speed, zone, check, bool1, a10,
					a11, a12, a13, a16, baseLevel, candidates, candidateCount, foundOnMap);
			}

			if (candidateCount >= MaxCandidates)
				break;

			// Bottom arm — always runs
			// Assembly: HIWORD(v88) = v80 + v24.X = originY + radius, no a15 guard
			const CellStruct bot { static_cast<short>(originX + dx), static_cast<short>(originY + radius) };
			CheckCell(this, bot, a8, a9, speed, zone, check, bool1, a10,
				a11, a12, a13, a16, baseLevel, candidates, candidateCount, foundOnMap);
		}

		if (candidateCount >= MaxCandidates)
			break;

		// -- Left & right column arms --
		// dy range: (1 - radius) to (radius - 1) — corners excluded (already hit above)
		// a15 trim: left arm is skipped when dy <= -radius
		// Assembly: 0x56DF40–46: cmp edi, eax / jle loc_56E141 (left arm only)
		for (int dy = 1 - radius; dy <= radius - 1; ++dy)
		{
			if (candidateCount >= MaxCandidates)
				break;

			// Left arm — trimmed by a15 at dy <= -radius
			if (!a15 || dy > -radius)
			{
				const CellStruct left { static_cast<short>(originX - radius), static_cast<short>(originY + dy) };
				CheckCell(this, left, a8, a9, speed, zone, check, bool1, a10,
					a11, a12, a13, a16, baseLevel, candidates, candidateCount, foundOnMap);
			}

			if (candidateCount >= MaxCandidates)
				break;

			// Right arm — always runs (no a15 guard in assembly)
			const CellStruct right { static_cast<short>(originX + radius), static_cast<short>(originY + dy) };
			CheckCell(this, right, a8, a9, speed, zone, check, bool1, a10,
				a11, a12, a13, a16, baseLevel, candidates, candidateCount, foundOnMap);
		}

		// CRITICAL: foundOnMap breaks AFTER the full ring completes, not mid-ring.
		// Assembly: check of var_1D5 is at 0x56E596, after both arm loops exit.
		if (foundOnMap)
			break;
	}

	// -----------------------------------------------------------------------
	// No candidates
	// -----------------------------------------------------------------------
	if (candidateCount <= 0)
	{
		*pOut = CellStruct::Empty;
		return pOut;
	}

	// -----------------------------------------------------------------------
	// Phase 2: split into onMap / offMap using tactical projection
	// This is NOT IsWithinUsableArea — it's the same coordmap math used in phase 1.
	// Assembly: 0x56E5B3–0x56E685
	// -----------------------------------------------------------------------
	CellStruct onMap[MaxCandidates];
	CellStruct offMap[MaxCandidates];
	int        onMapCount = 0;
	int        offMapCount = 0;

	for (int i = 0; i < candidateCount; ++i)
	{
		const CellStruct& cand = candidates[i];
		const CellStruct  projected = ProjectToTactical(cand);

		if (projected.X == cand.X && projected.Y == cand.Y)
			onMap[onMapCount++] = cand;
		else
			offMap[offMapCount++] = cand;
	}

	const CellStruct* pool = (onMapCount > 0) ? onMap : offMap;
	const int         poolCount = (onMapCount > 0) ? onMapCount : offMapCount;

	// -----------------------------------------------------------------------
	// Phase 3: selection
	// pHint == MapClass_Default_Cell → random via Frame % poolCount
	// else → nearest Euclidean to pHint
	// Assembly: 0x56E689–0x56E797
	// VERIFY: Frame global @ 0x00A8ED84, FastMath::Sqrt @ 0x004C3C40
	// -----------------------------------------------------------------------
	if (!pHint->IsValid())
	{
		*pOut = pool[Unsorted::CurrentFrame() % poolCount];
		return pOut;
	}

	// Nearest cell — initial sentinel matches 40F86A00h in assembly (large double)
	double     bestDist = 1.0e15;
	CellStruct best {};

	for (int i = 0; i < poolCount; ++i) {
		const CellStruct& cand = pool[i];
		const int dx = static_cast<int>(cand.X) - static_cast<int>(pHint->X);
		const int dy = static_cast<int>(cand.Y) - static_cast<int>(pHint->Y);
		// Assembly uses fild(distSq) → FastMath::Sqrt → fcom
		const double dist = Math::sqrt(dx * dx + dy * dy);
		if (dist < bestDist)
		{
			bestDist = dist;
			best = cand;
		}
	}

	*pOut = best;
	return pOut;
}

// Hook to replace the original function at 0x56DC20
DEFINE_FUNCTION_JUMP(LJMP, 0x56DC20, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x41A2C0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x41A36B, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x41A4D4, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x443957, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4443DC, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x446A14, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x446CCD, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x446E10, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x44DAF0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x458D2D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4597E3, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x482366, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4CD16F, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4CEA57, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D3B76, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D3DD8, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D44BC, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D69E3, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D6CE1, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D7ABE, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4D7BD3, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4DCB57, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4DE528, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4DF76A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4FBFED, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x4FD2C0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x5002E5, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x504931, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x504B4C, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x504D70, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x506193, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x509D9A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x50C999, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x51D41D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x54B36F, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x54B5DF, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x56BE76, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x56BF69, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x65E11D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x65E94F, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x65EEBB, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x65EF5A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6885B5, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6B0417, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6CD375, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6CD5B6, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EC944, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EE557, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EE78D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EEA59, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EF98A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x6EFC3A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x70369C, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7185D5, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x71900D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x719185, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x728CE5, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x728D3D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7296C1, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x729726, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x73841E, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x738EC8, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x738F40, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x73D7B0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x73DADD, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x73ED75, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x742042, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7430EE, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x743C6B, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744964, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7449B2, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744B39, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744C81, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744D12, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744E4D, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x744F36, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x74501B, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745069, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745182, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745247, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7452D8, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7453DF, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x74547E, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7454F9, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7455F4, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745697, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745712, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x7457E3, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745831, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x74594A, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745A0F, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745AA0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745B77, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745C06, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745CB0, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745CFE, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745E4F, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745F14, MouseClassExt::__NearByLocation);
DEFINE_FUNCTION_JUMP(CALL, 0x745FA5, MouseClassExt::__NearByLocation);

//HRESULT __stdcall FakeCellClass::__Load(IStream* pStm)
//{
//	HRESULT hr = this->CellClass::Load(pStm);
//
//	if (SUCCEEDED(hr)) {
//		if (!CellExtContainer::Instance.LoadByKey(this, pStm))
//			return PHOBOS_E_EXTDATA_LOAD_FAILED;
//	}
//
//	return hr;
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4F00, FakeCellClass::__Load)
//
//HRESULT __stdcall FakeCellClass::__Save(IStream* pStm, BOOL fClearDirty)
//{
//	HRESULT hr = this->CellClass::Save(pStm, fClearDirty);
//
//	if (SUCCEEDED(hr)) {
//		if (!CellExtContainer::Instance.SaveByKey(this, pStm))
//			return PHOBOS_E_EXTDATA_SAVE_FAILED;
//	}
//
//	return hr;
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4F04, FakeCellClass::__Save)
