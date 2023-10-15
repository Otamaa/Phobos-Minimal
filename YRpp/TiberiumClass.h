/*
	Tiberiums are initialized by INI files.
*/

#pragma once
#include <ArrayClasses.h>
#include <AbstractTypeClass.h>
#include <HeapClass.h>
#include <PriorityQueueClass.h>

struct MapSurfaceData
{
	static int __fastcall SurfaceDataCount() JMP_STD(0x42B1F0);
	static int __fastcall ToSurfaceIndex(const CellStruct& mapCoord) JMP_STD(0x42B1C0);

	int ToSurfaceIndex()
	{
		return ToSurfaceIndex(MapCoord);
	}

	CellStruct MapCoord;
	float Score;

	bool operator<(const MapSurfaceData& another) const
	{
		return Score < another.Score;
	}
};
static_assert(sizeof(MapSurfaceData) == 0x8);

class TiberiumLogic
{
public:
	void Construct(int nCount = MapSurfaceData::SurfaceDataCount())
	{
		Datas = (MapSurfaceData*)YRMemory::Allocate(sizeof(MapSurfaceData) * nCount);
		States = (bool*)YRMemory::Allocate(sizeof(bool) * nCount);
		std::memset(States, 0, sizeof(bool) * nCount);
		Heap = GameCreate<TPriorityQueueClass<MapSurfaceData>>(nCount);
	}

	void Decontruct()
	{
		GameDelete<true , true>(Heap);
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
	TPriorityQueueClass<MapSurfaceData>* Heap;
	bool* States;
	MapSurfaceData* Datas;
	CDTimerClass Timer;

private:
	TiberiumLogic(const TiberiumLogic&) = default;
	TiberiumLogic(TiberiumLogic&&) = default;
	TiberiumLogic&operator=(const TiberiumLogic& other) = default;
};
//static_assert(sizeof(TiberiumLogic) == 0x14, "Invalid Size !");

//forward declarations
class AnimTypeClass;
class OverlayTypeClass;

class DECLSPEC_UUID("C53DD373-151E-11D2-8175-006008055BB5")
	NOVTABLE TiberiumClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::Tiberium;
	static constexpr inline DWORD vtable = 0x7F5728;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<TiberiumClass*>, 0xB0F4E8u> const Array {};

	IMPL_Find(TiberiumClass)
	IMPL_FindOrAllocate(TiberiumClass)
	IMPL_FindIndexById(TiberiumClass)

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x721E40);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x721E80);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x7220D0);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override JMP_STD(0x7220A0);

	//Destructor
	virtual ~TiberiumClass() override JMP_THIS(0x723710);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x722140);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	//AbstactTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x721A50);

	//TiberiumClass

	void GrowthTiberium() JMP_THIS(0x722440);
	void RegisterForGrowth(CellStruct* cell)
		{ JMP_THIS(0x7235A0); }

	int sub_722AF0(CellStruct& mapcoords) JMP_THIS(0x722AF0);
	void sub_722F00() JMP_THIS(0x722F00);
	//static
	static int InitArray() JMP_STD(0x721676);
	static void __cdecl UninitArray() JMP_STD(0x721680);
	static bool __fastcall sub_721D10(CCINIClass* pINI) JMP_STD(0x721D10);
	static void __stdcall UpdateTiberium() JMP_STD(0x7221B0);
	static void __stdcall sub_0x722240() JMP_STD(0x722240);
	static void __stdcall sub_0x722390() JMP_STD(0x722390);
	static void __fastcall sub_722AB0(CellStruct& mapcoords) JMP_STD(0x722AB0);
	static void __stdcall UpdateGrowth() JMP_STD(0x722C40);
	static void __stdcall sub_722D00() JMP_STD(0x722D00);
	static void __stdcall sub_722E50() JMP_STD(0x722E50);

	static int __fastcall FindIndex(int idxOverlayType) {
		JMP_STD(0x5FDD20);
	}

	static TiberiumClass* Find(int idxOverlayType) {
		int idx = FindIndex(idxOverlayType);
		return Array->GetItemOrDefault(idx);
	}

	//Constructor
	TiberiumClass(const char* pID)
		: TiberiumClass(noinit_t())
	{ JMP_THIS(0x7216C0); }

protected:
	explicit __forceinline TiberiumClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int Spread;
	double SpreadPercentage;
	int Growth;
	DWORD field_AC;  //unused , can be used to store ExtData
	double GrowthPercentage;
	int Value;
	int Power;
	int Color;
	DECLARE_PROPERTY(TypeList<AnimTypeClass*>, Debris);
	OverlayTypeClass* Image;
	int NumFrames;
	int NumImages;
	int field_EC;
	DECLARE_PROPERTY(TiberiumLogic, SpreadLogic);
	DECLARE_PROPERTY(TiberiumLogic, GrowthLogic);
};

static_assert(sizeof(TiberiumClass) == 0x128, "Invalid Size !");