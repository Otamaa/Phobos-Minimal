#pragma once
#include <SmudgeTypeClass.h>
#include <Ext/ObjectType/Body.h>

class SmudgeTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = SmudgeTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	Valueable<bool> Clearable;

public:
	SmudgeTypeExtData(SmudgeTypeClass* pObj) : ObjectTypeExtData(pObj) , Clearable (true)
	{
 		this->AbsType = SmudgeTypeClass::AbsID;
	}

	SmudgeTypeExtData(SmudgeTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~SmudgeTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<SmudgeTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<SmudgeTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual SmudgeTypeClass* This() const override { return reinterpret_cast<SmudgeTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const SmudgeTypeClass* This_Const() const override { return reinterpret_cast<const SmudgeTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SmudgeTypeExtContainer final : public Container<SmudgeTypeExtData>
{
public:
	static SmudgeTypeExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter & Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

};

class NOVTABLE FakeSmudgeTypeClass : public SmudgeTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _CanPlaceHere(CellStruct*origin, bool underbuildings);

	bool _ReadFromINI(CCINIClass* pINI);

	SmudgeTypeExtData* _GetExtData() {
		return *reinterpret_cast<SmudgeTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSmudgeTypeClass) == sizeof(SmudgeTypeClass), "Invalid Size !");