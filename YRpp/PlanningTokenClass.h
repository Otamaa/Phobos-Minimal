#pragma once

#include <AbstractClass.h>

//Unfinised

class TechnoClass;
class PlanningMemberClass;
class PlanningBranchClass;

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
	TechnoClass * OwnerUnit;
	DynamicVectorClass<PlanningNodeClass*> NodeVector;
	BYTE byte1C;
    char byte1D[0x6C];
	DWORD dword8C;
	DWORD dword90;
	DWORD dword94;
    BYTE byte98;
    BYTE byte99;
	PROTECTED_PROPERTY(BYTE, Padding[2]);
};
static_assert(sizeof(PlanningTokenClass) == 0x9C);
