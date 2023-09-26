/*
	AnimTypes are initialized by INI files.
*/

#pragma once

#include <TechnoTypeClass.h>

struct DoStruct
{
	BYTE Interrupt;	// Can it be interrupted?
	BYTE IsMobile;		// Can it move while doing this?
	BYTE RandomStart;	// Should animation be "randomized"?
	BYTE Rate;		// Frame rate.
};

//static_assert(sizeof(DoStruct) == 0x4);
struct DoInfoSound
{
	int StartFrame { 0 };
	int Index { -1 };
};

struct DoInfoStruct
{
	int StartFrame { 0 }; //8
	int CountFrames { 0 }; //12
	int FacingMultiplier { 0 }; //16
	DoTypeFacing Facing { DoTypeFacing::None }; //20
	int SoundCount { 0 }; //24
	DoInfoSound SoundData[2] {};
};

//static_assert(sizeof(DoInfoStruct) == 0x24);
//static_assert(sizeof(DoType) == 0x4);

struct DoControls
{
	constexpr void __forceinline Initialize()
	{
		for (auto at = this->begin(); at != this->end(); ++at)
		{
			at->StartFrame = 0;
			at->CountFrames = 0;
			at->FacingMultiplier = 0;
			at->Facing = DoTypeFacing::None;
			at->SoundCount = 0;
			at->SoundData[0].Index = -1;
			at->SoundData[0].StartFrame = 0;
			at->SoundData[1].Index = -1;
			at->SoundData[1].StartFrame = 0;
		}
	}

	static inline constexpr int MaxCount = 42;
	static constexpr reference<DoStruct, 0x7EAF7Cu, MaxCount> const MasterArray { };
	static constexpr reference<const char*, 0x8255C8u, MaxCount> const DoType_toStr { };

	static const char* GetSequenceName(DoType sequence) {
		return DoType_toStr[(int)sequence];
	}

	DoInfoStruct GetSequence(DoType sequence) const {
		return this->Data[(int)sequence];
	}

	DoInfoStruct& GetSequence(DoType sequence) {
		return this->Data[(int)sequence];
	}

	static DoStruct* GetSequenceData(DoType sequence) {
		return MasterArray.begin() + (int)sequence;
	}

	constexpr DoInfoStruct* begin() { return std::begin(Data); }
	constexpr DoInfoStruct* end() { return std::end(Data); }

	DoInfoStruct Data[MaxCount];
};

static_assert(sizeof(DoControls) == (DoControls::MaxCount * sizeof(DoInfoStruct)), "Invalid Size !");

class DECLSPEC_UUID("AE8B33D8-061C-11D2-ACA4-006008055BB5")
	NOVTABLE InfantryTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::InfantryType;
	static constexpr inline DWORD vtable = 0x7EB610;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<InfantryTypeClass*>, 0xA8E348u> const Array {};

	IMPL_Find(InfantryTypeClass)

	static InfantryTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x524CB0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x523C90);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x524C70);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x524960);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x524B60);

	//Destructor
	virtual ~InfantryTypeClass() override JMP_THIS(0x524D70);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	Size() const R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x5240A0);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override  R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override  R0;

	void ReadSequence() const {
		JMP_THIS(0x523D00);
	}

	//Constructor
	InfantryTypeClass(const char* pID) noexcept
		: InfantryTypeClass(noinit_t())
	{ JMP_THIS(0x5236A0); }

	InfantryTypeClass(IStream* pStm) noexcept
		: InfantryTypeClass(noinit_t())
	{ JMP_THIS(0x523980); }

protected:
	explicit __forceinline InfantryTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	PipIndex Pip;
	PipIndex OccupyPip;
	WeaponStruct OccupyWeapon;
	WeaponStruct EliteOccupyWeapon;
	DoControls* Sequence;
	int FireUp;
	int FireProne;
	int SecondaryFire;
	int SecondaryProne;
	TypeList<AnimTypeClass*> DeadBodies;
	TypeList<AnimTypeClass*> DeathAnims;
	TypeList<int> VoiceComment;
	int EnterWaterSound;
	int LeaveWaterSound;
	bool Cyborg;
	bool NotHuman;
	bool Ivan; //used for the bomb attack cursor...
	int DirectionDistance;
	bool Occupier;
	bool Assaulter;
	int HarvestRate;
	bool Fearless;
	bool Crawls;
	bool Infiltrate;
	bool Fraidycat;
	bool TiberiumProof;
	bool Civilian;
	bool C4;
	bool Engineer;
	bool Agent;
	bool Thief;
	bool VehicleThief;
	bool Doggie;
	bool Deployer;
	bool DeployedCrushable;
	bool UseOwnName;
	bool JumpJetTurn;
	PRIVATE_PROPERTY(DWORD, align_ECC);
};

static_assert(sizeof(InfantryTypeClass) == 0xED0);
