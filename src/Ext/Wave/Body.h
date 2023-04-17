#pragma once
#include <WaveClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

enum class SonicBeamSurfacePatternType : int
{
	CIRCLE,
	ELLIPSE,
	RHOMBUS,
	SQUARE,

	COUNT
};

enum class SonicBeamSinePatternType : int
{
	CIRCLE,
	SQUARE,
	SAWTOOTH,
	TRIANGLE,

	COUNT
};

// will be generated when weapontype init
struct SonicBeamDrawingData
{
	bool Calculate_Sonic_Beam_Tables(
		SonicBeamSurfacePatternType SonicBeamSurfacePattern ,
		SonicBeamSinePatternType SonicBeamSinePattern ,
		double SonicBeamSineDuration ,
		double SonicBeamSineAmplitude ,
		double SonicBeamOffset
		)
	{
		if (!this->SonicBeamTablesCalculated)
		{
			for (int x = 0; x < 300; ++x)
			{
				for (int y = 0; y < 300; ++y)
				{
					switch (SonicBeamSurfacePattern)
					{
					default:
					case SonicBeamSurfacePatternType::CIRCLE:
						SonicBeamSurfacePatternTable[x][y] = (short)std::sqrt(x * x + y * y);
						break;
					case SonicBeamSurfacePatternType::ELLIPSE:
					{
						double x_stretch_coeff = 2.0;
						double y_stretch_coeff = 1.0;
						SonicBeamSurfacePatternTable[x][y] = (short)(std::sqrt((x * 2) / x_stretch_coeff + (y * 2) / y_stretch_coeff));
						break;
					}
					case SonicBeamSurfacePatternType::RHOMBUS:
						SonicBeamSurfacePatternTable[x][y] = (short)(std::sqrt(x + y));
						break;
					case SonicBeamSurfacePatternType::SQUARE:
						SonicBeamSurfacePatternTable[x][y] = (short)(std::sqrt(MaxImpl(std::abs(x), std::abs(y))));
						break;
					};
				}
			}

			/**
			 *  Generate the sonic beam sine pattern table.
			 */
			for (int i = 0; i < ARRAY_SIZE(SonicBeamSineTable); ++i)
			{
				switch (SonicBeamSinePattern)
				{
				default:
				case SonicBeamSinePatternType::CIRCLE:
					SonicBeamSineTable[i] = (short)(std::sin(i * SonicBeamSineDuration) * SonicBeamSineAmplitude + SonicBeamOffset);
					break;
				case SonicBeamSinePatternType::SQUARE:
					SonicBeamSineTable[i] = (short)(Math::signum(std::sin(i)));
					break;
				case SonicBeamSinePatternType::SAWTOOTH:
					SonicBeamSineTable[i] = (short)(2 * (i / SonicBeamSineDuration - std::floor(i / SonicBeamSineDuration + 1 / 2)));
					break;
				case SonicBeamSinePatternType::TRIANGLE:
				{
					double sawtooth = 2 * (i / SonicBeamSineDuration - std::floor(i / SonicBeamSineDuration + 1 / 2));
					SonicBeamSineTable[i] = (short)(2 * std::abs(sawtooth) - 1);
					break;
				}
				};
			}

			/**
			 *  The pixel offset of the wave.
			 */
			for (int i = 0; i < ARRAY_SIZE(SonicBeamIntensityTable); ++i)
			{
				SonicBeamIntensityTable[i] = 110 + (i * 8);
			}

			SonicBeamTablesCalculated = true;
		}

		return true;
	}

	/*
		*	Re - implementation of Draw_Sonic_Beam_Pixel() for WaveClassExtension.
		*
		*	@authors: CCHyper, tomsons26(additional research by askhati and Morton)
		*/

