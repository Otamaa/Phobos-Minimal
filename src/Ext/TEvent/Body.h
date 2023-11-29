#pragma once

#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <Helpers/Template.h>

#include <TEventClass.h>

#include <Utilities/Constructs.h>

class HouseClass;

enum PhobosTriggerEvent
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

	count
};

class TechnoTypeClass;
class TEventExtData final
{
public:
	static constexpr size_t Canary = 0x91919191;
	using base_type = TEventClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	OptionalStruct<TechnoTypeClass*, false> TechnoType {};

	TEventExtData() noexcept = default;
	~TEventExtData() noexcept = default;

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	// support
	TechnoTypeClass* GetTechnoType();

private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static bool HousesAreDestroyedTEvent(TEventClass* pThis);
	static bool HouseOwnsTechnoTypeTEvent(TEventClass* pThis);
	static bool HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis);

	static bool Occured(TEventClass* pThis, EventArgs const& args, bool& bHandled);

	template<bool IsGlobal, typename _Pr>
	static bool VariableCheck(TEventClass* pThis);
	template<bool IsSrcGlobal, bool IsGlobal, typename _Pr>
	static bool VariableCheckBinary(TEventClass* pThis);
};

class TEventExtContainer final : public Container<TEventExtData>
{
public:
	static TEventExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(TEventExtContainer, TEventExtData, "TEventClass");
};