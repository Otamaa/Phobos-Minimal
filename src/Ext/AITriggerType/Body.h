#pragma once

#include <AITriggerTypeClass.h>
#include <Ext/AbstractType/Body.h>

//this is a 1-based index.
enum class PhobosAIConditionTypes : int
{
	CustomizableAICondition = 1,
};

enum class PhobosAINewConditionTypes : int
{
	//PR #2119
	NumberOfTechBuildingsExist = 8,
	NumberOfBridgeRepairHutsExist = 9,
	//

	CheckPrereq = 10,
	CheckBridgeCondition = 11,

	EnemyOwnsConditionObject = 12,
	HouseOwnsConditionObject = 13,
	NeutralOwnsConditionObject = 14,
	AllEnemyOwnsConditionObject = 15,
	EnemyOwnsAITargetTypesLists = 16,
	HouseOwnsAITargetTypesLists = 17,
	NeutralOwnsAITargetTypesLists = 18,
	AllEnemyOwnsAITargetTypesLists = 19,
	AllyOwnsAITargetTypesLists = 20,

	EnemyOwnsAITargetTypesListsComp = 21,
	HouseOwnsAITargetTypesListsComp = 22,
	NeutralOwnsAITargetTypesListsComp = 23,
	AllEnemyOwnsAITargetTypesListsComp = 24,

	DestroyedBridgeCount = 25,
	UndamagedBridgeCount = 26,

	count

};

class AITriggerTypeExtData final : public AbstractTypeExtData
{
public:

	using base_type = AITriggerTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AITriggerTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AITriggerTypeClass";
	static COMPILETIMEEVAL DWORD Canary = 0xBE032C9F;

public:

	AITriggerTypeExtData(AITriggerTypeClass* pObj)
		: AbstractTypeExtData(pObj)
	{ }

	AITriggerTypeExtData() = default;

	virtual ~AITriggerTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractTypeExtData::Internal_SaveToStream(Stm);
		const_cast<AITriggerTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	AITriggerTypeClass* This() const { return reinterpret_cast<AITriggerTypeClass*>(this->AttachedToObject); }
	const AITriggerTypeClass* This_Const() const { return reinterpret_cast<const AITriggerTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) { return true; }
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
public:

	static bool GetComparatorResult(int operand1, AITriggerConditionComparator& cond);
	static bool NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner);
	static bool NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool NeutralOwns(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool HouseOwnsCredits(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool IronCurtainNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool ChronosphereNearReady(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool CheckBaseCenterMZone(AITriggerTypeClass* pThis, TeamTypeClass* pTeam, HouseClass* pOwner, HouseClass* pEnemy);
	static bool CheckConditionType(AITriggerTypeClass* pThis, AITriggerCondition condType, HouseClass* house1, HouseClass* house2, bool lessThanZeroIsNotAllowed );
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, TechnoTypeClass* pItem);
	static bool NeutralOwns(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, TechnoTypeClass* pItem);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, std::vector<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, std::vector<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, std::vector<TechnoTypeClass*>& list);
	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, TechnoTypeClass* pItem);
	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, std::vector<TechnoTypeClass*>& list);
	static bool CountConditionMet(AITriggerTypeClass* pThis, int nObjects);
	static bool IsValidTechno(TechnoClass* pTechno);
	static bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed);
	static bool OwnStuffs(TechnoTypeClass* pItem, TechnoClass* list);
	static bool SuperWeaponNearReady(HouseClass* pHouse, int swTypeIndex);
	static int CountOwnedType(TechnoTypeClass* pType, HouseClass* pHouse);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AITriggerTypeExtContainer final : public Container<AITriggerTypeExtData>
	, public ReadWriteContainerInterfaces<AITriggerTypeExtData>
	, public ContainerSaveLoad<AITriggerTypeExtContainer, AITriggerTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AITriggerTypeExtContainer";
	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

public:
	static AITriggerTypeExtContainer Instance;

	virtual void LoadFromINI(AITriggerTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AITriggerTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAITriggerTypeClass : public AITriggerTypeClass
{
public:
	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	bool _NewTeam( HouseClass* house1, HouseClass* house2, bool skip);
	bool _SaveToINI(CCINIClass* pINI);
	bool _LoadFromINI(CCINIClass* pINI);
	static bool _ReadScenarioINI(CCINIClass* pINI);
	static bool _WriteScenarioINI(CCINIClass* pINI);
};

static_assert(sizeof(FakeAITriggerTypeClass) == sizeof(AITriggerTypeClass), "Invalid Size !");