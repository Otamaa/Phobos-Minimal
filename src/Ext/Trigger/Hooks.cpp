#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>

#include <Misc/TriggerMPOwner.h>

DEFINE_HOOK(0x7272AE, TriggerTypeClass_LoadFromINI_House, 0x8)
{
	GET(TriggerTypeClass*, pThis, EBP);
	GET(const char*, pID, ESI);

	int idx = atoi(pID);

	if (HouseClass::PlayerAtA <= idx && idx <= HouseClass::PlayerAtH)
	{
		TriggerMPOwner::TriggerType_Owner[pThis->GetArrayIndex()] = idx;

		R->EDX(HouseTypeClass::Find("special"));

		return 0x7272C1;
	}

	R->EAX(HouseTypeClass::FindIndex(pID));

	return 0x7272B5;
}

DEFINE_HOOK(0x72612C, TriggerClass_CTOR_DestoryIfMultiplayerNonexist, 0x8)
{
	GET(TriggerClass*, pThis, ESI);

	if (pThis->Type == nullptr)
		return 0;

	int idx = pThis->Type->GetArrayIndex();
	const auto& houseIdxMapper = TriggerMPOwner::TriggerType_Owner;

	if (houseIdxMapper.count(idx))
	{
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper.at(idx));

		if (pHouse == nullptr)
			pThis->Destroyed = true;
	}

	return 0;
}


DEFINE_HOOK(0x726538, TriggerClass_RaiseEvent_ReplaceHouse, 0x5)
{
	GET(TriggerClass*, pThis, ESI);

	int idx = pThis->Type->GetArrayIndex();
	const auto& houseIdxMapper = TriggerMPOwner::TriggerType_Owner;

	if (houseIdxMapper.count(idx))
	{
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper.at(idx));
		R->EAX(pHouse == nullptr ? HouseExt::FindSpecial() : pHouse);
	}

	return 0;
}

DEFINE_HOOK(0x7265F7, TriggerClass_FireActions_ReplaceHouse, 0x6)
{
	GET(TriggerClass*, pThis, EDI);

	int idx = pThis->Type->GetArrayIndex();
	const auto& houseIdxMapper = TriggerMPOwner::TriggerType_Owner;

	if (houseIdxMapper.count(idx))
	{
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper.at(idx));
		R->EAX(pHouse == nullptr ? HouseExt::FindSpecial() : pHouse);

		return 0x726602;
	}

	return 0;
}

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrue ||
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable ||
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::LocalSet) ?
		0x72706E :
		0x727069;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrue ||
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable ||
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::GlobalSet) ?
		0x72702E :
		0x727029;
}