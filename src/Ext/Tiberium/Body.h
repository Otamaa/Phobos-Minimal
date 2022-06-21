#pragma once
#include <TiberiumClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;
class TiberiumExt
{
public:
	using base_type = TiberiumClass;

	class ExtData final : public TExtension<TiberiumClass>
	{
	public:

		Nullable<AnimTypeClass*> OreTwinkle;
		Nullable<int> OreTwinkleChance;
		Nullable<int> Ore_TintLevel;
		Nullable<ColorStruct> MinimapColor;

		ExtData(TiberiumClass* OwnerObject) : TExtension<TiberiumClass>(OwnerObject)
			, OreTwinkle {}
			, OreTwinkleChance {}
			, Ore_TintLevel {}
			, MinimapColor {}
		{ }

		virtual ~ExtData() override = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}
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

	_declspec(noinline) static TiberiumExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Tiberium
			? reinterpret_cast<TiberiumExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject):nullptr;
	}

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