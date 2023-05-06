/*
	Base class for units that can move (that have "feet")
*/

#pragma once

#include <TechnoClass.h>
#include <ParasiteClass.h>
#include <Helpers/ComPtr.h>
#include <Interface/ILocomotion.h>

//forward declarations
class LocomotionClass;
class TeamClass;

class NOVTABLE FootClass : public TechnoClass
{
public:
	static const auto AbsDerivateID = AbstractFlags::Foot;

	static constexpr constant_ptr<DynamicVectorClass<FootClass*>, 0x8B3DC0u> const Array {};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4DB3C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4DB690);

	//Destructor
	virtual ~FootClass() override JMP_THIS(0x4E0170);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x4D9960);
	virtual void Update() override JMP_THIS(0x4DA530);
			  
	//ObjectClass
		  
	//MissionClass
	virtual void Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) override JMP_THIS(0x4D8F40);

	//TechnoClass
	virtual void Destroyed(ObjectClass* Killer) override RX;
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) override JMP_THIS(0x4DBED0);
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) override R0;
	virtual bool IsInSameZoneAs(AbstractClass* pTarget) override JMP_THIS(0x4DBA50);
	virtual bool IsInSameZone(const CoordStruct* nZone) override JMP_THIS(0x4D3810);
	virtual bool Crash(ObjectClass* Killer) override JMP_THIS(0x4DEBB0);

	//FootClass
	virtual void ReceiveGunner(FootClass* Gunner) RX;
	virtual void RemoveGunner(FootClass* Gunner) RX;
	virtual bool IsLeavingMap() const R0;
	virtual bool vt_entry_4E0() const R0;
	virtual bool CanDeployNow() const R0;
	virtual void AddSensorsAt(CellStruct npCell) RX;
	virtual void RemoveSensorsAt(CellStruct nCell) RX;
	virtual CoordStruct* vt_entry_4F0(CoordStruct* pCrd) R0;
	virtual void vt_entry_4F4() RX;
	virtual bool vt_entry_4F8() R0;
	virtual bool MoveTo(CoordStruct* pCrd) R0;
	virtual bool StopMoving() R0;
	virtual bool vt_entry_504() R0;
	virtual bool ChronoWarpTo(CoordStruct pDest) R0; // fsds... only implemented for one new YR map trigger, other chrono events repeat the code...
	virtual void Draw_A_SHP(
		SHPStruct* SHP, int idxFacing, Point2D* Coords, RectangleStruct* Rectangle,
		DWORD dwUnk5, DWORD dwUnk6, DWORD dwUnk7, ZGradient ZGradient,
		DWORD dwUnk9, int extraLight, DWORD dwUnk11, DWORD dwUnk12,
		DWORD dwUnk13, DWORD dwUnk14, DWORD dwUnk15, DWORD dwUnk16) RX;

	virtual void Draw_A_VXL(
		VoxelStruct* VXL, int HVAFrameIndex, int Flags, IndexClass<int, int>* Cache, RectangleStruct* Rectangle,
		Point2D* CenterPoint, Matrix3D* Matrix, DWORD dwUnk8, DWORD DrawFlags, DWORD dwUnk10) RX;

	virtual void GoBerzerk() RX;
	virtual void Panic() RX;
	virtual void UnPanic() RX; //never
	virtual void PlayIdleAnim(int nIdleAnimNumber) RX;
	virtual DWORD vt_entry_524() R0;
	virtual TechnoClass* FindDockingBayInVector(DynamicVectorClass<TechnoTypeClass*>* pVec, int unusedarg3, bool bForced) const R0;
	virtual TechnoClass* FindDockingBayByType(TechnoTypeClass* pDock, int unusedarg3, bool bForced, int* curidx) const R0;
	virtual TechnoClass* FindDockingBay(TechnoTypeClass* pDock, int unusedarg3, bool bForced) const R0;
	virtual void vt_entry_534(DWORD dwUnk, DWORD dwUnk2) RX;
	virtual int GetCurrentSpeed() const R0;
	virtual bool ApproachTarget(bool bSomething) JMP_THIS(0x4D5690); //0x53C
	virtual void vt_entry_540(DWORD dwUnk) RX;
	virtual void SetSpeedPercentage(double percentage) RX;
	virtual void vt_entry_548() RX;
	virtual void vt_entry_54C() RX;
	virtual bool IsLandingZoneClear(AbstractClass* pLZ) R0;

	bool CanBeRecruited(HouseClass* ByWhom) const
	{ JMP_THIS(0x4DA230); }

	// non-virtual

	// only used by squid damage routines, normal wakes are created differently it seems
	// creates 3 wake animations behind the unit
	void CreateWakes(CoordStruct coords)
	{ JMP_THIS(0x629E90); }

	// can this jumpjet stay in this cell or not? (two jumpjets in one cell are not okay, locomotor kicks one of them out in the next frame)
	bool Jumpjet_LocationClear() const
	{ JMP_THIS(0x4135A0); }

	void Jumpjet_OccupyCell(CellStruct Cell)
	{ JMP_THIS(0x4E00B0); }

	// changes locomotor to the given one, Magnetron style
	// mind that this locks up the source too, Magnetron style
	void FootClass_ImbueLocomotor(FootClass* target, CLSID clsid)
	{ JMP_THIS(0x710000); }

	// var $this = this; $.each($this.Passengers, function(ix, p) { p.Location = $this.Location; });
	void UpdatePassengerCoords()
	{ JMP_THIS(0x7104F0); }

	void AbortMotion()
	{ JMP_THIS(0x4DF0D0); }

	bool UpdatePathfinding(CellStruct unkCell, CellStruct unkCell2, int unk3)
	{ JMP_THIS(0x4D3920); }

	// Removes the first passenger and updates the Gunner.
	FootClass* RemoveFirstPassenger()
	{ JMP_THIS(0x4DE710); }

	// Removes a specific passenger and updates the Gunner.
	FootClass* RemovePassenger(FootClass* pPassenger)
	{ JMP_THIS(0x4DE670); }

	// Adds a specific passenger and updates the Gunner.
	void EnterAsPassenger(FootClass* pPassenger)
	{ JMP_THIS(0x4DE630); }

	void ClearSomeVector() // clears 5AC
	{ JMP_THIS(0x4DA1C0); }

	// searches cell, sets destination, and returns whether unit is on that cell
	bool MoveToTiberium(int radius, bool scanClose = false)
	{ JMP_THIS(0x4DCFE0); }

	// searches cell, sets destination, and returns whether unit is on that cell
	bool MoveToWeed(int radius)
	{ JMP_THIS(0x4DDB90); }

	bool LiberateMember(int idx = -1, byte count = 0u);

	CellStruct GetRandomDirection(FootClass* pFoot);

	void Draw_(Point2D* pPoint, RectangleStruct* pRect) const
	{ JMP_THIS(0x4DB250); }

	CellStruct* SafetyPoint(CellStruct* retval, CellStruct* src, CellStruct* dst, int start, int max) const {
		JMP_THIS(0x4CBC40);
	}

	LPVOID SwapToDroppodLocomotor() const  {
		JMP_THIS(0x4DB8A0);
	}

	bool IsTryingToEnterSomething() const { JMP_THIS(0x4E0080); }

	//Constructor
	FootClass(HouseClass* pOwner) noexcept : FootClass(noinit_t())
		{ JMP_THIS(0x4D31E0); }

