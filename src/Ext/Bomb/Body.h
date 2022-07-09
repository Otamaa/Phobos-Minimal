#pragma once
#include <BombClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class BombExt
{
public:
	using base_type = BombClass;

	class ExtData final : public TExtension<BombClass>
	{
	public:

		ExtData(BombClass* OwnerObject) : TExtension<BombClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		virtual void InitializeConstants() override { }
	};

	static BombExt::ExtData* GetExtData(base_type* pThis)
	{
		return  pThis && pThis->WhatAmI() == AbstractType::Bomb ? reinterpret_cast<BombExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<BombExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};