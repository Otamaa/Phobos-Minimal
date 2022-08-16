#pragma once
#include "Savegame.h"

#include <CoordStruct.h>

//with Array
#define GlobalBaseClassTemplate(ptr)\
	static std::vector<ptr*> Array;\
	static void PointerGotInvalid(void* ptr, bool bDetach);\
	static bool LoadGlobals(PhobosStreamReader& Stm);\
	static bool SaveGlobals(PhobosStreamWriter& Stm);\
	static void OnUpdateAll() ;\
	static void Clear() ;

//no Array
#define NoArrayGlobalBaseClassTemplate()\
	static void PointerGotInvalid(void* ptr, bool bDetach);\
	static bool LoadGlobals(PhobosStreamReader& Stm);\
	static bool SaveGlobals(PhobosStreamWriter& Stm);
/*
class BaseClassTemplate
{
public:

	virtual void InvalidatePointer(void* ptr, bool bDetach) = 0;
	virtual bool Load(PhobosStreamReader& stm, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stm) const = 0;

};

class SaveLoadBaseClassTemplate
{
public:

	virtual bool Load(PhobosStreamReader& stm, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stm) const = 0;

};

class CCINIClass;
class ReadableBaseClassTemplate
{
public:
	virtual void InvalidatePointer(void* ptr, bool bDetach) = 0;
	virtual bool Load(PhobosStreamReader& stm, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stm) const = 0;
	virtual bool Read(CCINIClass* const pINI, const char* pSection) = 0;
};

class AbstractClass;
class ObjectClass;
class TemporalClass;
class HouseClass;
class WarheadTypeClass;
struct args_ReceiveDamage;
class BaseBehaviourClass
{
public:

	virtual void OnInit() = 0;
	virtual void OnUnInit() = 0;

	virtual void OnDetonate(CoordStruct* location) = 0;

	virtual void OnPut(CoordStruct pCoord, short faceDirValue8) = 0;
	virtual void OnRemove() = 0;

	virtual void OnReceiveDamage(args_ReceiveDamage* args) = 0;

	virtual void OnFire(AbstractClass* pTarget, int weaponIndex) = 0;

	virtual void OnSelect(bool& selectable) = 0;
	virtual void OnGuardCommand() = 0;
	virtual void OnStopCommand() = 0;
	virtual void OnDeploy() = 0;

	virtual void OnTemporalUpdate(TemporalClass* pTemporal) = 0;
	virtual void OnUpdate() = 0;

};*/