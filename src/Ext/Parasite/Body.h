#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class ParasiteExt
{
public:

	using base_type = ParasiteClass;

    class ExtData final : public TExtension<ParasiteClass>
    {
    public:

		ExtData(ParasiteClass* OwnerObject) : TExtension<ParasiteClass>(OwnerObject)
        { }

        virtual ~ExtData() override = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) {}
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override { }

	};

	_declspec(noinline) static ParasiteExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Parasite ? reinterpret_cast<ParasiteExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject):nullptr;
	}

	class ExtContainer final : public TExtensionContainer<ParasiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};