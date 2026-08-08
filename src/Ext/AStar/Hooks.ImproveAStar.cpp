#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

// ------------------------------------------------------------------------------
//  FIXED variant (recommended) - 0x52 is `mov edi, [edx]` (8B 3A), matching vanilla
//------------------------------------------------------------------------------
DEFINE_PATCH(0x42A54A,
	0x8B, 0x54, 0x24, 0x18,             // mov     edx, [esp+18h]            ; a3 = CellClass** ppCell
	0x8B, 0x5C, 0x24, 0x1C,             // mov     ebx, [esp+1Ch]            ; a4 = CellStruct* pTarget
	0x8B, 0x3A,                         // mov     edi, [edx]                ; edi = *ppCell  (see note)
	0x0F, 0xBF, 0x0B,                   // movsx   ecx, word ptr [ebx]       ; target.X
	0x0F, 0xBF, 0x47, 0x24,             // movsx   eax, word ptr [edi+24h]   ; cell.MapCoords.X
	0x2B, 0xC1,                         // sub     eax, ecx
	0x99,                               // cdq
	0x33, 0xC2,                         // xor     eax, edx
	0x2B, 0xC2,                         // sub     eax, edx                  ; eax = |dx|
	0x8B, 0xC8,                         // mov     ecx, eax
	0x0F, 0xBF, 0x47, 0x26,             // movsx   eax, word ptr [edi+26h]   ; cell.MapCoords.Y
	0x0F, 0xBF, 0x53, 0x02,             // movsx   edx, word ptr [ebx+2]     ; target.Y
	0x2B, 0xC2,                         // sub     eax, edx
	0x99,                               // cdq
	0x33, 0xC2,                         // xor     eax, edx
	0x2B, 0xC2,                         // sub     eax, edx                  ; eax = |dy|
	0x8D, 0x14, 0x01,                   // lea     edx, [ecx+eax]            ; dx + dy
	0x39, 0xC1,                         // cmp     ecx, eax
	0x0F, 0x4F, 0xC8,                   // cmovg   ecx, eax                  ; ecx = min(dx,dy)
	0x68, 0x1A, 0xF6, 0x15, 0xBF,       // push    0BF15F61Ah                ; float -0.58578646f = sqrt(2)-2
	0xD9, 0x04, 0x24,                   // fld     dword ptr [esp]
	0x51,                               // push    ecx
	0xDB, 0x04, 0x24,                   // fild    dword ptr [esp]
	0xDE, 0xC9,                         // fmulp   st(1), st                 ; min * (sqrt2-2)
	0x52,                               // push    edx
	0xDB, 0x04, 0x24,                   // fild    dword ptr [esp]
	0xDE, 0xC1,                         // faddp   st(1), st                 ; + (dx+dy)
	0x83, 0xC4, 0x0C,                   // add     esp, 0Ch
	0xD8, 0x46, 0x04,                   // fadd    dword ptr [esi+4]         ; + entry->Cost (g)
	0xD9, 0x5E, 0x08,                   // fstp    dword ptr [esi+8]         ; entry->Total  (f)
	0x8B, 0xC6,                         // mov     eax, esi                  ; return entry
	0x5F, 0x5E, 0x5D, 0x5B,             // pop     edi / esi / ebp / ebx
	0xC2, 0x10, 0x00                    // retn    10h
);


