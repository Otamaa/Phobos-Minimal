#pragma once
#include <SuperClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <Ext/SWType/Body.h>

class SuperExt
{
public:
	class ExtData final : public Extension<SuperClass>
	{
	public:
		static constexpr size_t Canary = 0x12311111;
		using base_type = SuperClass;
	public:

		SWTypeExt::ExtData* Type;
		bool Temp_IsPlayer;
		CellStruct Temp_CellStruct;
		TechnoClass* Firer;

		ExtData(SuperClass* OwnerObject) : Extension<SuperClass>(OwnerObject)
			, Type { nullptr }
			, Temp_IsPlayer { false }
			, Temp_CellStruct { }
			, Firer { nullptr }
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

	static ExtContainer ExtMap;
};