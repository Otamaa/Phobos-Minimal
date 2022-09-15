#pragma once
#include <InfantryTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class InfantryTypeExt
{
public:
	static constexpr size_t Canary = 0xAAAAACCA;
	using base_type = InfantryTypeClass;
	static constexpr size_t ExtOffset = 0xECC;

	class ExtData final : public Extension<base_type>
	{
	public:

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);
		void InvalidatePointer(void* const ptr, bool bRemoved);
		void InitializeConstants();
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<InfantryTypeExt
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
