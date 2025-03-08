#include <TechnoClass.h>

#include <Utilities/Macro.h>

#include <Ext/WeaponType/Body.h>
#include <Misc/DynamicPatcher/Techno/DamageSelf/DamageSelfType.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/DynamicPatcher/Techno/ExtraFire/ExtraFirefunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTargetFunctional.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
//#include <Misc/DynamicPatcher/Techno/FireSW/FireSWFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>
//#include <Misc/DynamicPatcher/Techno/AttackBeacon/AttackBeaconFunctional.h>

#include <SpawnManagerClass.h>

/*
DEFINE_HOOK(0x4149EE, AircraftClass_Render2, 0x5)
{
	GET(AircraftClass*, pThis, EBP);
	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	return 0x4149F5;
}

DEFINE_HOOK(0x519616, InfantryClass_Render2, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	R->ECX(pThis);
	return 0x51961F;
}

DEFINE_HOOK(0x73D40C, UnitClass_Render2, 0x7)
{
	GET(UnitClass*, pThis, ESI);

	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	R->ECX(pThis);
	return 0x73D415;
}

DEFINE_HOOK(0x730F1C, ObjectClass_StopCommand, 0x5)
{
	GET(ObjectClass*, pObject, ESI);

	if (auto pTechno = generic_cast<TechnoClass*>(pObject))
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pTechno);
		if (auto pManager = pExt->AnotherData.MyManager.get())
			pManager->StopCommand();
	}
	return 0;
}


DEFINE_HOOK(0x69252D, ScrollClass_ProcessClickCoords_VirtualUnit, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt && pExt->VirtualUnit ? 0x6925E6 : 0x0;
}
#endif



static bool CeaseFire(TechnoClass* pThis)
{
	bool bCeaseFire = false;
	PassengersFunctional::CanFire(pThis, bCeaseFire);
	return bCeaseFire;
}

DEFINE_HOOK(0x6FC339, TechnoClass_CanFire_DP, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	//GET(WeaponTypeClass*, pWeapon, EDI);
	//GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	return CeaseFire(pThis) ?//
		0x6FCB7E
		: 0x0;
}
*/

 DEFINE_HOOK(0x6B743E, SpawnManagerAI_SpawnSupportFLH, 0x6)
 {
 	GET(SpawnManagerClass*, pSpawn, ESI);
 	//GET_STACK(int, nArrIdx, STACK_OFFS(0x68, 0x54));

 	if (auto pOwner = pSpawn->Owner)
 	{
 		//if ((*pFLH) == CoordStruct::Empty)
 		{
 			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());

 			if (pTypeExt->MySpawnSupportDatas.Enable)
 			{
 				//CoordStruct nFLH = CoordStruct::Empty;

 				SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;
 				if (auto const pTransporter = pOwner->Transporter)
 				{
 					if (auto const pTransportExt = TechnoTypeExtContainer::Instance.Find(pTransporter->GetTechnoType()))
 					{
 						nFLHData = pTransportExt->MySpawnSupportFLH;
 					}
 				}

 				CoordStruct nFLH = pOwner->Veterancy.IsElite() ? nFLHData.EliteSpawnSupportFLH : nFLHData.SpawnSupportFLH;

 				if (nFLH == CoordStruct::Empty)
 					return 0x0;

 				if (auto pSpawnExt = TechnoExtContainer::Instance.Find(pOwner))
 				{
 					if (pTypeExt->MySpawnSupportDatas.SwitchFLH)
 					{
 						nFLH.Y *= pSpawnExt->MySpawnSuport.supportFLHMult;
 						pSpawnExt->MySpawnSuport.supportFLHMult *= -1;
 					}
 				}

 				R->EAX(&nFLH);
 				return 0x6B7498;
 			}
 		}
 	}


 	return 0x0;
 }

namespace CalculatePinch
{
	static void Calc(TechnoClass* pFirer, int nWeaponIdx)
	{
		const auto pWeapon = pFirer->GetWeapon(nWeaponIdx);

		{
			if (!pFirer->IsVoxel())
				return;

			const auto ext = WeaponTypeExtContainer::Instance.Find(pWeapon->WeaponType);

			{
				if (Math::abs(ext->RockerPitch.Get()) < 0.005)
					return;

				const double theta = pFirer->GetRealFacing().GetRadian<32>() -
					pFirer->PrimaryFacing.Current().GetRadian<32>();

				pFirer->AngleRotatedForwards = (float)(-ext->RockerPitch.Get() * Math::cos(theta));
				pFirer->AngleRotatedSideways = (float)(ext->RockerPitch.Get() * Math::sin(theta));

			}
		}
	}
}

// DEFINE_HOOK(0x5F45A0, TechnoClass_Selectable_Early , 0x7)
// {
// 	GET(TechnoClass*, pThis, EDI);

// 	const auto pUnit = specific_cast<UnitClass*>(pThis);
// 	const bool bNotSlectable = pUnit && pUnit->DeathFrameCounter > 0;

// 	bool Slectable = true;


// 	if (auto pExt = TechnoExtContainer::Instance.Find(pThis))
// 		Slectable = !pExt->VirtualUnit.Get();


// 	return Slectable || !bNotSlectable ? 0x0 : 0x5F45A9;

// 	if(!bNotSlectable)
// 		return 0x5F45A9;

// 	return 0x0;
// }

