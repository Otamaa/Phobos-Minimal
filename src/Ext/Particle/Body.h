#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/ParticleType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/Trails.h>
#endif

class ParticleExt
{
public:
	static constexpr size_t Canary = 0xAAAABBBB;
	using base_type = ParticleClass;
	//static constexpr size_t ExtOffset = 0x134;

	class ExtData final : public Extension<ParticleClass>
	{
	public:

		std::vector<LaserTrailClass> LaserTrails;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<UniversalTrail> Trails;
#endif
		ExtData(ParticleClass* OwnerObject) : Extension<ParticleClass>(OwnerObject)
			,  LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		void Uninitialize();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};