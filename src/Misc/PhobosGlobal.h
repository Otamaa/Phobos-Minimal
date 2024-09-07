#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>

class AbstractClass;
class ObjectClass;
class AlphaShapeClass;
class TechnoClass;
class PhobosGlobal
{
	static std::unique_ptr<PhobosGlobal> GlobalObject;
public:
	bool DetonateDamageArea { true };

	std::vector<CellStruct> TempFoundationData1 { };
	std::vector<CellStruct> TempFoundationData2 { };
	std::vector<CellStruct> TempCoveredCellsData { };
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
			if (ptr == Finder)
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
public:
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	constexpr FORCEINLINE static PhobosGlobal* Instance() {
		return GlobalObject.get();
	}

	PhobosGlobal() = default;
	~PhobosGlobal() = default;

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
			.Success();
	}

	struct ColorsData {
		DWORD Forceshield_Color;
		DWORD IronCurtain_Color;
		DWORD LaserTarget_Color;
		DWORD Berserk_Color;
		bool Initialized;

		constexpr void reset() {
			Forceshield_Color = 0u;
			IronCurtain_Color = 0u;
			LaserTarget_Color = 0u;
			Berserk_Color = 0u;
			Initialized = false;
		}

	} static ColorDatas;

};