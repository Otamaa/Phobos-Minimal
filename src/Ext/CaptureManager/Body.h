#pragma once
#include <CaptureManagerClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <AnimTypeClass.h>
#include <RulesClass.h>
#include <FootClass.h>

class CaptureExtData final : public AbstractExtended
{
public:
	using base_type = CaptureManagerClass;
	static COMPILETIMEEVAL const char* ClassName = "CaptureExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "CaptureManagerClass";
	static COMPILETIMEEVAL DWORD Canary = 0xC8997DAE;
public:
	
public:

	CaptureExtData(CaptureManagerClass* pObj) : AbstractExtended(pObj)
	{
		this->AbsType = base_type::AbsID;
	}

	CaptureExtData() = default;

	virtual ~CaptureExtData() = default;

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
		const_cast<CaptureExtData*>(this)->Internal_SaveToStream(Stm);
		const_cast<CaptureExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {}

	CaptureManagerClass* This() const { return reinterpret_cast<CaptureManagerClass*>(this->AttachedToObject); }
	const CaptureManagerClass* This_Const() const { return reinterpret_cast<const CaptureManagerClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static AnimTypeClass* GetMindcontrollAnimType(TechnoClass* pController, TechnoClass* pTarget, AnimTypeClass* pFallback);
};

class CaptureExtContainer final : public Container<CaptureExtData>
	, public ContainerSaveLoad<CaptureExtContainer, CaptureExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "CaptureExtContainer";

public:
	static CaptureExtContainer Instance;
	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

};

class NOVTABLE FakeCaptureManagerClass : public CaptureManagerClass
{
public:
	bool __FreeUnit(TechnoClass* pTarget, bool bSilent);
	bool __FreeUnit_Wrap(TechnoClass* pTarget);
	bool __CanCapture(TechnoClass* pTarget);
	bool __CaptureUnit(TechnoClass* pTarget, bool bRemoveFirst, bool bSilent, AnimTypeClass* pControlledAnimType, int threatDelay);
	bool __CaptureUnit(TechnoClass* pTechno, bool bSilent, int threatDelay);
	bool __CaptureUnit_Wrap(AbstractClass* pTechno);
	void __DecideUnitFate(TechnoClass* pTechno, bool Captured);
	void __DecideUnitFate_Wrap(TechnoClass* pTechno);
	int __GetControlledCount();
	int __GetControlledTotalSize();
	bool __IsOverloading(bool* isIt);
	bool __CanControlMore();
	bool __Should_Draw_Link();
	void __DrawControlLinks();
	bool __SetOwnerToCivilian(TechnoClass* pTarget);
};