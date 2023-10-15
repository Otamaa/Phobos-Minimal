#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>

#include <Misc/TriggerMPOwner.h>


// PR #746: [QOL] Trigger owner can set as player of multiplayer game map.
DEFINE_HOOK(0x7272B5, TriggerTypeClass_FillIn_HouseType, 0x6)
{
	GET(int, nIndex, EAX);

	// Only if the house wasn't found
	if (nIndex == -1)
	{
		GET(const char*, pString, ECX);
		nIndex = HouseClass::GetPlayerAtFromString(pString);
		if (nIndex == -1)
		{
			nIndex = atoi(pString);

			if (nIndex == -1)
				nIndex = 0; // process it like <none>
		}

		if (HouseClass::IsPlayerAtType(nIndex))
		{
			if (auto pHouse = HouseClass::FindByPlayerAt(nIndex))
				nIndex = pHouse->Type->ArrayIndex;
		}
		else
			nIndex = 0;

		R->EAX(nIndex);
	}

	return 0;
}

DEFINE_HOOK(0x7272AE, TriggerTypeClass_LoadFromINI_House, 0x7) //8
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

DEFINE_HOOK(0x72612C, TriggerClass_CTOR_DestoryIfMultiplayerNonexist, 0x7) //8
{
	GET(TriggerClass*, pThis, ESI);

	if (pThis->Type == nullptr)
		return 0;

	int idx = pThis->Type->GetArrayIndex();
	const auto& houseIdxMapper = TriggerMPOwner::TriggerType_Owner;

	if (houseIdxMapper.count(idx))
	{
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper[idx]);

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
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper[idx]);
		R->EAX(pHouse == nullptr ? HouseExtData::FindSpecial() : pHouse);
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
		HouseClass* pHouse = HouseClass::FindByIndex(houseIdxMapper[idx]);
		R->EAX(pHouse == nullptr ? HouseExtData::FindSpecial() : pHouse);

		return 0x726602;
	}

	return 0;
}
