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

		char* m_ColorDatas;
		Blitter* NewBlitters[17];
		RLEBlitter* NewRLEBlitters[18];

		ExtData(ConvertClass* OwnerObject) : Extension<ConvertClass>(OwnerObject)
			, m_ColorDatas { nullptr }
			, NewBlitters {}
			, NewRLEBlitters {}
		{
			//Built the Color data replacements here
			//Built the new blitters here
		}

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; };

		virtual void InitializeConstants() override;
	};

	class ExtContainer final : public Container<ConvertExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};