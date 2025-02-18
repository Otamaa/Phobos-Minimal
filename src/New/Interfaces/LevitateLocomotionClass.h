#pragma once

#include "Base.h"
#include <FootClass.h>
#include <VocClass.h>
#include <DebugLog.h>
#include <MapClass.h>

#include <Interface/IPiggyback.h>

struct LevitateCharacteristics
{

	double Drag { 0.05 }; // rate that jellyfish slows down
	// max velocity that jellyfish can move again when...
	double Vel_Max_Happy { 4.0 }; //   ...just puttering around
	double Vel_Max_WhenFollow { 5.0 }; //	...going someplace in particular
	double Vel_Max_WhenPissedOff { 10.5 }; //	...tracking down some mofo
	double Accel { 0.5 }; // How much a puff accelerates
	double Accel_Prob { 0.01 }; // Chance happy jellyfish will "puff"
	int Accel_Dur { 20 }; // How long a puff accelerates the jellyfish
	double Initial_Boost { 1.5 }; // How much of an initial speed boost does jellyfish get when puffing
	//BounceVelocity = 3.5 // How fast does jellyfish bounce away after hitting a wall.Don't screw with this
	//CollisionWaitDuration = 15 //How long does jellyfish wait before puffing after hitting a wall ?
	int BlockCount_Max { 4 }; // How many times will jellyfish block against a wall before giving up on destination?
	std::vector<int> Propulsion_Sounds { }; //Sound effect when puffing
	double Intentional_Deacceleration { 0.15 }; //How fast does it deaccelerate when it wants to? (When going to waypoint or target)
	double Intentional_DriftVelocity { 0.3 }; //How fast does it move when it is near its target?
	double ProximityDistance { 1.5 }; //How close before special deacceleration & drift logic take over?
};

DEFINE_LOCO(Levitate, 3DC0B295-6546-11D3-80B0-00902792494C)
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject)
	{
		return LocomotionClass::QueryInterface(iid, ppvObject);
	}

	virtual ULONG __stdcall AddRef() { return LocomotionClass::AddRef(); }
	virtual ULONG __stdcall Release() { return LocomotionClass::Release(); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) {

		if (pClassID == nullptr) {
			return E_POINTER;
		}

		*pClassID = __uuidof(this);

		return S_OK;
	}

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() { return LocomotionClass::IsDirty(); }
	virtual HRESULT __stdcall Load(IStream* pStm) {

		HRESULT hr = LocomotionClass::Internal_Load(this,pStm);
		return hr;
	}
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)  {

		this->Characteristic.Drag = 1.00;
		HRESULT hr = LocomotionClass::Internal_Save(this, pStm , fClearDirty);
		if (SUCCEEDED(hr))
		{
			GameDebugLog::Log("LevitateLoco Save !");
			// Insert any data to be loaded here.
		}

		return hr;
	}
	virtual ~LevitateLocomotionClass() override = default; // should be SDDTOR in fact
	virtual int Size() override { return sizeof(*this); }
	virtual HRESULT __stdcall Link_To_Object(void* pointer) override {
		return LocomotionClass::Link_To_Object(pointer);
	}

	virtual bool __stdcall Is_Moving() override { return LinkedTo != nullptr; };
	virtual CoordStruct __stdcall Destination() override { return CoordStruct::Empty; }
	virtual CoordStruct __stdcall Head_To_Coord() override { return LinkedTo->GetCoords(); }
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override {
		return LinkedTo->IsCellOccupied(MapClass::Instance->GetCellAt(cell), FacingType::None, -1, nullptr, false);
	}
	virtual bool __stdcall Process() override;

	virtual Layer __stdcall In_Which_Layer()override { return Layer::Ground; }
	virtual bool __stdcall Is_Moving_Now() override { return LinkedTo != nullptr; }
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override {
		auto headto = Head_To_Coord();
		if (mark)
		{
			LinkedTo->MarkAllOccupationBits(headto);
		}
		else
		{
			LinkedTo->UnmarkAllOccupationBits(headto);
		}
	}

	void ProcessHovering();
	void DoPhase1();
	void DoPhase2();
	void DoPhase3();
	void DoPhase4();
	void DoPhase5(CoordStruct coord);
	void DoPhase6();
	void DoPhase7();
	bool IsDestValid();
	bool IsTargetValid();
	bool IsCloseEnough(CoordStruct nCoord);
	bool IsLessSameThanProximityDistance(CoordStruct nCoord);
	void CalculateDir_Close(CoordStruct nTarget);
	void CalculateDir_Far(CoordStruct nTarget);
	void DirtoSomething(double dValue);
	void ProcessSomething();
	bool IsAdjentCellEligible(CoordStruct nArgsCoord);

	void Reset(int state) {
		State = state;
		CurrentVelocity = 0;
		Delta = {};
		AccelerationDurationCosinus = 0;
		AccelerationDurationNegSinus = 0;
		AccelerationDuration = 0;
		BlocksCounter = 0;
		CurrentSpeed = 0;
		Dampen = 0;
		field_58 = 0;
	}

	LevitateLocomotionClass() : LocomotionClass {}
		, Characteristic {}
		, State { 0 }
		, CurrentVelocity { 0.0 }
		, Delta { }
		, AccelerationDurationCosinus { 0.0 }
		, AccelerationDurationNegSinus { 0.0 }
		, AccelerationDuration { 0 }
		, BlocksCounter { 4 }
		, CurrentSpeed { 0.0 }
		, Dampen { 0.0 }
		, field_58 { 0.0 }
	{ }

	LevitateLocomotionClass(noinit_t) : LocomotionClass {noinit_t()}
	{ }

public:
	LevitateCharacteristics Characteristic;
	int State; // State?
	double CurrentVelocity; // CurrentVelocity?
	Vector2D<double> Delta;
	double AccelerationDurationCosinus; // AccelerationDurationCosinus?
	double AccelerationDurationNegSinus; // AccelerationDurationNegSinus?
	int AccelerationDuration; // AccelerationDuration?
	int BlocksCounter; // BlocksCounter?
	double CurrentSpeed; // CurrentSpeed?
	double Dampen; // Dampen? 50
	double field_58;
};