#pragma once

#include <CoordStruct.h>
#include <CellStruct.h>

#include <Utilities/Helpers.h>

class HouseClass;
class MapClass;
class CellClass;
class MapRevealer
{
public:
	MapRevealer(const CoordStruct& coords);
	MapRevealer(const CoordStruct* pCoords);
	MapRevealer(const CellStruct& cell);
	MapRevealer(const CellStruct* pCell);

	COMPILETIMEEVAL const CellStruct& Base() const
	{
		return this->BaseCell;
	}

	void Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const;

	void Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const;

	void UpdateShroud(short start, size_t radius, bool fog = false) const;

	void Process0(CellClass* pCell, bool unknown, bool fog, bool add) const;

	void Process1(CellClass* pCell, bool fog, bool add) const;

	COMPILETIMEEVAL bool IsCellAllowed(const CellStruct& cell) const
	{
		if (this->RequiredChecks)
		{
			for (const auto& checkedCell : CheckedCells)
			{
				if (checkedCell == cell)
				{
					return false;
				}
			}
		}
		return true;
	}

	COMPILETIMEEVAL bool IsCellAvailable(const CellStruct& cell) const
	{
		auto const sum = cell.X + cell.Y;

		return sum > this->MapWidth
			&& cell.X - cell.Y < this->MapWidth
			&& cell.Y - cell.X < this->MapWidth
			&& sum <= this->MapWidth + 2 * this->MapHeight;
	}

	bool CheckLevel(const CellStruct& offset, int level) const;

	static bool AffectsHouse(HouseClass* const pHouse);

	static COMPILETIMEEVAL bool RequiresExtraChecks()
	{
		return Helpers::Alex::is_any_of(SessionClass::Instance->GameMode, GameMode::LAN, GameMode::Internet) &&
			SessionClass::Instance->MPGameMode && !SessionClass::Instance->MPGameMode->vt_entry_04();
	}

	static COMPILETIMEEVAL CellStruct GetRelation(const CellStruct& offset)
	{
		return{ static_cast<short>(Math::signum(-offset.X)),
			static_cast<short>(Math::signum(-offset.Y)) };
	}

private:
	FORCEDINLINE CellStruct TranslateBaseCell(const CoordStruct& coords) const
	{
		auto const adjust = (Game::AdjustHeight(coords.Z) / -30) << 8;
		auto const baseCoords = coords + CoordStruct { adjust, adjust, 0 };
		return CellClass::Coord2Cell(baseCoords);
	}

	COMPILETIMEEVAL CellStruct GetOffset(const CoordStruct& coords, const CellStruct& base) const
	{
		return base - CellClass::Coord2Cell(coords) - CellStruct { 2, 2 };
	}

	template <typename T>
	void RevealImpl(const CoordStruct& coords, int radius, HouseClass* pHouse, bool onlyOutline, bool allowRevealByHeight, T func) const;

	CellStruct BaseCell;
	CellStruct CellOffset;
	CellStruct CheckedCells[3];
	bool RequiredChecks;
	int MapWidth;
	int MapHeight;

public:
	// Reveal_Area
	static void __fastcall MapClass_RevealArea0(MapClass* pThis, void*, CoordStruct* pCoord,
		int nRadius, HouseClass* pHouse, int bOutlineOnly, bool bNoShroudUpdate, bool bFog,
		bool bAllowRevealByHeight, bool bHideOnRadar)
	{
		MapRevealer const revealer(*pCoord);
		revealer.Reveal0(*pCoord, nRadius, pHouse, bOutlineOnly, bNoShroudUpdate, bFog, bAllowRevealByHeight, bHideOnRadar);
		revealer.UpdateShroud(0, static_cast<size_t>(MaxImpl(nRadius, 0)), false);
	}

	// Sight_From
	static void __fastcall MapClass_RevealArea1(MapClass* pThis, void*, CoordStruct* pCoord,
		int nRadius, HouseClass* pHouse, int bOutlineOnly, bool bNoShroudUpdate, bool bFog,
		bool bAllowRevealByHeight, bool bIncreaseShroudCounter)
	{
		MapRevealer const revealer(*pCoord);
		revealer.Reveal1(*pCoord, nRadius, pHouse, bOutlineOnly, bFog, bAllowRevealByHeight, bIncreaseShroudCounter);
	}

	static void __fastcall MapClass_RevealArea2(MapClass* pThis, void*,
		CoordStruct* Coords, int Height, int Radius, bool bSkipReveal)
	{
		MapRevealer const revealer(*Coords);
		revealer.UpdateShroud(static_cast<short>(MaxImpl(Height, 0)), static_cast<size_t>(MaxImpl(Radius, 0)), bSkipReveal);
	}
};
