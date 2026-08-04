#pragma once
#include <TaskForceClass.h>
#include <Ext/AbstractType/Body.h>

class TaskForceExtData final : public AbstractTypeExtData
{
public:
	using base_type = TaskForceClass;
	static COMPILETIMEEVAL const char* ClassName = "TaskForceExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TaskForceClass";
	static COMPILETIMEEVAL DWORD Canary = 0x6BE885A8;

public:

	TaskForceEntryStruct OriginalEntries[6] {};
	int OriginalCountEntries {};
	bool IsModified {};

public:

	TaskForceExtData(TaskForceClass* pObj) : AbstractTypeExtData(pObj)
	{
		this->AbsType = base_type::AbsID;
	}

	TaskForceExtData() = default;

	virtual ~TaskForceExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<TaskForceExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return  base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {}

	TaskForceClass* This() const { return reinterpret_cast<TaskForceClass*>(this->AttachedToObject); }
	const TaskForceClass* This_Const() const { return reinterpret_cast<const TaskForceClass*>(this->AttachedToObject); }

	void CaptureOriginal();
	void RestoreOriginal();

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) { return true; }
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TaskForceExtContainer final : public Container<TaskForceExtData>
	, public ContainerSaveLoad<TaskForceExtContainer, TaskForceExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "TaskForceExtContainer";
	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; };
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; };

public:
	static TaskForceExtContainer Instance;
};

class TActionClass;
class TeamTypeClass;
class TeamTypeExtData;
class TaskForceManipulator
{
public:
	static void ClearTaskForce(TActionClass* pThis);
	static void CopyTaskForce(TActionClass* pThis);
	static void ModifyTaskForceEntry(TActionClass* pThis);
	static void RebindTeamTypeTaskForce(TActionClass* pThis);
	static void ResetTeamTypeTaskForce(TActionClass* pThis);
	static void ResetAllTeamTypeTaskForces();

	static void RestoreTaskForce(TActionClass* pThis);
	static void RestoreAllTaskForces();

	static void RefreshTeamsUsingTaskForce(TaskForceClass* pTF);
	static void CaptureFromINI(CCINIClass* pINI);

private:
	static TaskForceClass* FindTaskForce(int param);
	static TeamTypeClass* FindTeamType(int param);
	static void CaptureOriginalContent(TaskForceClass* pTF);
	static void CaptureOriginalTaskForceIndex(TeamTypeExtData* pExt, TeamTypeClass* pTeamType);
	static void RefreshTeamsOfType(TeamTypeClass* pTeamType);
};


class NOVTABLE FakTaskForceClass : public TaskForceClass
{
public:
	static bool __fastcall _LoadEntryINI(CCINIClass* pINI, TaskForceType type);
	bool _LoadFromINI(CCINIClass* pINI);
	bool _WriteToINI(CCINIClass* pINI);

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

}; static_assert(sizeof(FakTaskForceClass) == sizeof(TaskForceClass), "Invalid Size !");