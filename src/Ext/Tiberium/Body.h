#pragma once
#include <TiberiumClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;

class TiberiumExtData final
{
public:
	static constexpr size_t Canary = 0xB16B00B5;
	using base_type = TiberiumClass;

	//Dont forget to remove this if ares one re-enabled
#ifndef aaa
	static constexpr size_t ExtOffset = 0xAC;
#endif

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<PaletteManager*> Palette {}; //CustomPalette::PaletteMode::Temperate
	Nullable<AnimTypeClass*> OreTwinkle {};
	Nullable<int> OreTwinkleChance {};
	Nullable<int> Ore_TintLevel {};
	Nullable<ColorStruct> MinimapColor {};
	Valueable<bool> EnableLighningFix { true };
	Valueable<bool> UseNormalLight { true };
	Valueable<bool> EnablePixelFXAnim { true };

	Nullable<int> Damage {};
	Nullable<WarheadTypeClass*> Warhead {};

	Nullable<int> Heal_Step {};
	Nullable<int> Heal_IStep {};
	Nullable<int> Heal_UStep {};
	Nullable<double> Heal_Delay {};

	Nullable<WarheadTypeClass*> ExplosionWarhead {};
	Nullable<int> ExplosionDamage {};

	Valueable<int> DebrisChance { 33 };

	TiberiumExtData()noexcept = default;
	~TiberiumExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	__forceinline AnimTypeClass* GetTwinkleAnim() const
	{
		return this->OreTwinkle.Get(RulesClass::Instance->OreTwinkle);
	}

	__forceinline int GetTwinkleChance() const
	{
		return this->OreTwinkleChance.Get(RulesClass::Instance->OreTwinkleChance);
	}

	__forceinline double GetHealDelay() const
	{
		return this->Heal_Delay.Get(RulesClass::Instance->TiberiumHeal);
	}

	int GetHealStep(TechnoClass* pTechno) const;

	__forceinline int GetDamage() const
	{
		return this->Damage.Get(MinImpl((this->AttachedToObject->Power / 10), 1));
	}

	__forceinline WarheadTypeClass* GetWarhead() const
	{
		return this->Warhead.Get(RulesClass::Instance->C4Warhead);
	}

	__forceinline WarheadTypeClass* GetExplosionWarhead() const
	{
		return this->ExplosionWarhead.Get(RulesClass::Instance->C4Warhead);
	}

	__forceinline int GetExplosionDamage() const
	{
		return this->ExplosionDamage.Get(RulesClass::Instance->TiberiumExplosionDamage);
	}

	__forceinline int GetDebrisChance() const
	{
		return this->DebrisChance;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TiberiumExtExtContainer final : public Container<TiberiumExtData>
{
public:
	static TiberiumExtExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(TiberiumExtExtContainer, TiberiumExtData, "TiberiumClass");
};