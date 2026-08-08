#include "Body.h"

#include <Utilities/Simd.h>
#include "./BlitterPack/BlitterPack.levels.h"

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <ColorStruct.h>
#include <Surface.h>

void ConvertClassExt::DeallocBlitters()
{
	if (this->BytesPerPixel == 1)
		this->DeallocBlitters8();
	else
		this->DeallocBlitters16();

	std::fill(std::begin(this->Blitters), std::end(this->Blitters), nullptr);
	std::fill(std::begin(this->RLEBlitters), std::end(this->RLEBlitters), nullptr);
}

void ConvertClassExt::AllocBlitters()
{
	Simd::Initialize();

	if (this->BytesPerPixel == 1)
		this->AllocBlitters8();
	else
		this->AllocBlitters16();
}

void ConvertClassExt::AllocBlitters8()
{
	auto* const pPaletteData = static_cast<BYTE*>(this->NormalShadeTable);
	auto* const pRemapData = reinterpret_cast<BYTE*>(this->DarkenTable);
	auto const ppCurrentZRemap = reinterpret_cast<BYTE**>(&this->CurrentZRemap);

	switch (Simd::GetCurrentLevel())
	{
	case Simd::Level::AVX512:
	{
		//auto const pPack = new BlitterPack8AVX512(pPaletteData, pRemapData, ppCurrentZRemap);
		//MapBlitterPack8(pPack, this);
		Debug::FatalError("AVX512 blitters are not implemented !.");
		return;
	}
	case Simd::Level::AVX2:
	{
		auto const pPack = new BlitterPack8AVX2(pPaletteData, pRemapData, ppCurrentZRemap);
		MapBlitterPack8(pPack, this);
		return;
	}
	case Simd::Level::SSE2:
	{
		auto const pPack = new BlitterPack8SSE2(pPaletteData, pRemapData, ppCurrentZRemap);
		MapBlitterPack8(pPack, this);
		return;
	}
	case Simd::Level::Scalar:
	default:
	{
		auto const pPack = new BlitterPack8Scalar(pPaletteData, pRemapData, ppCurrentZRemap);
		MapBlitterPack8(pPack, this);
		return;
	}
	}
}

void ConvertClassExt::DeallocBlitters8()
{
	switch (Simd::GetCurrentLevel())
	{
	case Simd::Level::AVX512:
		//delete reinterpret_cast<BlitterPack8AVX512*>(this->Blitters[0]);
		return;
	case Simd::Level::AVX2:
		delete reinterpret_cast<BlitterPack8AVX2*>(this->Blitters[0]);
		return;
	case Simd::Level::SSE2:
		delete reinterpret_cast<BlitterPack8SSE2*>(this->Blitters[0]);
		return;
	case Simd::Level::Scalar:
	default:
		delete reinterpret_cast<BlitterPack8Scalar*>(this->Blitters[0]);
		return;
	}
}

void ConvertClassExt::AllocBlitters16()
{
	auto* const pPaletteData = static_cast<WORD*>(this->NormalShadeTable);
	auto* const pFullColorData = static_cast<WORD*>(this->ShadeTables);
	auto const ppCurrentZRemap = reinterpret_cast<BYTE**>(&this->CurrentZRemap);
	const WORD halfTranslucencyMask = this->HalfColorMask;
	const WORD quatTranslucencyMask = this->QuarterColorMask;
	const int shadeCount = this->ShadeCount;

	switch (Simd::GetCurrentLevel())
	{
	case Simd::Level::AVX512:
	{
		//BlitterPack16AVX512* const pPack = new BlitterPack16AVX512(pPaletteData, pFullColorData, ppCurrentZRemap, halfTranslucencyMask, quatTranslucencyMask, shadeCount);
		//MapBlitterPack16(pPack, this);
		Debug::FatalError("AVX512 blitters are not implemented !.");
		return;
	}
	case Simd::Level::AVX2:
	{
		BlitterPack16AVX2* const pPack = new BlitterPack16AVX2(pPaletteData, pFullColorData, ppCurrentZRemap, halfTranslucencyMask, quatTranslucencyMask, shadeCount);
		MapBlitterPack16(pPack, this);
		return;
	}
	case Simd::Level::SSE2:
	{
		BlitterPack16SSE2* const pPack = new BlitterPack16SSE2(pPaletteData, pFullColorData, ppCurrentZRemap, halfTranslucencyMask, quatTranslucencyMask, shadeCount);
		MapBlitterPack16(pPack, this);
		return;
	}
	case Simd::Level::Scalar:
	default:
	{
		BlitterPack16Scalar* const pPack = new BlitterPack16Scalar(pPaletteData, pFullColorData, ppCurrentZRemap, halfTranslucencyMask, quatTranslucencyMask, shadeCount);
		MapBlitterPack16(pPack, this);
		return;
	}
	}
}

