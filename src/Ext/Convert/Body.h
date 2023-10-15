#pragma once
#include <ConvertClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ConvertExt
{
public:
	/*
	class ExtData final : public Extension<ConvertClass>
	{
	public:
		static constexpr DWORD Canary = 0xAACAACCC;
		using base_type = ConvertClass;
		static constexpr size_t ExtOffset = 0x178;

	public:

		char* m_ColorDatas { nullptr };
		Blitter* NewBlitters[17] {};
		RLEBlitter* NewRLEBlitters[18] {};

		ExtData(ConvertClass* OwnerObject) : Extension<ConvertClass>(OwnerObject)
		{
			//Built the Color data replacements here
			//Built the new blitters here
		}

		virtual ~ExtData() override = default;
	};

	class ExtContainer final : public Container<ConvertExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	*/
};