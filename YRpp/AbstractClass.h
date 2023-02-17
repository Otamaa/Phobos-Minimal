#pragma once

#include <Interface/IRTTITypeInfo.h>
#include <Interface/IDontKnow.h>
#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <ArrayClasses.h>
#include <CoordStruct.h>
#include <GameStrings.h>
#include <IndexClass.h>
#include <memory>

class CCINIClass;

enum class InitState {
	Blank = 0x0, // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2, // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3, // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4 // INI has been read and values set
};

//Extension Interface !
class IExtension {
public:
	InitState Initialized { InitState::Blank };

	IExtension() = default;
	virtual ~IExtension() = default;

	IExtension(const IExtension& other) = delete;
	void operator=(const IExtension& RHS) = delete;
};

//forward declarations
class TechnoClass;
class HouseClass;
class Checksummer;

struct StorageClass final : public ArrayWrapper<float , 4u>
{
	float GetAmount(int index) const
		{ JMP_THIS(0x6C9680); }

	float GetTotalAmount() const
		{ JMP_THIS(0x6C9650); }

	float AddAmount(float amount, int index)
		{ JMP_THIS(0x6C9690); }

	float RemoveAmount(float amount, int index)
		{ JMP_THIS(0x6C96B0); }

	int GetTotalValue() const
		{ JMP_THIS(0x6C9600); }

	int First_Used_Slot() const { JMP_THIS(0x6C9820); };

	StorageClass operator+(StorageClass& that) const { JMP_THIS(0x6C96E0); }
	StorageClass operator+=(StorageClass& that) { JMP_THIS(0x6C9740); }
	StorageClass operator-(StorageClass& that) const { JMP_THIS(0x6C9780); }
	StorageClass operator-=(StorageClass& that) { JMP_THIS(0x6C97E0); }
	
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
	static constexpr inline size_t TypeCount = 74;
	static const AbstractType AbsID = AbstractType::Abstract;
	static constexpr reference<IndexClass<int, AbstractClass*>, 0xB0E840u> const TargetIndex{};
	static constexpr constant_ptr<DynamicVectorClass<AbstractClass*>, 0xB0F720u> const Array{};
	static constexpr constant_ptr<DynamicVectorClass<AbstractClass*>, 0x00B0F698u> const Array2{};
	static constexpr reference<AbsTypeNames, 0x816EE0u, TypeCount> const RTTIToString{};

	static const char* GetAbstractClassName(AbstractType abs) {
		return RTTIToString[static_cast<int>(abs)].Name;
	}

	static void __fastcall AnnounceExpiredPointer(AbstractClass* pAbstract, bool removed = true)
	{ JMP_THIS(0x7258D0); }

	static void __fastcall RemoveAllInactive() JMP_STD(0x725C70);
	static int __fastcall GetbuildCat(AbstractType abstractID, int idx) JMP_STD(0x5004E0);

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
	virtual int __stdcall Fetch_ID() const JMP_STD(0x410220);
	virtual void __stdcall Create_ID() RX;

	//INoticeSink
	virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) R0;

	//INoticeSource
	virtual void __stdcall INoticeSource_Unknown() RX;

	//Destructor
	virtual ~AbstractClass() RX;

	//AbstractClass
	virtual void Init() RX;
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) RX;
	virtual AbstractType WhatAmI() const RX;
	virtual int Size() const RX;
	virtual void CalculateChecksum(Checksummer& checksum) const RX;
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

	CoordStruct GetCoords() const {
		CoordStruct ret;
		this->GetCoords(&ret);
		return ret;
	}

	CoordStruct GetDestination(TechnoClass* pDocker = nullptr) const {
		CoordStruct ret;
		this->GetDestination(&ret, pDocker);
		return ret;
	}
	
	CoordStruct GetCenterCoords() const {
		CoordStruct ret;
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

	void Compute_CRC_Impl(Checksummer&) const {
		JMP_THIS(0x410410);
	}

	void CreateID() const {
		JMP_THIS(0x410230);
	}

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
	int unknown_18;
	LONG RefCount;
	bool Dirty;		// for IPersistStream.
	PROTECTED_PROPERTY(BYTE, padding_21[0x3]);
private:
	// Copy and assignment not implemented; prevent their use by declaring as private.
	AbstractClass(const AbstractClass&) = delete;
	AbstractClass& operator=(const AbstractClass&) = delete;
};
static_assert(sizeof(AbstractClass) == 0x24, "Invalid size.");
//typedef AbstractClass* TARGET;
//#pragma pack(pop)