#pragma once
#include <ObjectClass.h>
#include <TiberiumClass.h>
#include <RulesClass.h>
#include <CRT.h>

class VeinholeLogic
{
public:

	//74DFAF
	void Construct(int nCount = RulesClass::Instance->MaxVeinholeGrowth)
	{
		Datas = (MapSurfaceData*)YRMemory::Allocate(sizeof(MapSurfaceData) * nCount);
		States = (bool*)YRMemory::Allocate(sizeof(bool) * nCount);
		std::memset(States, 0, sizeof(bool) * nCount);
		Heap = GameCreate<PointerHeapClass<MapSurfaceData>>(nCount);
	}

	//74E8A0
	void Deconstruct()
	{
		GameDelete<true, true>(Heap);
		Heap = nullptr;

		if (Datas)
		{
			YRMemory::Deallocate(Datas);
			Datas = nullptr;
		}

		if (States)
		{
			YRMemory::Deallocate(States);
			States = nullptr;
		}
		Count = 0;
	}

	int Count;
	PointerHeapClass<MapSurfaceData>* Heap;
	MapSurfaceData* Datas;
	CDTimerClass Timer;
	bool* States;

private:
	VeinholeLogic(const VeinholeLogic&) = default;
	VeinholeLogic(VeinholeLogic&&) = default;
	VeinholeLogic& operator=(const VeinholeLogic& other) = default;

};

class DECLSPEC_UUID("5192D06A-C632-11D2-B90B-006008C809ED")
	NOVTABLE VeinholeMonsterClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::VeinholeMonster;

	static constexpr reference<bool*, 0xB1D2F0u> const IsCurrentPosAffected {};
	static constexpr reference<SHPFrame*, 0xB1D2ECu> const VeinSHPData {};
	static constexpr constant_ptr<DynamicVectorClass<VeinholeMonsterClass*>, 0xB1D290u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_THIS(0x74F2D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0; //none
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_THIS(0x74EEE0);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override R0;

	//Destructor
	virtual ~VeinholeMonsterClass() JMP_THIS(0x74C9F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x74F310);
	virtual int Size() const override JMP_THIS(0x74F320);
	virtual void Update() override JMP_THIS(0x74CE50);

	//ObjectClass
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x74D490); //114
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
  ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override JMP_THIS(0x74D5D0);

	static VeinholeMonsterClass* __fastcall GetVeinholeMonsterAt(CellStruct* pCell)
	{ JMP_STD(0x74CDB0); }

	static VeinholeMonsterClass* __fastcall GetVeinholeMonsterFrom(CellStruct* pCell)
	{ JMP_STD(0x74CD60); }

	void RemoveFrom(CellClass* pCell) const
	{ JMP_THIS(0x74EF10); }

	void ClearVector() const
	{ JMP_THIS(0x74EA30); }

	void ClearGrowthData()
	{ GrowthLogic.Deconstruct(); }

	void Recalculate() const
	{ JMP_THIS(0x74E930); }

	void RecalculateSpread() const
	{ JMP_THIS(0x74E6B0); }

	void Func_74E1C0_RecalculateCellVector() const
	{ JMP_THIS(0x74E1C0); }

	void Func_74DC00() const
	{ JMP_THIS(0x74DC00); }

	void UpdateGrowth() const
	{ JMP_THIS(0x74D7C0); }

	void RegisterAffectedCells() const
	{ JMP_THIS(0x74E1C0); }

	static void __fastcall ClearVeinGrowthData()
	{  JMP_STD(0x74E100); }

	//called 687A80
	static void __fastcall InitVeinGrowhData(bool bAllocate = true)
	{ JMP_STD(0x74DE90); }

	static bool __fastcall IsCellEligibleForVeinHole(CellStruct& nWhere)
	{ JMP_STD(0x74D670); }

	static bool __fastcall IsCellEligibleForVeinHole(CellStruct* pWhere)
	{ JMP_STD(0x74D670); }

	static void __fastcall TheaterInit(TheaterType nType)
	{ JMP_STD(0x74D450); }

	static TerrainTypeClass* __fastcall GetTerrainType()
	{ JMP_STD(0x74EF00); }

	static HRESULT __fastcall SaveVector(void* stream, DynamicVectorClass<VeinholeMonsterClass*>* a2)
	{ JMP_STD(0x74ED60); }

	static HRESULT __fastcall LoadVector(LPSTREAM a1)
	{ JMP_STD(0x74EA70); }

	static void __fastcall DestroyAll()
	{ JMP_STD(0x74EA30); }

	static void __fastcall DrawAll()
	{ JMP_STD(0x74D430); }

	static void __fastcall DeleteAll()
	{ JMP_STD(0x74D760); }

	static void __fastcall DeleteVeinholeGrowthData()
	{ JMP_STD(0x74E880); }

	static void __fastcall LoadVeinholeArt(int idxTheatre)
	{ JMP_STD(0x74D450); }

	static void __cdecl UpdateAllVeinholes()
	{ JMP_STD(0x74CDF0); }

	VeinholeMonsterClass(CellStruct* pWhere) noexcept
		: VeinholeMonsterClass(noinit_t())
	{ JMP_THIS(0x74C5B0); }

protected:
	explicit __forceinline VeinholeMonsterClass(noinit_t) noexcept
		: ObjectClass(noinit_t()) {}
public:

	DECLARE_PROPERTY(VeinholeLogic, GrowthLogic);
	int CurrentState;
	int NextState;
	int  MonsterFrameIdx;
	bool  IsAnimationUpToDate;
	CDTimerClass UpdateAnimationFrameTimer;
	int AnimationUpdatePeriod;
	int MonsterFrameIdxChange;
	CDTimerClass UpdateStateTimer;
	CellStruct MonsterCell;
	int ShapeFrame;
	bool SkipDraw;
	bool ToPuffGas;
	int VeinCount;
};

static_assert(sizeof(VeinholeMonsterClass) == 0x108, "Invalid size."); //264
