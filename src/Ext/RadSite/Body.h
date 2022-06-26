#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
	using base_type = RadSiteClass;

	class ExtData final : public TExtension<base_type>
	{
	public:
		WeaponTypeClass* Weapon;
		RadTypeClass* Type;
		HouseClass* RadHouse;
		TechnoClass* TechOwner;
		bool NoOwner;

		ExtData(base_type* OwnerObject) : TExtension<base_type>(OwnerObject)
			, RadHouse { nullptr }
			, Type { nullptr }
			, Weapon { nullptr }
			, TechOwner { nullptr }
			, NoOwner { true }
		{ }

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }

		virtual void InvalidatePointer(void* ptr, bool bRemoved) {

			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::House:
			case AbstractType::Building:
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
			{
				AnnounceInvalidPointer(RadHouse, ptr);
				AnnounceInvalidPointer(TechOwner, ptr);
			}break;
			}
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override { }

		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static RadSiteExt::ExtData* TryGetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::RadSite ? reinterpret_cast<RadSiteExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}
	__declspec(noinline) static RadSiteExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis ? reinterpret_cast<RadSiteExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt , TechnoClass* const pTech);

	class ExtContainer final : public TExtensionContainer<RadSiteExt>
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
