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
	class ExtData final : public Extension<RadSiteClass>
	{
	public:
		static constexpr size_t Canary = 0x87654321;
		using base_type = RadSiteClass;

	public:
		RadTypeClass* Type { nullptr };
		WeaponTypeClass* Weapon { nullptr };
		TechnoClass* TechOwner { nullptr };
		HouseClass* HouseOwner { nullptr };
		bool NoOwner { true };

		ExtData(base_type* OwnerObject) : Extension<base_type>(OwnerObject)
		{}

		virtual ~ExtData() override = default;

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

		static bool InvalidateIgnorable(AbstractClass* ptr) {

			switch (VTable::Get(ptr))
			{
			case BuildingClass::vtable:
			case AircraftClass::vtable:
			case UnitClass::vtable:
			case InfantryClass::vtable:
			case HouseClass::vtable:
				return false;
			}

			return true;
		}

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void CreateLight();
		void Add(int amount);
		void SetRadLevel(int amount);
		const double GetRadLevelAt(CellStruct const& cell);
		const double GetRadLevelAt(double distance);
		const bool ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<RadSiteExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void CreateInstance(CoordStruct const& nCoord, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, TechnoClass* const pTech);
	static CoordStruct __fastcall GetAltCoords_Wrapper(RadSiteClass* pThis, void* _) {
		auto const pCell = MapClass::Instance->GetCellAt(pThis->BaseCell);
		 return pCell->GetCoordsWithBridge();
	}
};
