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
	static constexpr size_t Canary = 0x87654321;
	using base_type = RadSiteClass;

	class ExtData final : public Extension<base_type>
	{
	public:
		RadTypeClass* Type;
		WeaponTypeClass* Weapon;
		TechnoClass* TechOwner;
		HouseClass* HouseOwner;
		bool NoOwner;

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
			, Type { nullptr }
			, Weapon { nullptr }
			, TechOwner { nullptr }
			, HouseOwner { nullptr }
			, NoOwner { true }
		{}

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		virtual bool InvalidateIgnorable(void* const ptr) const override { 
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
			case AbstractType::House:
				return false;
			}

			return true;
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		void Uninitialize() { }

		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);
		const bool ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static void CreateInstance(CoordStruct const& nCoord, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt , TechnoClass* const pTech);

	class ExtContainer final : public Container<RadSiteExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static CoordStruct __fastcall GetAltCoords_Wrapper(RadSiteClass* pThis, void* _) {
		auto const pCell = MapClass::Instance->GetCellAt(pThis->BaseCell);
		 return pCell->GetCoordsWithBridge();
	}
};
