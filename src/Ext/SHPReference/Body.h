#pragma once

#include <FileFormats/SHP.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SHPRefExt
{
public:
	static constexpr DWORD Canary = 0xAB5005BA;
	using base_type = SHPReference;
	static constexpr size_t ExtOffset = 0x20;

	class ExtData final : public Extension<SHPReference>
	{
	public:

		SHPReference* Alpha;
		RectangleStruct Data;
		ExtData(SHPReference* OwnerObject) : Extension<SHPReference>(OwnerObject)
			, Alpha { nullptr }
			, Data { 0,0,0,0 }
		{}

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void InitializeConstants() override;

	};

	class ExtContainer final : public Container<SHPRefExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};