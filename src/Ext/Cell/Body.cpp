#include "Body.h"

#include <TiberiumClass.h>

#include <Utilities/Macro.h>
#include <Ext/Tiberium/Body.h>
#include <TacticalClass.h>
#include <IsometricTileTypeClass.h>
#include <BuildingClass.h>
#include <TerrainClass.h>
#include <ScenarioClass.h>

#include <Phobos.SaveGame.h>

#include <cmath>
#include <algorithm>

int FakeCellClass::_Reduce_Tiberium(int levels)
{
	if (!MapClass::Instance || !TiberiumClass::Array)
		return 0;

	RectangleStruct dirty = RectangleStruct::Union(this->Overlay_Render_Rect(), this->Overlay_Shadow_Render_Rect());
	dirty.Y -= DSurface::ViewBounds->Y;

	int tibtype = this->GetContainedTiberiumIndex();
	int reducer = levels;

	if (levels > 0 && tibtype != -1 && tibtype < TiberiumClass::Array->Count)
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

TiberiumClass* CellExtData::GetTiberium(CellClass* pCell)
{
	if (!pCell)
		return nullptr;

	int overlay_ = CellExtData::GetTiberiumType(pCell->OverlayTypeIndex);

	if (overlay_ != -1)
		if (const auto pTiberium = TiberiumClass::Array->get_or_default(overlay_))
			return pTiberium;

	return nullptr;
}

int CellExtData::GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium)
{
		if (!pCell || !pTiberium || !pTiberium->Image)
			return 0;

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
	return -1;
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
	return -1;
}

