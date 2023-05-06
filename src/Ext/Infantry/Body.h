#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/CompileTimeDirStruct.h>

class InfantryExt
{
public:
	class ExtData final : public Extension<InfantryClass>
	{
	public:
		static constexpr size_t Canary = 0xACCAAAAA;
		using base_type = InfantryClass;
		static constexpr size_t ExtOffset = 0x6EC;

	public:

		bool IsUsingDeathSequence;
		int CurrentDoType;
		bool ForceFullRearmDelay;
		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, IsUsingDeathSequence { false }
			, CurrentDoType { -1 }
			, ForceFullRearmDelay { false }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
