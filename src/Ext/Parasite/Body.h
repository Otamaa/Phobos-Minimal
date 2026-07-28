#pragma once
#include <ParasiteClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParasiteExtData final : public AbstractExtended
{
public:
	static COMPILETIMEEVAL DWORD Canary = 0xE609F40B;
	using base_type = ParasiteClass;
	static COMPILETIMEEVAL const char* ClassName = "ParasiteExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "ParasiteClass";
public:

	bool CreateParticleSystems { true };

public:
	ParasiteExtData(ParasiteClass* pObj) : AbstractExtended(pObj)
	{
		this->AbsType = base_type::AbsID;
	}

	ParasiteExtData() = default;

	virtual ~ParasiteExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType type) override
	{

	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<ParasiteExtData*>(this)->Internal_SaveToStream(Stm);
		const_cast<ParasiteExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {}

	ParasiteClass* This() const { return reinterpret_cast<ParasiteClass*>(this->AttachedToObject); }
	const ParasiteClass* This_Const() const { return reinterpret_cast<const ParasiteClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};


class ParasiteExtContainer final : public Container<ParasiteExtData>
	, public ContainerSaveLoad<ParasiteExtContainer, ParasiteExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "ParasiteExtContainer";

public:
	static ParasiteExtContainer Instance;
	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

};

class FakeParasiteClass : public ParasiteClass { 
public:

	FORCEDINLINE FakeParasiteClass* _AsParasite() const
	{
		return ((FakeParasiteClass*)this);
	}

	CoordStruct __Detach_From_Victim();
	bool __Victims_Cell_Valid();
	void __Infect(FootClass* target);
	void __Uninfect();
	void __Detach(AbstractClass* detachingObject, bool permanent);
	void __AI();
	void __Grapple_AI();
	bool __Update_GrappleAnim_Frame();
	void __ClearAnim();

	HRESULT __stdcall _Load(IStream* pStm)
	{
		return S_OK;
	}

	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty)
	{
		return S_OK;
	}

};
static_assert(sizeof(FakeParasiteClass) == sizeof(ParasiteClass), "FakeParasiteClass size mismatch");