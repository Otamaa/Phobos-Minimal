#pragma once

#include <TriggerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>

#include <TriggerTypeClass.h>

class TriggerClass;
class TriggerExtData final : public AbstractExtended
{
public:
	using base_type = TriggerClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	std::vector<TEventClass*> SortedEventsList;
	PhobosMap<int, CDTimerClass> SequentialTimers;
	PhobosMap<int, int> SequentialTimersOriginalValue;
	PhobosMap<int, CDTimerClass> ParallelTimers;
	PhobosMap<int, int> ParallelTimersOriginalValue;
	int SequentialSwitchModeIndex = -1;

public:
	TriggerExtData(TriggerClass* pObj) : AbstractExtended(pObj)
		, SortedEventsList {}
		, SequentialTimers {}
		, SequentialTimersOriginalValue {}
		, ParallelTimers {}
		, ParallelTimersOriginalValue {}
		, SequentialSwitchModeIndex { -1 }
	{
		this->Name = pObj->Type->ID;
		this->AbsType = TriggerClass::AbsID;
	}

	TriggerExtData(TriggerClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~TriggerExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

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

	TriggerClass* This() const override { return reinterpret_cast<TriggerClass*>(this->AttachedToObject); }
	const TriggerClass* This_Const() const override { return reinterpret_cast<const TriggerClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TriggerExtContainer final : public Container<TriggerExtData>
{
public:
	static TriggerExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

};

class NOVTABLE FakeTriggerClass : public TriggerClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	TriggerExtData* _GetExtData()
	{
		return *reinterpret_cast<TriggerExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTriggerClass) == sizeof(TriggerClass), "Invalid Size !");
