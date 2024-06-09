#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/ParticleType/Body.h>

class ParticleExtData final
{
public:
	static constexpr size_t Canary = 0xAAAABBBB;
	using base_type = ParticleClass;
	//static constexpr size_t ExtOffset = 0x134;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	std::vector<LaserTrailClass> LaserTrails { };

	ParticleExtData() noexcept = default;
	~ParticleExtData() noexcept = default;

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(ParticleExtData) -
			(4u //AttachedToObject
			 );
	}

	static std::pair<TechnoClass*, HouseClass*> GetOwnership(ParticleClass* pThis);
private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleExtContainer final : public Container<ParticleExtData>
{
public:
	static ParticleExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(ParticleExtContainer, ParticleExtData, "ParticleClass");
};