//; ==============================================================================
//; DiagonalPathImprovement - decoded patch blob
//;
//; Source: ApplyPatch(&v1) with v1.pPattchAddres = 0x0042A54A
//;         gated by INIClass::ReadBool(*pINI, "<0x826278>", "DiagonalPathImprovement", false)
//;
//; Target: AStarClass::Calc_sqrt(0x0042A460), heuristic tail at 0x0042A54A
//; Size: 21 dwords(84) + word 0x10C2 (2) + byte 0x00 (1) = 87 bytes
//;         0x0042A54A .. 0x0042A5A0 inclusive
//;         (vanilla tail ran to 0x0042A5A6->last 6 bytes are left as dead tail)
//; ==============================================================================
//
//; ------------------------------------------------------------------------------
//; 1) _patchData[]->raw bytes(little endian)
//; ------------------------------------------------------------------------------
//; idx  decimal          hex          bytes
//; --- -------------- - ---------- - ---------- -
//;  0    0x1824548B      1824548B     8B 54 24 18
//;  1    0x1C245C8B      1C245C8B     8B 5C 24 1C
//;  2 - 1089471861      BF0FFA8B     8B FA 0F BF
//;  3    1203703563      47BF0F0B     0B 0F BF 47
//;  4 - 1715393756      99C12B24     24 2B C1 99
//;  5 - 1037319629      C22BC233     33 C2 2B C2
//;  6 - 1089484661      BF0FC88B     8B C8 0F BF
//;  7 - 1089526201      BF0F2647     47 26 0F BF
//;  8 - 1037368749      C22B0253     53 02 2B C2
//;  9     734147481      2BC23399     99 33 C2 2B
//; 10      18124226      01148DC2     C2 8D 14 01
//; 11    1326432569      4F0FC139     39 C1 0F 4F
//; 12 - 166041400      F61A68C8     C8 68 1A F6
//; 13      81379093      04D9BF15     15 BF D9 04
//; 14      81482020      04DB5124     24 51 DB 04
//; 15    1388961316      52C9DE24     24 DE C9 52
//; 16 - 568064805      DE2404DB     DB 04 24 DE
//; 17     214205377      0CC483C1     C1 83 C4 0C
//; 18 - 654031144      D90446D8     D8 46 04 D9
//; 19 - 963966882      C68B085E     5E 08 8B C6
//; 20    0x5B5D5E5F      5B5D5E5F     5F 5E 5D 5B
//;  v3   0x10C2 (word)C2 10
//;  v4   0x00   (byte)00
//
//; ------------------------------------------------------------------------------
//; 2) disassembly
//; ------------------------------------------------------------------------------
//0042A54A  8B 54 24 18       mov     edx, [esp + 18h]; a3 = CellClass * *ppCell
//0042A54E  8B 5C 24 1C       mov     ebx, [esp + 1Ch]; a4 = CellStruct * pTarget
//0042A552  8B FA             mov     edi, edx; << < SUSPECT: see note(A)
//	0042A554  0F BF 0B          movsx   ecx, word ptr[ebx]; target.X
//	0042A557  0F BF 47 24       movsx   eax, word ptr[edi + 24h]; cell.MapCoords.X
//	0042A55B  2B C1             sub     eax, ecx
//	0042A55D  99                cdq
//	0042A55E  33 C2 xor eax, edx
//	0042A560  2B C2             sub     eax, edx; eax = | dx |
//	0042A562  8B C8             mov     ecx, eax; ecx = | dx |
//	0042A564  0F BF 47 26       movsx   eax, word ptr[edi + 26h]; cell.MapCoords.Y
//	0042A568  0F BF 53 02       movsx   edx, word ptr[ebx + 2]; target.Y
//	0042A56C  2B C2             sub     eax, edx
//	0042A56E  99                cdq
//	0042A56F  33 C2 xor eax, edx
//	0042A571  2B C2             sub     eax, edx; eax = | dy |
//	0042A573  8D 14 01          lea     edx, [ecx + eax]; edx = | dx | +| dy |
//	0042A576  39 C1             cmp     ecx, eax
//	0042A578  0F 4F C8          cmovg   ecx, eax; ecx = min(| dx | , | dy | )
//	0042A57B  68 1A F6 15 BF    push    0BF15F61Ah; float - 0.58578646f  (= sqrt(2) - 2)
//	0042A580  D9 04 24          fld     dword ptr[esp]; st0 = (sqrt2 - 2)
//	0042A583  51                push    ecx
//	0042A584  DB 04 24          fild    dword ptr[esp]; st0 = min, st1 = k
//	0042A587  DE C9             fmulp   st(1), st; st0 = min * (sqrt2 - 2)
//	0042A589  52                push    edx
//	0042A58A  DB 04 24          fild    dword ptr[esp]; st0 = dx + dy
//	0042A58D  DE C1             faddp   st(1), st; st0 = (dx + dy) + min * (sqrt2 - 2)
//	0042A58F  83 C4 0C          add     esp, 0Ch; drop the 3 temporaries
//	0042A592  D8 46 04          fadd    dword ptr[esi + 4]; +entry->Cost(g)
//	0042A595  D9 5E 08          fstp    dword ptr[esi + 8]; entry->Total(f)
//	0042A598  8B C6             mov     eax, esi; return entry
//	0042A59A  5F                pop     edi
//	0042A59B  5E                pop     esi
//	0042A59C  5D                pop     ebp
//	0042A59D  5B                pop     ebx
//	0042A59E  C2 10 00          retn    10h
//	0042A5A1  ..; 6 bytes of vanilla dead tail
//
//	; ------------------------------------------------------------------------------
//	; 3) what it changes
//	; ------------------------------------------------------------------------------
//	; vanilla   h = sqrt(dx * dx + dy * dy)                  (Euclidean, via FastMath::Sqrt 0x4CAC40)
//	; patched   h = (dx + dy) + (sqrt(2) - 2) * min(dx, dy)
//	; = max(dx, dy) + (sqrt(2) - 1) * min(dx, dy)->octile / diagonal distance
//	;
//; *exact match for 8 - way movement with diagonal cost sqrt(2); admissible + consistent
//; *strictly >= the Euclidean estimate, so A* expands fewer nodes and stops
//;   "fanning out" into the wide equal - cost plateaus that Euclidean produces
//; *no sqrt call at all - one imul - free integer pass + 3 x87 ops
//;
//; ------------------------------------------------------------------------------
//; NOTE(A) - probable bug in the blob
//; ------------------------------------------------------------------------------
//; vanilla at this spot is : 8B 3A   mov edi, [edx]; edi = *ppCell = CellClass*
//; the patch emits : 8B FA   mov edi, edx; edi = ppCell(NOT dereferenced)
//;
//; Every later access([edi + 24h] / [edi + 26h]) is a CellClass::MapCoords read, and the
//; rest of the function stores / loads a3 as a CellClass * *(see 0x42A495..0x42A4A6:
//; mov[edi], eax / mov eax, [eax]).So the missing deref reads 0x24 bytes past the
//; pointer slot->garbage heuristic.One - nibble fix :
//;
//;     _patchData[2] = 0xBF0F3A8B;   //  == -1089521013   (8B 3A 0F BF)
//;
//; VERIFY in the IDB before "fixing" it - if the upstream mod ships it this way it is
//; either untested or a3 is something other than CellClass * *in that build.


