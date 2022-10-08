#pragma once
#include <WaveClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

//typedef enum SonicBeamSinePatternType
//{
//	SINE_PATTERN_CIRCLE,
//	SINE_PATTERN_SQUARE,
//	SINE_PATTERN_SAWTOOTH,
//	SINE_PATTERN_TRIANGLE,
//
//	SINE_PATTERN_COUNT
//} SonicBeamSinePatternType;
//MAKE_ENUM_FLAGS(SonicBeamSinePatternType)
//
//typedef enum SonicBeamSurfacePatternType
//{
//	SURFACE_PATTERN_CIRCLE,
//	SURFACE_PATTERN_ELLIPSE,
//	SURFACE_PATTERN_RHOMBUS,
//	SURFACE_PATTERN_SQUARE,
//
//	SURFACE_PATTERN_COUNT
//} SonicBeamSurfacePatternType;
//MAKE_ENUM_FLAGS(SonicBeamSurfacePatternType)

class WaveExt
{
public:
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t Canary = 0xAAAAAAAC;
	using base_type = WaveClass;

	class ExtData final : public TExtension<WaveClass>
	{
	public:
		/**
		*  The following are copied from WeaponTypeExtension on creation.
		*/
		//WeaponTypeClass* MyWeapon;
		//ColorStruct SonicBeamColor;
		//double SonicBeamAmplitude;
		//double SonicBeamDuration;
		//double SonicBeamOffset;
		//bool SonicBeamIsClear;
		//double SonicBeamAlpha;
		//Vector3D<double> SonicBeamStartPinLeft;
		//Vector3D<double>  SonicBeamStartPinRight;
		//Vector3D<double>  SonicBeamEndPinLeft;
		//Vector3D<double>  SonicBeamEndPinRight;
		//SonicBeamSurfacePatternType SonicBeamSurfacePattern;
		//SonicBeamSinePatternType SonicBeamSinePattern;

		/**
		*  Generated tables based on the custom values.
		*/
		//bool SonicBeamTablesCalculated;
		//short SonicBeamSineTable[500];
		//short SonicBeamSurfacePatternTable[300][300];
		//int custom_SonicBeam_LUT_Linear1[14];

		ExtData(WaveClass* OwnerObject) : TExtension<WaveClass>(OwnerObject)
			//, MyWeapon(nullptr),
			//SonicBeamDuration(0.125),
			//SonicBeamColor{ 0, 0, 0 },
			//SonicBeamAmplitude(12.0),
			//SonicBeamOffset(0.49),
			//SonicBeamAlpha(0.5),
			//SonicBeamSurfacePattern(SonicBeamSurfacePatternType::SURFACE_PATTERN_CIRCLE),
			//SonicBeamSinePattern(SonicBeamSinePatternType::SINE_PATTERN_CIRCLE),
			//SonicBeamStartPinLeft({ -30.0, -100.0, 0.0 }),
			//SonicBeamStartPinRight({ -30.0, 100.0, 0.0 }),
			//SonicBeamEndPinLeft({ 30.0, -100.0, 0.0 }),
			//SonicBeamEndPinRight({ 30.0, 100.0, 0.0 }),
			//SonicBeamIsClear(true),
			//SonicBeamTablesCalculated(false),
			//SonicBeamSineTable(),
			//SonicBeamSurfacePatternTable(),
			//custom_SonicBeam_LUT_Linear1()

		{ }

		void Draw_Sonic_Beam_Pixel(int a1, int a2, int a3, unsigned short *buffer);
		bool Calculate_Sonic_Beam_Tables();
		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); };
		void InvalidatePointer(void *ptr, bool bRemoved) { }
		//{
		//	AnnounceInvalidPointer(MyWeapon, ptr);
		//}
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void Initialize() override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<WaveExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
#endif
};

