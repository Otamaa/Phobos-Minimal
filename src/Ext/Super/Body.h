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

	void reset() {
		Available = false;
		PowerSourced = false;
		Charging = false;
	}
};

class SuperExt
{
public:
	class ExtData final : public Extension<SuperClass>
	{
	public:
		static constexpr size_t Canary = 0x12311111;
		using base_type = SuperClass;
	public:

		SWTypeExt::ExtData* Type { nullptr };
		bool Temp_IsPlayer { false };
		CellStruct Temp_CellStruct { };
		TechnoClass* Firer { nullptr };
		SWStatus Statusses { };
		LauchData LauchDatas { };

		ExtData(SuperClass* OwnerObject) : Extension<SuperClass>(OwnerObject)
		{ }

		virtual ~ExtData() override  = default;
		void InvalidatePointer(void* ptr, bool bRemoved)
		{
			AnnounceInvalidPointer(Firer, ptr);
		}

		bool InvalidateIgnorable(void* ptr) const
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

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SuperExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void UpdateSuperWeaponStatuses(HouseClass* pHouse);
	static ExtContainer ExtMap;
};