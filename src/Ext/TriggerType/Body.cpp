#include "Body.h"

#include <string>
#include <Ext/Rules/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/TAction/Body.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

#include <TriggerClass.h>
#include <TagTypeClass.h>

HouseClass* TriggerTypeExtData::ResolveHouseParam(int const param, HouseClass* const pOwnerHouse)
{
	if (param == 8997) {
		return pOwnerHouse;
	}

	HouseClass* const result = HouseClass::Index_IsMP(param) ?
		HouseClass::FindByIndex(param) : HouseClass::FindByCountryIndex(param);
	return !result ? pOwnerHouse : result;
}
//
//ASMJIT_PATCH(0x7265E7, TriggerClass_FireActions, 7)
//{
//	GET(TriggerClass*, pThis, EDI);
//
//	const auto pExt = TriggerTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pExt->HouseParam == -1)
//		return 0x0;
//
//	const auto pHouse =
//		TriggerTypeExt::ResolveHouseParam(pExt->HouseParam, pThis->House ?
//			HouseClass::FindByCountryIndex(pThis->House->ArrayIndex) : nullptr);
//
//	GET(TActionClass*, pAction, ESI);
//	GET(ObjectClass*, pObject, EBP);
//	LEA_STACK(CellStruct*, pCell, 0x18);
//
//	return pAction->Occured(pHouse, pObject, pThis, pCell) ?
//		0x72660E : 0x726610;
//}
//
//ASMJIT_PATCH(0x72652D, TriggerClass_RegisterEvent_PlayerX, 6)
//{
//	GET(TriggerClass*, pThis, ESI);
//
//	const auto pExt = TriggerTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pExt->HouseParam == -1)
//		return 0x0;
//
//	const auto pHouse =
//		TriggerTypeExt::ResolveHouseParam(pExt->HouseParam,nullptr);
//
//	if (!pHouse)
//		return 0x0;
//
//	R->EAX(pHouse);
//
//	return 0x726538;
//}
//
//ASMJIT_PATCH(0x684E44 , GameInitialize_AddTagsForHouse, 5)
//{
//	GET(TagClass*, pTag, EAX);
//
//	const auto pExt = TriggerTypeExt::ExtMap.Find(pTag->Type->FirstTrigger);
//
//	if (pExt->HouseParam == -1)
//		return 0x0;
//
//	const auto pHouse =
//		TriggerTypeExt::ResolveHouseParam(pExt->HouseParam, nullptr);
//
//	if (!pHouse)
//		return 0x0;
//
//	pHouse->RelatedTags.AddItem(pTag);
//	return 0x684EA2;
//}

// =============================
// container
/*
TriggerTypeExt::ExtContainer TriggerTypeExt::ExtMap;

ASMJIT_PATCH(0x726DE6, TriggerTypeClass_CTOR, 6)
{
	GET(TriggerTypeClass*, pThis, ESI);
	TriggerTypeExt::ExtMap.Allocate(pThis);
	return 0x0;
}

ASMJIT_PATCH(0x726EAC, TriggerTypeClass_DTOR, 6)
{
	GET(TriggerTypeClass*, pThis, EDI);
	TriggerTypeExt::ExtMap.Remove(pThis);
	return 0x0;
}

ASMJIT_PATCH_AGAIN(0x727C80, TriggerTypeClass_SaveLoad_Prefix, 8)
ASMJIT_PATCH(0x727BF0, TriggerTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TriggerTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TriggerTypeExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

ASMJIT_PATCH(0x727C73, TriggerTypeClass_Load_Suffix, 6)
{
	GET(HRESULT, res, EAX);

	if (SUCCEEDED(res))
		TriggerTypeExt::ExtMap.LoadStatic();

	return 0;
}

ASMJIT_PATCH(0x727C94, TriggerTypeClass_Save_Suffix, 6)
{
	GET(HRESULT, res, EAX);

	if (SUCCEEDED(res)) {
		TriggerTypeExt::ExtMap.SaveStatic();
		R->EAX(0x0);
	}

	return 0x727C9A;
}*/

