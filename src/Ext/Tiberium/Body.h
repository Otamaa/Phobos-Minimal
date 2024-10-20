#pragma once
#include <TiberiumClass.h>

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
	static constexpr size_t ExtOffset = 0xAC;

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

	Valueable<std::string> LinkedOverlayType {};
	Valueable<int> PipIndex { -1 };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE AnimTypeClass* GetTwinkleAnim() const
	{
		return this->OreTwinkle.Get(RulesClass::Instance->OreTwinkle);
	}

	constexpr FORCEINLINE int GetTwinkleChance() const
	{
		return this->OreTwinkleChance.Get(RulesClass::Instance->OreTwinkleChance);
	}

	constexpr FORCEINLINE double GetHealDelay() const
	{
		return this->Heal_Delay.Get(RulesClass::Instance->TiberiumHeal);
	}

	int GetHealStep(TechnoClass* pTechno) const;

	constexpr FORCEINLINE int GetDamage() const
	{
		return this->Damage.Get(MinImpl((this->AttachedToObject->Power / 10), 1));
	}

	constexpr FORCEINLINE WarheadTypeClass* GetWarhead() const
	{
		return this->Warhead.Get(RulesClass::Instance->C4Warhead);
	}

	constexpr FORCEINLINE WarheadTypeClass* GetExplosionWarhead() const
	{
		return this->ExplosionWarhead.Get(RulesClass::Instance->C4Warhead);
	}

	constexpr FORCEINLINE int GetExplosionDamage() const
	{
		return this->ExplosionDamage.Get(RulesClass::Instance->TiberiumExplosionDamage);
	}

	constexpr FORCEINLINE int GetDebrisChance() const
	{
		return this->DebrisChance;
	}

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(TiberiumExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class TiberiumExtContainer final : public Container<TiberiumExtData>
{
public:
	static TiberiumExtContainer Instance;
	static PhobosMap<OverlayTypeClass* , TiberiumClass*> LinkedType;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	void Clear();

	//CONSTEXPR_NOCOPY_CLASSB(TiberiumExtContainer, TiberiumExtData, "TiberiumClass");
};

class FakeTiberiumClass : public TiberiumClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	TiberiumExtData* _GetExtData() {
		return *reinterpret_cast<TiberiumExtData**>(((DWORD)this) + TiberiumExtData::ExtOffset);
	}
};
static_assert(sizeof(FakeTiberiumClass) == sizeof(TiberiumClass), "Invalid Size !");