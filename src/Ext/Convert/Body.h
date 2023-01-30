#pragma once
#include <ConvertClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ConvertExt
{
public:
	static constexpr DWORD Canary = 0xAACAACCC;
	using base_type = ConvertClass;
	static constexpr size_t ExtOffset = 0x178;

	class ExtData final : public Extension<ConvertClass>
	{
	public:

		PhobosFixedString<0x100> Name;
		char* m_ColorDatas;
		Blitter* NewBlitters[17];
		RLEBlitter* NewRLEBlitters[18];

		ExtData(ConvertClass* OwnerObject) : Extension<ConvertClass>(OwnerObject)
			, Name { }
			, m_ColorDatas { nullptr }
			, NewBlitters {}
			, NewRLEBlitters {}
		{
			//Built the Color data replacements here
			//Built the new blitters here
		}

		virtual ~ExtData() = default;
		void InvalidatePointer(void *ptr, bool bRemoved) {}
		void InitializeConstants();
	};

	class ExtContainer final : public Container<ConvertExt ,true , true , true >
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
	static void GetOrSetName(ConvertClass* const pConvert , const std::string_view nName);
};