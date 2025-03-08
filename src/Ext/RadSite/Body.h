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
	static COMPILETIMEEVAL size_t Canary = 0x87654321;
	using base_type = RadSiteClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	RadTypeClass* Type { nullptr };
	WeaponTypeClass* Weapon { nullptr };
	TechnoClass* TechOwner { nullptr };
	HouseClass* HouseOwner { nullptr };
	bool NoOwner { true };
	int CreationFrame { 0 };
	PhobosMap<BuildingClass*, int> damageCounts {};

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
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

	static void CreateInstance(CellClass* pCell, int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(RadSiteExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class RadSiteExtContainer final : public Container<RadSiteExtData>
{
public:
	static RadSiteExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(RadSiteExtContainer, RadSiteExtData, "RadSiteClass");
};

class FakeRadSiteClass : public RadSiteClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	HouseClass* _GetOwningHouse();
	CoordStruct __GetAltCoords()
	{
		auto const pCell = MapClass::Instance->GetCellAt(this->BaseCell);
		return pCell->GetCoordsWithBridge();
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	RadSiteExtData* _GetExtData() {
		return *reinterpret_cast<RadSiteExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeRadSiteClass) == sizeof(RadSiteClass), "Invalid Size !");
