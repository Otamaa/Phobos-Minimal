#pragma once

#include <LocomotionClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <DebugLog.h>
#include <MapClass.h>

#include <Interface/IPiggyback.h>

struct LevitateCharacteristics
{

	double Drag; // rate that jellyfish slows down
	// max velocity that jellyfish can move again when...
	double Vel_Max_Happy; //   ...just puttering around
	double Vel_Max_WhenFollow; //	...going someplace in particular
	double Vel_Max_WhenPissedOff; //	...tracking down some mofo
	double Accel; // How much a puff accelerates
	double Accel_Prob; // Chance happy jellyfish will "puff"
	int Accel_Dur; // How long a puff accelerates the jellyfish
	double Initial_Boost; // How much of an initial speed boost does jellyfish get when puffing
	//BounceVelocity = 3.5 // How fast does jellyfish bounce away after hitting a wall.Don't screw with this
	//CollisionWaitDuration = 15 //How long does jellyfish wait before puffing after hitting a wall ?
	int BlockCount_Max; // How many times will jellyfish block against a wall before giving up on destination?
	std::vector<int> Propulsion_Sounds; //Sound effect when puffing
	double Intentional_Deacceleration; //How fast does it deaccelerate when it wants to? (When going to waypoint or target)
	double Intentional_DriftVelocity; //How fast does it move when it is near its target?
	double ProximityDistance; //How close before special deacceleration & drift logic take over?

	LevitateCharacteristics() :
		Drag { 0.05 }
		, Vel_Max_Happy { 4.0 }
		, Vel_Max_WhenFollow { 5.0 }
		, Vel_Max_WhenPissedOff { 10.5 }
		, Accel { 0.5 }
		, Accel_Prob { 0.01 }
		, Accel_Dur { 20 }
		, Initial_Boost { 1.5 }
		, BlockCount_Max { 4 }
		, Propulsion_Sounds { }
		, Intentional_Deacceleration { 0.15 }
		, Intentional_DriftVelocity { 0.3 }
		, ProximityDistance { 1.5 }
	{}
};

_COM_SMARTPTR_TYPEDEF(IPiggyback, __uuidof(IPiggyback));

