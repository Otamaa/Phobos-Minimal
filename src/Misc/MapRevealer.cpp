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
	// Vanilla 0x5677A6: keep the cell when Level <= level + 3 (it does
	// `add ecx, 3 / cmp eax, ecx / jg skip`). With Unsorted::BridgeLevels == 4 this
	// `<` form is equivalent. VERIFY: vanilla walks a precomputed direction table
	// (dword_ABCF60) in lockstep with the spread array rather than deriving the step
	// from signum(-offset); the two agree for every entry I checked, but it is worth
	// a spot-check if RevealByHeight maps ever look wrong.
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

	// VERIFY: vanilla reads [ebp+0F4h] / [ebp+0F8h] (MapClass::MapSize.Width/Height).
	// Confirm MapRect is the same pair of fields in YRpp and not VisibleRect.
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
{}

MapRevealer::MapRevealer(const CellStruct* pCell) :
	MapRevealer(MapClass::Instance->GetCellAt(*pCell)->GetCoordsWithBridge())
{}

static COMPILETIMEEVAL reference<int, 0xABDE88> SightFrom {};

template <typename T>
void MapRevealer::RevealImpl(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool const onlyOutline, bool const allowRevealByHeight, T func) const
{
	auto const level = coords.Z / SightFrom();
	auto const& base = this->Base();

	if (!this->AffectsHouse(pHouse) || !this->IsCellAvailable(base) || radius <= 0)
		return;

	// DIFF: vanilla clamps the sight range to 10 (0x5674F6: `cmp eax, 0Bh / jge` then
	// `mov [sightrange], 0Ah`) because it indexes a fixed spread table. The
	// CellSpreadEnumerator has no such limit, so this keeps the Ares-style 255 cap.
	// Note this also widens the trailing UpdateShroud radius - see MapClass_RevealArea0.
	auto const spread = MinImpl(short(radius), 255);
	auto const spread_limit_sqr = (spread + 1) * (spread + 1);

	auto const start = (!RulesClass::Instance->RevealByHeight && onlyOutline && spread > 2)
		? spread - 3 : 0u;

	auto const checkLevel = allowRevealByHeight && RulesClass::Instance->RevealByHeight;

	for (CellSpreadEnumerator it((short)spread, (short)start); it; ++it)
	{
		auto const cell = base + *it;

		if (this->IsCellAvailable(cell))
		{
			// Matches vanilla 0x56769E / 0x5676EE: skip when abs(dX) > spread, or when
			// the truncated sqrt of the squared distance exceeds spread - which is
			// exactly d^2 < (spread + 1)^2.
			if (Math::abs(it->X) <= static_cast<int>(spread) && it->pow() < spread_limit_sqr)
			{
				if (!checkLevel || this->CheckLevel(*it, level))
				{
					func(MapClass::Instance->GetCellAt(cell));
				}
			}
		}
	}

};

void MapRevealer::Reveal0(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool unknown, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell)
 {
	 this->Process0(pCell, unknown, fog, add);
	});
}

void MapRevealer::Reveal1(const CoordStruct& coords, int const radius, HouseClass* const pHouse, bool onlyOutline, bool fog, bool allowRevealByHeight, bool add) const
{
	this->RevealImpl(coords, radius, pHouse, onlyOutline, allowRevealByHeight, [=](CellClass* const pCell)
 {
	 this->Process1(pCell, fog, add);
	});
}

void MapRevealer::UpdateShroud(short start, size_t radius, bool fog) const
{
	if (fog)
		return;

	auto const& base = this->Base();
	radius = MinImpl(radius, 255);
	start = MinImpl(start, 255 - 3);

	for (CellSpreadEnumerator it((short)radius, start); it; ++it)
	{
		// BUGFIX: the `#else` (_FOW) variant of this loop only recomputed Visibility
		// (shroud occlusion) and dropped the Foggedness half entirely - no
		// GetOcclusion(cell, true), no Foggedness write, no RegisterCellAsVisible on a
		// foggedness change. The known-good reference has no such split.
		//
		// This is THE fog-lifting mechanism. Vanilla Sight_From2 ends at 0x5678B7 with
		// Sight_From3(&adjustedCoord, 0, sightrange + 3, 0), which lands here; nothing
		// else on the reveal path clears fog. In particular CellClass::Unshroud
		// (0x4876F0) only does `AltFlags |= 0x18` and, when ShroudCounter > 0,
		// `Flags |= 0x20` - it never touches CellFlags::Fogged or Foggedness.
		// So with _FOW defined, fog never lifted from anything.
		// The two variants are merged here; if the split existed for a reason,
		// re-introduce it around the Foggedness block only and say why.
		auto const& offset = *it;
		auto const cell = base + offset;
		auto const pCell = MapClass::Instance->GetCellAt(cell);

		bool bVisibilityChanged = false;

		// VERIFY: 0xFF is the "occluded completely" sentinel for the signed char
		// Visibility / Foggedness fields (i.e. -1 read as unsigned). Kept verbatim.
		if (pCell->Visibility != 0xFF)
		{
			auto const shroudOcclusion = TacticalClass::Instance->GetOcclusion(cell, false);
			if (pCell->Visibility != shroudOcclusion)
			{
				pCell->Visibility = shroudOcclusion;
				pCell->VisibilityChanged = true;
				bVisibilityChanged = true;
			}
		}

		// Restructured to mirror the reference exactly. Note the Foggedness occlusion is
		// only queried when FogOfWar is enabled AND the cell is not fully occluded - the
		// previous shape ran the query unconditionally once past the early `continue`,
		// which is both slower and can write Foggedness with fog turned off.
		char foggedOcclusion = 0;
		bool bFoggednessChanged = false;

		if (ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar && pCell->Foggedness != 0xFF)
		{
			foggedOcclusion = TacticalClass::Instance->GetOcclusion(cell, true);
			bFoggednessChanged = pCell->Foggedness != foggedOcclusion;
		}

		if (bFoggednessChanged)
			pCell->Foggedness = foggedOcclusion;
		else if (!bVisibilityChanged)
			continue;

		TacticalClass::Instance->RegisterCellAsVisible(pCell);
	}
}

