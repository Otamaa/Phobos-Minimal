#pragma once

#include <TEventClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/OptionalStruct.h>

class HouseClass;

enum class PhobosTriggerEvent : int
{
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

	HousesDestroyed = 603,

	CellHasTechnoType = 604,
	CellHasAnyTechnoTypeFromList = 605,

	AttachedIsUnderAttachedEffect = 606,

	ForceSequentialEvents = 1000,

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

	count
};

class TechnoTypeClass;
class TEventExtData final : public AbstractExtended
{
public:
	using base_type = TEventClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	OptionalStruct<TechnoTypeClass*, false> TechnoType;

public:

	TEventExtData(TEventClass* pObj) : AbstractExtended(pObj) , TechnoType() {
		this->AbsType = TEventClass::AbsID;
	}
	TEventExtData(TEventClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~TEventExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
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

	virtual TEventClass* This() const override { return reinterpret_cast<TEventClass*>(this->AbstractExtended::This()); }
	virtual const TEventClass* This_Const() const override { return reinterpret_cast<const TEventClass*>(this->AbstractExtended::This_Const()); }

public:

	// support
	TechnoTypeClass* GetTechnoType();

private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static std::pair<TriggerAttachType, bool> GetTriggetAttach(PhobosTriggerEvent nAction)
	{
		switch (nAction)
		{
		case PhobosTriggerEvent::LocalVariableGreaterThan:
		case PhobosTriggerEvent::LocalVariableLessThan:
		case PhobosTriggerEvent::LocalVariableEqualsTo:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableAndIsTrue:
		case PhobosTriggerEvent::GlobalVariableGreaterThan:
		case PhobosTriggerEvent::GlobalVariableLessThan:
		case PhobosTriggerEvent::GlobalVariableEqualsTo:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
			return { TriggerAttachType::Logic , true };
		case PhobosTriggerEvent::HouseOwnsTechnoType:
		case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
		case PhobosTriggerEvent::HousesDestroyed:
			return { TriggerAttachType::House , true };
		case PhobosTriggerEvent::CellHasTechnoType:
		case PhobosTriggerEvent::CellHasAnyTechnoTypeFromList:
			return { TriggerAttachType::Object , true };
		case PhobosTriggerEvent::AttachedIsUnderAttachedEffect:
			return { TriggerAttachType::Object , true };
		case PhobosTriggerEvent::ShieldBroken:
			return { TriggerAttachType::None , true };
		default:
			return { TriggerAttachType::None , false };
		}
	}

	static std::pair<LogicNeedType, bool> GetLogicNeed(PhobosTriggerEvent nAction)
	{
		switch (nAction)
		{
		case PhobosTriggerEvent::LocalVariableGreaterThan:
		case PhobosTriggerEvent::LocalVariableLessThan:
		case PhobosTriggerEvent::LocalVariableEqualsTo:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableAndIsTrue:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThan:
		case PhobosTriggerEvent::GlobalVariableLessThan:
		case PhobosTriggerEvent::GlobalVariableEqualsTo:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableAndIsTrue:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
			return { LogicNeedType::NumberNTech , true };

		case PhobosTriggerEvent::HouseOwnsTechnoType:
		case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
		case PhobosTriggerEvent::HousesDestroyed:
			return { LogicNeedType::House , true };

		case PhobosTriggerEvent::CellHasTechnoType:
		case PhobosTriggerEvent::CellHasAnyTechnoTypeFromList:
			return { LogicNeedType::Cell , true };

		case PhobosTriggerEvent::AttachedIsUnderAttachedEffect:
			return { LogicNeedType::None , true };

		case PhobosTriggerEvent::ShieldBroken:
			return { LogicNeedType::None , true };

		default:
			return { LogicNeedType::None , false };
		}
	}

	static std::pair<bool, bool> GetPersistableFlag(PhobosTriggerEvent nAction)
	{
		return { true , true };
	}

	static bool HousesAreDestroyedTEvent(TEventClass* pThis);
	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool AttachedIsUnderAttachedEffectTEvent(TEventClass* pThis, ObjectClass* pObject);

	static bool Occured(TEventClass* pThis, EventArgs const& args, bool& bHandled);

	static HouseClass* GetHouse(int TEvetValue, HouseClass* pEventHouse);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);

	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);
};

class TEventExtContainer final : public Container<TEventExtData>
{
public:
	static TEventExtContainer Instance;

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

class NOVTABLE FakeTEventClass : public TEventClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _Occured(TriggerEvent event, HouseClass* house, ObjectClass* obj, CDTimerClass* td, bool* bool1, AbstractClass* source);

	TEventExtData* _GetExtData() {
		return *reinterpret_cast<TEventExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTEventClass) == sizeof(TEventClass), "Invalid Size !");
