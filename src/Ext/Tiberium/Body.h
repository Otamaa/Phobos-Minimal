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
	class ExtData final : public Extension<TiberiumClass>
	{
	public:
		static constexpr size_t Canary = 0xB16B00B5;
		using base_type = TiberiumClass;

		//Dont forget to remove this if ares one re-enabled
		static constexpr size_t ExtOffset = 0xAC;

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

		Valueable<int> DebrisChance {33};

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject) { }
		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize() { } //Init After INI Read

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

	class ExtContainer final : public Container<TiberiumExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};