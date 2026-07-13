#pragma once
#include <TriggerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosFixedString.h>

#include <TriggerTypeClass.h>

class TriggerClass;
class TriggerExtData final : public AbstractExtended
{
public:
	using base_type = TriggerClass;
	static COMPILETIMEEVAL const char* ClassName = "TriggerExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TriggerClass";
	static COMPILETIMEEVAL DWORD Canary = 0x030376D3;

public:

#pragma region ClassMember

	PhobosFixedString<0x18> Name {};

	std::vector<TEventClass*> SortedEventsList {};

	PhobosMap<int, CDTimerClass> SequentialTimers {};
	PhobosMap<int, int>          SequentialTimersOriginalValue {};

	PhobosMap<int, CDTimerClass> ParallelTimers {};
	PhobosMap<int, int>          ParallelTimersOriginalValue {};

	int SequentialSwitchModeIndex { -1 };

#pragma endregion


public:

	TriggerExtData(TriggerClass* pObj);
	TriggerExtData() = default;

	virtual ~TriggerExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TriggerExtData*>(this)->Internal_SaveToStream(Stm);
		const_cast<TriggerExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	TriggerClass* This() const { return reinterpret_cast<TriggerClass*>(this->AttachedToObject); }
	const TriggerClass* This_Const() const { return reinterpret_cast<const TriggerClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TriggerExtContainer final : public Container<TriggerExtData>
	, public ContainerSaveLoad<TriggerExtContainer, TriggerExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TriggerExtContainer";

public:
	static TriggerExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

};

class NOVTABLE FakeTriggerClass : public TriggerClass
{
public:
	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	void _Detach(AbstractClass* target, bool all);
	void _Reset();

	TriggerExtData* _GetExtData()
	{
		return *reinterpret_cast<TriggerExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTriggerClass) == sizeof(TriggerClass), "Invalid Size !");
