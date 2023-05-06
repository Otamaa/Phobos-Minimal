#pragma once
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/LaserTrailTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

class ParticleTypeExt
{
public:
	class ExtData final : public Extension<ParticleTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xEAEEEEEE;
		using base_type = ParticleTypeClass;

	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
		Valueable<bool> ReadjustZ;
		Valueable<PaletteManager*> Palette; //CustomPalette::PaletteMode::Temperate
		Valueable<double> DamageRange;
		Valueable<bool> DeleteWhenReachWater;

		ExtData(ParticleTypeClass* OwnerObject) : Extension<ParticleTypeClass>(OwnerObject)
			, LaserTrail_Types()
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, ReadjustZ { true }
			, Palette {}
			, DamageRange { 0.0 }
			, DeleteWhenReachWater { false }
		{ }

		virtual ~ExtData() override  = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};