bool FakeCellClass::_SpreadTiberium_2(TerrainClass* pTerrain, bool force)
{
	if (!pTerrain)
		Debug::FatalErrorAndExit(__FUNCTION__" Need `TerrainClass` !");

	auto pTerrainTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

	int tib_ = pTerrainTypeExt->SpawnsTiberium_Type;

	// Check for invalid tiberium type
	if (tib_ < 0 || tib_ >= TiberiumClass::Array->Count)
		tib_ = CellExtData::GetTiberiumType(this->OverlayTypeIndex);

	if (!force)
	{
		if (!ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
		{
			return false;
		}

		if (tib_ < 0 || tib_ >= TiberiumClass::Array->Count || (TiberiumClass::Array->Items[tib_]->SlopeFrames <= 0 && this->SlopeIndex))
			return false;

		if (TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
			|| this->FirstObject)
		{
			return false;
		}

	}
	else
	{
		if (tib_ < 0 || tib_ >= TiberiumClass::Array->Count)
		{
			tib_ = 0;
		}
	}

	// Final validation before use
	if (tib_ < 0 || tib_ >= TiberiumClass::Array->Count)
		return false;

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

bool CellExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(CellExtContainer::ClassName))
	{
		auto& container = root[CellExtContainer::ClassName];

		for (auto& entry : container[CellExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, CellExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool CellExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[CellExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : CellExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[CellExtData::ClassName] = std::move(_extRoot);

	return true;
}

// =============================
// container hooks

ASMJIT_PATCH(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);
	CellExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x47BB60, CellClass_DTOR, 0x6) {
	GET(CellClass*, pItem, ECX);

	CellExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4F14, FakeCellClass::_Invalidate);

// =============================
// FakeMapClass::__NearByLocation implementation
// Backport of MapClass::NearByLocation (0x56DC20)

// Maximum number of candidate cells to collect
static constexpr int MaxCandidates = 24;

/**
 * Reimplementation of CellClass_can_enter_cell at 0x486FF0
 * Checks if a cell can be entered for burrowing/subterranean purposes.
 */
static bool CanEnterCell(CellClass* pCell)
{
	if (!pCell)
		return false;

	// Check 1: If not in radar/usable area, cell is "enterable"
	if (!MapClass::Instance->IsWithinUsableArea(pCell, true))
		return true;

	// Check 2: TileType and AllowBurrowing
	int tileType = pCell->IsoTileTypeIndex;
	if (tileType >= 0 && tileType < IsometricTileTypeClass::Array->Count)
	{
		IsometricTileTypeClass* pTile = IsometricTileTypeClass::Array->Items[tileType];
		if (pTile && !pTile->AllowBurrowing)
			return false;
	}

	// Check 3: Ramp (SlopeIndex)
	if (pCell->SlopeIndex != 0)
		return false;

	// Check 4: Flags check (Bridge and other flags)
	if ((pCell->UINTFlags & 0x500) != 0)
		return false;

	// Check 5: GameActive - if game not running, allow entry
	if (!Game::IsActive)
		return true;

	// Check 6: Loop through occupiers looking for buildings
	for (ObjectClass* pObj = pCell->FirstObject; pObj != nullptr; pObj = pObj->NextObject)
	{
		if (pObj->WhatAmI() == AbstractType::Building)
			return false;
	}

	// Check 7: GameActive again
	if (!Game::IsActive)
		return true;

	// Check 8: Loop through occupiers looking for terrain
	for (ObjectClass* pObj = pCell->FirstObject; pObj != nullptr; pObj = pObj->NextObject)
	{
		if (pObj->WhatAmI() == AbstractType::Terrain)
			return false;
	}

	return true;
}

// Check if foundation rect is buildable
static bool IsBuildableRect(MapClass* pMap, const CellStruct& cell, int sizeX, int sizeY)
{
	for (int x = 0; x < sizeX; ++x)
	{
		for (int y = 0; y < sizeY; ++y)
		{
			CellStruct checkCell = {
				static_cast<short>(cell.X + x),
				static_cast<short>(cell.Y + y)
			};

			auto pCheckCell = pMap->TryGetCellAt(checkCell);
			if (!pCheckCell)
				return false;

			if (pCheckCell->GetBuilding())
				return false;

			if (pCheckCell->GetTerrain(false))
				return false;
		}
	}
	return true;
}

// Check if a cell is visible on the tactical screen
static bool IsCellOnScreen(TacticalClass* pTactical, const CellStruct& cell)
{
	if (!pTactical)
		return true;

	CoordStruct worldCoord = {
		(cell.X << 8) + 128,
		(cell.Y << 8) + 128,
		0
	};

	CellStruct result;
	pTactical->CoordsToCell(&result, &worldCoord);

	return (result.X == cell.X && result.Y == cell.Y);
}

CellStruct* FakeMapClass::__NearByLocation(
	CellStruct* pOutBuffer,
	const CellStruct* pPosition,
	SpeedType speed,
	int zone,
	MovementZone movementZone,
	bool alt,
	int spaceSizeX,
	int spaceSizeY,
	bool disallowOverlay,
	bool checkLevel,
	bool requireBurrowable,
	bool allowBridge,
	const CellStruct* pCloseTo,
	bool skipFirstCheck,
	bool checkBuildable)
{
	Debug::Log("FakeMapClass::__NearByLocation called at position (%d, %d)\n", pPosition->X, pPosition->Y);

	const int posX = pPosition->X;
	const int posY = pPosition->Y;

	// Handle zone type - 0xFFFF means None (-1)
	if (zone == 0xFFFF)
		zone = -1;

	// Get the starting cell and its level
	CellClass* pStartCell = this->GetCellAt(*pPosition);
	int baseLevel = pStartCell->Level;

	// If alt flag set and cell has bridge, add bridge levels
	if (alt)
	{
		CellClass* pAltCell = this->GetCellAt(*pPosition);
		if (pAltCell->ContainsBridge())
		{
			baseLevel += Unsorted::BridgeLevels;
		}
	}

	// Calculate maximum search radius (map dimensions, capped at 32)
	int maxRadius = this->MapSize->Width + this->MapSize->Height;
	if (maxRadius > 32)
		maxRadius = 32;

	if (maxRadius <= 0)
	{
		*pOutBuffer = CellStruct::Empty;
		return pOutBuffer;
	}

	// Storage for candidate cells
	CellStruct candidates[MaxCandidates];
	int candidateCount = 0;
	bool foundVisibleCell = false;

	// Search in expanding squares around the center
	for (int radius = 0; radius < maxRadius && candidateCount < MaxCandidates && !foundVisibleCell; ++radius)
	{
		// Top and bottom edges
		for (int dx = -radius; dx <= radius && candidateCount < MaxCandidates; ++dx)
		{
			// Top edge: (posX + dx, posY - radius)
			if (!skipFirstCheck)
			{
				CellStruct testCell = {
					static_cast<short>(posX + dx),
					static_cast<short>(posY - radius)
				};

				CellClass* pTestCell = this->GetCellAt(testCell);

				if (this->IsWithinUsableArea(pTestCell, true))
				{
					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
					{
						bool passLevelCheck = true;

						if (checkLevel)
						{
							int cellLevel = pTestCell->Level;
							if (pTestCell->ContainsBridge())
								cellLevel += Unsorted::BridgeLevels;

							int levelDiff = baseLevel - cellLevel;
							if (levelDiff < 0)
								levelDiff = -levelDiff;

							if (levelDiff >= 2)
								passLevelCheck = false;
						}

						if (passLevelCheck)
						{
							if (!requireBurrowable || CanEnterCell(pTestCell))
							{
								if (allowBridge || !pTestCell->ContainsBridge())
								{
									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
									{
										candidates[candidateCount++] = testCell;

										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
										{
											foundVisibleCell = true;
										}
									}
								}
							}
						}
					}
				}
			}

			if (candidateCount >= MaxCandidates)
				break;

			if (skipFirstCheck && dx <= -radius)
				continue;

			// Bottom edge: (posX + dx, posY + radius)
			{
				CellStruct testCell = {
					static_cast<short>(posX + dx),
					static_cast<short>(posY + radius)
				};

				CellClass* pTestCell = this->GetCellAt(testCell);

				if (this->IsWithinUsableArea(pTestCell, true))
				{
					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
					{
						bool passLevelCheck = true;
						if (checkLevel)
						{
							int cellLevel = pTestCell->Level;
							if (pTestCell->ContainsBridge())
								cellLevel += Unsorted::BridgeLevels;

							int levelDiff = baseLevel - cellLevel;
							if (levelDiff < 0)
								levelDiff = -levelDiff;

							if (levelDiff >= 2)
								passLevelCheck = false;
						}

						if (passLevelCheck)
						{
							if (!requireBurrowable || CanEnterCell(pTestCell))
							{
								if (allowBridge || !pTestCell->ContainsBridge())
								{
									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
									{
										candidates[candidateCount++] = testCell;

										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
										{
											foundVisibleCell = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (candidateCount >= MaxCandidates)
			break;

		// Left and right edges (excluding corners already covered)
		for (int dy = 1 - radius; dy <= radius - 1 && candidateCount < MaxCandidates; ++dy)
		{
			// Left edge: (posX - radius, posY + dy)
			if (!skipFirstCheck)
			{
				CellStruct testCell = {
					static_cast<short>(posX - radius),
					static_cast<short>(posY + dy)
				};

				CellClass* pTestCell = this->GetCellAt(testCell);

				if (this->IsWithinUsableArea(pTestCell, true))
				{
					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
					{
						bool passLevelCheck = true;
						if (checkLevel)
						{
							int cellLevel = pTestCell->Level;
							if (pTestCell->ContainsBridge())
								cellLevel += Unsorted::BridgeLevels;

							int levelDiff = baseLevel - cellLevel;
							if (levelDiff < 0)
								levelDiff = -levelDiff;

							if (levelDiff >= 2)
								passLevelCheck = false;
						}

						if (passLevelCheck)
						{
							if (!requireBurrowable || CanEnterCell(pTestCell))
							{
								if (allowBridge || !pTestCell->ContainsBridge())
								{
									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
									{
										candidates[candidateCount++] = testCell;

										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
										{
											foundVisibleCell = true;
										}
									}
								}
							}
						}
					}
				}
			}

			if (candidateCount >= MaxCandidates)
				break;

			// Right edge: (posX + radius, posY + dy)
			{
				CellStruct testCell = {
					static_cast<short>(posX + radius),
					static_cast<short>(posY + dy)
				};

				CellClass* pTestCell = this->GetCellAt(testCell);

				if (this->IsWithinUsableArea(pTestCell, true))
				{
					if (this->CanMoveHere(testCell, spaceSizeX, spaceSizeY, speed, zone, movementZone, -1, alt, disallowOverlay))
					{
						bool passLevelCheck = true;
						if (checkLevel)
						{
							int cellLevel = pTestCell->Level;
							if (pTestCell->ContainsBridge())
								cellLevel += Unsorted::BridgeLevels;

							int levelDiff = baseLevel - cellLevel;
							if (levelDiff < 0)
								levelDiff = -levelDiff;

							if (levelDiff >= 2)
								passLevelCheck = false;
						}

						if (passLevelCheck)
						{
							if (!requireBurrowable || CanEnterCell(pTestCell))
							{
								if (allowBridge || !pTestCell->ContainsBridge())
								{
									if (!checkBuildable || IsBuildableRect(this, testCell, spaceSizeX, spaceSizeY))
									{
										candidates[candidateCount++] = testCell;

										if (alt || IsCellOnScreen(TacticalClass::Instance, testCell))
										{
											foundVisibleCell = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// No candidates found - return default cell
	if (candidateCount <= 0)
	{
		*pOutBuffer = CellStruct::Empty;
		return pOutBuffer;
	}

	// Phase 2: Separate candidates into visible and off-screen lists
	CellStruct visibleCells[MaxCandidates];
	CellStruct offscreenCells[MaxCandidates];
	int visibleCount = 0;
	int offscreenCount = 0;

	for (int i = 0; i < candidateCount; ++i)
	{
		const CellStruct& cell = candidates[i];

		if (IsCellOnScreen(TacticalClass::Instance, cell))
		{
			visibleCells[visibleCount++] = cell;
		}
		else
		{
			offscreenCells[offscreenCount++] = cell;
		}
	}

	// Prefer visible cells
	CellStruct* selectedList = (visibleCount > 0) ? visibleCells : offscreenCells;
	int selectedCount = (visibleCount > 0) ? visibleCount : offscreenCount;

	if (selectedCount <= 0)
	{
		*pOutBuffer = CellStruct::Empty;
		return pOutBuffer;
	}

	// Phase 3: Select final cell
	if (pCloseTo->X == CellStruct::Empty.X && pCloseTo->Y == CellStruct::Empty.Y)
	{
		// Random selection based on current frame
		int index = Unsorted::CurrentFrame % selectedCount;
		*pOutBuffer = selectedList[index];
	}
	else
	{
		// Find closest cell to closeTo point
		double minDist = 1e30;
		CellStruct closestCell = selectedList[0];

		for (int i = 0; i < selectedCount; ++i)
		{
			const CellStruct& cell = selectedList[i];
			int dx = cell.X - pCloseTo->X;
			int dy = cell.Y - pCloseTo->Y;
			double distSq = static_cast<double>(dx * dx + dy * dy);
			double dist = std::sqrt(distSq);

			if (dist < minDist)
			{
				minDist = dist;
				closestCell = cell;
			}
		}

		*pOutBuffer = closestCell;
	}

	return pOutBuffer;
}

// Hook to replace the original function at 0x56DC20
DEFINE_FUNCTION_JUMP(LJMP, 0x56DC20, FakeMapClass::__NearByLocation);
