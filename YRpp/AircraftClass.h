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

_COM_SMARTPTR_TYPEDEF(IFlyControl, __uuidof(IFlyControl));

//AircraftClass
class DECLSPEC_UUID("0E272DC2-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE AircraftClass : public FootClass, public IFlyControl

{
public:
	//Static
	static const AbstractType AbsID = AbstractType::Aircraft;
	static constexpr constant_ptr<DynamicVectorClass<AircraftClass*>, 0xA8E390u> const Array{};
	static void __fastcall Read_INI(CCINIClass& ini) JMP_STD(0x41B110);
	static void __fastcall Write_INI(CCINIClass& ini) JMP_STD(0x41AF80);

	static void AircraftTracker_4134A0(AircraftClass* pThis)
	{
		EPILOG_THISCALL;
		PUSH_IMM(pThis);
		THISCALL_EX(0x887888, 0x4134A0);
	}

	//IFlyControl
	IFACEMETHOD_(LONG, Landing_Altitude()) JMP_STD(0x41B6A0); //
	IFACEMETHOD_(LONG, Landing_Direction()) JMP_STD(0x41B760); //
	IFACEMETHOD_(BOOL, Is_Loaded()) JMP_STD(0x41B7D0); //
	IFACEMETHOD_(LONG, Is_Strafe()) JMP_STD(0x41B7F0); //
	IFACEMETHOD_(LONG, Is_Fighter()) JMP_STD(0x41B840); //
	IFACEMETHOD_(LONG, Is_Locked()) JMP_STD(0x41B860); //

	//IUnknown
	IFACEMETHOD_(HRESULT,QueryInterface(REFIID iid, LPVOID* ppvObject)) JMP_STD(0x414290);
	IFACEMETHOD_(ULONG,AddRef()) JMP_STD(0x4142F0);
	IFACEMETHOD_(ULONG,Release()) JMP_STD(0x414300);

	//IPersist
	IFACEMETHOD_(HRESULT,GetClassID(CLSID* pClassID)) JMP_STD(0x41C190);

	//IPersistStream
	IFACEMETHOD_(HRESULT,Load(IStream* pStm)) JMP_STD(0x41B430);
	IFACEMETHOD_(HRESULT,Save(IStream* pStm, BOOL fClearDirty)) JMP_STD(0x41B5C0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x41C180);
	virtual int	Size() const override JMP_THIS(0x41C170);
	virtual void Update() override JMP_THIS(0x414BB0);

	//MisionClass
	virtual void Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) override JMP_THIS(0x41BB30);

	//TechnoClass
	virtual BulletClass* Fire(AbstractClass* pTarget, int nWeaponIndex) JMP_THIS(0x415EE0);

	//FootClass
	virtual TechnoClass* FindDockingBayInVector(DynamicVectorClass<TechnoTypeClass*>* pVec, int unusedarg3, bool bForced)  const override JMP_THIS(0x41BBD0);

	//Destructor
	virtual ~AircraftClass() RX;

	//some stuffs here may from FootClass::vtable , which is missing
	CellClass* GoodLandingZone() const { JMP_THIS(0x41A160); }
	CellClass* NewLandingZone(AbstractClass* pOldCell) { JMP_THIS(0x418E20); }
	AbstractClass* GoodTargetLoc(AbstractClass* pTarget) const { JMP_THIS(0x4197C0); }
	bool CellSeemsOk(CellStruct& nCell, bool bStrich) { JMP_THIS(0x419B00); }

	void DropOffCarryAllCargo() const { JMP_THIS(0x416AF0); }
	void DropOffParadropCargo() const { JMP_THIS(0x415C60); }

	void Tracker_4134A0() { AircraftTracker_4134A0(this); }

	int GetLandDir() const {
		return RulesClass::Instance->PoseDir;
	}

	//Constructor
	AircraftClass(AircraftTypeClass* pType, HouseClass* pOwner) noexcept
		: AircraftClass(noinit_t())
	{ JMP_THIS(0x413D20); }


protected:
	explicit __forceinline AircraftClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{
		//AircraftClass_vftable
		//*((unsigned long*)this) = (unsigned long)0x7E22A4;
		//`IRTTITypeInfo'
		//*((unsigned long*)this + 1) = (unsigned long)0x7E2288;
		// `INoticeSink'
		//*((unsigned long*)this + 2) = (unsigned long)0x7E2280;
		// `INoticeSource'
		//*((unsigned long*)this + 3) = (unsigned long)0x7E2278;
		//IFlyControl
		//*((unsigned long*)this + 432) = (unsigned long)0x7E2250;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	AircraftTypeClass* Type;
	bool unknown_bool_6C8; //loseAmmo
	bool HasPassengers;	//parachutes
	bool IsKamikaze; // when crashing down, duh
	TechnoClass* DockedTo;
	bool unknown_bool_6D0;
	bool unknown_bool_6D1;
	bool __DoingOverfly;
	BYTE ___paradrop_attempts; //6D3
	bool carrayall6D4;
	bool retreating_idle; //6D5
};

static_assert(sizeof(AircraftClass) == 0x6D8, "Invalid size.");