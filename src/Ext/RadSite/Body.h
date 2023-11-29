#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;
class RadSiteExtData final
{
public:
	static constexpr size_t Canary = 0x87654321;
	using base_type = RadSiteClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	RadTypeClass* Type { nullptr };
	WeaponTypeClass* Weapon { nullptr };
	TechnoClass* TechOwner { nullptr };
	HouseClass* HouseOwner { nullptr };
	bool NoOwner { true };

	RadSiteExtData() noexcept = default;
	~RadSiteExtData() noexcept = default;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	static bool InvalidateIgnorable(AbstractClass* ptr)
	{

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

	enum DamagingState
	{
		Dead, Ignore, Continue
	};

	const DamagingState ApplyRadiationDamage(TechnoClass* pTarget, int damage, int distance);

	static void CreateInstance(CoordStruct const& nCoord, int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech);
	static CoordStruct __fastcall GetAltCoords_Wrapper(RadSiteClass* pThis, void* _)
	{
		auto const pCell = MapClass::Instance->GetCellAt(pThis->BaseCell);
		return pCell->GetCoordsWithBridge();
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class RadSiteExtContainer final : public Container<RadSiteExtData>
{
public:
	static RadSiteExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(RadSiteExtContainer, RadSiteExtData, "RadSiteClass");
};