//==============================================================================
//  AStarClass::Calc_sqrt  -  0x0042A460  (vanilla YR)
//
//  Backport + optional "DiagonalPathImprovement" heuristic
//  (patch blob decoded in DiagonalPathImprovement_decoded.asm, applied at 0x42A54A)
//
//  Callers (vanilla):
//      0x0042A5B6 ish  AStarClass::Find_Path_Regular + 0x156
//      0x0042AA00 ish  AStarClass::Find_Path_Regular + 0x5A0
//
//  Hook table: NO ASMJIT_PATCH inside [0x0042A460, 0x0042A5A7) at time of writing.
//              Re-grep before enabling the whole-function replacement below.
//==============================================================================

#include <YRPP.h>
#include <Helpers/Macro.h>

#include <algorithm>
#include <cmath>

//------------------------------------------------------------------------------
//  config
//------------------------------------------------------------------------------
// EXTENSION: wire this to your RulesExt / [General] parser.
// Upstream gate was: INIClass::ReadBool(pINI, <0x826278>, "DiagonalPathImprovement", false)
static bool DiagonalPathImprovement = false;

namespace AStarInternal
{
	//--------------------------------------------------------------------------
	//  pool records
	//  VERIFY: names invented; sizes/offsets are confirmed from the disassembly.
	//--------------------------------------------------------------------------

	// 12-byte record, pool base = *(void**)(this + 0x0C), count = *(int*)(pool + 0x180000)
	// capacity = 0x180000 / 12 = 0x20000 entries
	struct NodeRecord
	{
		CellClass** Cell;      // +0x00  the caller's cell handle (deref -> CellClass*)
		int         Direction; // +0x04
		NodeRecord* Parent;    // +0x08
	};
	static_assert(sizeof(NodeRecord) == 0x0C, "AStar NodeRecord must be 12 bytes (lea edi,[edx+edx*2] / *4)");

