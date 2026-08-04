#pragma once

#include <Ext/AbstractType/Body.h>
#include <ScriptTypeClass.h>

class ScriptTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = ScriptTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "ScriptTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "ScriptTypeClass";

	static COMPILETIMEEVAL DWORD Canary = 0xC5B5D16D;

public:
	ScriptActionNode OriginalActions[50] {};
	int OriginalActionsCount {};
	bool IsModified {};

	ScriptTypeExtData(base_type* pObj) : AbstractTypeExtData(pObj)
	{
		this->AbsType = base_type::AbsID;
	}

	ScriptTypeExtData() = default;

	virtual ~ScriptTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<ScriptTypeExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<ScriptTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{}

	base_type* This() const { return reinterpret_cast<base_type*>(this->AttachedToObject); }
	const base_type* This_Const() const { return reinterpret_cast<const base_type*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) override { return true;  }
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	void CaptureOriginal();
	void RestoreOriginal();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ScriptTypeExtContainer final : public Container<ScriptTypeExtData>
	, public ReadWriteContainerInterfaces<ScriptTypeExtData>
	, public ContainerSaveLoad<ScriptTypeExtContainer, ScriptTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "ScriptTypeExtContainer";
	static ScriptTypeExtContainer Instance;

public:

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }
	virtual void LoadFromINI(ScriptTypeClass* key, CCINIClass* pINI, bool parseFailAddr) {};
	virtual void WriteToINI(ScriptTypeClass* key, CCINIClass* pINI) {};
};

class TActionClass;
class TeamTypeExtData;
class TeamTypeClass;
class ScriptManipulator
{
public:
	static void ClearScript(TActionClass* pThis);
	static void CopyScript(TActionClass* pThis);
	static void ModifyScriptByParam(TActionClass* pThis);
	static void ModifyScriptByLocalVar(TActionClass* pThis);
	static void ModifyScriptByGlobalVar(TActionClass* pThis);

	static void RebindTeamTypeScript(TActionClass* pThis);
	static void ResetTeamTypeScript(TActionClass* pThis);
	static void ResetAllTeamTypeScripts();

	static void RestoreScriptContent(TActionClass* pThis);
	static void RestoreAllScriptContents();
	static void SeekTeamTypeScript(TActionClass* pThis);

	static void CaptureFromINI(CCINIClass* pINI);

private:
	static void ResetTeamsUsingScript(ScriptTypeClass* pScript);
	static void CaptureOriginalScriptIndex(TeamTypeExtData* pExt, TeamTypeClass* pTeamType);
};

class NOVTABLE FakeScriptTypeClass : public ScriptTypeClass
{
public:

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	bool _ReadFromINI(CCINIClass* pINI);
};