#include "MapRevealer.h"

#include <MouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/Macro.h>

#include <YRMath.h>
#include <HouseClass.h>
#include <CellClass.h>

#include <TacticalClass.h>

bool MapRevealer::AffectsHouse(HouseClass* const pHouse)
{
	auto Player = HouseClass::CurrentPlayer();

	if (pHouse == Player)
	{
		return true;
	}

	if (!pHouse || !Player)
	{
		return false;
	}

	return pHouse->RadarVisibleTo.Contains(Player) ||
		(RulesClass::Instance->AllyReveal && pHouse->IsAlliedWith(Player));
}

bool MapRevealer::CheckLevel(const CellStruct& offset, int level) const
{
	auto const cellLevel = this->Base() + offset + GetRelation(offset) - this->CellOffset;
	return MapClass::Instance->GetCellAt(cellLevel)->Level < level + Unsorted::BridgeLevels;
}

MapRevealer::MapRevealer(const CoordStruct& coords) :
	BaseCell {},
	CellOffset {},
	CheckedCells {},
	RequiredChecks {},
	MapWidth {},
	MapHeight {}
{

	const auto base = this->TranslateBaseCell(coords);
	this->BaseCell = base;
	this->CellOffset = this->GetOffset(coords, base);
	this->RequiredChecks = this->RequiresExtraChecks();
	auto const& Rect = MapClass::Instance->MapRect;
	this->MapWidth = Rect.Width;
	this->MapHeight = Rect.Height;

	this->CheckedCells[0].X = 7;
	this->CheckedCells[0].Y = static_cast<short>(this->MapWidth + 5);

	this->CheckedCells[1].X = 13;
	this->CheckedCells[1].Y = static_cast<short>(this->MapWidth + 11);

	this->CheckedCells[2].X = static_cast<short>(this->MapHeight + 13);
	this->CheckedCells[2].Y = static_cast<short>(this->MapHeight + this->MapWidth - 15);
}

MapRevealer::MapRevealer(const CoordStruct* pCoords) : MapRevealer { *pCoords } {}

MapRevealer::MapRevealer(const CellStruct& cell) :
	MapRevealer(MapClass::Instance->GetCellAt(cell)->GetCoordsWithBridge())
{
}

MapRevealer::MapRevealer(const CellStruct* pCell) :
	MapRevealer(MapClass::Instance->GetCellAt(*pCell)->GetCoordsWithBridge())
{
}

static reference<int ,0xABDE88> SightFrom {};

template <typename T>
void MapRevealer::RevealImpl(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool const onlyOutline, bool const allowRevealByHeight, T func) const
{
	auto const level = coords.Z / SightFrom();
	auto const& base = this->Base();

	if (this->AffectsHouse(pHouse) && this->IsCellAvailable(base) && radius > 0)
	{
		auto const spread = MinImpl(size_t(radius), CellSpreadEnumerator::Max);
		auto const spread_limit_sqr = (spread + 1) * (spread + 1);

		auto const start = (!RulesClass::Instance->RevealByHeight && onlyOutline && spread > 2)
			? spread - 3 : 0u;

		auto const checkLevel = allowRevealByHeight && RulesClass::Instance->RevealByHeight;

		for (CellSpreadEnumerator it(spread, start); it; ++it) {
			auto const cell = base + *it;

			if (this->IsCellAvailable(cell)) {
				if (Math::abs(it->X) <= static_cast<int>(spread) && it->pow() < spread_limit_sqr) {
					if (!checkLevel || this->CheckLevel(*it, level)) {
						func(MapClass::Instance->GetCellAt(cell));
					}
				}
			}
		}
	}
};

void MapRevealer::Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell) {
		this->Process0(pCell, unknown, fog, add);
	});
}

void MapRevealer::Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell) {
		this->Process1(pCell, fog, add);
	});
}

void MapRevealer::UpdateShroud(size_t start, size_t radius, bool fog) const
{
	if (!fog)
	{
		auto const& base = this->Base();
		radius = MinImpl(radius, CellSpreadEnumerator::Max);
		start = MinImpl(start, CellSpreadEnumerator::Max - 3);

		for (CellSpreadEnumerator it(radius, start); it; ++it)
		{
			auto const cell = base + *it;
			auto  pCell = MapClass::Instance->GetCellAt(cell);

			auto shroudOcculusion = TacticalClass::Instance->GetOcclusion(cell, false);
			if (pCell->Visibility != shroudOcculusion)
			{
				pCell->Visibility = shroudOcculusion;
				pCell->VisibilityChanged = true;
				TacticalClass::Instance->RegisterCellAsVisible(pCell);
			}
		}
	}
}

