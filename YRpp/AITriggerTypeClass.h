/*
	[AITriggerTypes]
*/

#pragma once

#include <YRPP.h>
#include <HouseTypeClass.h>
#include <TeamTypeClass.h>
#include <AbstractTypeClass.h>

//forward declarations
class TechnoTypeClass;
class TeamTypeClass;

struct AITriggerConditionComparator
{
	int ComparatorType;
	int ComparatorOperand;
};

class DECLSPEC_UUID("BA093524-4CF4-11D2-BC26-00104B8FB04D")
	NOVTABLE AITriggerTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::AITriggerType;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<AITriggerTypeClass*>, 0xA8B200u> const Array {};

	IMPL_Find(AITriggerTypeClass)

	static AITriggerTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_FAST(0x41FCA0);
	}

	IMPL_FindIndexById(AITriggerTypeClass)

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~AITriggerTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	static bool __fastcall LoadFromINIList(CCINIClass *pINI, bool Global)
		{ JMP_FAST(0x41F2E0); }

	// Global == saving into AI.ini as opposed to map?
	// if !Global, [AITriggerTypesEnable] gets saved as well
	static bool __fastcall SaveToINIList(CCINIClass *pINI, bool Global)
		{ JMP_FAST(0x41F490); }

	// non-virtual

	// teams finished script, and
	void RegisterSuccess()
		{ JMP_THIS(0x41FD60); }

	void RegisterFailure()
		{ JMP_THIS(0x41FE20); }

	// the main condition
	bool ConditionMet(HouseClass *CallingHouse, HouseClass *TargetHouse, bool EnoughBaseDefense) const
		{ JMP_THIS(0x41E720); }

	// slaves
	bool OwnerHouseOwns(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41EE90); }

	bool CivilianHouseOwns(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41EC90); }

	bool EnemyHouseOwns(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41EAF0); }

	bool IronCurtainCharged(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41F0D0); }

	bool ChronoSphereCharged(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41F180); }

	bool HouseCredits(HouseClass *CallingHouse, HouseClass *TargetHouse) const
		{ JMP_THIS(0x41F230); }

	void FormatForSaving(char * buffer, size_t size) const {
		const char *Team1Name = GameStrings::NoneStr();
		const char *Team2Name = GameStrings::NoneStr();
		const char *HouseName = GameStrings::NoneStr();
		const char *ConditionName = GameStrings::NoneStr();

		TeamTypeClass *T = this->Team1;
		if(T) {
			Team1Name = T->get_ID();
		}
		T = this->Team2;
		if(T) {
			Team2Name = T->get_ID();
		}

		if(this->OwnerHouseType == AITriggerHouseType::Single) {
			auto const idxHouse = this->HouseIndex;
			if(idxHouse != -1) {
				HouseName = HouseTypeClass::Array->GetItem(idxHouse)->get_ID();
			}
		} else if(this->OwnerHouseType == AITriggerHouseType::Any) {
			HouseName = "<all>";
		}

		TechnoTypeClass *O = this->ConditionObject;
		if(O) {
			ConditionName = O->get_ID();
		}

		char ConditionString[68];
		int idx = 0;
		char * condStr = ConditionString;
		auto buf = reinterpret_cast<const byte*>(&this->Conditions);
		do {
			sprintf_s(condStr, 4, "%02x", *buf);
			++buf;
			++idx;
			condStr += 2;
		} while(idx < 0x20 );
		*condStr = '\0';

		sprintf_s(buffer, size, "%s = %s,%s,%s,%d,%d,%s,%s,%lf,%lf,%lf,%u,%d,%d,%u,%s,%u,%u,%u\n",
			this->ID,
			this->Name,
			Team1Name,
			HouseName,
			this->TechLevel,
			this->ConditionType,
			ConditionName,
			ConditionString,
			this->Weight_Current,
			this->Weight_Minimum,
			this->Weight_Maximum,
			this->IsForSkirmish,
			0,
			this->SideIndex,
			this->IsForBaseDefense,
			Team2Name,
			this->Enabled_Easy,
			this->Enabled_Normal,
			this->Enabled_Hard
		);

	}

	//Constructor
	AITriggerTypeClass(const char* pID) noexcept
		: AITriggerTypeClass(noinit_t())
	{ JMP_THIS(0x41E350); }

protected:
	explicit __forceinline AITriggerTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	AITriggerCondition ConditionType;
	int              IsGlobal;
	AITriggerHouseType OwnerHouseType;
	bool             IsEnabled;
	int              HouseIndex;
	int              SideIndex;
	int              TechLevel;
	int              unknown_B4; //this seems unsided ?
	double           Weight_Current;
	double           Weight_Minimum;
	double           Weight_Maximum;
	bool             IsForSkirmish;
	bool             IsForBaseDefense;
	bool             Enabled_Easy;
	bool             Enabled_Normal;
	bool             Enabled_Hard;
	TechnoTypeClass* ConditionObject;
	TeamTypeClass*   Team1;
	TeamTypeClass*   Team2;
	AITriggerConditionComparator Conditions [4]; // don't ask
	int              TimesExecuted;
	int              TimesCompleted;
	int              unknown_10C;
};

static_assert(sizeof(AITriggerTypeClass) == (0x110), "Invalid Size");