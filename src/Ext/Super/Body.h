#pragma once
#include <SuperClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

// cache all super weapon statuses
struct SWStatus
{
	bool Available; //0
	bool PowerSourced; //1
	bool Charging;

	void __forceinline reset()
	{
		std::memset(this, 0, sizeof(SWStatus));
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SWStatus*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Available)
			.Process(PowerSourced)
			.Process(Charging)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

class SuperExtData final
{
public:
	static constexpr size_t Canary = 0x12311111;
	using base_type = SuperClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	SWTypeExtData* Type { nullptr };
	bool Temp_IsPlayer { false };
	CellStruct Temp_CellStruct { };
	bool CameoFirstClickDone { false };
	bool FirstClickAutoFireDone { false };
	//TechnoClass* Firer { nullptr };
	SWStatus Statusses { };

	SuperExtData() noexcept = default;
	~SuperExtData() noexcept = default;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved)
	{
		//	AnnounceInvalidPointer(Firer, ptr);
	}

	static bool InvalidateIgnorable(AbstractClass* ptr)
	{
		switch (VTable::Get(ptr))
		{
		case BuildingClass::vtable:
		case AircraftClass::vtable:
		case UnitClass::vtable:
		case InfantryClass::vtable:
			return false;
		}

		return true;
	}

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static void UpdateSuperWeaponStatuses(HouseClass* pHouse);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SuperExtContainer final : public Container<SuperExtData>
{
public:
	static SuperExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(SuperExtContainer, SuperExtData, "SuperClass");
};