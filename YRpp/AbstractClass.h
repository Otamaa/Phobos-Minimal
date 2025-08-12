#pragma once
#include <Helpers/VTable.h>

//#define GetVtableAddr(var) (VTable::Get(var))
//
//#define Is_Aircraft(var) (GetVtableAddr(var) == AircraftClass::vtable)
//#define Is_Infantry(var) (GetVtableAddr(var) == InfantryClass::vtable)
//#define Is_Building(var) (GetVtableAddr(var) == BuildingClass::vtable)
//#define Is_Unit(var) (GetVtableAddr(var) == UnitClass::vtable)
//
//#define Is_Foot(var) (Is_Aircraft(var) || Is_Infantry(var) || Is_Unit(var))
//#define Is_Techno(var) (Is_Foot(var) || Is_Building(var))
//#define Is_House(var) (GetVtableAddr(var) == HouseClass::vtable)
//#define Is_HouseType(var) (GetVtableAddr(var) == HouseTypeClass::vtable)
//
//#define Is_Terrain(var) (GetVtableAddr(var) == TerrainClass::vtable)
//#define Is_TerrainType(var) (GetVtableAddr(var) == TerrainTypeClass::vtable)
//
//#define Is_Bullet(var) (GetVtableAddr(var) == BulletClass::vtable)
//#define Is_BulletType(var) (GetVtableAddr(var) == BulletTypeClass::vtable)
//
//#define Is_Cell(var) (GetVtableAddr(var) == CellClass::vtable)
//#define Is_cell(var) (GetVtableAddr(var) == CellClass::vtable)
//
//#define Is_SW(var) (GetVtableAddr(var) == SuperClass::vtable)
//#define Is_SWType(var) (GetVtableAddr(var) == SuperWeaponTypeClass::vtable)
//
//#define Is_Anim(var) (GetVtableAddr(var) == AnimClass::vtable)
//#define Is_AnimType(var) (GetVtableAddr(var) == AnimTypeClass::vtable)
//
//#define Is_AircraftType(var) (GetVtableAddr(var) == AircraftTypeClass::vtable)
//#define Is_InfantryType(var) (GetVtableAddr(var) == InfantryTypeClass::vtable)
//#define Is_BuildingType(var) (GetVtableAddr(var) == BuildingTypeClass::vtable)
//#define Is_UnitType(var) (GetVtableAddr(var) == UnitTypeClass::vtable)
//
//#define Is_TechnoType(var) (Is_AircraftType(var) || Is_InfantryType(var) || Is_BuildingType(var) || Is_UnitType(var))


#include <Interface/IRTTITypeInfo.h>
#include <Interface/IDontKnow.h>
#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <ArrayClasses.h>
#include <CoordStruct.h>
#include <GameStrings.h>
#include <IndexClass.h>
#include <memory>
#include <CRC.h>

class CCINIClass;

//forward declarations
class TechnoClass;
class HouseClass;
class CRCEngine;

struct StorageClass final
{
	static COMPILETIMEEVAL OPTIONALINLINE size_t  Count = 0x4u;

	double GetStoragePercentage(int total) const
	{
		return (double)this->GetTotalAmount() / (double)total;
	}

	float GetAmount(int index) const
	{
		JMP_THIS(0x6C9680);
		//return this->Tiberiums[index];
	}

	float GetTotalAmount() const
	{
		JMP_THIS(0x6C9650);
		//float amounttotal = 0.0f;
		//
		//for(size_t i = 0; i < Count; ++i)
		//	amounttotal += this->Tiberiums[i];
		//
		//return amounttotal;
	}

	float AddAmount(float amount, int index)
	{
		JMP_THIS(0x6C9690);
	   //float result = amount + this->Tiberiums[index];
   	   //this->Tiberiums[index] = result;
	   //return result;
	}

	float RemoveAmount(float amount, int index)
	{
		JMP_THIS(0x6C96B0);
		//float result = 0.0f;
		//float v4 = 0.0f;

	    //if ( this->Tiberiums[index] >= amount )
   		//{
     //   	result = amount;
     //   	v4 = this->Tiberiums[index] - amount;
    	//}
    	//else
    	//{
     //   	result = this->Tiberiums[index];
     //   	v4 = result - result;
    	//}

    	//this->Tiberiums[index] = v4;
    	//return result;
	}

	int GetTotalValue() const
		{ JMP_THIS(0x6C9600); }

	int First_Used_Slot() const { JMP_THIS(0x6C9820); };

	int GetHighestStorageIdx() const
	{
		int nIdx = 0;
		for (int p = 0; p < Count; p++)
			nIdx += (this->Tiberiums[nIdx] < this->Tiberiums[p]) * (p - nIdx);

		return nIdx;
	}

	int GEtFirstUsedSlot() const {
		JMP_THIS(0x6C9820);
	}

	StorageClass operator+(StorageClass& that) const { JMP_THIS(0x6C96E0); }
	StorageClass operator+=(StorageClass& that) { JMP_THIS(0x6C9740); }
	StorageClass operator-(StorageClass& that) const { JMP_THIS(0x6C9780); }
	StorageClass operator-=(StorageClass& that) { JMP_THIS(0x6C97E0); }

	float Tiberiums[4];
};
static_assert(sizeof(StorageClass) == 0x10, "Invalid Size !");
//---

typedef NamedValue<AbstractType> AbsTypeNames;
static_assert(sizeof(AbsTypeNames) == 0x8, "Invalid Size !");

