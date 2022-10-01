#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class InfantryExt
{
public:
	static constexpr size_t Canary = 0xACCAAAAA;
	using base_type = InfantryClass;
	static constexpr size_t ExtOffset = 0x6E0;

	class ExtData final : public Extension<base_type>
	{
	public:

		bool IsUsingDeathSequence;
		int CurrentDoType;
		bool ForceFullRearmDelay;
		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, IsUsingDeathSequence { false }
			, CurrentDoType { -1 }
			, ForceFullRearmDelay { false }
		{ }

		virtual ~ExtData() = default;

		void InvalidatePointer(void* const ptr, bool bRemoved);
		void InitializeConstants();
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryExt
		, true
		, true
		, true
	>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
