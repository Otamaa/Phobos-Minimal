#pragma once
#include <TiberiumClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;
class TiberiumExt
{
public:
	static constexpr size_t Canary = 0xB16B00B5;
	using base_type = TiberiumClass;
//#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
//#endif

	class ExtData final : public Extension<TiberiumClass>
	{
	public:

		Nullable<AnimTypeClass*> OreTwinkle;
		Nullable<int> OreTwinkleChance;
		Nullable<int> Ore_TintLevel;
		Nullable<ColorStruct> MinimapColor;
		Valueable<bool> EnableLighningFix;

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject)
			, OreTwinkle {}
			, OreTwinkleChance {}
			, Ore_TintLevel {}
			, MinimapColor {}
			, EnableLighningFix { true } //TODO : default to true
		{ }

		virtual ~ExtData() override = default;
		void LoadFromINIFile(CCINIClass* pINI);
		// void InvalidatePointer(void *ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void Initialize() { } //Init After INI Read
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

	static TiberiumExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<TiberiumExt ,true>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};