//class WaveClassFake final : public WaveClass
//{
//public:
//
//	void Draw_Sonic_Beam_Pixel_Intercept(int a1, int a2, int a3, unsigned short *buffer)
//	{
//		if (auto waveext = WaveExt::ExtMap.Find(this))
//			waveext->Draw_Sonic_Beam_Pixel(a1, a2, a3, buffer);
//		else
//			Draw_Sonic_Beam_Pixel(a1, a2, a3, buffer);
//	}
//
//	bool Generate_Tables_Intercept()
//	{
//		if (auto waveext = WaveExt::ExtMap.Find(this))
//			waveext->Calculate_Sonic_Beam_Tables();
//		else
//			Generate_Tables();
//	}
//};

//
//struct SonicBeamDataContainer
//{
//	Valueable<ColorStruct> SonicBeamColor;
//	Valueable<double> SonicBeamAmplitude;
//	Valueable<double> SonicBeamDuration;
//	Valueable<double> SonicBeamOffset;
//	Valueable<bool> SonicBeamIsClear;
//	Valueable<double> SonicBeamAlpha;
//	Valueable<Vector3D<double>> SonicBeamStartPinLeft;
//	Valueable<Vector3D<double>> SonicBeamStartPinRight;
//	Valueable<Vector3D<double>> SonicBeamEndPinLeft;
//	Valueable<Vector3D<double>> SonicBeamEndPinRight;
//	SonicBeamSurfacePatternType SonicBeamSurfacePattern;
//	SonicBeamSinePatternType SonicBeamSinePattern;
//
//	const void LoadFromINI(INI_EX &parser, CCINIClass* pIni, const char* pSection)
//	{
//		if (!pSection)
//			return;
//
//		SonicBeamColor.Read(parser, pSection, "Sonic.BeamColor");
//		SonicBeamAmplitude.Read(parser, pSection, "SonicBeamAmplitude");
//		SonicBeamDuration.Read(parser, pSection, "SonicBeamDuration");
//		SonicBeamOffset.Read(parser, pSection, "SonicBeamOffset");
//		SonicBeamIsClear.Read(parser, pSection, "SonicBeamIsClear");
//		SonicBeamAlpha.Read(parser, pSection, "SonicBeamAlpha");
//		SonicBeamStartPinLeft.Read(parser, pSection, "SonicBeamStartPinLeft");
//		SonicBeamStartPinRight.Read(parser, pSection, "SonicBeamStartPinRight");
//		SonicBeamEndPinLeft.Read(parser, pSection, "SonicBeamEndPinLeft");
//		SonicBeamEndPinRight.Read(parser, pSection, "SonicBeamEndPinRight");
//		SonicBeamSurfacePattern = (SonicBeamSurfacePatternType)pIni->ReadInteger(pSection, "SonicBeamSurfacePattern",0);
//		SonicBeamSinePattern = (SonicBeamSinePatternType)pIni->ReadInteger(pSection, "SonicBeamSinePattern", 0);
//	}
//
//	SonicBeamDataContainer() noexcept :
//		SonicBeamDuration(0.125),
//		SonicBeamColor({ 0, 0, 0 }),
//		SonicBeamAmplitude(12.0),
//		SonicBeamOffset(0.49),
//		SonicBeamAlpha(0.5),
//		SonicBeamSurfacePattern(SonicBeamSurfacePatternType::SURFACE_PATTERN_CIRCLE),
//		SonicBeamSinePattern(SonicBeamSinePatternType::SINE_PATTERN_CIRCLE),
//		SonicBeamStartPinLeft({ -30.0, -100.0, 0.0 }),
//		SonicBeamStartPinRight({ -30.0, 100.0, 0.0 }),
//		SonicBeamEndPinLeft({ 30.0, -100.0, 0.0 }),
//		SonicBeamEndPinRight({ 30.0, 100.0, 0.0 }),
//		SonicBeamIsClear(true)
//	{}
//
//	explicit SonicBeamDataContainer(noinit_t) noexcept
//	{ }
//};