// Vanilla per-cell body of MapClass::Sight_From2 (0x5673A0), from 0x5677B5.
void MapRevealer::Process0(CellClass* const pCell, bool unknown, bool fog, bool add) const
{
	// 0x5677C7: `and esi, 0FFFFFFBFh` on [edi+140h] - always, before either branch.
	pCell->Flags &= ~CellFlags::IsPlot;

	// In Sight_From2 the checked-cell test guards BOTH branches (the `a5a` block at
	// 0x5677D2 falls through to LABEL_56 only when no checked cell matched). Note this
	// differs from Process1 below, where it guards the non-fog branch only.
	if (this->IsCellAllowed(pCell->MapCoords))
	{
		if (fog)
		{
			// BUGFIX, confirmed twice: this read `pCell->UINTFlags & 0x8`.
			// Vanilla 0x567837 is `test byte ptr [edi+12Ch], 8`, and the reference
			// reads `*(_BYTE *)(v21 + 300) & 8`. 300 == 0x12C == AltFlags; Flags is
			// 0x140 (== 320), used two lines earlier in both. So the second term is
			// AltCellFlags::Mapped, matching Process1.
			if ((pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed
				&& (pCell->AltFlags & AltCellFlags::Mapped) != AltCellFlags::Empty)
			{
				MouseClass::Instance->MapCellFoggedness(pCell->MapCoords, HouseClass::CurrentPlayer());
			}
		}
		else
		{
			// 0x567856: (AltFlags & 8) == 0 || (AltFlags & 0x10) == 0
			//           || (Flags & 1) == 0 || (Flags & 2) == 0
			if ((pCell->AltFlags & AltCellFlags::Clear) != AltCellFlags::Clear
				|| (pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed)
			{
				if (!unknown)
				{
					// CONFIRMED twice - vanilla 0x56787A and the reference's
					// `if (BYTE2(a8)) Map_Cell2(..., 0) else Unshroud()`. The last
					// argument really is a branch selector, and the third argument to
					// Map_Cell2 really is a hardcoded 0, unlike Process1 which forwards
					// it. Earlier suspicion that this was a misread was wrong.
					if (add)
					{
						MouseClass::Instance->RevealFogShroud(pCell->MapCoords, HouseClass::CurrentPlayer(), false);
					}
					else
					{
						// CellClass::Unshroud (0x4876F0) is:
						//     AltFlags |= 0x18                      (Mapped | NoFog)
						//     if (ShroudCounter > 0) Flags |= 0x20
						// It is a *soft* reveal: it does not zero ShroudCounter, does not
						// clear CellFlags::Fogged and does not call CleanFog. The cell
						// re-shrouds once whatever was holding ShroudCounter down goes
						// away, and its fog only lifts via the UpdateShroud pass.
						pCell->Unshroud();
					}
				}
			}
		}
	}
}

// Vanilla per-cell body of MapClass::Sight_From (0x5678E0), from 0x567CCB.
void MapRevealer::Process1(CellClass* const pCell, bool fog, bool add) const
{
	pCell->Flags &= ~CellFlags::IsPlot;

	if (fog)
	{
		// 0x567CF8: `test byte ptr [esi+12Ch], 8` -> AltCellFlags::Mapped.
		// Not guarded by the checked-cell test in vanilla - only the else branch is.
		if ((pCell->Flags & CellFlags::Revealed) != CellFlags::Revealed
			&& (pCell->AltFlags & AltCellFlags::Mapped) != AltCellFlags::Empty)
		{
			MouseClass::Instance->MapCellFoggedness(pCell->MapCoords, HouseClass::CurrentPlayer());
		}
	}
	else
	{
		if (this->IsCellAllowed(pCell->MapCoords))
		{
			// 0x567D60: Map_Cell2(&Map, &a1, house, a9) - a9 IS forwarded here.
			MouseClass::Instance->RevealFogShroud(pCell->MapCoords, HouseClass::CurrentPlayer(), add);
		}
	}
}

#ifdef aaa
ASMJIT_PATCH(0x5673A0, MapClass_RevealArea0, 5)
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
	if (revealer.Reveal0(*pCoords, radius, pHouse, onlyOutline, a6, fog, allowRevealByHeight, add))
		revealer.UpdateShroud(0, static_cast<size_t>(MaxImpl(radius, 0)) + 3, false);

	return 0x5678D6;
}

ASMJIT_PATCH(0x5678E0, MapClass_RevealArea1, 5)
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

ASMJIT_PATCH(0x567DA0, MapClass_RevealArea2, 5)
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
DEFINE_FUNCTION_JUMP(LJMP, 0x5673A0, MapRevealer::MapClass_RevealArea0);
DEFINE_FUNCTION_JUMP(LJMP, 0x5678E0, MapRevealer::MapClass_RevealArea1);
DEFINE_FUNCTION_JUMP(LJMP, 0x567DA0, MapRevealer::MapClass_RevealArea2);
#endif