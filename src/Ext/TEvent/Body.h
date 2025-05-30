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

	count
};

class TechnoTypeClass;
class TEventExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x91919191;
	using base_type = TEventClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	OptionalStruct<TechnoTypeClass*, false> TechnoType {};

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	// support
	TechnoTypeClass* GetTechnoType();

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TEventExtData) -
			(4u //AttachedToObject
			 );
	}
private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static std::pair<TriggerAttachType, bool> GetFlag(PhobosTriggerEvent nAction)
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

	static std::pair<LogicNeedType, bool> GetMode(PhobosTriggerEvent nAction)
	{
		switch (nAction)
		{
		case PhobosTriggerEvent::LocalVariableGreaterThan:
		case PhobosTriggerEvent::LocalVariableLessThan:
		case PhobosTriggerEvent::LocalVariableEqualsTo:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::LocalVariableAndIsTrue:
			return { LogicNeedType::Local , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThan:
		case PhobosTriggerEvent::GlobalVariableLessThan:
		case PhobosTriggerEvent::GlobalVariableEqualsTo:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		case PhobosTriggerEvent::GlobalVariableAndIsTrue:
			return { LogicNeedType::Global , true };

		case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
			return { LogicNeedType::Local , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
			return { LogicNeedType::Global , true };

		case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
			return { LogicNeedType::Local , true };

		case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
			return { LogicNeedType::Global , true };

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

	//CONSTEXPR_NOCOPY_CLASSB(TEventExtContainer, TEventExtData, "TEventClass");
};

class NOVTABLE FakeTEventClass : public TEventClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	TEventExtData* _GetExtData() {
		return *reinterpret_cast<TEventExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTEventClass) == sizeof(TEventClass), "Invalid Size !");