	void Draw_Sonic_Beam_Pixel(WaveClass* pThis, int a1, int a2, int a3, WORD* buffer ,
		SonicBeamSurfacePatternType SonicBeamSurfacePattern,
		SonicBeamSinePatternType SonicBeamSinePattern,
		double SonicBeamSineDuration,
		double SonicBeamSineAmplitude,
		double SonicBeamOffset,
		double SonicBeamAlpha,
		bool SonicBeamIsClear,
		ColorStruct SonicBeamColor
	)
	{
		/**
		 *  One-time generate the required sonic beam effect tables.
		 */
		if (!SonicBeamTablesCalculated)
		{
			Calculate_Sonic_Beam_Tables(SonicBeamSurfacePattern , SonicBeamSinePattern , SonicBeamSineDuration , SonicBeamSineAmplitude , SonicBeamOffset);
		}

		/**
		 *  Calculate the pixel offset position based on the surface pattern.
		 */
		int wave_pos = pThis->WaveCount + (WORD)SonicBeamSurfacePatternTable[a3][std::abs(a1 - pThis->InitialWavePixels_0.X - a2)];
		int pos = std::abs(SonicBeamSineTable[wave_pos]);

		/**
		 *  #issue-540
		 *
		 *  Possible bug-fix for the common crash, back-ported from Red Alert 2.
		 */
		if (pos > 9)
		{
			pos = 9;
		}

		/**
		 *  Get the intensity adjustment value based on the pattern position.
		 */
		int intensity = SonicBeamIntensityTable[pos];

		/**
		 *  Fetch the offset pixel from the buffer.
		 */
		WORD pixel = buffer[pThis->intensity_tables[pos]];

		/**
		 *  Re-color the buffer pixel.
		 */
		ColorStruct result;

		/**
		 *  Clear beam, not color change.
		 */
		if (SonicBeamIsClear)
		{
			const ColorStruct nPixel = Drawing::WordColor(pixel);
			result.R = nPixel.R;
			result.G = nPixel.G;
			result.B = nPixel.B;

			/**
			 *  Custom color has been set.
			 */
		}
		else if (SonicBeamColor)
		{

			int color_r = SonicBeamColor.R;
			int color_g = SonicBeamColor.G;
			int color_b = SonicBeamColor.B;
			const ColorStruct nPixel = Drawing::WordColor(pixel);
			/**
			 *  Calculate the alpha of the beam.
			 */
			const int alpha = (int)(float(intensity) * 1.0);
			color_r += (int)(float(nPixel.R) * SonicBeamAlpha);
			color_g += (int)(float(nPixel.G) * SonicBeamAlpha);
			color_b += (int)(float(nPixel.B) * SonicBeamAlpha);

			result.R = (BYTE)std::clamp(color_r + ((alpha * nPixel.R) / 256), 0 ,255);
			result.G = (BYTE)std::clamp(color_g + ((alpha * nPixel.G) / 256), 0, 255);
			result.B = (BYTE)std::clamp(color_b + ((alpha * nPixel.B) / 256), 0, 255);


			/**
			 *  Original sonic beam color code.
			 */
		}
		else
		{
			const ColorStruct nPixel = Drawing::WordColor(pixel);
			result.R = nPixel.R;
			result.G = (BYTE)std::clamp(((intensity * nPixel.G) / 256) + nPixel.G, 0, 255);
			result.B = (BYTE)std::clamp(((intensity * nPixel.B) / 256) + nPixel.B, 0, 255);
		}

		/**
		 *  Clamp the color within expected ranges and put the new pixel
		 *  back into the buffer.
		 */
		*buffer = Drawing::ColorStructToWord(result);
	}

	///  Generated tables based on the custom values.
	/// vinivera
	bool SonicBeamTablesCalculated { false };
	short SonicBeamSineTable[500] {};
	short SonicBeamSurfacePatternTable[300][300] {};
	int SonicBeamIntensityTable[14] {};
};


class WeaponTypeClass;
class WaveExt
{
public:

	static constexpr size_t Canary = 0xAABAAAAC;
	using base_type = WaveClass;

	class ExtData final : public Extension<WaveClass>
	{
	public:

		WeaponTypeClass* Weapon;
		int WeaponIdx;
		bool ReverseAgainstTarget;
		CoordStruct SourceCoord;
		bool CanDoUpdate;
		/// vinivera
		/*
		bool SonicBeamIsClear;
		double SonicBeamAlpha;
		double SonicBeamSineDuration;
		double SonicBeamSineAmplitude;
		double SonicBeamOffset;
		SonicBeamSurfacePatternType SonicBeamSurfacePattern;
		SonicBeamSinePatternType SonicBeamSinePattern;
		Vector3D<double> SonicBeamStartPinLeft;
		Vector3D<double> SonicBeamStartPinRight;
		Vector3D<double> SonicBeamEndPinLeft;
		Vector3D<double> SonicBeamEndPinRight;
		*/
		ExtData(WaveClass* OwnerObject) : Extension<WaveClass>(OwnerObject)
			, Weapon { nullptr }
			, WeaponIdx { -1 }
			, ReverseAgainstTarget { false }
			, SourceCoord { }
			, CanDoUpdate { false }
			/*
			, SonicBeamIsClear { false }
			, SonicBeamAlpha { 0.5 }
			, SonicBeamSineDuration { 0.125 }
			, SonicBeamSineAmplitude { 12.0 }
			, SonicBeamOffset { 0.49 }
			, SonicBeamSurfacePattern { SonicBeamSurfacePatternType::CIRCLE }
			, SonicBeamSinePattern { SonicBeamSinePatternType::CIRCLE }
			, SonicBeamStartPinLeft { -30.0, -100.0, 0.0 }
			, SonicBeamStartPinRight { -30.0, 100.0, 0.0 }
			, SonicBeamEndPinLeft { 30.0, -100.0, 0.0 }
			, SonicBeamEndPinRight { 30.0, 100.0, 0.0 }
			*/
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void Initialize() override;

		void InitWeaponData();
		void SetWeaponType(WeaponTypeClass* pWeapon, int nIdx);
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WaveExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static WaveClass* Create(CoordStruct nFrom, CoordStruct nTo, TechnoClass* pOwner, WaveType nType, AbstractClass* pTarget, WeaponTypeClass* pWeapon , bool FromSourceCoord = false);
	static bool ModifyWaveColor(WORD const src, WORD& dest, int const intensity, WaveClass* const pWave, WaveColorData const* colorDatas);
	static Point3D GetIntent(WeaponTypeClass* pWeapon);
	static ColorStruct GetColor(WeaponTypeClass* pWeapon, WaveClass* pWave);
	static WaveColorData GetWaveColor(WaveClass* pWave);

};