protected:
	explicit __forceinline FootClass(noinit_t) noexcept
		: TechnoClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int             PlanningPathIdx; // which planning path am I following?
	short           unknown_short_524;
	short           unknown_short_526;
	short           unknown_short_528;
	short           unknown_short_52A;
	DWORD           unknown_52C;	//unused?
	DWORD           unknown_530;
	DWORD           unknown_534;
	int				WalkedFramesSoFar;
	bool            __PlayingMovingSound; //53C
	int           __MovingSoundDelay; //540

	DECLARE_PROPERTY(AudioController, Audio7);

	CellStruct      CurrentMapCoords;
	CellStruct      LastMapCoords; // ::UpdatePosition uses this to remove threat from last occupied cell, etc
	CellStruct      LastJumpjetMapCoords; // which cell was I occupying previously? only for jumpjets
	CellStruct      CurrentJumpjetMapCoords; // which cell am I occupying? only for jumpjets
	CoordStruct     CurrentMechPos; //unknown_coords_568 5B0832
	PROTECTED_PROPERTY(DWORD,   unused_574);
	double          SpeedPercentage;
	double          SpeedMultiplier;
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, unknown_abstract_array_588);
	AbstractClass*  unknown_5A0;  //TODO
	AbstractClass*  Destination; //navcom possibly other objects as well
	AbstractClass*  LastDestination; //suspendednavcom
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, unknown_abstract_array_5AC);
	int             state5C4;
	DWORD           target5C8_CandidateTarget;
	DWORD           ___CandidateTarget_5CC;
	BYTE            unknown_5D0;	//unused?
	bool            newtargetassigned_5D1;
	TeamClass*      Team;
	FootClass*      NextTeamMember;        //next unit in team
	DWORD           unknown_5DC;
	FacingTypeI      PathDirections[24]; // list of directions to move in next, like tube directions
	DECLARE_PROPERTY(CDTimerClass, PathDelayTimer); //CDTimerClass
	int             TryTryAgain; //64C
	DECLARE_PROPERTY(CDTimerClass, unknown_timer_650); //BaseAttackTimer  CDTimerClass
	DECLARE_PROPERTY(CDTimerClass, SightTimer);
	DECLARE_PROPERTY(CDTimerClass, BlockagePathTimer);
	DECLARE_PROPERTY(YRComPtr<ILocomotion>, Locomotor);
	CoordStruct       __HeadTo; //_678
	signed char       TubeIndex;	//I'm in this tunnel
	bool              unknown_bool_685;
	signed char       WaypointIndex; // which waypoint in my planning path am I following?
	bool              IsToScatter; //678
	bool              IsScanLimited; //688
	bool              IsTeamLeader;
	bool              ShouldScanForTarget;
	bool              IsPlanningToLook; //68B
	bool              IsDeploying;	//68C
	bool              IsFiring;	//68D
	bool               __AssignNewThreat; //68E
	bool              ShouldEnterAbsorber; // orders the unit to enter the closest bio reactor
	bool              ShouldEnterOccupiable; // orders the unit to enter the closest battle bunker
	bool              ShouldGarrisonStructure; // orders the unit to enter the closest neutral building
	FootClass*        ParasiteEatingMe; // the tdrone/squid that's eating me
	DWORD             __ParasiteFireBlock_698;
	ParasiteClass*    ParasiteImUsing;	// my parasitic half, nonzero for, eg, terror drone or squiddy
	DECLARE_PROPERTY(CDTimerClass, ParalysisTimer); // for squid victims
	bool              unknown_bool_6AC;
	bool              IsAttackedByLocomotor; //6AD, the unit's locomotor is jammed by a magnetron
	bool              IsLetGoByLocomotor; //6AE a magnetron attacked this unit and let it go. falling, landing, or sitting on the ground
	bool              IsRotating; //6AF
	bool              IsUnloading; //6B0
	bool              IsNavQueueLoop;//6B1
	bool              IsScattering;//6B2
	bool              isidle_6B3;
	bool              height_subtract_6B4;
	bool              iscrusher_6B5;
	bool              FrozenStill; // frozen in first frame of the proper facing - when magnetron'd or warping
	bool              IsPathBlocked; //6B7
	bool              removed;//6B8
	PROTECTED_PROPERTY(DWORD,   unused_6BC);	//???
};

static_assert(sizeof(FootClass) == 0x6C0 , "Invalid Size !");
