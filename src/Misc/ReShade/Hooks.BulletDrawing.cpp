// ---------------------------------------------------------------------------
// BulletClass::Draw_It (0x468090) hooks
//
// Replaces the SKIPPED block at the bottom of PhobosAlphaMask.cpp. Two
// independent features share this function in the reference fork:
//
//   0x4680E2  missile flash  - draws an EXTRA shp before the vanilla art.
//                              Nothing to do with the mask.
//   0x4683D7  FX mask        - replaces the final CC_Draw_Shape and queues the
//                              same draw into the alpha mask.
//
// Register/frame mapping for 0x4683D7 confirmed against the disassembly at
// esp depth 0x0C4:
//
//   ESI          this            (BulletClass*)
//   EBX          drawer          (ConvertClass*, selected at 0x468379-0x4683D1)
//   EDI          point           (mov edi, [ebp+point] @ 0x468301)
//   [EBP+0x0C]   rect
//   [ESP+0x10]   shape           (IDA "a2.Y"; Get_Image_Data stored @ 0x468261)
//   [ESP+0x2C]   shapenum        (IDA "shapenum"; typed REGISTERS* in pseudocode
//                                 by mistake - it is a plain int)
// ---------------------------------------------------------------------------

#include "PhobosAlphaMask.h"

#include <Ext/BulletType/Body.h>

#include <BulletClass.h>
#include <SurfaceExt.h>
#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Final SHP draw + mask queue. 0x4683D7, returns 0x468422 (the epilogue).
//
// NOTE: this hook REPLACES the vanilla CC_Draw_Shape at 0x46841D rather than
// running alongside it - the whole range [0x4683D7, 0x468422) is subsumed. The
// call below must stay byte-for-byte equivalent in argument values or bullets
// stop rendering entirely.
//
// VERIFY: grep the patch table for any other patch inside that range before
// merging.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4683D7, BulletClass_Draw_It_FinalSHP, 0x6)
{
	enum { Continue = 0x468422 };

	GET(BulletClass*, pThis, ESI);
	GET(ConvertClass*, pDrawer, EBX);
	GET(Point2D*, pPoint, EDI);
	GET_BASE(RectangleStruct*, pRect, 0xC);
	GET_STACK(SHPCaches*, pShape, 0x10);
	GET_STACK(int, frame, 0x2C);

	// vtable +0x1D0 == Get_Z_Coord. `mov ecx, 0FFFFFFE2h; sub ecx, eax`.
	const int zCoord = pThis->ObjectClass::GetZCoord();
	const int zAdjust = -30 - ZDepth_Adjust_For_Height(zCoord);

	// 0x46841D. Flags 0x2E00, intensity 1000, everything else zero.
	CC_Draw_Shape(
		DSurface::Temp,
		pDrawer,
		pShape,
		frame,
		pPoint,
		pRect,
		static_cast<DWORD>(0x2E00),
		0,
		zAdjust,
		0,
		1000,
		0,
		0,
		0,
		0,
		0);

	// BulletTypeExtData byte at +42 gates the queue.
	//
	// VERIFY: field name is a placeholder - substitute your actual member. The
	// pseudocode only gives me the offset.
	if (BulletTypeExtContainer::Instance.Find(pThis->Type)->FXLightEnable)
	{
		// DIFF: the parameter is hardcoded 127 here, where the anim producer
		// reads a configurable low|high<<8 pair from AnimTypeExtData. So every
		// bullet contributes intensity 127 with a zero high byte, whatever the
		// high byte ends up meaning.
		//
		// EXTENSION: this is the obvious place to make it INI-driven, matching
		// the anim side. Left at 127 so this stays a faithful backport - change
		// it deliberately, not by accident.
		PhobosAlphaMask::Instance().Queue(
			pShape, frame, static_cast<DWORD>(0x2E00),
			*pRect, pPoint->X, pPoint->Y,
			static_cast<uint16_t>(127));
	}

	return Continue;
}

// ---------------------------------------------------------------------------
// Missile flash. 0x4680E2, returns 0 (vanilla continues).
//
// Draws an extra SHP at the bullet's position before any vanilla art. This is
// a separate cosmetic feature - it does NOT touch the alpha mask.
//
// SUSPECT: the fork's version does not emit into the FX queue, so a missile
// flash produces no light even though it is literally flash art. Whether that
// is intentional or an oversight I cannot tell from the pseudocode. If you want
// it lit, the Queue() call goes right after each CC_Draw_Shape below, with the
// same rect/xy/flags.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4680E2, BulletClass_Draw_It_MissileFlash, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	BulletTypeClass* const pType = pThis->Type;

	// BUGFIX: the reference fork hooks 0x4680E2, which sits BEFORE the two
	// visibility tests at 0x4680E8 (Type+0x29E, IsInvisible) and 0x4680F6
	// (this+0x158, __SpawnNextAnim). Both of those jump straight to the
	// epilogue, so in the fork an invisible bullet still draws its flash.
	//
	// The tests are replicated here rather than moving the hook, because the
	// patch has to land on a clean 6-byte boundary and 0x4680E2 is the only one
	// available before the branch.
	if (pType->IsInvisible || pThis->SpawnNextAnim)
		return 0;

	auto* const pExt = BulletTypeExtContainer::Instance.Find(pType);

	// VERIFY: both field names are placeholders for the bytes at ext+200 (the
	// enable flag) and ext+204 (the style selector).
	if (!pExt->MissileFlash)
		return 0;

	Point2D xy {};
	TacticalClass::Instance->CoordsToClient(pThis->Location, &xy);

	RectangleStruct rect {};
	DSurface::Temp->GetRect(&rect);

	// The engine's own pattern: shrink the usable height by 32 so art near the
	// bottom edge is not clipped mid-blit. Mirrors `HIDWORD(v179) -= 32` in the
	// anim updater.
	rect.Height -= 32;

	const int zCoord = pThis->ObjectClass::GetZCoord();
	const int zAdjust = -31 - ZDepth_Adjust_For_Height(zCoord);

	// The five styles differ only in shape, palette, and blit flags. 0x2E06
	// carries the extra translucency bits the other three do not.
	SHPCaches* pShape = nullptr;
	ConvertClass* pPalette = nullptr;
	DWORD flags = 0x2E00;

	switch (pExt->MissileFlashStyle)
	{
	case 0:
		pShape = MissileFlashSHP;
		pPalette = MissileFlashWhitePAL;
		flags = 0x2E06;
		break;

	case 1:
		pShape = MissileFlashSHP;
		pPalette = MissileFlashOrangePAL;
		flags = 0x2E06;
		break;

	case 2:
		pShape = GMissileSHP;
		pPalette = ObjectsConvert;
		break;

	case 3:
		pShape = NMissileSHP;
		pPalette = NormalConvert;
		break;

	case 4:
		pShape = ISFireMissileSHP;
		pPalette = NormalConvert;
		break;

	default:
		return 0;
	}

	if (pShape == nullptr || pPalette == nullptr)
		return 0;

	CC_Draw_Shape(
		DSurface::Temp,
		pPalette,
		pShape,
		0,
		&xy,
		&rect,
		flags,
		0,
		zAdjust,
		0,
		1000,
		0,
		0,
		0,
		0,
		0);

	return 0;
}