DEFINE_HOOK(0x6FDD50, TechnoClass_FireAt_PreFire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(const int, nWeapon, 0x8);
	//GET(AbstractClass*, pTarget, EDI);

	CalculatePinch::Calc(pThis, nWeapon);
	ExtraFirefunctional::GetWeapon(pThis, pTarget, nWeapon);
	//FireSWFunctional::OnFire(pThis, pTarget, nWeapon);
	SpawnSupportFunctional::OnFire(pThis, pTarget);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	//if ()
	//{
		pExt->CurrentWeaponIdx = nWeapon;
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		{
			AircraftDiveFunctional::OnFire(pExt, pTypeExt, pTarget, nWeapon);
			//AttackBeaconFunctional::OnFire(pExt, pTarget, nWeapon);
		}
	//}

	return 0x0;
}

static WeaponStruct* __fastcall GetWeapon_(TechnoClass* pTech, void*, int idx) {
	return pTech->GetWeapon(TechnoExtContainer::Instance.Find(pTech)->CurrentWeaponIdx);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x6FDD69, GetWeapon_);

 DEFINE_HOOK(0x6F6CA0, TechnoClass_Unlimbo_Early, 0x7)
 {
 	GET(TechnoClass*, pThis, ECX);
 	GET_STACK(CoordStruct*, pCoord, (0x4));
 	//GET_STACK(DirType, faceDir, (0x8));

	auto pExt = TechnoExtContainer::Instance.Find(pThis); {
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType()); {
 			DamageSelfState::OnPut(pExt->DamageSelfState, pTypeExt->DamageSelfData);
 			GiftBoxFunctional::Init(pExt, pTypeExt);
 			AircraftPutDataFunctional::OnPut(pExt, pTypeExt, pCoord);
 		}
 	}

 	return 0;
 }

DEFINE_HOOK(0x6FBFE9, TechnoClass_Select_SkipVoice, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return TechnoExtContainer::Instance.Find(pThis)->SkipVoice ? 0x6FC01E :0x0;
}

WeaponTypeClass* GetWeaponType(TechnoClass* pThis, int which)
{
	WeaponTypeClass* pBuffer = nullptr;

    if ( which == -1 ) {
        auto const pType = pThis->GetTechnoType();

        if (pType->TurretCount > 0) {
			if (auto const pCurWeapon = pThis->GetWeapon(pThis->CurrentGattlingStage)) {
				pBuffer = pCurWeapon->WeaponType;
			}
		} else {
            if (auto const pPriStruct = pThis->GetWeapon(0)) {
				pBuffer = pPriStruct->WeaponType;
			}

            if (auto const pSecStruct = pThis->GetWeapon(1) ) {
				pBuffer = pSecStruct->WeaponType;
            }
        }
    }
    else
    {
        if (auto const pSelected = pThis->GetWeapon(which) )
        {
            pBuffer = pSelected->WeaponType;
		}
    }

    return  pBuffer;
}
//9
 DEFINE_HOOK(0x6F9039, TechnoClass_Greatest_Threat_GuardRange, 0x9)
 {
 	GET(TechnoClass*, pTechno, ESI);
 	auto const pTypeGuardRange = pTechno->GetTechnoType()->GuardRange;
 	auto nGuarRange = pTypeGuardRange == -1 ? 512 : pTypeGuardRange;

 	if (auto pPri = GetWeaponType(pTechno , 0)) {
 		if (pPri->Range > nGuarRange)
 			nGuarRange = pPri->Range;
 	}

 	if(auto pSec = GetWeaponType(pTechno ,1)) {
 		if (pSec->Range > nGuarRange)
 			nGuarRange = pSec->Range;
 	}

 	R->EDI(nGuarRange);
 	return 0x6F903E;
 }

// DEFINE_HOOK(0x41A697, AircraftClass_Mission_Guard_NoTarget_Enter , 6)
// {
// 	GET(TechnoClass*, pTechno, ESI);
//
// 	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
//
// 	if (!pExt->MyFighterData)
// 		return 0x0;
//
// 	if(pExt->MyFighterData->IsAreaGuardRolling())
// 		return 0x41A6AC;
//
// 	return 0;
// }

//DEFINE_HOOK(0x4CF780, FlyLocomotionClass_Draw_Matrix_Rolling , 5)
//{
//	GET(ILocomotion*, Iloco, ESI);
//
//	const FlyLocomotionClass* pFly = static_cast<FlyLocomotionClass*>(Iloco);
//	auto pExt = TechnoExtContainer::Instance.Find(pFly->LinkedTo);
//
//	if (!pExt->MyFighterData)
//		return 0x0;
//
//	if (pFly->LinkedTo->GetTechnoType()->RollAngle != 0) {
//		auto const& pData  = pExt->MyFighterData;
//		if (pData->State == AircraftGuardState::ROLLING) {
//			return pData->Clockwise ? 0x4CF7B0 : 0x4CF7DF;
//		}
//	}
//
//	return 0;
//}

//DEFINE_HOOK_AGAIN(0x730DEB, ObjectClass_GuardCommand ,6 ) //Building
//DEFINE_HOOK(0x730E56, ObjectClass_GuardCommand , 6)
//{
//	GET(ObjectClass*, pObj, ESI);
//
//	if (auto pTechno = generic_cast<TechnoClass*>(pObj)) {
//
//	}
//
//   return 0;
//}

//DEFINE_HOOK(0x730EEB, ObjectClass_StopCommand, 6)
//{
//	GET(ObjectClass*, pObj, ESI);
//
//	if (auto pTechno = generic_cast<TechnoClass*>(pObj)) {
//		auto pExt = TechnoExtContainer::Instance.Find(pTechno);
//		if (pExt->MyFighterData)
//			pExt->MyFighterData->OnStopCommand();
//	}
//
//	return 0;
//}