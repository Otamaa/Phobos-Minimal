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

	constexpr FORCEINLINE static size_t size_Of()
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
	static std::vector<RadSiteExtData*> Pool;
	static RadSiteExtContainer Instance;

	RadSiteExtData* AllocateUnchecked(RadSiteClass* key)
	{
		RadSiteExtData* val = nullptr;
		if (!Pool.empty())
		{
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
			val->RadSiteExtData::RadSiteExtData();
		}
		else
		{
			val = new RadSiteExtData();
		}

		if (val)
		{
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	RadSiteExtData* Allocate(RadSiteClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (RadSiteExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(RadSiteClass* key)
	{
		if (RadSiteExtData* Item = TryFind(key))
		{
			Item->~RadSiteExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	CONSTEXPR_NOCOPY_CLASSB(RadSiteExtContainer, RadSiteExtData, "RadSiteClass");
};