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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	RadTypeClass* Type;
	WeaponTypeClass* Weapon;
	TechnoClass* TechOwner;
	HouseClass* HouseOwner;
	bool NoOwner;
	int CreationFrame;
	PhobosMap<BuildingClass*, int> damageCounts;
#pragma endregion

public:
	RadSiteExtData(RadSiteClass* pObj) : AbstractExtended(pObj),
		Type(nullptr),
		Weapon(nullptr),
		TechOwner(nullptr),
		HouseOwner(nullptr),
		NoOwner(true),
		CreationFrame(0)
	{ }
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

	virtual RadSiteClass* This() const override { return reinterpret_cast<RadSiteClass*>(this->AbstractExtended::This()); }
	virtual const RadSiteClass* This_Const() const override { return reinterpret_cast<const RadSiteClass*>(this->AbstractExtended::This_Const()); }

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
	static RadSiteExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

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

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	RadSiteExtData* _GetExtData() {
		return *reinterpret_cast<RadSiteExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeRadSiteClass) == sizeof(RadSiteClass), "Invalid Size !");
