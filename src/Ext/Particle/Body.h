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

	class ExtData final : public Extension<ParticleClass>
	{
	public:
		static constexpr size_t Canary = 0xAAAABBBB;
		using base_type = ParticleClass;
		//static constexpr size_t ExtOffset = 0x134;

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

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};