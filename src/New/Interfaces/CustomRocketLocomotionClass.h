#pragma once

#include "Base.h"
#include <AircraftClass.h>

#include <New/Type/RocketTypeClass.h>

enum class MissionState
{
	None = 0,
	Pause = 1,
	Tilt = 2,
	GainingAltitude = 3,
	Flight = 4,
	ClosingIn = 5,
	VerticalTakeOff = 6,
};

int constexpr RocketSpeed = 416;
class RocketTypeClass;

DEFINE_LOCO(CustomRocket,4AD76F43-090A-44BF-BB1A-5BFDE52BC842)
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID * ppvObject) override
	{
		return LocomotionClass::QueryInterface(iid, ppvObject);
	}

	virtual ULONG __stdcall AddRef() override { return LocomotionClass::AddRef(); }
	virtual ULONG __stdcall Release() override { return LocomotionClass::Release(); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID * pClassID) override 
	{

		if (pClassID == nullptr)
		{
			return E_POINTER;
		}

		*pClassID = __uuidof(this);

		return S_OK;
	}
	/**
	 *  IPersistStream
	 */
	virtual HRESULT __stdcall Load(IStream* pStm) override
	{

		HRESULT hr = LocomotionClass::Internal_Load(this, pStm);
		if (FAILED(hr))
		{
			return E_FAIL;
			// Insert any data to be loaded here.
		}

		new (this) CustomRocketLocomotionClass(noinit_t());

		return hr;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override
	{
		return this->LocomotionClass::Internal_Save(this, pStm, fClearDirty);
	}

	// ILocomotion
	virtual bool __stdcall Is_Moving() override
	{
		return DestinationCoord != CoordStruct::Empty;
	}

	CoordStruct __stdcall Destination() override
	{
		return DestinationCoord;
	}

	Matrix3D* __stdcall Draw_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key) override
	{
		return pMatrix;
	}

	virtual Point2D __stdcall Shadow_Point() override
	{
		return { 0, 0 };
	}

	virtual bool __stdcall Process() override
	{
		return true;
	}

	virtual void __stdcall Move_To(CoordStruct to) override
	{
	}

	virtual void __stdcall Stop_Moving() override { }

	virtual Layer __stdcall In_Which_Layer() override
	{
		return Layer::Top;
	}

	virtual bool __stdcall Is_Moving_Now() override
	{
		return MissionState >= MissionState::GainingAltitude && MissionState <= MissionState::ClosingIn;
	}

	CustomRocketLocomotionClass() :
		LocomotionClass(),
		DestinationCoord(),
		MissionTimer(),
		TrailTimer(),
		MissionState(MissionState::None),
		CurrentSpeed(0),
		NeedToSubmit(true),
		IsSpawnerElite(false),
		CurrentPitch(0.0),
		ApogeeDistance(0)
	{ }

	~CustomRocketLocomotionClass() override = default;

	virtual int Size() override { return sizeof(*this); }

private:
	 //TODO
public:
	CustomRocketLocomotionClass(const CustomRocketLocomotionClass&) = delete;
	CustomRocketLocomotionClass(noinit_t) : LocomotionClass { noinit_t() } { }
	CustomRocketLocomotionClass& operator=(const CustomRocketLocomotionClass&) = delete;

protected:
	/**
	 *  This is the desired destination coordinate of the rocket.
	 */
	Coordinate DestinationCoord;

	/**
	 *  This is the timer used by various mission states of the rocket.
	 */
	RepeatableTimer MissionTimer;

	/**
	 *  This is the timer used for timing the trail animation.
	 */
	CDTimerClass TrailTimer;

	/**
	 *  The current state of the rocket.
	 */
	MissionState MissionState;

	/**
	 *  The current speed of the rocket.
	 */
	double CurrentSpeed;

	/**
	 *  This boolean gets used to determine if the rocket needs to be submit to DisplayClass.
	 */
	bool NeedToSubmit;

	/**
	 *  Is this rocket's spawner elite?
	 */
	bool IsSpawnerElite;

	/**
	 *  The current pitch of the rocket.
	 */
	double CurrentPitch;

	/**
	 *  The distance to the destination from when the rocket has reached its desired altitude.
	 */
	int ApogeeDistance;
};
