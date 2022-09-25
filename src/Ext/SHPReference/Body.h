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
	static constexpr size_t ExtOffset = sizeof(base_type);

	class ExtData final : public Extension<SHPReference>
	{
	public:

		SHPReference* Alpha;
		RectangleStruct Data;
		ExtData(SHPReference* OwnerObject) : Extension<SHPReference>(OwnerObject)
			, Alpha { nullptr }
			, Data { 0,0,0,0 }
		{}

		virtual ~ExtData() = default;
		void InvalidatePointer(void *ptr, bool bRemoved) {}
		void InitializeConstants();

	};

	class ExtContainer final : public Container<SHPRefExt, true, true, true>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

};