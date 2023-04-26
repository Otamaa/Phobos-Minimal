#pragma once
#include <TiberiumClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;
class TiberiumExt
{
public:
	static constexpr size_t Canary = 0xB16B00B5;
	using base_type = TiberiumClass;

	class ExtData final : public Extension<TiberiumClass>
	{
	public:
		Valueable<PaletteManager*> Palette; //CustomPalette::PaletteMode::Temperate
		Nullable<AnimTypeClass*> OreTwinkle;
		Nullable<int> OreTwinkleChance;
		Nullable<int> Ore_TintLevel;
		Nullable<ColorStruct> MinimapColor;
		Valueable<bool> EnableLighningFix;
		Valueable<bool> UseNormalLight;
		Valueable<bool> EnablePixelFXAnim;
		//int Replaced_EC;

		Nullable<int> Damage;
		Nullable<WarheadTypeClass*> Warhead;

		Nullable<int> Heal_Step;
		Nullable<int> Heal_IStep;
		Nullable<int> Heal_UStep;
		Nullable<double> Heal_Delay;

		Nullable<WarheadTypeClass*> ExplosionWarhead;
		Nullable<int> ExplosionDamage;

		Valueable<int> DebrisChance;

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject)
			, Palette { }
			, OreTwinkle {}
			, OreTwinkleChance {}
			, Ore_TintLevel {}
			, MinimapColor {}
			, EnableLighningFix { true }
			, UseNormalLight { true }
			, EnablePixelFXAnim { true }
			//, Replaced_EC { 0 }
			, Damage()
			, Warhead()
			, Heal_Step()
			, Heal_IStep()
			, Heal_UStep()
			, Heal_Delay()
			, ExplosionWarhead()
			, ExplosionDamage()
			, DebrisChance(33)
		{ }

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void Initialize() override { } //Init After INI Read

		inline AnimTypeClass* GetTwinkleAnim() const {
			return this->OreTwinkle.Get(RulesClass::Instance->OreTwinkle);
		}

		inline int GetTwinkleChance() const {
			return this->OreTwinkleChance.Get(RulesClass::Instance->OreTwinkleChance);
		}

		double GetHealDelay() const;
		int GetHealStep(TechnoClass* pTechno) const;
		int GetDamage() const;
		WarheadTypeClass* GetWarhead() const;
		WarheadTypeClass* GetExplosionWarhead() const;
		int GetExplosionDamage() const;
		int GetDebrisChance() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TiberiumExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};