#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/PhobosMap.h>

struct ColorsData
{
	DWORD Forceshield_Color;
	DWORD IronCurtain_Color;
	DWORD LaserTarget_Color;
	DWORD Berserk_Color;
	bool Initialized;

	constexpr void reset()
	{
		Forceshield_Color = 0u;
		IronCurtain_Color = 0u;
		LaserTarget_Color = 0u;
		Berserk_Color = 0u;
		Initialized = false;
	}

};

class AbstractClass;
class ObjectClass;
class AlphaShapeClass;
class TechnoClass;
class AircraftClass;
class BuildingClass;
class InfantryClass;
class UnitClass;
class PhobosGlobal
{
	static PhobosGlobal GlobalObject;
public:
	bool DetonateDamageArea { true };

	std::vector<CellStruct> TempFoundationData1 { };
	std::vector<CellStruct> TempFoundationData2 { };
	HelperedVector<CellStruct> TempCoveredCellsData { };

	struct CopyArray{
		std::vector<AircraftClass*> Aircraft {};
		std::vector<BuildingClass*> Building {};
		std::vector<InfantryClass*> Infantry {};
		std::vector<UnitClass*> Unit {};

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Serialize(Stm);
		}

		bool Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<CopyArray*>(this)->Serialize(Stm);
		}

		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(Aircraft)
				.Process(Building)
				.Process(Infantry)
				.Process(Unit)
				.Success()
				;
		}

		constexpr void clear() {
			Aircraft.clear();
			Building.clear();
			Infantry.clear();
			Unit.clear();
		}

		void Invalidate(AbstractClass* ptr, bool removed)
		{
			for (auto& item : Aircraft) {
				if (removed && (AbstractClass*)item == ptr) {
					item = nullptr; //we null it since we dont want to change the iterator when the detonation in process
				}
			}

			for (auto& item : Building)
			{
				if (removed && (AbstractClass*)item == ptr)
				{
					item = nullptr; //we null it since we dont want to change the iterator when the detonation in process
				}
			}

			for (auto& item : Infantry)
			{
				if (removed && (AbstractClass*)item == ptr)
				{
					item = nullptr; //we null it since we dont want to change the iterator when the detonation in process
				}
			}

			for (auto& item : Unit)
			{
				if (removed && (AbstractClass*)item == ptr)
				{
					item = nullptr; //we null it since we dont want to change the iterator when the detonation in process
				}
			}
		}
	};

	PhobosMap<WarheadTypeClass*, CopyArray> CurCopyArray {};

	struct PathfindLastData {
		TechnoClass* Finder;
		CellStruct From;
		CellStruct To;

		constexpr bool IsValid() const {
			return Finder != nullptr;
		}

		constexpr void Clear() {
			Finder = nullptr;
			From = CellStruct::Empty;
			To = CellStruct::Empty;
		}

		void InvalidatePointer(AbstractClass* ptr, bool bDetach) {
			if (ptr == (AbstractClass*)Finder)
				this->Clear();
		}

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Stm
				.Process(this->Finder, true)
				.Process(this->From)
				.Process(this->To)
				.Success();
		}

		bool Save(PhobosStreamWriter& Stm) const
		{
			return Stm
				.Process(this->Finder)
				.Process(this->From)
				.Process(this->To)
				.Success();
		}

	};

	PathfindLastData PathfindTechno { };
	ColorsData ColorDatas { };

public:
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	constexpr FORCEINLINE static PhobosGlobal* Instance() {
		return &GlobalObject;
	}

	static void Clear();
	static void PointerGotInvalid(AbstractClass* ptr, bool removed);

	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->DetonateDamageArea)
			.Process(this->TempFoundationData1)
			.Process(this->TempFoundationData2)
			.Process(this->TempCoveredCellsData)
			.Process(this->PathfindTechno)
			.Process(this->CurCopyArray)
			.Process(this->ColorDatas)
			.Success();
	}

};