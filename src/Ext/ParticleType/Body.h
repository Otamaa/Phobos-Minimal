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

	class ExtData final : public TExtension<ParticleTypeClass>
	{
	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
		ExtData(ParticleTypeClass* OwnerObject) : TExtension<ParticleTypeClass>(OwnerObject)
			, LaserTrail_Types()
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);
		//void InvalidatePointer(void *ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		void Initialize() { LaserTrail_Types.reserve(1); }
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};