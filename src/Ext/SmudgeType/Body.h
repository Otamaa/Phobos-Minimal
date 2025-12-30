#pragma once
#include <SmudgeTypeClass.h>
#include <Ext/ObjectType/Body.h>

class SmudgeTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = SmudgeTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "SmudgeTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "SmudgeTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

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

	SmudgeTypeClass* This() const { return reinterpret_cast<SmudgeTypeClass*>(this->AttachedToObject); }
	const SmudgeTypeClass* This_Const() const { return reinterpret_cast<const SmudgeTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SmudgeTypeExtContainer final : public Container<SmudgeTypeExtData>
	, public ReadWriteContainerInterfaces<SmudgeTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "SmudgeTypeExtContainer";
	using ext_t = SmudgeTypeExtData;

public:
	static SmudgeTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeSmudgeTypeClass : public SmudgeTypeClass
{
public:

	bool _CanPlaceHere(CellStruct*origin, bool underbuildings);

	bool _ReadFromINI(CCINIClass* pINI);

	SmudgeTypeExtData* _GetExtData() {
		return *reinterpret_cast<SmudgeTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSmudgeTypeClass) == sizeof(SmudgeTypeClass), "Invalid Size !");