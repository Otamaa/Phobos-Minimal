#pragma once
#include "Base.h"

DEFINE_LOCO(TSJumpJet,4C8171D5-E7A7-43D1-80F3-0C285CF6B352)
{
public:

	/**
	 *  Represents the current flight state of the jumpjet.
	 *  - GROUNDED: Landed and idle
	 *  - ASCENDING: Taking off
	 *  - HOVERING: Stationary at cruise height
	 *  - CRUISING: Moving toward destination at cruise height
	 *  - DESCENDING: Attempting to land
	 */
	enum class JumpjetState : BYTE
	{
		GROUNDED = 0,
		ASCENDING,
		HOVERING,
		CRUISING,
		DESCENDING
	};


	/**
		 *  IPersist methods.
		 */
	IFACEMETHOD_(HRESULT, GetClassID)(CLSID * retval) override;

	/**
	 *  IPersistStream methods.
	 */
	IFACEMETHOD_(HRESULT, Load)(IStream * stream) override;
	IFACEMETHOD_(HRESULT, Save)(IStream* pStm, BOOL fClearDirty) override;

	/**
	 *  ILocomotion methods.
	 */
	IFACEMETHOD(Link_To_Object)(void* object);
	IFACEMETHOD_(bool, Is_Moving)() override;
	IFACEMETHOD_(Coordinate, Destination)() override;
	IFACEMETHOD_(Coordinate, Head_To_Coord)() override;
	IFACEMETHOD_(bool, Process)() override;
	IFACEMETHOD_(void, Move_To)(Coordinate to) override;
	IFACEMETHOD_(void, Stop_Moving)() override;
	IFACEMETHOD_(void, Do_Turn)(DirStruct coord) override;
	IFACEMETHOD_(Layer, In_Which_Layer)() override;
	IFACEMETHOD_(bool, Is_Moving_Now)() override;
	IFACEMETHOD_(void, Mark_All_Occupation_Bits)(int mark) override;

	/**
	 *  LocomotionClass methods.
	 */
	virtual int Size() override { return sizeof(*this); }

public :

	TSJumpJetLocomotionClass();
	TSJumpJetLocomotionClass(noinit_t const& x) : LocomotionClass { noinit_t() } { }
	TSJumpJetLocomotionClass& operator=(const TSJumpJetLocomotionClass&) = delete;
	TSJumpJetLocomotionClass(const TSJumpJetLocomotionClass&) = delete;
	virtual ~TSJumpJetLocomotionClass() override = default;

public:

	void Process_Grounded();
	void Process_Ascent();
	void Process_Hover();
	void Process_Cruise();
	void Process_Descent();
	void Movement_AI();
	Coordinate Closest_Free_Spot(Coordinate const& to) const;
	int Desired_Flight_Level() const;

private:

	double JumpjetClimb;
	double JumpjetAcceleration;
	double JumpjetWobblesPerSecond;
	bool JumpjetNoWobbles;
	int JumpjetWobbleDeviation;
	int JumpjetCloakDetectionRadius;
	int JumpjetCruiseHeight;
	int JumpjetTurnRate;
	int JumpjetSpeed;

	Coordinate HeadToCoord;
	JumpjetState CurrentState;
	FacingClass Facing;

	double CurrentSpeed;
	double TargetSpeed;
	double CurrentWobble;

	int FlightLevel;

	bool IsLanding;
	bool IsMoving;
};