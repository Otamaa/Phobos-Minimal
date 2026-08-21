#pragma once
#include <TeamTypeClass.h>

#include <Ext/AbstractType/Body.h>

class TeamTypeExtData : public AbstractTypeExtData
{
public:
	static COMPILETIMEEVAL DWORD Canary = 0xD2F52D0C;
	using base_type = TeamTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "TeamTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TeamTypeClass";

public:

	Nullable<int> IsDischargedMemberAutocreateRecruitable {};
	Nullable<int> AI_SafeDIstance {};
	Nullable<int> AI_FriendlyDistance {};
	Valueable<bool> AttackWaypoint_AllowCell { true };
	Nullable<AircraftTypeClass*> ParaDropAircraft {};

	int OriginalScriptTypeIndex { -1 };
	int OriginalTaskForceIndex { -1 };

	TeamTypeExtData(base_type* pObj) : AbstractTypeExtData(pObj)
	{
		this->AbsType = base_type::AbsID;
	}

	TeamTypeExtData() = default;

	virtual ~TeamTypeExtData() = default;

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
		const_cast<TeamTypeExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<TeamTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	base_type* This() const { return reinterpret_cast<base_type*>(this->AttachedToObject); }
	const base_type* This_Const() const { return reinterpret_cast<const base_type*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TeamTypeExtContainer final : public Container<TeamTypeExtData>
	, public ReadWriteContainerInterfaces<TeamTypeExtData>
	, public ContainerSaveLoad<TeamTypeExtContainer, TeamTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TeamTypeExtContainer";
	using base_t = Container<TeamTypeExtData>;

public:
public:

	static TeamTypeExtContainer Instance;

	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }
	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }

	virtual void LoadFromINI(TeamTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(TeamTypeClass* key, CCINIClass* pINI);
};

class FootClass;

class NOVTABLE FakeTeamTypeClass : public TeamTypeClass
{
public:
	static bool __fastcall _DoReinforcement(TeamTypeClass* pType, int waypoint);
	static bool __fastcall _TunnelMaybe(TeamTypeClass* pType, FootClass* pGroup, CellStruct* waypointCell, bool inRadar);
	static FootClass* __fastcall _CreateGroup(TeamTypeClass* pType);
	TeamClass* _CreateOneOf(HouseClass* pHouse);

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);
};
static_assert(sizeof(FakeTeamTypeClass) == sizeof(TeamTypeClass), "Invalid Size !");