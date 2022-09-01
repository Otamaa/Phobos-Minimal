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
		void InvalidatePointer(void *ptr, bool bRemoved) {}
		void InitializeConstants();
	};

	class ExtContainer final : public Container<ConvertExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
	static void GetOrSetName(ConvertClass* const pConvert , const std::string_view nName);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};