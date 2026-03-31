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
	//CheckPrereq = 8,
	//CheckBridgeCondition = 9

	NumberOfTechBuildingsExist = 8,
	NumberOfBridgeRepairHutsExist = 9,
};

class AITriggerTypeExtData final : public AbstractTypeExtData
{
public:

	using base_type = AITriggerTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AITriggerTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AITriggerTypeClass";
	
	

public:

	AITriggerTypeExtData(AITriggerTypeClass* pObj)
		: AbstractTypeExtData(pObj)
	{ }

	AITriggerTypeExtData(AITriggerTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~AITriggerTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved);
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

	static int CheckConditions(AITriggerTypeClass* pThis, HouseClass* pOwner, HouseClass* pEnemy);
	static bool GetComparatorResult(int operand1, AITriggerConditionComparatorType operatorType, int operand2);
	static bool NumberOfTechBuildingsExist(AITriggerTypeClass* pThis, HouseClass* pOwner);
	static bool NumberOfBridgeRepairHutsExist(AITriggerTypeClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AITriggerTypeExtContainer final : public Container<AITriggerTypeExtData>
	, public ReadWriteContainerInterfaces<AITriggerTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AITriggerTypeExtContainer";

public:
	static AITriggerTypeExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

	virtual void LoadFromINI(AITriggerTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AITriggerTypeClass* key, CCINIClass* pINI);
};