void MapRevealer::Process0(CellClass* const pCell, bool unknown, bool fog, bool add) const
{
	pCell->UINTFlags &= ~0x40;

	if (this->IsCellAllowed(pCell->MapCoords)) {
		if (fog) {
			if ((pCell->UINTFlags & 0x3) != 0x3 && pCell->UINTFlags & 0x8) {
				MouseClass::Instance->MapCellFoggedness(pCell->MapCoords, HouseClass::CurrentPlayer());
			}
		} else {
			if ((pCell->UINTAltFlags & 0x18) != 0x18
				|| (pCell->UINTFlags & 3) != 3)
			{
				if (!unknown)
				{
					if (add)
					{
						MouseClass::Instance->RevealFogShroud(pCell->MapCoords, HouseClass::CurrentPlayer(), false);
					}
					else
					{
						pCell->Unshroud();
					}
				}
			}
		}
	}
}

void MapRevealer::Process1(CellClass* const pCell, bool fog, bool add) const
{
	pCell->UINTFlags &= ~0x40;

	if (fog) {
		if ((pCell->UINTFlags & 3) != 3 && pCell->UINTAltFlags & 0x08) {
			MouseClass::Instance->MapCellFoggedness(pCell->MapCoords, HouseClass::CurrentPlayer());
		}
	} else {
		if (this->IsCellAllowed(pCell->MapCoords)) {
			MouseClass::Instance->RevealFogShroud(pCell->MapCoords, HouseClass::CurrentPlayer, add);
		}
	}
}

#ifdef aaa
DEFINE_HOOK(0x5673A0, MapClass_RevealArea0, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, radius, 0x8);
	GET_STACK(HouseClass*, pHouse, 0xC);
	GET_STACK(bool, onlyOutline, 0x10);
	GET_STACK(bool, a6, 0x14);
	GET_STACK(bool, fog, 0x18);
	GET_STACK(bool, allowRevealByHeight, 0x1C);
	GET_STACK(bool, add, 0x20);

	MapRevealer const revealer(pCoords);
	revealer.Reveal0(*pCoords, radius, pHouse, onlyOutline, a6, fog, allowRevealByHeight, add);
	revealer.UpdateShroud(0, static_cast<size_t>(MaxImpl(radius, 0)), false);

	return 0x5678D6;
}

DEFINE_HOOK(0x5678E0, MapClass_RevealArea1, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, radius, 0x8);
	GET_STACK(HouseClass*, pHouse, 0xC);
	GET_STACK(bool, onlyOutline, 0x10);
	//GET_STACK(bool, a6, 0x14);
	GET_STACK(bool, fog, 0x18);
	GET_STACK(bool, allowRevealByHeight, 0x1C);
	GET_STACK(bool, add, 0x20);

	MapRevealer const revealer(pCoords);
	revealer.Reveal1(*pCoords, radius, pHouse, onlyOutline, fog, allowRevealByHeight, add);

	return 0x567D8F;
}

DEFINE_HOOK(0x567DA0, MapClass_RevealArea2, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, start, 0x8);
	GET_STACK(int, radius, 0xC);
	GET_STACK(bool, fog, 0x10);

	MapRevealer const revealer(pCoords);
	revealer.UpdateShroud(static_cast<size_t>(MaxImpl(start, 0)), static_cast<size_t>(MaxImpl(radius, 0)), fog);

	return 0x567F61;
}
#else
DEFINE_JUMP(LJMP, 0x5673A0, MiscTools::to_DWORD(&MapRevealer::MapClass_RevealArea0));
DEFINE_JUMP(LJMP, 0x5678E0, MiscTools::to_DWORD(&MapRevealer::MapClass_RevealArea1));
DEFINE_JUMP(LJMP, 0x567DA0, MiscTools::to_DWORD(&MapRevealer::MapClass_RevealArea2));
#endif