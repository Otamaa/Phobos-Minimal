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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	int NewPowerups;
	int InfantryCount;
	UnitClass* IncomingUnit;
	UnitClass* IncomingUnitAlt;
	HelperedVector<RadSiteClass*> RadSites;
	HelperedVector<RadLevel> RadLevels;
#pragma endregion

	CellExtData(CellClass* pObj) : AbstractExtended(pObj)
		, NewPowerups(-1)
		, InfantryCount(0)
		, IncomingUnit(nullptr)
		, IncomingUnitAlt(nullptr)
		, RadSites()
		, RadLevels()

	{
		auto pIdent = Phobos::gEntt->try_get<ExtensionIdentifierComponent>(this->MyEntity);
		pIdent->Name = "CellClass";
		pIdent->AbsType = CellClass::AbsID;
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

	virtual CellClass* This() const override { return reinterpret_cast<CellClass*>(this->AbstractExtended::This()); }
	virtual const CellClass* This_Const() const override { return reinterpret_cast<const CellClass*>(this->AbstractExtended::This_Const()); }

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
	static CellExtContainer Instance;

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

class NOVTABLE FakeCellClass : public CellClass
{
public:
	bool _SpreadTiberium(bool force);
	bool _SpreadTiberium_2(TerrainClass* pTerrain, bool force);
	void _Invalidate(AbstractClass* ptr, bool removed);
	int _GetTiberiumType();
	bool _CanTiberiumGerminate(TiberiumClass* tiberium);
	bool _CanPlaceVeins();
	int _Reduce_Tiberium(int levels_reducer);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

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