	// 16-byte record, pool base = *(void**)(this + 0x10), count = *(int*)(pool + 0x100000)
	// capacity = 0x100000 / 16 = 0x10000 entries
	struct OpenRecord
	{
		NodeRecord* Node;   // +0x00
		float       Cost;   // +0x04  g
		float       Total;  // +0x08  f = g + h
		int         Length; // +0x0C
	};
	static_assert(sizeof(OpenRecord) == 0x10, "AStar OpenRecord must be 16 bytes (shl esi,4)");

	//--------------------------------------------------------------------------
	//  raw field access
	//--------------------------------------------------------------------------
	template<typename T>
	static inline T& FieldAt(void* pBase, int offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<char*>(pBase) + offset);
	}

	// VERIFY: CellClass +0x11B, signed byte. Used as a direction / link index and
	//         compared against NodeRecord::Direction with a constant +4 bias.
	static inline int Cell_Field11B(CellClass* pCell)
	{
		return FieldAt<signed char>(pCell, 0x11B);
	}

	// VERIFY: CellClass +0x140, bit 0x100. Gates the "+4" direction remap.
	static inline bool Cell_HasFlag140_100(CellClass* pCell)
	{
		return (FieldAt<DWORD>(pCell, 0x140) & 0x100) != 0;
	}

	// VERIFY: pins the MapCoords offset the vanilla code reads as [cell+0x24] / [cell+0x26].
	static_assert(offsetof(CellClass, MapCoords) == 0x24, "CellClass::MapCoords must sit at 0x24");

	// FastMath::Sqrt(double) -> st0, __cdecl (caller cleans: sub esp,8 / ... / add esp,8)
	// resolved from `call` at 0x0042A590 (E8 AB 06 0A 00 -> 0x0042A595 + 0xA06AB)
	using FastMathSqrt_t = double(__cdecl*)(double);
	static const auto FastMath_Sqrt = reinterpret_cast<FastMathSqrt_t>(0x004CAC40);

	//--------------------------------------------------------------------------
	//  heuristic
	//--------------------------------------------------------------------------
	static inline float Heuristic(const CellStruct& from, const CellStruct& to)
	{
		const int dx = std::abs(static_cast<int>(from.X) - static_cast<int>(to.X));
		const int dy = std::abs(static_cast<int>(from.Y) - static_cast<int>(to.Y));

		if (!DiagonalPathImprovement)
		{
			// vanilla: sqrt(dx*dx + dy*dy)   (integer square, fild, FastMath::Sqrt)
			return static_cast<float>(FastMath_Sqrt(static_cast<double>(dx * dx + dy * dy)));
		}

		// EXTENSION: octile / diagonal distance
		//   (dx + dy) + (sqrt(2) - 2) * min(dx, dy)
		// constant is bit-exact with the blob: 0xBF15F61A
		constexpr float DiagonalBias = -0.58578646f;
		const int sum = dx + dy;
		const int mn = std::min(dx, dy);

		return static_cast<float>(mn) * DiagonalBias + static_cast<float>(sum);
	}
}

