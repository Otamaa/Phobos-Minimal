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
ASMJIT_PATCH(0x4149EE, AircraftClass_Render2, 0x5)
{
	GET(AircraftClass*, pThis, EBP);
	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	return 0x4149F5;
}

ASMJIT_PATCH(0x519616, InfantryClass_Render2, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	R->ECX(pThis);
	return 0x51961F;
}

ASMJIT_PATCH(0x73D40C, UnitClass_Render2, 0x7)
{
	GET(UnitClass*, pThis, ESI);

	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	R->ECX(pThis);
	return 0x73D415;
}

ASMJIT_PATCH(0x730F1C, ObjectClass_StopCommand, 0x5)
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


ASMJIT_PATCH(0x69252D, ScrollClass_ProcessClickCoords_VirtualUnit, 0x8)
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

ASMJIT_PATCH(0x6FC339, TechnoClass_CanFire_DP, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	//GET(WeaponTypeClass*, pWeapon, EDI);
	//GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
	return CeaseFire(pThis) ?//
		0x6FCB7E
		: 0x0;
}
*/

 ASMJIT_PATCH(0x6B743E, SpawnManagerAI_SpawnSupportFLH, 0x6)
 {
 	GET(SpawnManagerClass*, pSpawn, ESI);
 	//GET_STACK(int, nArrIdx, STACK_OFFS(0x68, 0x54));

 	if (auto pOwner = pSpawn->Owner)
 	{
 		//if ((*pFLH) == CoordStruct::Empty)
 		{
 			auto pTypeExt = GET_TECHNOTYPEEXT(pOwner);

 			if (pTypeExt->MySpawnSupportDatas.Enable)
 			{
 				//CoordStruct nFLH = CoordStruct::Empty;

 				SpawnSupportFLHData nFLHData = pTypeExt->MySpawnSupportFLH;
 				if (auto const pTransporter = pOwner->Transporter)
 				{
 					if (auto const pTransportExt = GET_TECHNOTYPEEXT(pTransporter))
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

// ASMJIT_PATCH(0x5F45A0, TechnoClass_Selectable_Early , 0x7)
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


//Gscript 
//    v_Init((this + 36), &a2, v4, "ScatterChance", 0);
//	  v_bool((this + 19), &a2, v4, "ScatterOnInfantry", 0);
AbstractClass* ScatterAgainst(AbstractClass* pTarget , AbstractType abs, int chance){
	if(pTarget && pTarget->WhatAmI() == abs){
		if(ScenarioClass::Instance->Random.RandomFromMax(chance)){
			auto pCell = MapClass::Instance->GetCellAt( pTarget->GetCoords());
			static constexpr std::array<FacingType, 4> __facing = {
				FacingType::North, FacingType::East, FacingType::South, FacingType::West
			};

			FacingType _facing_get = __facing[ScenarioClass::Instance->Random.RandomFromMax(__facing.size() - 1)];
			return pCell->GetAdjacentCell(_facing_get);
		}
	}

	return pTarget;
}

ASMJIT_PATCH(0x6FDD50, TechnoClass_FireAt_PreFire, 0x6)
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
		auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
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

 ASMJIT_PATCH(0x6F6CA0, TechnoClass_Unlimbo_Early, 0x7)
 {
 	GET(TechnoClass*, pThis, ECX);
 	GET_STACK(CoordStruct*, pCoord, (0x4));
 	//GET_STACK(DirType, faceDir, (0x8));

	auto pExt = TechnoExtContainer::Instance.Find(pThis); {
		auto pTypeExt = GET_TECHNOTYPEEXT(pThis); {
 			DamageSelfState::OnPut(pExt->DamageSelfState, pTypeExt->DamageSelfData);
 			GiftBoxFunctional::Init(pExt, pTypeExt);
 			AircraftPutDataFunctional::OnPut(pExt, pTypeExt, pCoord);
 		}
 	}

 	return 0;
 }

ASMJIT_PATCH(0x6FBFE9, TechnoClass_Select_SkipVoice, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return TechnoExtContainer::Instance.Find(pThis)->SkipVoice ? 0x6FC01E :0x0;
}