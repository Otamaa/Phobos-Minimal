#pragma once
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/LaserTrailTypeClass.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

class ParticleTypeExtData final
{
public:
	static constexpr size_t Canary = 0xEAEEEEEE;
	using base_type = ParticleTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types { };
	TrailsReader Trails { };
	Valueable<bool> ReadjustZ { true };
	Valueable<PaletteManager*> Palette { }; //CustomPalette::PaletteMode::Temperate
	Valueable<double> DamageRange { 0.0 };
	Valueable<bool> DeleteWhenReachWater { false };

	std::array<Point2D, (size_t)FacingType::Count> WindMult {};

	Valueable<PartialVector2D<int>> Gas_DriftSpeed { {2, -2} };
	Valueable<bool> Transmogrify { false };
	Valueable<int> TransmogrifyChance { -1 };
	Valueable<UnitTypeClass*> TransmogrifyType { nullptr };
	Valueable<OwnerHouseKind> TransmogrifyOwner { OwnerHouseKind::Neutral };

	Valueable<bool> Fire_DamagingAnim { false };

	ParticleTypeExtData() noexcept = default;
	~ParticleTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void Initialize();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleTypeExtContainer final : public Container<ParticleTypeExtData>
{
public:
	static ParticleTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(ParticleTypeExtContainer, ParticleTypeExtData, "ParticleTypeClass");
};