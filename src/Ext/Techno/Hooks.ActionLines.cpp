
#include "Body.h"
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

namespace
{
	// -------------------------------------------------------------------------
	// Globals
	// -------------------------------------------------------------------------
	constexpr reference<CDTimerClass, 0xB0EA80u> const ActionLineTimer {};


	// Palette table indices used by vanilla for the two line kinds.
	constexpr int ColorIdx_TargetLine = 8;
	constexpr int ColorIdx_MoveLine = 3;


	// -------------------------------------------------------------------------
	// Vanilla callees
	// -------------------------------------------------------------------------
	namespace Vanilla
	{

		// 0x7049C0 — IDA: TechnoClass__action_lines_7049C0
		// Stack layout recovered from the two call sites:
		//   [esp+0x00] CoordStruct from   (12 bytes)
		//   [esp+0x0C] CoordStruct to     (12 bytes)
		//   [esp+0x18] ColorStruct color  (3 bytes, padded to 4)
		//   [esp+0x1C] int  a4
		//   [esp+0x20] int  a5
		// VERIFY: names of a4 / a5.
		//
		// Your tree already aliases this address as
		//   Drawing::Draw_action_lines_7049C0(x1,y1,z1, x2,y2,z2, int color, bool, bool)
		// i.e. the same layout with the coords flattened and ColorStruct passed
		// as ToInit(). That alias drops the this-pointer; vanilla always loads
		// ECX = ESI before the call, so this wrapper keeps it. If you have
		// confirmed the callee never touches ECX, swap this out for the alias
		// and delete the wrapper.
		void DrawActionLine(TechnoClass* pThis, CoordStruct from, CoordStruct to,
			ColorStruct color, int a4, int a5)
		{
			using Func = void(__fastcall*)(TechnoClass*, void*,
				CoordStruct, CoordStruct, ColorStruct, int, int);
			reinterpret_cast<Func>(0x7049C0u)(pThis, nullptr, from, to, color, a4, a5);
		}
	}

	// -------------------------------------------------------------------------
	// Helpers
	// -------------------------------------------------------------------------

	ColorStruct LineColor(int paletteIndex)
	{
		return Drawing::Int_To_RGB(FileSystem::PALETTE_PAL->inline_02(paletteIndex));
	}
}

// =============================================================================
//  Backport
// =============================================================================
class NOVTABLE FakeFootClass final : public FootClass
{
public:
	// 0x4DC060
	static void __fastcall _Draw_Action_Lines(FootClass* pThis, discard_t, bool bForce, int a3)
	{
		AbstractClass* const pTarget = pThis->Target;

		// 0x4DC069
		if (!pTarget && !pThis->Destination)
			return;

		// 0x4DC081 — timer gate is skipped entirely when forced.
		if (!bForce && ActionLineTimer->GetTimeLeft() <= 0)
			return;

		// ---------------------------------------------------------------------
		// 0x4DC0BB — target line
		// ---------------------------------------------------------------------
		if (pTarget)
		{
			CoordStruct from {}; pThis->vt_entry_300(&from, 0);
			CoordStruct to {}; pThis->GetMovingTargetCoords(&to);


			// EXTENSION: folded from
			//   ASMJIT_PATCH(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
			// Original registers -> locals:
			//   EAX = &to (get_Coord result)
			//   EBP / EBX             = from.X / from.Y
			//   STACK_OFFS(0x34,0x10) = from.Z
			// isset() short-circuits the vanilla line even when the colour is
			// Empty — that is the deliberate "draw nothing" opt-out, preserved.
			if (const auto pTypeExt = GET_TECHNOTYPEEXT(pThis))
			{
				if (pTypeExt->CommandLine_Attack_Color.isset())
				{
					const ColorStruct color = pTypeExt->CommandLine_Attack_Color.Fetch();

					if (color != ColorStruct::Empty)
						Vanilla::DrawActionLine(pThis, from, to, color, false, false);

					return; // hook Skip = 0x4DC1A0 (function epilogue)
				}
			}

			Vanilla::DrawActionLine(pThis, from, to, LineColor(ColorIdx_TargetLine), 0, 0);
			return;
		}

		// ---------------------------------------------------------------------
		// 0x4DC1AA — movement line
		// ---------------------------------------------------------------------
		const CoordStruct from = pThis->Location;

		// SUSPECT: vanilla does not null-check the queue entry. Preserved.
		const int navCount = pThis->NavQueue.Count;
		AbstractClass* const pEnd = navCount
			? pThis->NavQueue[navCount - 1]
			: pThis->Destination;

		CoordStruct to = pEnd->GetCoords();

		// 0x4DC205 — signed /256, truncating toward zero (cdq / and 0FFh / sar 8).
		const CellStruct cell {
			static_cast<short>(to.X / 256),
			static_cast<short>(to.Y / 256)
		};

		// 0x4DC23B — lift the endpoint onto the bridge deck when there is one.
		if (MapClass::Instance->CoordinatesLegal(cell))
		{
			CellClass* const pCell = MapClass::Instance->GetCellAt(to);
			if (pCell->ContainsBridge())
				to.Z = Unsorted::BridgeHeight + MapClass::Instance->GetCellFloorHeight(to);
		}

		// EXTENSION: folded from
		//   ASMJIT_PATCH(0x4DC280, FootClass_DrawActionLines_Move, 0x5)
		// Original registers -> locals:
		//   STACK_OFFS(0x34,0x24)   = to (X/Y)      EDI = bridge-adjusted to.Z
		//   EBP / EBX               = from.X/from.Y
		//   STACK_OFFS(0x34,0x10)   = from.Z
		//   STACK_OFFSET(0x34,0x8)  = a3
		// NOTE: the hook read X/Y from the stack copy and Z from EDI separately;
		// here `to` already carries the adjusted Z, so one coord covers both.
		if (const auto pTypeExt = GET_TECHNOTYPEEXT(pThis))
		{
			if (pTypeExt->CommandLine_Move_Color.isset())
			{
				const ColorStruct color = pTypeExt->CommandLine_Move_Color.Fetch();

				if (color != ColorStruct::Empty)
					Vanilla::DrawActionLine(pThis, from, to, color, a3, false);

				return; // hook Skip = 0x4DC328 (function epilogue)
			}
		}

		Vanilla::DrawActionLine(pThis, from, to, LineColor(ColorIdx_MoveLine), a3, 0);
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x4DC060, FakeFootClass::_Draw_Action_Lines);