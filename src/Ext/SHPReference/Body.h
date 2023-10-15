#pragma once

#include <FileFormats/SHP.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SHPRefExt
{
public:
	/*
	class ExtData final : public Extension<SHPReference>
	{
	public:
		static constexpr DWORD Canary = 0xAB5005BA;
		using base_type = SHPReference;
		static constexpr size_t ExtOffset = 0x20;

	public:

		SHPReference* Alpha { nullptr };
		RectangleStruct Data { 0,0,0,0 };
		ExtData(SHPReference* OwnerObject) : Extension<SHPReference>(OwnerObject)
		{}

		virtual ~ExtData() override = default;
		void Initialize();

	};

	class ExtContainer final : public Container<SHPRefExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;*/
};