void ConvertClassExt::DeallocBlitters16()
{
	switch (Simd::GetCurrentLevel())
	{
	case Simd::Level::AVX512:
		//delete reinterpret_cast<BlitterPack16AVX512*>(this->Blitters[0]);
		return;
	case Simd::Level::AVX2:
		delete reinterpret_cast<BlitterPack16AVX2*>(this->Blitters[0]);
		return;
	case Simd::Level::SSE2:
		delete reinterpret_cast<BlitterPack16SSE2*>(this->Blitters[0]);
		return;
	case Simd::Level::Scalar:
	default:
		delete reinterpret_cast<BlitterPack16Scalar*>(this->Blitters[0]);
		return;
	}
}

class FakeConvertClass
{
public:

	static void __fastcall _RecalcColorRemapTables(
	int srcRedShiftLeft, int srcRedShiftRight,
	int srcGreenShiftLeft, int srcGreenShiftRight,
	int srcBlueShiftLeft, int srcBlueShiftRight)
	{
		for (int index = 0; index < ConvertClass::Array->Count; ++index)
		{
			ConvertClass* const pConvert = ConvertClass::Array->Items[index];

			if (pConvert->BytesPerPixel != 2)
				continue;

			auto* pPixel = static_cast<unsigned short*>(pConvert->ShadeTables);

			// BUG (vanilla, preserved): the counter is a hard `mov edi, 100h` - only the FIRST
			// 256 entries are remapped, regardless of ShadeCount. Converts built with
			// ShadeCount > 1 keep stale-format data in every shade table past the first.
			for (int i = 0; i < 256; ++i)
			{
				const unsigned short source = *pPixel;

				// Each channel: 16-bit right shift to the channel base, 8-bit left shift to
				// normalise to 0..255 (old format), then 8-bit right + 32-bit left for the new
				// format. The 8-bit intermediates are what discard the neighbouring channels.
				unsigned char red = static_cast<unsigned char>(
					static_cast<unsigned char>(source >> srcRedShiftRight) << srcRedShiftLeft);
				unsigned char green = static_cast<unsigned char>(
					static_cast<unsigned char>(source >> srcGreenShiftRight) << srcGreenShiftLeft);
				unsigned char blue = static_cast<unsigned char>(
					static_cast<unsigned char>(source >> srcBlueShiftRight) << srcBlueShiftLeft);

				red = static_cast<unsigned char>(red >> Drawing::RedShiftRight());
				green = static_cast<unsigned char>(green >> Drawing::GreenShiftRight());
				blue = static_cast<unsigned char>(blue >> Drawing::BlueShiftRight());

				*pPixel++ = static_cast<unsigned short>(
					  (static_cast<unsigned int>(red) << Drawing::RedShiftLeft())
					| (static_cast<unsigned int>(green) << Drawing::GreenShiftLeft())
					| (static_cast<unsigned int>(blue) << Drawing::BlueShiftLeft()));
			}

			pConvert->HalfColorMask = DSurface::HalfbrightMask();
			pConvert->QuarterColorMask = DSurface::QuarterbrightMask();

			if (auto pExt = (ConvertExtData*)pConvert->GetPptrFromPad()) {
				pExt->Dealloc();
				pExt->Alloc();
			}
			//TODO : ReCreate the ExtBlitters

			pConvert->Dealloc_Blitters();
			pConvert->Alloc_Blitters();
		}
	}

