#pragma once
#include <CaptureManagerClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class CaptureExt
{
public:
	using base_type = CaptureManagerClass;

	class ExtData final : public TExtension<base_type>
	{
	public:

		ExtData(CaptureManagerClass* OwnerObject) : TExtension<base_type>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static CaptureExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::CaptureManager ? reinterpret_cast<CaptureExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject):nullptr;
	}

	class ExtContainer final : public TExtensionContainer<CaptureExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};