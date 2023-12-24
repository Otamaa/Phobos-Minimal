#pragma once

#include <CoordStruct.h>
#include <CellStruct.h>

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

	const CellStruct& Base() const
	{
		return this->BaseCell;
	}

	void Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const;

	void Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const;

	void UpdateShroud(size_t start, size_t radius, bool fog = false) const;

	void Process0(CellClass* pCell, bool unknown, bool fog, bool add) const;

	void Process1(CellClass* pCell, bool fog, bool add) const;

	bool IsCellAllowed(const CellStruct& cell) const
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

	bool IsCellAvailable(const CellStruct& cell) const;
	bool CheckLevel(const CellStruct& offset, int level) const;

	static bool AffectsHouse(HouseClass* const pHouse);
	static bool RequiresExtraChecks();
	static CellStruct GetRelation(const CellStruct& offset);

private:
	CellStruct TranslateBaseCell(const CoordStruct& coords) const;
	CellStruct GetOffset(const CoordStruct& coords, const CellStruct& base) const;

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
		revealer.UpdateShroud(static_cast<size_t>(MaxImpl(Height, 0)), static_cast<size_t>(MaxImpl(Radius, 0)), bSkipReveal);
	}
};
