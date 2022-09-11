#pragma once

#include <AbstractClass.h>

//Unfinised

class TechnoClass;
class PlanningMemberClass
{
public:
	int int0 ;
	DWORD dword4;
	DWORD field_8;
	BYTE field_C;
	BYTE field_D;
	BYTE field_E;
	BYTE field_F;
};
static_assert(sizeof(PlanningMemberClass) == 0x10);

class PlanningBranchClass
{
public:
	BYTE gap[112];
	DWORD  val_70;
	DWORD va74;
};
static_assert(sizeof(PlanningBranchClass) == 0x78);

class PlanningNodeClass
{
public:
~PlanningNodeClass() { JMP_THIS(0x633D30); }
 PlanningNodeClass(int nDword18) { JMP_THIS(0x633CC0) ;}
public:
	DECLARE_PROPERTY(DynamicVectorClass<PlanningMemberClass*>, MemberVector);
	DWORD dword18;
	DWORD field_1C;
	DECLARE_PROPERTY(DynamicVectorClass<PlanningBranchClass*>, BranchVector);
	char byte38[0x6F];
	DWORD dwordA8;
    DWORD dwordAC;
    DWORD BranchNumber;
    DWORD dwordB4;
};
static_assert(sizeof(PlanningNodeClass) == 0xB8);

class PlanningNode
{
public:
	DWORD field_0;
	DWORD field_4;
	DWORD field_8;
	DWORD field_C;
	int field_10;
	DWORD field_14;
};

class PlanningTokenClass
{
public:

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
TechnoClass *OwnerUnit;
	DynamicVectorClass<PlanningNode *> PlanningNodes;

	DECLARE_PROPERTY_ARRAY(DWORD, unknown, 0x1C);

	int field_8C;
	int ClosedLoopNodeCount;
	int StepsToClosedLoop;

	DECLARE_PROPERTY(DWORD, field_98);
};
static_assert(sizeof(PlanningTokenClass) == 0x9C);