class DECLSPEC_UUID("3DC0B295-6546-11D3-80B0-00902792494C") 
	LevitateLocomotionClass : public LocomotionClass 
	, public IPiggyback
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) { return LocomotionClass::QueryInterface(iid, ppvObject); }
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

		HRESULT hr = LocomotionClass::Load(pStm);
		if (SUCCEEDED(hr))
		{
			// Insert any data to be loaded here.
		}

		return hr;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)  {

		HRESULT hr = LocomotionClass::Save(pStm , fClearDirty);
		if (SUCCEEDED(hr))
		{
			// Insert any data to be loaded here.
		}

		return hr;
	}

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) { 
	
		if (pcbSize == nullptr) {
			return E_POINTER;
		}

		return LocomotionClass::GetSizeMax(pcbSize);
	}

	virtual ~LevitateLocomotionClass() override = default; // should be SDDTOR in fact
	virtual int Size() override { return sizeof(*this); }

	virtual HRESULT __stdcall Link_To_Object(void* pointer) override { 
		HRESULT hr = LocomotionClass::Link_To_Object(pointer);
		//if (SUCCEEDED(hr)) {
		//	GameDebugLog::Log("LevitateLocomotionClass - Sucessfully linked to \"%s\"\n", Owner->get_ID());
		//}
		return hr;
	
	}
	virtual bool __stdcall Is_Moving() override { return RefCount != 0; };
	virtual CoordStruct __stdcall Destination() override { return CoordStruct::Empty; }
	virtual CoordStruct __stdcall Head_To_Coord() override { return LinkedTo->GetCenterCoords(); }
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override { return LinkedTo->IsCellOccupied(MapClass::Instance->GetCellAt(cell), -1, -1, nullptr, false); }
	virtual bool __stdcall Is_To_Have_Shadow() override { return LocomotionClass::Is_To_Have_Shadow(); }
	virtual Matrix3D* __stdcall Draw_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key) override { return LocomotionClass::Draw_Matrix(pMatrix , key); }
	virtual Matrix3D* __stdcall Shadow_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key) override { return LocomotionClass::Shadow_Matrix(pMatrix ,key); }
	virtual Point2D __stdcall Draw_Point() override	{ return LocomotionClass::Draw_Point(); }
	virtual Point2D __stdcall Shadow_Point() override { return LocomotionClass::Shadow_Point(); }
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) override { return VisualType::Normal; }
	virtual int __stdcall Z_Adjust() override { return LocomotionClass::Z_Adjust(); }
	virtual ZGradient __stdcall Z_Gradient() override { return LocomotionClass::Z_Gradient(); }
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override { }
	virtual void __stdcall Stop_Moving() override { }
	virtual void __stdcall Do_Turn(DirStruct coord) override { LinkedTo->PrimaryFacing.Set_Current(coord); }
	virtual void __stdcall Unlimbo() override { Force_New_Slope(LinkedTo->GetCell()->RedrawCountMAYBE); }
	virtual void __stdcall Tilt_Pitch_AI() override { }
	virtual bool __stdcall Power_On() override { return LocomotionClass::Power_On(); }
	virtual bool __stdcall Power_Off() override { return LocomotionClass::Power_Off(); }
	virtual bool __stdcall Is_Powered() override { return Powered; }
	virtual bool __stdcall Is_Ion_Sensitive() override { return false; }
	virtual bool __stdcall Push(DirStruct dir) override { return false; }
	virtual bool __stdcall Shove(DirStruct dir) override { return false; }
	virtual void __stdcall Force_Track(int track, CoordStruct coord) override { }
	virtual Layer __stdcall In_Which_Layer()override { return Layer::Ground; }
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override { }
	virtual void __stdcall Force_New_Slope(int ramp) override { }
	virtual bool __stdcall Is_Moving_Now() override { return RefCount != 0; }
	virtual int __stdcall Apparent_Speed() override { return LinkedTo->GetCurrentSpeed(); }
	virtual int __stdcall Drawing_Code() override { return 0; }
	virtual FireError __stdcall Can_Fire() override { return FireError::OK; }
	virtual int __stdcall Get_Status() override { return 0; }
	virtual void __stdcall Acquire_Hunter_Seeker_Target() override { }
	virtual bool __stdcall Is_Surfacing() override { return LocomotionClass::Is_Surfacing(); }
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override {
		auto headto = Head_To_Coord();
		if (mark != 0)
		{
			LinkedTo->MarkAllOccupationBits(headto);
		}
		else
		{
			LinkedTo->MarkAllOccupationBits(headto);
		}
	}

	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override { 
		auto nBuff = CellClass::Coord2Cell(Head_To_Coord());
		CoordStruct headto_cell { nBuff.X , nBuff.Y ,0 };
		return nBuff.X == headto_cell.X && nBuff.Y == headto_cell.Y && std::abs(headto_cell.Z - to.Z) <= Unsorted::CellHeight;
	}

	virtual bool __stdcall Will_Jump_Tracks() override { return LocomotionClass::Will_Jump_Tracks(); }
	virtual bool __stdcall Is_Really_Moving_Now() override { return Is_Moving_Now(); }
	virtual void __stdcall Stop_Movement_Animation() override { LocomotionClass::Stop_Movement_Animation(); }
	virtual void __stdcall Clear_Coords() override { LocomotionClass::Clear_Coords(); }
	virtual void __stdcall Lock() override { LocomotionClass::Lock(); }
	virtual void __stdcall Unlock() override { LocomotionClass::Unlock(); }
	virtual int __stdcall Get_Track_Number() override { return LocomotionClass::Get_Track_Number(); }
	virtual int __stdcall Get_Track_Index() override { return LocomotionClass::Get_Track_Index(); }
	virtual int __stdcall Get_Speed_Accum() override { return LocomotionClass::Get_Speed_Accum(); }

	//IPiggy
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override { return S_OK; };	//Piggybacks a locomotor onto this one.
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override { return S_OK; };//End piggyback process and restore locomotor interface pointer.
	virtual bool __stdcall Is_Ok_To_End() override { return true; };	//Is it ok to end the piggyback process?
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override { return S_OK; };	//Fetches piggybacked locomotor class ID.
	virtual bool __stdcall Is_Piggybacking() override { return true; };	//Is it currently piggy backing another locomotor?

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
	bool IsMoreThanProximityDistance(CoordStruct nCoord);
	bool IsLessSameThanProximityDistance(CoordStruct nCoord);
	void CalculateDir_Close(CoordStruct nTarget);
	void CalculateDir_Far(CoordStruct nTarget);
	void DirtoSomething(double dValue);
	void ProcessSomething();
	bool IsAdjentCellEligible(CoordStruct nArgsCoord);

	LevitateLocomotionClass() : LocomotionClass {}
		, Characteristic {}
		, State { 0 }
		, CurrentVelocity { 0.0 }
		, DeltaX { 0.0 }
		, DeltaY { 0.0 }
		, AccelerationDurationCosinus { 0.0 }
		, AccelerationDurationNegSinus { 0.0 }
		, AccelerationDuration { 0 }
		, BlocksCounter { 4 }
		, CurrentSpeed { 0.0 }
		, Dampen { 0.0 }
		, field_58 { 0.0 }
	{ }

public:
	LevitateCharacteristics Characteristic;
	int State; // State?
	double CurrentVelocity; // CurrentVelocity?
	double DeltaX; // DeltaX?
	double DeltaY; // DeltaY?
	double AccelerationDurationCosinus; // AccelerationDurationCosinus?
	double AccelerationDurationNegSinus; // AccelerationDurationNegSinus?
	int AccelerationDuration; // AccelerationDuration?
	int BlocksCounter; // BlocksCounter?
	double CurrentSpeed; // CurrentSpeed?
	double Dampen; // Dampen? 50
	double field_58;
};