//------------------------------------------------------------------------------
//  AStarClass::Calc_sqrt  -  full backport
//
//  Original: __thiscall, 4 stack args, retn 10h.
//  Delivered as static __fastcall with an explicit pThis + unused edx slot, which
//  is ABI-compatible with __thiscall here (all real args stay on the stack).
//------------------------------------------------------------------------------
//static AStarInternal::OpenRecord* __fastcall AStarClass_Calc(
//	AStarClass * pThis,
//	void* /* unused edx */,
//	AStarInternal::OpenRecord * pParent, // a2 - parent open-list entry, may be null
//	CellClass * *ppCell,                 // a3 - cell handle for the node being added
//	CellStruct * pTarget,                // a4 - goal cell
//	float cost)                         // a5 - step cost
//{
//	using namespace AStarInternal;
//
//	// --- 0x42A460: allocate the open-list entry (16-byte pool, count at +0x100000)
//	auto* const pOpenPool = FieldAt<OpenRecord*>(pThis, 0x10);
//	int& openCount = FieldAt<int>(pOpenPool, 0x100000);
//	OpenRecord* const pEntry = pOpenPool + openCount;
//	++openCount;
//
//	// --- 0x42A47F: allocate the node record (12-byte pool, count at +0x180000)
//	auto* const pNodePool = FieldAt<NodeRecord*>(pThis, 0x0C);
//	int& nodeCount = FieldAt<int>(pNodePool, 0x180000);
//	NodeRecord* const pNode = pNodePool + nodeCount;
//	++nodeCount;
//
//	pNode->Cell = ppCell;
//
//	if (!pParent)
//	{
//		// --- 0x42A516
//		pNode->Parent = nullptr;
//		pNode->Direction = FieldAt<int>(pThis, 0x30); // VERIFY: start facing / initial direction
//	}
//	else
//	{
//		// --- 0x42A49F
//		pNode->Parent = pParent->Node;
//
//		CellClass* const pCur = *ppCell;
//		CellClass* const pPrev = *pParent->Node->Cell;
//
//		const int curDir = Cell_Field11B(pCur);
//		pNode->Direction = curDir;
//
//		if (Cell_HasFlag140_100(pCur))
//		{
//			if (Cell_HasFlag140_100(pPrev))
//			{
//				// --- 0x42A4D4
//				if (pParent->Node->Direction == Cell_Field11B(pPrev) + 4)
//					pNode->Direction = curDir + 4;
//			}
//			// --- 0x42A4F5: only when the previous cell does NOT carry the flag
//			else if (std::abs(curDir - pParent->Node->Direction + 3) <= 1)
//			{
//				pNode->Direction = curDir + 4;
//			}
//		}
//	}
//
//	// --- 0x42A523
//	pEntry->Node = pNode;
//
//	if (pParent)
//	{
//		pEntry->Cost = cost + pParent->Cost;
//		pEntry->Length = pParent->Length + 1;
//	}
//	else
//	{
//		// --- 0x42A53C (vanilla stores integer 0 / 1; 0 aliases 0.0f exactly)
//		pEntry->Cost = 0.0f;
//		pEntry->Length = 1;
//	}
//
//	// --- 0x42A54A: heuristic tail (this is the range the patch blob overwrites)
//	// DIFF: vanilla evaluates this whole expression on the x87 stack in 80-bit
//	//       precision. Built for SSE2 the intermediate rounds to 32-bit, so results
//	//       can differ in the last ulp. Relevant for MP determinism - keep the
//	//       FPStateGuard in place and prefer replacing BOTH callers, not one.
//	pEntry->Total = Heuristic((*ppCell)->MapCoords, *pTarget) + pEntry->Cost;
//
//	return pEntry;
//}

//------------------------------------------------------------------------------
//  whole-function replacement
//
//  Entry hook: nothing has been pushed yet, so the args sit at [esp+4 .. esp+0x10]
//  and 0x0042A5A4 is the bare `retn 10h`, which balances the caller's 4 pushes.
//
//  VERIFY: macro spelling depends on your Phobos revision (ASMJIT_PATCH vs DEFINE_HOOK).
//------------------------------------------------------------------------------
//ASMJIT_PATCH(0x42A460, AStarClass_Calc_Replace, 0x6)
//{
//	GET(AStarClass*, pThis, ECX);
//	GET_STACK_OFFSET(AStarInternal::OpenRecord*, pParent, 0x4);
//	GET_STACK_OFFSET(CellClass**, ppCell, 0x8);
//	GET_STACK_OFFSET(CellStruct*, pTarget, 0xC);
//	GET_STACK_OFFSET(float, cost, 0x10);
//
//	R->EAX(AStarClass_Calc(pThis, nullptr, pParent, ppCell, pTarget, cost));
//	return 0x42A5A4; // retn 10h
//}

//------------------------------------------------------------------------------
//  Byte-level alternative (if you'd rather not own the whole function)
//
//  Patch only the tail at 0x0042A54A with the 87 bytes listed in
//  DiagonalPathImprovement_decoded.asm - but apply the NOTE (A) fix:
//      _patchData[2] = 0xBF0F3A8B;   // 8B 3A  mov edi,[edx]   (upstream ships 8B FA)
//------------------------------------------------------------------------------