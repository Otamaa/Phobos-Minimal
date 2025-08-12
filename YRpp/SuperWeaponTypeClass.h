/*
	SuperWeaponTypes!! =D
*/

#pragma once

#include <FileSystem.h>
#include <AbstractTypeClass.h>

//forward declarations
class BuildingTypeClass;
class ObjectClass;
class WeaponTypeClass;

class  DECLSPEC_UUID("0CF2BCE7-36E4-11D2-B8D8-006008C809ED")
	NOVTABLE SuperWeaponTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::SuperWeaponType;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F4090;
	static COMPILETIMEEVAL reference<const char* const, 0x8425C0u , (size_t)SuperWeaponType::count> const SuperweaponTypeName {};
	static COMPILETIMEEVAL reference<const char* const, 0x7E4C50u, (size_t)Action::count> const ActionTypeName {};

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SuperWeaponTypeClass*>, 0xA8E330u> const Array {};

	IMPL_Find(SuperWeaponTypeClass)

	static SuperWeaponTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_FAST(0x6CEEF0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_FAST(0x6CEE60);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6CE800);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6CE8D0);

	//Destructor
	virtual ~SuperWeaponTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//SuperWeaponTypeClass
	virtual Action MouseOverObject(CellStruct const& cell, ObjectClass* pObjBelowMouse) const RT(::Action);

	// non-virtual
	static SuperWeaponTypeClass * __fastcall FindFirstOfAction(Action Action)
		{ JMP_FAST(0x6CEEB0); }

	//Constructor
	SuperWeaponTypeClass(const char* pID) noexcept
		: SuperWeaponTypeClass(noinit_t())
	{ JMP_THIS(0x6CE5B0); }

protected:
	explicit __forceinline SuperWeaponTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int     ArrayIndex;
	WeaponTypeClass* WeaponType;

	//I believe these four are the leftover TS sounds
	int     RechargeVoice; // not read, unused
	int     ChargingVoice; // not read, unused
	int     ImpatientVoice; // not read, unused
	int     SuspendVoice; // not read, unused
	//---

	int     RechargeTime; //in frames
	SuperWeaponType Type;
	SHPStruct* SidebarImage;
	Action Action;
	int     SpecialSound;
	int     StartSound;
	BuildingTypeClass* AuxBuilding;
	char SidebarImageFile[0x18];
	PROTECTED_PROPERTY(BYTE, zero_E4);
	bool    UseChargeDrain;
	bool    IsPowered;
	bool    DisableableFromShell;
	int     FlashSidebarTabFrames;
	bool    AIDefendAgainst;
	bool    PreClick;
	bool    PostClick;
	SuperWeaponType	PreDependent;
	bool    ShowTimer;
	bool    ManualControl;
	float   Range;
	int     LineMultiplier;

};
//COMPILE_TIME_SIZEOF(SuperWeaponTypeClass)
static_assert(sizeof(SuperWeaponTypeClass) == 0x100, "Invalid Size !");