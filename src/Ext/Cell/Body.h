#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/SavegameDef.h>

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

class CellExtData final
{
public:

	static COMPILETIMEEVAL size_t Canary = 0x87688621;
	using base_type = CellClass;

	static COMPILETIMEEVAL size_t ExtOffset = 0x144;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	int NewPowerups { -1 };
	UnitClass* IncomingUnit { nullptr };
	UnitClass* IncomingUnitAlt { nullptr };
	HelperedVector<RadSiteClass*> RadSites {};
	HelperedVector<RadLevel> RadLevels {};

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
	static HelperedVector<CellExtData*> Array;

	CellExtData* AllocateUnchecked(CellClass* key)
	{
		auto val = Array.emplace_back(new CellExtData());
		val->AttachedToObject = key;
		return val;

	}

	CellExtData* Allocate(CellClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame || key == CellClass::Instance())
			return nullptr;

		this->ClearExtAttribute(key);
		CellExtData* val = AllocateUnchecked(key);
		this->SetExtAttribute(key, val);
		return val;
	}

	void Remove(CellClass* key)
	{
		if (Phobos::Otamaa::DoingLoadGame || key == CellClass::Instance())
			return;

		if (CellExtData* Item = TryFind(key)) {
			Array.remove(Item);
			this->ClearExtAttribute(key);
			delete Item;
		}
	}

	void Clear()
	{
		for (auto& item : Array) {
			if (item)
				delete item;
		}

		Array.clear();
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
