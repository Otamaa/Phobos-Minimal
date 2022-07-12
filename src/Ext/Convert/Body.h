#pragma once
#include <ConvertClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ConvertExt
{
public:
	using base_type = ConvertClass;

	class ExtData final : public Extension<ConvertClass>
	{
	public:

		PhobosFixedString<0x100> Name;
		ExtData(ConvertClass* OwnerObject) : Extension<ConvertClass>(OwnerObject)
			, Name { }
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); };
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}
		virtual void InitializeConstants() override;
	};

	class ExtContainer final : public Container<ConvertExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static void GetOrSetName(ConvertClass* const pConvert , const std::string_view nName);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};