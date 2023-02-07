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

	class ExtData final : public TExtension<TiberiumClass>
	{
	public:
		CustomPalette Palette;
		Nullable<AnimTypeClass*> OreTwinkle;
		Nullable<int> OreTwinkleChance;
		Nullable<int> Ore_TintLevel;
		Nullable<ColorStruct> MinimapColor;
		Valueable<bool> EnableLighningFix;
		Valueable<bool> UseNormalLight;
		Valueable<bool> EnablePixelFXAnim;
		//int Replaced_EC;

		ExtData(TiberiumClass* OwnerObject) : TExtension<TiberiumClass>(OwnerObject)
			, Palette { CustomPalette::PaletteMode::Temperate }
			, OreTwinkle {}
			, OreTwinkleChance {}
			, Ore_TintLevel {}
			, MinimapColor {}
			, EnableLighningFix { true }
			, UseNormalLight { true }
			, EnablePixelFXAnim { true }
			//, Replaced_EC { 0 }
		{ }

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void Initialize() override { } //Init After INI Read

		inline AnimTypeClass* GetTwinkleAnim() const {
			return this->OreTwinkle.Get(RulesGlobal->OreTwinkle);
		}

		inline int GetTwinkleChance() const {
			return this->OreTwinkleChance.Get(RulesGlobal->OreTwinkleChance);
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<TiberiumExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};