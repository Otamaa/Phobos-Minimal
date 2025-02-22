#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/SavegameDef.h>

class CellExtData final
{
public:

	static COMPILETIMEEVAL size_t Canary = 0x87688621;
	using base_type = CellClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	int NewPowerups { -1 };
	UnitClass* IncomingUnit { nullptr };
	UnitClass* IncomingUnitAlt { nullptr };

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(CellExtData) -
			(4u //AttachedToObject
			 );
	}

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

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

	//CONSTEXPR_NOCOPY_CLASSB(CellExtContainer, CellExtData, "CellClass");
};

class FakeCellClass : public CellClass
{
public:
	bool _SpreadTiberium(bool force);
	bool _SpreadTiberium_2(TerrainClass* pTerrain, bool force);
	void _Invalidate(AbstractClass* ptr, bool removed);
	int _GetTiberiumType();

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEDINLINE CellClass* _AsCell() const
	{
		return (CellClass*)this;
	}

	FORCEDINLINE CellExtData* _GetExtData()
	{
		return *reinterpret_cast<CellExtData**>(((DWORD)this) + AbstractExtOffset);
	}

}; 
static_assert(sizeof(FakeCellClass) == sizeof(CellClass), "Missmathc size !");
