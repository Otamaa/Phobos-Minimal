#pragma once

#include <FileFormats/SHP.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SHPRefExt
{
public:
	using base_type = SHPReference;

	class ExtData final : public Extension<SHPReference>
	{
	public:

		ExtData(SHPReference* OwnerObject) : Extension<SHPReference>(OwnerObject) {}

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {}

	};

	class ExtContainer final : public Container<SHPRefExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};