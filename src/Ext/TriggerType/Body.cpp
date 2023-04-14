#include "Body.h"

#include <string>
#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

#include <TriggerClass.h>
#include <TagTypeClass.h>

TriggerTypeExt::ExtContainer TriggerTypeExt::ExtMap;

HouseClass* TriggerTypeExt::ResolveHouseParam(int const param, HouseClass* const pOwnerHouse)
{
	if (param == 8997) {
		return pOwnerHouse;
	}

	HouseClass* const result = HouseClass::Index_IsMP(param) ?
		HouseClass::FindByIndex(param) : HouseClass::FindByCountryIndex(param);
	return !result ? pOwnerHouse : result;
}

//DEFINE_HOOK(0x7272AE ,TriggerTypeClass_LoadFromINI_CountryName, 7)
//{
//	GET(TriggerTypeClass*, pThis, EBP);
//	GET(const char*, pData, ESI);
//	const int nParam = atoi(pData);
//
//	Debug::Log("TriggerTypeClass_LoadFromINI_CountryName[%s] %s - %d \n", pThis->ID, pData, nParam);
//
//	if ((nParam - 4475) > 7) {
//		R->EDX(HouseTypeClass::Array->GetItem(HouseTypeClass::FindIndexByIdAndName(pData)));
//		return 0x7272C1;
//	} else {
//		TriggerTypeExt::ExtMap.Find(pThis)->HouseParam = nParam;
//		return 0x7272A4;
//	}
//}
//
//DEFINE_HOOK(0x7265E7, TriggerClass_FireActions, 7)
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
//DEFINE_HOOK(0x72652D, TriggerClass_RegisterEvent_PlayerX, 6)
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
//DEFINE_HOOK(0x684E44 , GameInitialize_AddTagsForHouse, 5)
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
// load / save

void TriggerTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TriggerTypeClass>::LoadFromStream(Stm);
	// Nothing yet
}

void TriggerTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TriggerTypeClass>::SaveToStream(Stm);
	// Nothing yet
}

// =============================
// container

TriggerTypeExt::ExtContainer::ExtContainer() : Container("TriggerTypeClass") { }
TriggerTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x726DD7, TriggerTypeClass_CTOR, 6)
{
	GET(TriggerTypeClass*, pThis, ESI);
	TriggerTypeExt::ExtMap.Allocate(pThis);
	return 0x0;
}

DEFINE_HOOK(0x726F46, TriggerTypeClass_DTOR, 5)
{
	GET(TriggerTypeClass*, pThis, EDI);
	TriggerTypeExt::ExtMap.Remove(pThis);
	return 0x0;
}

DEFINE_HOOK_AGAIN(0x727C80, TriggerTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(0x727BF0, TriggerTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TriggerTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TriggerTypeExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x727C75, TriggerTypeClass_Load_Suffix, 5)
{
	TriggerTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x727C9A, TriggerTypeClass_Save_Suffix, 5)
{
	TriggerTypeExt::ExtMap.SaveStatic();
	return 0;
}