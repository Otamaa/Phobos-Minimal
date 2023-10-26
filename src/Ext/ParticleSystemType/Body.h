#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class ParticleSystemTypeExtData final
{
public:
	static constexpr size_t Canary = 0xEAAEEEEE;
	using base_type = ParticleSystemTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<bool> ApplyOptimization { true };
	std::array<Point2D, (size_t)FacingType::Count> FacingMult {};
	Valueable<bool> AdjustTargetCoordsOnRotation { true };

	ParticleSystemTypeExtData() noexcept = default;
	~ParticleSystemTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemTypeExtContainer final : public Container<ParticleSystemTypeExtData>
{
public:
	static ParticleSystemTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(ParticleSystemTypeExtContainer, ParticleSystemTypeExtData, "ParticleSystemTypeClass");
};