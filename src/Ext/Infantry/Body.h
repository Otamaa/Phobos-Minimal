#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/CompileTimeDirStruct.h>

class InfantryExt
{
public:
	static constexpr size_t Canary = 0xACCAAAAA;
	using base_type = InfantryClass;
	static constexpr size_t ExtOffset = 0x6EC;

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

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void InitializeConstants() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