	static ConvertClass* __fastcall _CTOR(ConvertClass* pThis, discard_t,
	BytePalette* palette, BytePalette* eightbitpalette,
	DSurface* pSurface, size_t shadeCount, bool skipBlitters)
	{
		// --- 0x48E748 .. 0x48E753 ------------------------------------------------------
		pThis->_BytesPerPixel = pSurface->Get_Bytes_Per_Pixel();

		// --- 0x48E756 .. 0x48E939 : zero [+0x08 .. +0x168] and [+0x170 .. +0x17C] --------
		std::memset(pThis->Blitters, 0, sizeof(pThis->Blitters));       // +0x008 .. +0x0CF
		std::memset(pThis->RLEBlitters, 0, sizeof(pThis->RLEBlitters)); // +0x0D0 .. +0x16B
		pThis->ShadeCount = shadeCount;                                 // +0x16C
		pThis->ShadeTables = nullptr;                                       // +0x170
		pThis->NormalShadeTable = nullptr;                                     // +0x174
		pThis->DarkenTable = nullptr;                                       // +0x178
		pThis->CurrentZRemap = 0;                                       // +0x17C

		// BUG (vanilla, preserved): HalfColorMask/QuarterColorMask (+0x180/+0x184) are NOT
		// initialised here. On the 8bpp path they are never written at all and stay whatever
		// the allocator left behind. Do not "fix" without checking Alloc_Blitters consumers.

		// --- 0x48E93B ------------------------------------------------------------------
		VTable::Set(pThis, ConvertClass::vtable); // 0x7E5358

		// --- 0x48E941 : clamp -----------------------------------------------------------
		if (shadeCount < 1)
			pThis->ShadeCount = 1;

		const int shades = pThis->ShadeCount;

		if (pThis->BytesPerPixel == 1) {
			// --- 0x48E955 : half-brightness ramp inside the source palette --------------
			auto* const pRamp = static_cast<unsigned char*>(operator new(0x100u));
			pThis->DarkenTable = reinterpret_cast<char*>(pRamp);
			pRamp[0] = 0;

			for (int i = 1; i < 256; ++i) {
				HSVClass hsv {};
				palette->Entries[i].ConstructHSV(&hsv);
				hsv.Value = static_cast<unsigned char>(hsv.Value >> 1); // 0x48E9A3 `shr al, 1`
				ColorStruct rgb = hsv.operator ColorStruct();				
				pRamp[i] = static_cast<unsigned char>(palette->Closest_Color(rgb));
			}

			// --- 0x48E9E1 : shade tables (1 byte per entry) -----------------------------
			auto* const pBuffer = static_cast<unsigned char*>(operator new(static_cast<size_t>(shades) << 8));
			pThis->ShadeTables = pBuffer;
			pThis->NormalShadeTable = pBuffer + (static_cast<size_t>((shades - 1) >> 1) << 8);

			if (shades == 1) {
				// --- 0x48EA18 : straight remap of the source palette into the dest ------
				pBuffer[0] = 0;

				for (int i = 1; i < 256; ++i) {
					ColorStruct _closest = palette->Entries[i];
					pBuffer[i] = static_cast<unsigned char>(eightbitpalette->Closest_Color(_closest));
				}
			}
			else {
				// --- 0x48EA51 : `shades` brightness ramps, index 0 always transparent ---
				unsigned char* pWrite = pBuffer;

				for (int shade = 0; shade < shades; ++shade) {
					*pWrite++ = 0;

					for (int i = 1; i < 256; ++i) {
						HSVClass hsv {};
						palette->Entries[i].ConstructHSV(&hsv);

						// SUSPECT (preserved verbatim): `(2 * shade * Val) / (shades - 1)` is
						// computed in 32-bit and then truncated to a byte by `mov [..], al`.
						// For shade == shades - 1 this reaches 2 * Val, so any Val > 127 wraps
						// instead of saturating. Vanilla behaviour - do not clamp.
						const int value = hsv.Value;
						hsv.Value = static_cast<unsigned char>((2 * shade * value) / (shades - 1));
						ColorStruct rgb = hsv.operator ColorStruct();
						*pWrite++ = static_cast<unsigned char>(eightbitpalette->Closest_Color(rgb));
					}
				}
			}
		}
		else {
			// --- 0x48EB12 : shade tables (2 bytes per entry) ----------------------------
			auto* const pBuffer = static_cast<unsigned char*>(operator new(static_cast<size_t>(shades) << 9));
			pThis->ShadeTables = pBuffer;
			pThis->NormalShadeTable = pBuffer + (static_cast<size_t>((shades - 1) >> 1) << 9);

			if (!skipBlitters)
				DSurface::Convert_Palette_16bit(pBuffer, shades, palette);

			// --- 0x48EB56 ---------------------------------------------------------------
			pThis->HalfColorMask = DSurface::HalfbrightMask();
			pThis->QuarterColorMask = DSurface::QuarterbrightMask();

		
		}

		auto pExt = new ConvertExtData();
		pThis->SetPadToPtr((void*)pExt);
		pExt->AttachedToObject = pThis;

		// --- 0x48EB73 -------------------------------------------------------------------
		if (!skipBlitters) {
			pExt->Alloc();
			pThis->Alloc_Blitters();
		}

		// --- 0x48EB83 : inlined DynamicVectorClass<ConvertClass*>::AddItem --------------
		ConvertClass::Array->push_back(pThis);

		return pThis;
	}

	static void __fastcall _DTOR(ConvertClass* pThis)
	{
		VTable::Set(pThis, ConvertClass::vtable);

		pThis->Dealloc_Blitters();

		delete (std::exchange(pThis->ShadeTables, nullptr));
		delete (std::exchange(pThis->DarkenTable, nullptr));

		if (auto pExt = (ConvertExtData*)pThis->GetPptrFromPad()) {
			//TODO : Dealloct Ext Blitters
			pExt->Dealloc();
			delete ((ConvertExtData*)pExt);
		}

		ConvertClass::Array->erase(pThis);
	}
};

DEFINE_FUNCTION_JUMP(LJMP, 0x48E740, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x4B6EE9, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x4B6F23, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x4B6FD8, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52BE36, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52BF08, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52BFBC, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52C070, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52C124, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52C1D8, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x52C2B6, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x53135C, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x555DBE, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x5CB678, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x5D3031, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x68C636, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x68CEAC, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x68D4C0, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x6A58A8, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x6A5AA0, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x72AECB, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x747473, FakeConvertClass::_CTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x778382, FakeConvertClass::_CTOR)

DEFINE_FUNCTION_JUMP(LJMP, 0x556516, FakeConvertClass::_DTOR)
DEFINE_FUNCTION_JUMP(CALL, 0x556529, FakeConvertClass::_DTOR)

DEFINE_FUNCTION_JUMP(LJMP, 0x491100, FakeConvertClass::_RecalcColorRemapTables)
DEFINE_FUNCTION_JUMP(CALL, 0x561046, FakeConvertClass::_RecalcColorRemapTables)