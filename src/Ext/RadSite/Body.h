#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;

class RadSiteExt
{
public:
	using base_type = RadSiteClass;

	class ExtData final : public Extension<RadSiteClass>
	{
	public:
		WeaponTypeClass* Weapon;
		RadTypeClass* Type;
		HouseClass* RadHouse;
		TechnoClass* TechOwner;
		bool NoOwner;

		ExtData(RadSiteClass* OwnerObject) : Extension<RadSiteClass>(OwnerObject)
			, RadHouse { nullptr }
			, Type { nullptr }
			, Weapon { nullptr }
			, TechOwner { nullptr }
			, NoOwner { true }
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) {
			AnnounceInvalidPointer(RadHouse, ptr);
			AnnounceInvalidPointer(TechOwner, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override {
			RadSiteExt::Array.Remove(this);
		}
		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static DynamicVectorClass<RadSiteExt::ExtData*> Array;

	static void CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt , TechnoClass* const pTech);

	class ExtContainer final : public Container<RadSiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::House:
			case AbstractType::Building:
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
				return false;
			default:
				return true;
			}
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
