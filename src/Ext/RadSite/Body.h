#pragma once

#include <RadSiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

class RadTypeClass;
class RadSiteExtData final : public AbstractExtended
{
public:
	using base_type = RadSiteClass;
	static COMPILETIMEEVAL const char* ClassName = "RadSiteExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "RadSiteClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates
	// ============================================================
	PhobosFixedString<0x18> Name;
	PhobosMap<BuildingClass*, int> damageCounts;

	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	RadTypeClass* Type;
	WeaponTypeClass* Weapon;
	TechnoClass* TechOwner;
	HouseClass* HouseOwner;

	// ============================================================
	// 4-byte aligned: int
	// ============================================================
	int CreationFrame;

	// ============================================================
	// 1-byte aligned: bool (at the end)
	// ============================================================
	bool NoOwner;
	// 1 byte + 3 bytes padding for alignment

#pragma endregion

public:
	RadSiteExtData(RadSiteClass* pObj) : AbstractExtended(pObj)
		// Large aggregates
		, Name()
		, damageCounts()
		// Pointers
		, Type(nullptr)
		, Weapon(nullptr)
		, TechOwner(nullptr)
		, HouseOwner(nullptr)
		// int
		, CreationFrame(0)
		// bool
		, NoOwner(true)
	{
		this->Name = GameStrings::NoneStr();
		this->AbsType = RadSiteClass::AbsID;
	}

	RadSiteExtData(RadSiteClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~RadSiteExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<RadSiteExtData*>(this)->Internal_SaveToStream(Stm);
		const_cast<RadSiteExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	RadSiteClass* This() const { return reinterpret_cast<RadSiteClass*>(this->AttachedToObject); }
	const RadSiteClass* This_Const() const { return reinterpret_cast<const RadSiteClass*>(this->AttachedToObject); }

public:

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

public:

	static void CreateInstance(CellClass* pCell, int spread, int amount, WeaponTypeExtData* pWeaponExt, TechnoClass* const pTech);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class RadSiteExtContainer final : public Container<RadSiteExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "RadSiteExtContainer";

public:
	static RadSiteExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
};

class NOVTABLE FakeRadSiteClass : public RadSiteClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	HouseClass* _GetOwningHouse();
	CoordStruct __GetAltCoords()
	{
		auto const pCell = MapClass::Instance->GetCellAt(this->BaseCell);
		return pCell->GetCoordsWithBridge();
	}

	template<typename Func>
	void ForEachCellInRadiationArea(Func&& callback);

	void __AI();
	void __Reduce_In_Area();
	void __Increase_In_Area();
	void __Reduce_Radiation();
	double __Radiation_At(CellStruct* cell) const;

	RadSiteExtData* _GetExtData() {
		return *reinterpret_cast<RadSiteExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeRadSiteClass) == sizeof(RadSiteClass), "Invalid Size !");
