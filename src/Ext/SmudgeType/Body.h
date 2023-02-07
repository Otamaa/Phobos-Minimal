#pragma once
#include <SmudgeTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class SmudgeTypeExt
{
public:
	static constexpr size_t Canary = 0xBEE75008;
	using base_type = SmudgeTypeClass;

	class ExtData final : public TExtension<SmudgeTypeClass>
	{
	public:

		Valueable<bool> Clearable;

		ExtData(SmudgeTypeClass* OwnerObject) : TExtension<SmudgeTypeClass>(OwnerObject)
			, Clearable { true }
		{ }

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<SmudgeTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer() = default;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};