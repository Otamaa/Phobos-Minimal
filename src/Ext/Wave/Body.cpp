#include "Body.h"
//#include <Helpers/RoundingAlgorithms.h>
#include <Ext/WeaponType/Body.h>

#include <YRPPGlobal.h>

#ifdef ENABLE_NEWHOOKS
WaveExt::ExtContainer WaveExt::ExtMap;

#pragma warning(push)
#pragma warning(disable : 4244)

//#pragma region Array
//ARRAY_DEF(0xB46254, short, Wave_SonicBeamSineTable, 500)
//ARRAY_DEF(0xB45E68, short, Wave_MagneticBeamSineTable, 496)
//ARRAY2D_DEF(0xB4669C, short, Wave_LUT_Pythagoras, 300, 300)
//
//ARRAY_DEF(0xB46648, int, Wave_LUT_Linear1, 14)
//ARRAY2D_DEF(0xB45DA8, Vector3D<double>, WaveVectors, 4, 3)
//ARRAY2D_DEF(0xB45CA0, Vector3D<double>, MagneticWaveVectors, 4, 3)
//#pragma endregion Array

void WaveExt::ExtData::Initialize()
{
	/* No Data ! yet
	if (MyWeapon)
	{
		auto pExt = WeaponTypeExt::ExtMap.Find(MyWeapon);

		if (pExt && this->OwnerObject()->Type == WaveType::Sonic) {
			SonicBeamDuration = pExt->SonicBeamDuration;
			SonicBeamAlpha = pExt->SonicBeamAlpha;
			SonicBeamAmplitude = pExt->SonicBeamAmplitude;
			SonicBeamOffset = pExt->SonicBeamOffset;
			SonicBeamSurfacePattern = pExt->SonicBeamSurfacePattern;
			SonicBeamSinePattern = pExt->SonicBeamSinePattern;
			SonicBeamColor = pExt->SonicBeamColor;
			SonicBeamIsClear = pExt->SonicBeamIsClear;
			SonicBeamStartPinLeft = pExt->SonicBeamStartPinLeft; // endleft
			SonicBeamStartPinRight = pExt->SonicBeamStartPinRight; // startright
			SonicBeamEndPinLeft = pExt->SonicBeamEndPinLeft; //
			SonicBeamEndPinRight = pExt->SonicBeamEndPinRight; //
		}
	}*/
}
//
//bool WaveExt::ExtData::Calculate_Sonic_Beam_Tables()
//{
//	if (!SonicBeamTablesCalculated) {
//
//		double period = SonicBeamDuration;
//		double amplitude = SonicBeamAmplitude;
//		double offset = SonicBeamOffset;
//
//		double x_stretch_coeff = 2.0;
//		double y_stretch_coeff = 1.0;
//
//
//		/**
//		 *  Generate the sonic beam surface pattern.
//		 */
//		for (int x = 0; x < 300; ++x) {
//			for (int y = 0; y < 300; ++y) {
//				switch (SonicBeamSurfacePattern) {
//				default:
//				case SURFACE_PATTERN_CIRCLE:
//					SonicBeamSurfacePatternTable[x][y] = Math::sqrt(x * x + y * y);
//					break;
//				case SURFACE_PATTERN_ELLIPSE:
//					SonicBeamSurfacePatternTable[x][y] = Math::sqrt((x * 2) / x_stretch_coeff + (y * 2) / y_stretch_coeff);
//					break;
//				case SURFACE_PATTERN_RHOMBUS:
//					SonicBeamSurfacePatternTable[x][y] = Math::sqrt(x + y);
//					break;
//				case SURFACE_PATTERN_SQUARE:
//					SonicBeamSurfacePatternTable[x][y] = Math::sqrt(std::max(std::abs(x), std::abs(y)));
//					break;
//				};
//			}
//		}
//
//		/**
//		*
//		*/
//		for (int i = 0; i < ARRAY_SIZE(SonicBeamSineTable); ++i)
//
//			switch (SonicBeamSinePattern)
//			{
//			default:
//			case SINE_PATTERN_CIRCLE:
//				SonicBeamSineTable[i] = (short)(Math::sin(i * period) * amplitude + offset);
//				break;
//			case SINE_PATTERN_SQUARE:
//				SonicBeamSineTable[i] = (short)(Math::signum(Math::sin((double)i)));
//				break;
//			case SINE_PATTERN_SAWTOOTH:
//				SonicBeamSineTable[i] = (short)(2 * (i / period - std::floor(i / period + 1 / 2)));
//				break;
//			case SINE_PATTERN_TRIANGLE:
//			{
//				double sawtooth = 2 * (i / period - std::floor(i / period + 1 / 2));
//				SonicBeamSineTable[i] = (short)(2 * std::abs(sawtooth) - 1);
//				break;
//			}
//			}
//
//		/**
//		  *  #TODO: The offset of the wave?
//		  */
//		for (int i = 0; i < ARRAY_SIZE(custom_SonicBeam_LUT_Linear1); ++i) {
//			custom_SonicBeam_LUT_Linear1[i] = 110 + (i * 8);
//		}
//
//		SonicBeamTablesCalculated = true;
//	}
//
//	return true;
//}
//
//void WaveExt::ExtData::Draw_Sonic_Beam_Pixel(int a1, int a2, int a3, unsigned short *buffer)
//{
//
//	/**
//	 *  One-time generate the required sonic beam effect tables.
//	 */
//	if (!SonicBeamTablesCalculated) {
//		Calculate_Sonic_Beam_Tables();
//	}
//
//	// this block prepares the warp offset?
//	auto ThisPtr = this->OwnerObject();
//	int nIndex = ThisPtr->WaveIntensity + (unsigned short)SonicBeamSurfacePatternTable[a3][std::abs(a1 - ThisPtr->field_CC.X - a2)];
//
//	// ???????
//	int nTableIndex = std::abs(SonicBeamSineTable[nIndex]);
//
//	nTableIndex = Math::max(nTableIndex, 13);
//
//	// intensity of color in wave?
//	int nLutLiniear1 = custom_SonicBeam_LUT_Linear1[nTableIndex];
//
//	// pixel from the buffer
//	unsigned short nUnknownTable = buffer[ThisPtr->unknown_208[nTableIndex]];
//
//	// recolor pixel
//	int r = 0;
//	int g = 0;
//	int b = 0;
//
//	/**
//	 *  Clear beam, not color change.
//	 */
//	if (SonicBeamIsClear) {
//
//		r = (unsigned char)((unsigned char)(nUnknownTable >> DSurface::RedLeft) << DSurface::RedRight);
//		g = (unsigned char)((unsigned char)(nUnknownTable >> DSurface::GreenLeft) << DSurface::GreenRight);
//		b = (unsigned char)((unsigned char)(nUnknownTable >> DSurface::BlueLeft) << DSurface::BlueRight);
//
//
//	}
//	else if (SonicBeamColor.R != 0 && SonicBeamColor.G != 0 && SonicBeamColor.B != 0) {
//
//		int color_r = SonicBeamColor.R;
//		int color_g = SonicBeamColor.G;
//		int color_b = SonicBeamColor.B;
//
//		/**
//		 *  Calculate the alpha of the beam.
//		 */
//		int alpha = float(nLutLiniear1) * 1.0;
//		color_r += float((unsigned char)((unsigned char)(nUnknownTable >> DSurface::RedLeft) << DSurface::RedRight)) * SonicBeamAlpha;
//		color_g += float((unsigned char)((unsigned char)(nUnknownTable >> DSurface::GreenLeft) << DSurface::GreenRight)) * SonicBeamAlpha;
//		color_b += float((unsigned char)((unsigned char)(nUnknownTable >> DSurface::BlueLeft) << DSurface::BlueRight)) * SonicBeamAlpha;
//
//		r = color_r + ((alpha * (unsigned char)((unsigned char)(nUnknownTable >> DSurface::RedLeft) << DSurface::RedRight)) / 256);
//		g = color_g + ((alpha * (unsigned char)((unsigned char)(nUnknownTable >> DSurface::GreenLeft) << DSurface::GreenRight)) / 256);
//		b = color_b + ((alpha * (unsigned char)((unsigned char)(nUnknownTable >> DSurface::BlueLeft) << DSurface::BlueRight)) / 256);
//
//
//		/**
//		 *  Original sonic beam color code.
//		 */
//	}
//	else {
//
//		r = (unsigned char)((unsigned char)(nUnknownTable >> DSurface::RedLeft) << DSurface::RedRight);
//
//		g = ((nUnknownTable * (unsigned char)((unsigned char)(nUnknownTable >> DSurface::GreenLeft) << DSurface::GreenRight)) / 256)
//			+ (unsigned char)((unsigned char)(nUnknownTable >> DSurface::GreenLeft) << DSurface::GreenRight);
//
//		b = ((nUnknownTable * (unsigned char)((unsigned char)(nUnknownTable >> DSurface::BlueLeft) << DSurface::BlueRight)) / 256)
//			+ (unsigned char)((unsigned char)(nUnknownTable >> DSurface::BlueLeft) << DSurface::BlueRight);
//
//	}
//
//
//	// clamp color
//	r = std::min(r, 255);
//	g = std::min(g, 255);
//	b = std::min(b, 255);
//
//	// rebuild pixel from new rgb.
//	*buffer = DSurface::RGBA_To_Pixel(r, g, b);
//}
//
//static void do_vectors(WaveClass* this_ptr)
//{
//	bool use_sonic_beam_defaults = true;
//	auto nWaveSonic = (int)(WaveType::Sonic);
//	auto nWaveBigLaser = (int)(WaveType::BigLaser);
//	auto nWaveLaser = (int)(WaveType::Laser);
//
//	if (auto waveext = WaveExt::ExtMap.Find(this_ptr))
//	{
//
//		/**
//		 *  Copy over this waves custom sonic beam values.
//		 */
//		WaveVectors[0][nWaveSonic] = waveext->SonicBeamStartPinLeft;
//		WaveVectors[1][nWaveSonic] = waveext->SonicBeamStartPinRight;
//		WaveVectors[2][nWaveSonic] = waveext->SonicBeamEndPinLeft;
//		WaveVectors[3][nWaveSonic] = waveext->SonicBeamEndPinRight;
//
//		use_sonic_beam_defaults = false;
//	}
//
//	if (use_sonic_beam_defaults) {
//		/**
//		 *  Original sonic beam values.
//		 */
//		WaveVectors[0][nWaveSonic] = Vector3D<double>{ -30.0, -100.0, 0.0 };
//		WaveVectors[1][nWaveSonic] = Vector3D<double>{ -30.0, 100.0, 0.0 };
//		WaveVectors[2][nWaveSonic] = Vector3D<double>{ 30.0, -100.0, 0.0 };
//		WaveVectors[3][nWaveSonic] = Vector3D<double>{ 30.0, 100.0, 0.0 };
//	}
//
//	/**
//	 *  Original game values.
//	 */
//	WaveVectors[0][nWaveBigLaser] = Vector3D<double>{ -34.0, -44.0, 0.0 };
//	WaveVectors[1][nWaveBigLaser] = Vector3D<double>{ -34.0, 44.0, 0.0 };
//	WaveVectors[2][nWaveBigLaser] = Vector3D<double>{ 34.0, -44.0, 0.0 };
//	WaveVectors[3][nWaveBigLaser] = Vector3D<double>{ 34.0, 44.0, 0.0 };
//
//	WaveVectors[0][nWaveLaser] = Vector3D<double>{ -27.0, -34.0, 0.0 };
//	WaveVectors[1][nWaveLaser] = Vector3D<double>{ -27.0, 34.0, 0.0 };
//	WaveVectors[2][nWaveLaser] = Vector3D<double>{ 27.0, -34.0, 0.0 };
//	WaveVectors[3][nWaveLaser] = Vector3D<double>{ 27.0, 34.0, 0.0 };
//}

