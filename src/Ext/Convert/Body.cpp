#include "Body.h"

#include <Utilities/Simd.h>
#include "./BlitterPack/BlitterPack.levels.h"

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
	auto* const pPaletteData = static_cast<BYTE*>(this->BufferMid);
	auto* const pRemapData = reinterpret_cast<BYTE*>(this->BufferB);
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
	auto* const pPaletteData = static_cast<WORD*>(this->BufferMid);
	auto* const pFullColorData = static_cast<WORD*>(this->BufferA);
	auto const ppCurrentZRemap = reinterpret_cast<BYTE**>(&this->CurrentZRemap);
	const WORD halfTranslucencyMask = static_cast<WORD>(this->HalfColorMask);
	const WORD quatTranslucencyMask = static_cast<WORD>(this->QuarterColorMask);
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
		delete reinterpret_cast<BlitterPack16AVX512*>(this->Blitters[0]);
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