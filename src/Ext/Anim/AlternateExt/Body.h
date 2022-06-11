#pragma once

#include <Ext/Abstract/Body.h>
#include <AnimClass.h>
#include <Utilities/Debug.h>

class AnimExtAlt
{
public:
	using base_type = AnimClass;

	class ExtData final : public TExtension<base_type>
	{
	public:
		short DeathUnitFacing;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit;
		bool DeathUnitHasTurret;
		TechnoClass* Invoker;

		ExtData(base_type* OwnerObject) : TExtension<base_type>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, Invoker { nullptr }
		{}

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* const ptr, bool bRemoved) override {

			if (Invoker == ptr)
				Invoker = nullptr;

		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static AnimExtAlt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Anim ? reinterpret_cast<AnimExtAlt::ExtData*>
				(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<AnimExtAlt>
	{
	public:
		ExtContainer();
		~ExtContainer();

	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};