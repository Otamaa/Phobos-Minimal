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

	// CONFIRMED against vanilla 0x567686 (`cmp esi, edx / jg skip`): the last term is
	// inclusive, so `<=` is correct here. An earlier note claiming this was an
	// off-by-one against Ares' `<` was wrong.
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
		// Vanilla 0x5673D1-0x5673F6: Adjust_For_Height(Z), then the 0x77777777 magic
		// multiply + `sar 4` idiom, which is division by -30, then << 8. Applied to
		// both X and Y. This matches.
		auto const adjust = (Game::AdjustHeight(coords.Z) / -30) << 8;
		auto const baseCoords = coords + CoordStruct { adjust, adjust, 0 };
		return CellClass::Coord2Cell(baseCoords);
	}

	COMPILETIMEEVAL CellStruct GetOffset(const CoordStruct& coords, const CellStruct& base) const
	{
		// SUSPECT: vanilla derives the unadjusted cell with a plain `/ 256`
		// (0x56743F: `mov eax,[esi] / cdq / sar eax, 8`) while using the
		// Coord2Cell rounding idiom for the adjusted one. Coord2Cell is used for both
		// here. The two only diverge for negative coordinates, which should not occur
		// on a real map, so it is kept uniform.
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
	// Reveal_Area  (vanilla MapClass::Sight_From2, 0x5673A0)
	static void __fastcall MapClass_RevealArea0(MapClass* pThis, void*, CoordStruct* pCoord,
		int nRadius, HouseClass* pHouse, int bOutlineOnly, bool bNoShroudUpdate, bool bFog,
		bool bAllowRevealByHeight, bool bHideOnRadar)
	{
		MapRevealer const revealer(*pCoord);
		revealer.Reveal0(*pCoord, nRadius, pHouse, bOutlineOnly, bNoShroudUpdate, bFog, bAllowRevealByHeight, bHideOnRadar);

		// RETRACTED: an earlier revision added +3 here and made the call conditional,
		// because vanilla 0x5678B7 tails out with Sight_From3(&coord, 0, sightrange+3, 0)
		// after its early `retn` guards. The known-good reference implementation does
		// neither - it calls the update unconditionally with the plain radius:
		//     sub_100CAAB0(...); if (radius < 0) radius = 0; sub_100CA8A0(0, radius, 0);
		// Matching the reference, since that is the build where fog demonstrably works.
		//
		// WORTH A/B TESTING if fog still lingers at the edge of a unit's sight: vanilla
		// covers radius+3 here, and the reference's own enumerator is seeded with
		// radius+1 (its UpdateShroud does `spread = MinImpl(radius + 1, 256)`, while
		// UpdateShroud below uses `MinImpl(radius, 255)`). Both cover strictly more
		// ground than this does, so a stale outer ring is still plausible.
		revealer.UpdateShroud(0, static_cast<size_t>(MaxImpl(nRadius, 0)), false);
	}

	// Sight_From  (vanilla MapClass::Sight_From, 0x5678E0)
	// Correctly has no trailing UpdateShroud - vanilla does not call one here.
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