// =============================
// load / save
template <typename T>
void WaveExt::ExtData::Serialize(T& Stm)
{
	Stm
		//.Process(MyWeapon)
		//.Process(SonicBeamDuration)
		//.Process(SonicBeamColor)
		//.Process(SonicBeamAmplitude)
		//.Process(SonicBeamOffset)
		//.Process(SonicBeamAlpha)
		//.Process(SonicBeamSurfacePattern)
		//.Process(SonicBeamSinePattern)
		//.Process(SonicBeamStartPinLeft)
		//.Process(SonicBeamStartPinRight)
		//.Process(SonicBeamEndPinLeft)
		//.Process(SonicBeamEndPinRight)
		//.Process(SonicBeamTablesCalculated)
		//.Process(SonicBeamSineTable)
		//.Process(SonicBeamSurfacePatternTable)
		//.Process(custom_SonicBeam_LUT_Linear1)
		//.Process(SonicBeamIsClear)

		;
}

void WaveExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<WaveClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WaveExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<WaveClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void WaveExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool WaveExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool WaveExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

WaveExt::ExtContainer::ExtContainer() : TExtensionContainer("WaveClass") {}
WaveExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x75EA59, WaveClass_CTOR, 0x5)
{
	GET(WaveClass*, pItem, ESI);

	WaveExt::ExtMap.JustAllocate(pItem , !pItem , "Invalid !");

	return 0;
}

DEFINE_HOOK_AGAIN(0x75F7D0, WaveClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75F650, WaveClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(WaveClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WaveExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x75F7BA, WaveClass_Load_Suffix, 0x5)
{
	WaveExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x75F82F, WaveClass_Save_Suffix, 0x6)
{
	WaveExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x763226, WaveClass_DTOR, 0x6)
{
	GET(WaveClass*, pItem, EDI);

	WaveExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK(0x75F623, WaveClass_Detach, 0x6)
{
	GET(WaveClass*, pItem, ESI);
	GET_BASE(void*, pTarget, 0x4);
	GET_BASE(bool, bRemove, 0x8);

	if (auto pExt = WaveExt::ExtMap.Find(pItem))
		pExt->InvalidatePointer(pTarget, bRemove);

	return 0;
}
#pragma warning(pop)
#endif
