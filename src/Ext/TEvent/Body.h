#pragma once

#include <TEventClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/OptionalStruct.h>

class HouseClass;

enum class PhobosTriggerEvent : int
{
	DestroyedOnly = 499,

	LocalVariableGreaterThan = 500,
	LocalVariableLessThan = 501,
	LocalVariableEqualsTo = 502,
	LocalVariableGreaterThanOrEqualsTo = 503,
	LocalVariableLessThanOrEqualsTo = 504,
	LocalVariableAndIsTrue = 505,

	GlobalVariableGreaterThan = 506,
	GlobalVariableLessThan = 507,
	GlobalVariableEqualsTo = 508,
	GlobalVariableGreaterThanOrEqualsTo = 509,
	GlobalVariableLessThanOrEqualsTo = 510,
	GlobalVariableAndIsTrue = 511,

	LocalVariableGreaterThanLocalVariable = 512,
	LocalVariableLessThanLocalVariable = 513,
	LocalVariableEqualsToLocalVariable = 514,
	LocalVariableGreaterThanOrEqualsToLocalVariable = 515,
	LocalVariableLessThanOrEqualsToLocalVariable = 516,
	LocalVariableAndIsTrueLocalVariable = 517,

	GlobalVariableGreaterThanLocalVariable = 518,
	GlobalVariableLessThanLocalVariable = 519,
	GlobalVariableEqualsToLocalVariable = 520,
	GlobalVariableGreaterThanOrEqualsToLocalVariable = 521,
	GlobalVariableLessThanOrEqualsToLocalVariable = 522,
	GlobalVariableAndIsTrueLocalVariable = 523,

	LocalVariableGreaterThanGlobalVariable = 524,
	LocalVariableLessThanGlobalVariable = 525,
	LocalVariableEqualsToGlobalVariable = 526,
	LocalVariableGreaterThanOrEqualsToGlobalVariable = 527,
	LocalVariableLessThanOrEqualsToGlobalVariable = 528,
	LocalVariableAndIsTrueGlobalVariable = 529,

	GlobalVariableGreaterThanGlobalVariable = 530,
	GlobalVariableLessThanGlobalVariable = 531,
	GlobalVariableEqualsToGlobalVariable = 532,
	GlobalVariableGreaterThanOrEqualsToGlobalVariable = 533,
	GlobalVariableLessThanOrEqualsToGlobalVariable = 534,
	GlobalVariableAndIsTrueGlobalVariable = 535,

	ShieldBroken = 600,
	HouseOwnsTechnoType = 601,
	HouseDoesntOwnTechnoType = 602,

	//PR #1071
	HousesDestroyed = 603,

	CellHasTechnoType = 604,
	CellHasAnyTechnoTypeFromList = 605,

	AttachedIsUnderAttachedEffect = 606,

	ForceSequentialEvents = 1000,

	//PR #1925
	EnteredByByID = 19001,
	SpiedByByID = 19002,
	HouseDiscoveredByID = 19005,
	DestroyedUnitsAllByID = 19009,
	DestroyedBuildingsAllByID = 19010,
	DestroyedAllByID = 19011,
	BuildBuildingTypeByID = 19019,
	BuildUnitTypeByID = 19020,
	BuildInfantryTypeByID = 19021,
	BuildAircraftTypeByID = 19022,
	ZoneEntryByByID = 19024,
	CrossesHorizontalLineByID = 19025,
	CrossesVerticalLineByID = 19026,
	LowPowerByID = 19030,
	BuildingExistsByID = 19032,
	AttackedByHouseByID = 19044,
	SpyAsHouseByID = 19053,
	SpyAsInfantryByID = 19054,
	DestroyedUnitsNavalByID = 19055,
	DestroyedUnitsLandByID = 19056,
	BuildingDoesNotExistByID = 19057,
	PowerFullByID = 19058,
	EnteredOrOverflownByByID = 19059,
};

class TechnoTypeClass;
class TEventExtData final : public AbstractExtended
{
public:
	using base_type = TEventClass;
	static COMPILETIMEEVAL const char* ClassName = "TEventExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TEventClass";
	static COMPILETIMEEVAL DWORD Canary = 0x634EE2D7;
	
public:
	OptionalStruct<TechnoTypeClass*, false> TechnoType;

public:

	TEventExtData(TEventClass* pObj) : AbstractExtended(pObj) , TechnoType() {
		this->AbsType = TEventClass::AbsID;
	}

	TEventExtData() = default;

	virtual ~TEventExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TEventExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<TEventExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
	}

	TEventClass* This() const { return reinterpret_cast<TEventClass*>(this->AttachedToObject); }
	const TEventClass* This_Const() const { return reinterpret_cast<const TEventClass*>(this->AttachedToObject); }

public:

	// support
	TechnoTypeClass* GetTechnoType();

private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static std::pair<TriggerAttachType, bool> GetTriggetAttach(PhobosTriggerEvent nAction);
	static std::pair<LogicNeedType, bool> GetLogicNeed(PhobosTriggerEvent nAction);
	static std::pair<bool, bool> GetPersistableFlag(PhobosTriggerEvent nAction);

	static bool HousesAreDestroyedTEvent(TEventClass* pThis);
	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool AttachedIsUnderAttachedEffectTEvent(TEventClass* pThis, ObjectClass* pObject);

	static HouseClass* GetHouse(int TEvetValue, HouseClass* pEventHouse);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);

	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);

	// Resolves a param to a house.
	static HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse = nullptr);

	// original game using between 0 - 2 ?
	// why these were 256 257 ? , something not right ,..
	static std::pair<bool, bool> GetPersistableFlag(AresTriggerEvents nAction);
	static std::pair<LogicNeedType, bool >  GetLogicNeed(AresTriggerEvents nAction);
	static std::pair<TriggerAttachType, bool> GetAttachFlags(AresTriggerEvents nEvent);

	static bool FindTechnoType(TEventClass* pThis, int args, HouseClass* pWho);

	// the function return is deciding if the case is handled or not
	// the bool result pointer is for the result of the Event itself
	static bool AresTriggerEventOccured(TEventClass* pThis, EventArgs& Args, bool& result);
	static bool PhobosTriggerEventOccured(TEventClass* pThis, EventArgs const& args, bool& bHandled);
	static bool VanillaTriggerEventOccured(TEventClass* pThis, EventArgs& Args, bool& result);

	static LogicNeedType ClassifyEvent(int event);
};

class TEventExtContainer final : public Container<TEventExtData>
, public ContainerSaveLoad<TEventExtContainer, TEventExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TEventExtContainer";

public:
	static TEventExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

	virtual void LoadFromINI(AircraftTypeClass* key, CCINIClass* pINI, bool parseFailAddr) {}
	virtual void WriteToINI(AircraftTypeClass* key, CCINIClass* pINI) {}
};

class NOVTABLE FakeTEventClass : public TEventClass
{
public:
	static TriggerAttachType __fastcall AttachesTo(unsigned int a1);

public:

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	bool _Occured(TriggerEvent requestedEvent, HouseClass* house, ObjectClass* obj, CDTimerClass* td, bool * isPresistent, AbstractClass* source);
	bool _IsPresistable();
	bool _IsTemporal();

	void _ReadINI();
	std::string _BuildINIEntry();
	TEventExtData* _GetExtData() {
		return *reinterpret_cast<TEventExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTEventClass) == sizeof(TEventClass), "Invalid Size !");
