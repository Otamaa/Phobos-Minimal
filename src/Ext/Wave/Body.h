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
						SonicBeamSurfacePatternTable[x][y] = std::sqrt(x * x + y * y);
						break;
					case SonicBeamSurfacePatternType::ELLIPSE:
					{
						double x_stretch_coeff = 2.0;
						double y_stretch_coeff = 1.0;
						SonicBeamSurfacePatternTable[x][y] = std::sqrt((x * 2) / x_stretch_coeff + (y * 2) / y_stretch_coeff);
						break;
					}
					case SonicBeamSurfacePatternType::RHOMBUS:
						SonicBeamSurfacePatternTable[x][y] = std::sqrt(x + y);
						break;
					case SonicBeamSurfacePatternType::SQUARE:
						SonicBeamSurfacePatternTable[x][y] = std::sqrt(std::max(std::abs(x), std::abs(y)));
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

	void Draw_Sonic_Beam_Pixel(WaveClass* pThis, int a1, int a2, int a3, unsigned short* buffer ,
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
		int wave_pos = pThis->WaveCount + (unsigned short)SonicBeamSurfacePatternTable[a3][std::abs(a1 - pThis->InitialWavePixels_0.X - a2)];
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
		unsigned short pixel = buffer[pThis->intensity_tables[pos]];

		/**
		 *  Re-color the buffer pixel.
		 */
		int r = 0;
		int g = 0;
		int b = 0;

		/**
		 *  Clear beam, not color change.
		 */
		if (SonicBeamIsClear)
		{

			r = (unsigned char)((unsigned char)(pixel >> DSurface::RedLeft) << DSurface::RedRight);
			g = (unsigned char)((unsigned char)(pixel >> DSurface::GreenLeft) << DSurface::GreenRight);
			b = (unsigned char)((unsigned char)(pixel >> DSurface::BlueLeft) << DSurface::BlueRight);


			/**
			 *  Custom color has been set.
			 */
		}
		else if (SonicBeamColor.R != 0 && SonicBeamColor.G != 0 && SonicBeamColor.B != 0)
		{

			int color_r = SonicBeamColor.R;
			int color_g = SonicBeamColor.G;
			int color_b = SonicBeamColor.B;

			/**
			 *  Calculate the alpha of the beam.
			 */
			int alpha = float(intensity) * 1.0;
			color_r += float((unsigned char)((unsigned char)(pixel >> DSurface::RedLeft) << DSurface::RedRight)) * SonicBeamAlpha;
			color_g += float((unsigned char)((unsigned char)(pixel >> DSurface::GreenLeft) << DSurface::GreenRight)) * SonicBeamAlpha;
			color_b += float((unsigned char)((unsigned char)(pixel >> DSurface::BlueLeft) << DSurface::BlueRight)) * SonicBeamAlpha;

			r = color_r + ((alpha * (unsigned char)((unsigned char)(pixel >> DSurface::RedLeft) << DSurface::RedRight)) / 256);
			g = color_g + ((alpha * (unsigned char)((unsigned char)(pixel >> DSurface::GreenLeft) << DSurface::GreenRight)) / 256);
			b = color_b + ((alpha * (unsigned char)((unsigned char)(pixel >> DSurface::BlueLeft) << DSurface::BlueRight)) / 256);


			/**
			 *  Original sonic beam color code.
			 */
		}
		else
		{

			r = (unsigned char)((unsigned char)(pixel >> DSurface::RedLeft) << DSurface::RedRight);

			g = ((intensity * (unsigned char)((unsigned char)(pixel >> DSurface::GreenLeft) << DSurface::GreenRight)) / 256)
				+ (unsigned char)((unsigned char)(pixel >> DSurface::GreenLeft) << DSurface::GreenRight);

			b = ((intensity * (unsigned char)((unsigned char)(pixel >> DSurface::BlueLeft) << DSurface::BlueRight)) / 256)
				+ (unsigned char)((unsigned char)(pixel >> DSurface::BlueLeft) << DSurface::BlueRight);

		}

		/**
		 *  Clamp the color within expected ranges and put the new pixel
		 *  back into the buffer.
		 */
		r = std::min<int>(r, 255);
		g = std::min<int>(g, 255);
		b = std::min<int>(b, 255);
		*buffer = DSurface::RGBA_To_Pixel(r, g, b);
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
			, WeaponIdx { 0 }
			, ReverseAgainstTarget { false }

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
};
