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
	static constexpr size_t Canary = 0xEAEEEEEE;
	using base_type = ParticleTypeClass;

	class ExtData final : public Extension<ParticleTypeClass>
	{
	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
		Valueable<bool> ReadjustZ;
		CustomPalette Palette;
		Valueable<double> DamageRange;

		ExtData(ParticleTypeClass* OwnerObject) : Extension<ParticleTypeClass>(OwnerObject)
			, LaserTrail_Types()
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, ReadjustZ { true }
			, Palette(CustomPalette::PaletteMode::Temperate)
			, DamageRange(0.0)
		{ }

		virtual ~ExtData() override  = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}
		virtual bool InvalidateIgnorable(void* const ptr) const { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void Initialize() { }
		virtual void InitializeConstants() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};