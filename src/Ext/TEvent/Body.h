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

	static bool HousesAreDestroyedTEvent(TEventClass* pThis);
	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);
	static bool CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pHouse);

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
