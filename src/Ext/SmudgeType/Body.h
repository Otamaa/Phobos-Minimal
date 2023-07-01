#pragma once
#include <SmudgeTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class SmudgeTypeExt
{
public:
	class ExtData final : public Extension<SmudgeTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xBEE75008;
		using base_type = SmudgeTypeClass;

	public:

		Valueable<bool> Clearable { true };

		ExtData(SmudgeTypeClass* OwnerObject) : Extension<SmudgeTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SmudgeTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};