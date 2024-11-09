/*
	Aircraft
*/

#pragma once

#include <FootClass.h>
#include <AircraftTypeClass.h>
#include <Interface/IFlyControl.h>
//forward declarations

#include <comip.h>
#include <comdef.h>

//_COM_SMARTPTR_TYPEDEF(IFlyControl, __uuidof(IFlyControl));

//AircraftClass
class DECLSPEC_UUID("0E272DC2-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE AircraftClass : public FootClass, public IFlyControl

{
public:
	//Static
	static const AbstractType AbsID = AbstractType::Aircraft;
	static constexpr constant_ptr<DynamicVectorClass<AircraftClass*>, 0xA8E390u> const Array{};
	static constexpr inline DWORD vtable = 0x7E22A4;

	static void __fastcall Read_INI(CCINIClass& ini);// JMP_STD(0x41B110);
	static void __fastcall Write_INI(CCINIClass& ini);// JMP_STD(0x41AF80);

	static void AircraftTracker_4134A0(AircraftClass* pThis);//
	//{
	//	EPILOG_THISCALL;
	//	PUSH_IMM(pThis);
	//	THISCALL_EX(0x887888, 0x4134A0);
	//}

	//IFlyControl
	virtual LONG __stdcall Landing_Altitude() override;//  JMP_STD(0x41B6A0);
	virtual LONG __stdcall Landing_Direction() override;//  JMP_STD(0x41B760);
	virtual BOOL __stdcall Is_Loaded() override;//  JMP_STD(0x41B7D0);
	virtual LONG __stdcall Is_Strafe() override;//  JMP_STD(0x41B7F0);
	virtual LONG __stdcall Is_Fighter() override;//  JMP_STD(0x41B840);
	virtual LONG __stdcall Is_Locked() override;//  JMP_STD(0x41B860);

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) override;// JMP_STD(0x414290);
	virtual ULONG __stdcall AddRef() override;//  JMP_STD(0x4142F0);
	virtual ULONG __stdcall Release() override;//  JMP_STD(0x414300);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override;//  JMP_STD(0x41C190);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override;//  JMP_THIS(0x41B430);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override;// JMP_THIS(0x41B5C0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	Size() const override R0;
	virtual void Update() override;// JMP_THIS(0x414BB0);

	//ObjectClass
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
	ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override;//  JMP_THIS(0x4165C0);

	//MisionClass
	virtual void Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) override;// JMP_THIS(0x41BB30);

	//TechnoClass
	virtual BulletClass* Fire(AbstractClass* pTarget, int nWeaponIndex);// JMP_THIS(0x415EE0);

	//FootClass
	virtual TechnoClass* FindDockingBayInVector(DynamicVectorClass<TechnoTypeClass*>* pVec, int unusedarg3, bool bForced)  const override;// JMP_THIS(0x41BBD0);

	//Destructor
	virtual ~AircraftClass() RX;

	//some stuffs here may from FootClass::vtable , which is missing
	CellClass* GoodLandingZone_() const;//  { JMP_THIS(0x41A160); }
	CellClass* NewLandingZone_(AbstractClass* pOldCell) const;//  { JMP_THIS(0x418E20); }
	AbstractClass* GoodTargetLoc_(AbstractClass* pTarget) const;//  { JMP_THIS(0x4197C0); }
	bool CellSeemsOk_(const CellStruct& nCell, bool bStrich);//  { JMP_THIS(0x419B00); }

	void DropOffCarryAllCargo() const;//  { JMP_THIS(0x416AF0); }
	void DropOffParadropCargo() const;// { JMP_THIS(0x415C60); }

	void Tracker_4134A0() { AircraftTracker_4134A0(this); }

	int GetLandDir() const {
		return RulesClass::Instance->PoseDir;
	}

	int Mission_Move_Carryall() const;//  { JMP_THIS(0x416D50); }

	//Constructor
	AircraftClass(AircraftTypeClass* pType, HouseClass* pOwner);

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	AircraftTypeClass* Type;
	bool loseammo_6c8; //loseAmmo
	bool HasPassengers;	//parachutes
	bool IsKamikaze; // when crashing down, duh
	TechnoClass* DockedTo;
	bool unknown_bool_6D0;
	bool unknown_bool_6D1;
	bool IsLocked; // Whether or not aircraft is locked to a firing run (strafing)
	char NumParadropsLeft;
	bool IsCarryallNotLanding;
	bool IsReturningFromAttackRun; // Aircraft finished attack run and/or went idle and is now returning from it
};

static_assert(sizeof(AircraftClass) == 0x6D8, "Invalid size.");