//#pragma pack(push, 4)
//The AbstractClass is the base class of all game objects.
class NOVTABLE AbstractClass : public IPersistStream, public IRTTITypeInfo, public INoticeSink, public INoticeSource
{
public:
	//static
	static COMPILETIMEEVAL OPTIONALINLINE size_t TypeCount = 74;
	static const AbstractType AbsID = AbstractType::Abstract;
	static COMPILETIMEEVAL reference<IndexClass<int, AbstractClass*>, 0xB0E840u> const TargetIndex{};
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<AbstractClass*>, 0xB0F720u> const Array{};
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<AbstractClass*>, 0x00B0F698u> const Array2{};
	static COMPILETIMEEVAL reference<AbsTypeNames, 0x816EE0u, TypeCount> const RTTIToString{};

	static const char* GetAbstractClassName(AbstractType abs) {
		return RTTIToString[static_cast<int>(abs)].Name;
	}

	static void __fastcall AnnounceExpiredPointer(AbstractClass* pAbstract, bool removed = true)
	{ JMP_THIS(0x7258D0); }

	static void __fastcall RemoveAllInactive() JMP_FAST(0x725C70);
	static int __fastcall GetbuildCat(AbstractType abstractID, int idx) JMP_FAST(0x5004E0);

	const char* GetThisClassName() const
	{
		return AbstractClass::GetAbstractClassName(this->WhatAmI());
	}

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() R0;

	//Pure virtual , Only for inheritance
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) R0;

	//IRTTITypeInfo
	virtual AbstractType __stdcall What_Am_I() const RT(AbstractType);
	virtual int __stdcall Fetch_ID() const R0;
	virtual void __stdcall Create_ID() RX;

	//INoticeSink
	virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) R0;

	//INoticeSource
	virtual void __stdcall INoticeSource_Unknown() RX;

	//Destructor
	virtual ~AbstractClass() RX;

	//AbstractClass
	virtual void Init() RX;
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) RX;
	virtual AbstractType WhatAmI() const RX;
	virtual int Size() const RX;
	virtual void ComputeCRC(CRCEngine& checksum) const RX;
	virtual int GetOwningHouseIndex() const R0;
	virtual HouseClass* GetOwningHouse() const R0;
	virtual int GetArrayIndex() const R0;
	virtual bool IsDead() const R0;
	virtual CoordStruct* GetCoords(CoordStruct* pCrd) const R0; //center coords
	virtual CoordStruct* GetDestination(CoordStruct* pCrd, TechnoClass* pDocker = nullptr) const R0; // where this is moving, or a building's dock for a techno. iow, a rendez-vous point
	virtual bool IsOnFloor() const R0;
	virtual bool IsInAir() const R0;
	virtual CoordStruct* GetCenterCoords(CoordStruct* pCrd) const R0; //GetCoords__ / __Get_Coords__As_Coord
	virtual void Update() RX;


	void AnnounceExpiredPointer(bool removed = true) {
		AnnounceExpiredPointer(this, removed);
	}

	CoordStruct FORCEDINLINE GetCoords() const {
		CoordStruct ret {};
		this->GetCoords(&ret);
		return ret;
	}

	CoordStruct FORCEDINLINE GetDestination(TechnoClass* pDocker = nullptr) const {
		CoordStruct ret {};
		this->GetDestination(&ret, pDocker);
		return ret;
	}

	CoordStruct FORCEDINLINE GetCenterCoords() const {
		CoordStruct ret {};
		this->GetCenterCoords(&ret);
		return ret;
	}

	//Operators
	bool operator < (const AbstractClass& rhs) const {
		return this->UniqueID < rhs.UniqueID;
	}

	bool IsSameWith(const AbstractClass& rhs) const {
		JMP_THIS(0x588C10);
	}

	TechnoClass* AsTechno() const
		{ JMP_THIS(0x40DD70); }

	static HRESULT STDMETHODCALLTYPE _Load(AbstractClass* pAbs, IStream* pStm) {
		JMP_STD(0x410380);
	}

	static HRESULT STDMETHODCALLTYPE _Save(AbstractClass* pAbs, IStream* pStm, BOOL fClearDirty) {
		JMP_STD(0x410320);
	}

	void Compute_CRC_Impl(CRCEngine*) const {
		JMP_THIS(0x410410);
	}

	void CreateID() const {
		JMP_THIS(0x410230);
	}

	int DistanceFrom(AbstractClass *that) const
		{ JMP_THIS(0x5F6440); }

	int DistanceFromSquared(AbstractClass* pThat)const
		{ JMP_THIS(0x5F6360); }

	int DistanceFromSquared(const CoordStruct* pThat)const
		{ JMP_THIS(0x5F6560); }

	//Constructor
	AbstractClass() noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x410170); }

protected:
	explicit __forceinline AbstractClass(noinit_t) noexcept
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD UniqueID; // generated by IRTTIInfo::Create_ID through an amazingly simple sequence of return ++ScenarioClass::Instance->UniqueID;
	AbstractFlags AbstractFlags;	// flags, see AbstractFlags enum in GeneralDefinitions.
	DWORD unknown_18;
	LONG RefCount;
	bool Dirty;		// for IPersistStream.
	PROTECTED_PROPERTY(BYTE, padding_21[0x3]);
};
static_assert(sizeof(AbstractClass) == 0x24, "Invalid size.");
//typedef AbstractClass* TARGET;
//#pragma pack(pop)

template <class T>
concept HasDeriveredAbsID = requires(T) { T::AbsDerivateID; };