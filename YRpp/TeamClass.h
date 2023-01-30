#pragma once

#include <AbstractClass.h>
#include <ScriptClass.h>

class HouseClass;
class ObjectClass;
class FootClass;
class CellClass;
class ScriptClass;
class TagClass;
class TeamTypeClass;
class TechnoTypeClass;

class DECLSPEC_UUID("0E272DCF-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE TeamClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Team;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<TeamClass*>, 0x8B40E8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x6EC560);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x6EC450);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) JMP_STD(0x6EC540);

	//Destructor
	virtual ~TeamClass() JMP_THIS(0x6F0450);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x6F0430);
	virtual int Size() const override JMP_THIS(0x6F0440);
	
	// fills dest with all types needed to complete this team. each type is
	// included as often as it is needed.
	void GetTaskForceMissingMemberTypes(DynamicVectorClass<TechnoTypeClass *>& dest) const { JMP_THIS(0x6EF4D0); }
	bool IsReallyLeavingMap() const { JMP_THIS(0x6EC300); }
	void LiberateMember(FootClass* pFoot, int idx=-1, byte count=0) const { JMP_THIS(0x6EA870); }
	void RemoveMember(FootClass* pFoot, int idx = -1, byte count = 0) const { JMP_THIS(0x6EA870); }
	bool Recuit(int nIdx) const { JMP_THIS(0x6EAA90); }
	// if bKeepQuantity is false, this will not change the quantity of each techno member
	bool AddMember(FootClass* pFoot) const { JMP_THIS(0x6EA4F0); }
	bool AddMember(FootClass* pFoot, bool bForce) { JMP_THIS(0x6EA500); }
	bool CanAddMember(FootClass* a2, int* entry, char somebool) const { JMP_THIS(0x6EA610); }
	bool HasMissionRemaining() const JMP_THIS(0x6915D0);
	FootClass* FetchLeader() const { JMP_THIS(0x6EC3D0); }
	bool Reacalculate() const { JMP_THIS(0x6EA3E0); }
	void TookDamage(FootClass* a2, DamageState result, ObjectClass* source) const { JMP_THIS(0x6EB380); }
	bool Team_Func6EA080() const { JMP_THIS(0x6EA080); }
	void Regroup() const { JMP_THIS(0x6EA0D0); }
	void CalCulateCenter() const { JMP_THIS(0x6EAEE0); }
	void CoordinateAttack() const { JMP_THIS(0x6EB490); }
	bool CoordinateRegroup() const { JMP_THIS(0x6EB870); }
	bool CoordinateMove() const { JMP_THIS(0x6EBAD0); }
	bool LaggingUnits() const { JMP_THIS(0x6EBF50); }
	bool CoordinateConscript(FootClass* a2) const { JMP_THIS(0x6EC130); }
	bool CoordinateDo(ScriptActionNode* nNode, int nUnused) const { JMP_THIS(0x6ED7E0); }
	bool IsAMember(FootClass* member) const { JMP_THIS(0x6EC220); }
	bool HasEnteredMap() const { JMP_THIS(0x6EC370); }
	void ScanLimit() const { JMP_THIS(0x6EC3A0); }
	void AssignMissionTarget(AbstractClass* new_target) const { JMP_THIS(0x6E9050); }
	bool HasAircraft() const { JMP_THIS(0x6EF470); }
	int GetStrayDistance () const { JMP_THIS(0x6F03B0); } //return in lepton
	bool DoesAnyMemberHaveAmmo() const { JMP_THIS(0x6F03F0); }

	static void __fastcall Suspend_Teams(int priority, HouseClass* house) { JMP_STD(0x6EC250); }
	static ThreatType __fastcall ThreatFromQuarry(QuarryType q) { JMP_THIS(0x645BB0); }

	//Constructor
	TeamClass(TeamTypeClass* pType , HouseClass* pOwner, int investigate_me) noexcept
		: TeamClass(noinit_t())
	{ JMP_THIS(0x6E8A90); }

protected:
	explicit __forceinline TeamClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TeamTypeClass* Type;
	ScriptClass*   CurrentScript;
	HouseClass*    Owner;
	HouseClass*    Target;
	CellClass*     SpawnCell;
	FootClass*	   ClosestMember;
	AbstractClass* QueuedFocus;
	AbstractClass* Focus;
	int            unknown_44;
	int            TotalObjects;
	int            TotalThreatValue;
	int            CreationFrame;
	FootClass*    FirstUnit;
	DECLARE_PROPERTY(TimerStruct ,GuardAreaTimer);
	DECLARE_PROPERTY(TimerStruct ,SuspendTimer);
	TagClass*      Tag;
	bool           IsTransient;
	bool           NeedsReGrouping; //75
	bool           GuardSlowerIsNotUnderStrength; //76
	bool           IsForcedActive; //77

	bool           IsHasBeen; //78
	bool           IsFullStrength; //79
	bool           IsUnderStrength;
	bool           IsReforming;

	bool           IsLagging;
	bool           NeedsToDisappear;
	bool           JustDisappeared;
	bool           IsMoving;

	bool           StepCompleted; // can proceed to the next step of the script
	bool           TargetNotAssigned;
	bool           IsLeavingMap;
	bool           IsSuspended;

	bool           AchievedGreatSuccess; // executed script action 49, 0

	ArrayWrapper<int , 6u> CountObjects; // counts of each object specified in the Type
};

static_assert(sizeof(TeamClass) == 0xA0 , "Invalid Size !");
