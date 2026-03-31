#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/SavegameDef.h>
#include <Utilities/VectorHelper.h>

struct RadLevel
{
	RadSiteClass* Rad { nullptr };
	int Level { 0 };

	bool Load(PhobosStreamReader& stm, bool registerForChange) {
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const {
		return const_cast<RadLevel*>(this)->Serialize(stm);
	}

private:

	template <typename T>
	bool Serialize(T& stm) {
		return stm
			.Process(this->Rad)
			.Process(this->Level)
			.Success();
	}
};

class CellExtData final : public AbstractExtended
{
public:
	using base_type = CellClass;
	static COMPILETIMEEVAL const char* ClassName = "CellExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "CellClass";
	
	

public:

#pragma region ClassMembers
	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	UnitClass* IncomingUnit {};
	UnitClass* IncomingUnitAlt {};

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	HelperedVector<RadSiteClass*> RadSites {};
	HelperedVector<RadLevel> RadLevels {};

	// ============================================================
	// 4-byte aligned: int
	// ============================================================
	int NewPowerups {-1};
	int InfantryCount {};

#pragma endregion

public:
	CellExtData(CellClass* pObj) : AbstractExtended(pObj)
	{
		this->AbsType = CellClass::AbsID;
	}

	CellExtData(CellClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~CellExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) {
		const_cast<CellExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<CellExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	CellClass* This() const { return reinterpret_cast<CellClass*>(this->AttachedToObject); }
	const CellClass* This_Const() const { return reinterpret_cast<const CellClass*>(this->AttachedToObject); }

public:

	static int __fastcall GetTiberiumType(int Overlay);

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	// Don t call it without checking Tiberium existence
	// otherwise crash
	static TiberiumClass* GetTiberium(CellClass* pCell);
	static int GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium);
	static int GetOverlayIndex(CellClass* pCell);
};

class CellExtContainer final : public Container<CellExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "CellExtContainer";

public:
	static CellExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }
};

enum class CollectResult : char {
	cannot, can
};

class NOVTABLE FakeCellClass : public CellClass
{
public:

	CollectResult _CollecCrate(FootClass* pCollector);
	bool _SpreadTiberium(bool force);
	bool _SpreadTiberium_2(TerrainClass* pTerrain, bool force);
	void _Invalidate(AbstractClass* ptr, bool removed);
	int _GetTiberiumType();
	bool _CanTiberiumGerminate(TiberiumClass* tiberium);
	bool _CanPlaceVeins();
	int _Reduce_Tiberium(int levels_reducer);

	static void __fastcall _ChainReaction(CellStruct* coords);

	FORCEDINLINE CellClass* _AsCell() const
	{
		return (CellClass*)this;
	}

	FORCEDINLINE CellExtData* _GetExtData()
	{
		return *reinterpret_cast<CellExtData**>(((DWORD)this) + 0x18);
	}

};
static_assert(sizeof(FakeCellClass) == sizeof(CellClass), "Missmathc size !");

class NOVTABLE FakeMapClass : public MapClass
{
public:
	CellStruct* __NearByLocation(
		CellStruct* pOutBuffer,
		const CellStruct* pPosition,
		SpeedType speed,
		int zone,
		MovementZone movementZone,
		bool alt,
		int spaceSizeX,
		int spaceSizeY,
		bool disallowOverlay,
		bool checkLevel,
		bool requireBurrowable,
		bool allowBridge,
		const CellStruct* pCloseTo,
		bool skipFirstCheck,
		bool checkBuildable);
};
