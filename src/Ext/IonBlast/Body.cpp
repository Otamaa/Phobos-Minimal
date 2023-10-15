#include "Body.h"
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <Ext/TAction/Body.h>

// invalidation and stuffs can be done easily 
// the thing is , how this behave on S/L
// this thing doesnt have any S/L at all ,..
//PhobosMap<IonBlastClass*, std::unique_ptr<IonBlastExt::ExtData>> IonBlastExt::IonExtMap;
//
//bool IonBlastExt::DoAffects(IonBlastClass* pIon)
//{
//	int nDamage = RulesClass::Instance->IonCannonDamage;
//	auto pWarhead = RulesClass::Instance->IonCannonWarhead;
//	auto pBlastAnim = RulesClass::Instance->IonBlast;
//	auto pBeamAnim = RulesClass::Instance->IonBeam;
//	bool bAllowWater = false;
//	bool bRockUnitAround = true;
//	HouseClass* pHouseOwner = nullptr;
//	TechnoClass* pTechnoOwner = nullptr; 
//
//	if (auto pData = IonBlastExt::Find(pIon))
//	{
//		Debug::Log_Masselist(__FUNCTION__" ExtData[%x] size [%d]",pData,pData->Size() );
//		if (auto pWarheadExt = pData->AttachedWarheadData)
//		{
//			if (pWarheadExt->IonWH.isset())
//				pWarhead = pWarheadExt->IonWH.Get();
//
//			if (pWarheadExt->IonBlast.isset())
//				pBlastAnim = pWarheadExt->IonBlast.Get();
//
//			if (pWarheadExt->IonBeam.isset())
//				pBeamAnim = pWarheadExt->IonBeam.Get();
//
//			if (pWarheadExt->IonDamage.isset())
//				nDamage = pWarheadExt->IonDamage.Get();
//
//			bAllowWater = pWarheadExt->IonBlastAllowWater.Get();
//		}
//
//		pHouseOwner = pData->IonHouseOwner;
//		pTechnoOwner = pData->TechnoOwner;
//	}
//
//	auto nAnimLoc = pIon->Location;
//		 nAnimLoc.Z += 5;
//
//	auto pCell = MapClass::Instance->TryGetCellAt(pIon->Location);
//
//	if (pCell->LandType == LandType::Water && !bAllowWater)
//		pBlastAnim = RulesClass::Instance->SplashList[ScenarioClass::Instance->Random((RulesClass::Instance->SplashList.Count - 1))];
//
//	if (pBeamAnim)
//		if (auto pBeam = GameCreate<AnimClass>(pBeamAnim, nAnimLoc))
//			AnimExtData::SetAnimOwnerHouseKind(pBeam, pHouseOwner, nullptr, AnimTypeExtContainer::Instance.Find(pBeam->Type)->Anim_Owner.Get(), false);
//
//	if (pBlastAnim)
//		if (auto pBlast = GameCreate<AnimClass>(pBlastAnim, nAnimLoc))
//			AnimExtData::SetAnimOwnerHouseKind(pBlast, pHouseOwner, nullptr, AnimTypeExtContainer::Instance.Find(pBlast->Type)->Anim_Owner.Get(), false);
//
//	if (pWarhead)
//	{
//		auto nLocation = pCell->GetCoordsWithBridge();
//		MapClass::DamageArea(nLocation, nDamage, pTechnoOwner, pWarhead, pWarhead->Tiberium, pHouseOwner);
//		MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
//	}
//
//	return bRockUnitAround;
//}
//
//DEFINE_HOOK(0x53CC0D, IonBlastClass_BlastAI_DTOR, 0x5)
//{
//	GET(IonBlastClass *, pThis, EBX);
//
//	IonBlastExt::RemoveFromMap(pThis);
//
//	return 0;
//}

/*
DEFINE_HOOK(0x53CC5B, IonBlastClass_BlastAI_BeforeAres, 0x5)
{
	GET(int, nState, EAX);
	GET(IonBlastClass*, pThis, EBX);

	return (nState) ? 0x53CE0A: ((IonBlastExt::DoAffects(pThis)) ? 0x53CE0A : 0x53D302);
}*/