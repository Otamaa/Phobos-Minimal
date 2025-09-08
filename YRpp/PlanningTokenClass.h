#pragma once

#include <AbstractClass.h>
#include <ArrayClasses.h>

//Unfinised

class TechnoClass;
class PlanningBranchClass;

class PlanningMemberClass
{
public:
	TechnoClass* Owner;
	DWORD Packet;
	int field_8;
	char field_C;
};
static_assert(sizeof(PlanningMemberClass) == 0x10);

class PlanningNodeClass
{
public:
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4B30u> const Unknown1 {};
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4C18u> const Unknown2 {};
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4C98u> const Unknown3 {};
	static COMPILETIMEEVAL reference<bool, 0xAC4CF4u> const PlanningModeActive {};
	static COMPILETIMEEVAL reference <PlanningNodeClass* , 0xAC4CCC> const LastPlanning {};

	~PlanningNodeClass() { JMP_THIS(0x633D30); }
 	PlanningNodeClass(int nDword18) { JMP_THIS(0x633CC0);}
	PlanningNodeClass(TechnoClass* pOwner) { JMP_THIS(0x638A80); }
public:
	DynamicVectorClass<PlanningMemberClass*> PlanningMembers;
	int field_18;
	bool field_1C;
	DynamicVectorClass<PlanningBranchClass*> PlanningBranches;
};
//static_assert(sizeof(PlanningNodeClass) == 0xB8); size is broken actually

class PlanningTokenClass
{
public:
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<PlanningTokenClass*>, 0xAC4C78u> const Array {};

	void Clear(){JMP_THIS(0x636310);}
	TechnoClass* GetTechno() {JMP_THIS(0x636110);}
	BOOL IsMissionParaWait() {JMP_THIS(0x636550);}
	int NodeCount() {JMP_THIS(0x636DC0);}

	//byte1C
	//1
	void byte1C_636520() {JMP_THIS(0x636520);}

	//0
	void byte1C_636530() {JMP_THIS(0x636530);}

	//Get
	bool byte1C_636540() {JMP_THIS(0x636540);}
	//
	~PlanningTokenClass() {JMP_THIS(0x635F80);}
	PlanningTokenClass(TechnoClass* pTechno)
		{JMP_THIS(0x635F20);}
	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	TechnoClass* OwnerUnit;
	DynamicVectorClass<PlanningNodeClass*> PlanningNodes;
	bool field_1C;
	bool field_1D;
    DECLARE_PROPERTY_ARRAY(DWORD, unknown_20_88, 0x1B);
	int field_8C;
	int ClosedLoopNodeCount;
	int StepsToClosedLoop;
	bool field_98;
	bool field_99;
};
static_assert(sizeof(PlanningTokenClass) == 0x9C);