bool FakeTriggerTypeClass::_SaveToINI(CCINIClass* pINIs)
{
	// Vanilla: v3 = AttachedTrigger; v4 = v3->IniName; if (!v3) v4 = "<none>"
	// IDA had null check after dereference — reconstruction error, fixed here.
	const char* const attachedName = this->NextTrigger
		? this->NextTrigger->ID
		: "<none>";

	const char* const houseName = this->House
		? this->House->ID
		: "<none>";

	// --- [Triggers] ---
	// Vanilla: sprintf(a2, "%s,%s,%s,%d,%d,%d,%d,%d", ...)
	// IsActive == 0 written as disabled flag (inverted).
	const std::string triggerEntry = fmt::format("{},{},{},{},{},{},{},{}",
		houseName,
		attachedName,
		this->ID, 
		this->Enabled == 0,
		this->Difficulty[0] != 0,
		this->Difficulty[1] != 0,
		this->Difficulty[2] != 0,
		this->MustTransfer != 0);

	pINIs->WriteString("Triggers", this->ID, triggerEntry.c_str());

	// --- [Events] ---
	// Vanilla: count loop -> sprintf(a2,"%d",count) -> strcat/Build_INI_Entry loop.
	int eventCount = 0;
	for (TEventClass* e = this->FirstEvent; e; e = e->NextEvent)
		++eventCount;

	std::string eventEntry = fmt::format("{}", eventCount);

	for (FakeTEventClass* e = (FakeTEventClass*)this->FirstEvent; e; e = (FakeTEventClass*)e->NextEvent)
		eventEntry += ',' + e->_BuildINIEntry();

	pINIs->WriteString("Events", this->ID, eventEntry.c_str());

	// --- [Actions] ---
	// Same pattern as events.
	int actionCount = 0;
	for (TActionClass* a = this->FirstAction; a; a = a->NextAction)
		++actionCount;

	std::string actionEntry = fmt::format("{}", actionCount);

	for (FakeTActionClass* a = (FakeTActionClass*)this->FirstAction; a; a = (FakeTActionClass*)a->NextAction)
		actionEntry += ',' + a->_BuildINIEntry();

	pINIs->WriteString("Actions", this->ID, actionEntry.c_str());

	return 1;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F596C, FakeTriggerTypeClass::_SaveToINI)
DEFINE_FUNCTION_JUMP(LJMP, 0x7276A0, FakeTriggerTypeClass::_SaveToINI)

static bool Is_Global_Variable_Event(TriggerEvent event)
{
	switch (event)
	{
	case TriggerEvent::GlobalSet:
	case TriggerEvent::GlobalCleared:
		return true;
	default:
		break;
	}

	//i reckon they has none so eh
	switch ((AresTriggerEvents)event)
	{
	default:
		break;
	}

	//this the only i know
	switch (static_cast<PhobosTriggerEvent>(event))
	{
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
	case PhobosTriggerEvent::GlobalVariableLessThan:
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		return true;

	default:
		break;
	}

	return false;
}


static bool Is_Local_Variable_Event(TriggerEvent event)
{
	switch (event)
	{
	case TriggerEvent::LocalSet:
		case TriggerEvent::LocalCleared:
		return true;
	default:
		break;
	}

	//i reckon they has none so eh
	switch ((AresTriggerEvents)event)
	{
	default:
		break;
	}

	//this the only i know
	switch (static_cast<PhobosTriggerEvent>(event))
	{
	case PhobosTriggerEvent::LocalVariableGreaterThan:
	case PhobosTriggerEvent::LocalVariableLessThan:
	case PhobosTriggerEvent::LocalVariableEqualsTo:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		return true;

	default:
		break;
	}

	return false;
}

bool FakeTriggerTypeClass::_HasGlobalSetOrClearedEvent(int global)
{
	bool tied = false;
	TEventClass* event = this->FirstEvent;
	while (event != nullptr) {
		if (Is_Global_Variable_Event(event->EventKind) && event->Value == global) {
			tied = true;
			break;
		}
		event = event->NextEvent;
	}
	return tied;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x727010, FakeTriggerTypeClass::_HasGlobalSetOrClearedEvent)

bool FakeTriggerTypeClass::_HasLocalSetOrClearedEvent(int local)
{
	bool tied = false;
	TEventClass* event = this->FirstEvent;
	while (event != nullptr) {
		if (Is_Local_Variable_Event(event->EventKind) && event->Value == local) {
			tied = true;
			break;
		}
		event = event->NextEvent;
	}

	return tied;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x727050, FakeTriggerTypeClass::_HasLocalSetOrClearedEvent)