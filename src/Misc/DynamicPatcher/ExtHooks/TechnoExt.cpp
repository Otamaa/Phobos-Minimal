#ifdef COMPILE_PORTED_DP_FEATURES
#include <TechnoClass.h>

#include <Utilities/Macro.h>

#include <Ext/WeaponType/Body.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/DynamicPatcher/Techno/ExtraFire/ExtraFirefunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTargetFunctional.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
//#include <Misc/DynamicPatcher/Techno/FireSW/FireSWFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>
//#include <Misc/DynamicPatcher/Techno/AttackBeacon/AttackBeaconFunctional.h>

/*
DEFINE_HOOK(0x4149EE, AircraftClass_Render2, 0x5)
{
	GET(AircraftClass*, pThis, EBP);
	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	return 0x4149F5;
}

DEFINE_HOOK(0x519616, InfantryClass_Render2, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
	if (auto pManager = pExt->AnotherData.MyManager.get())
		pManager->Render2(pThis, !pThis->IsActive());

	R->ECX(pThis);
	return 0x51961F;
}

DEFINE_HOOK(0x73D40C, UnitClass_Render2, 0x7)
{
	GET(UnitClass*, pThis, ESI);

	if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
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
		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
		if (auto pManager = pExt->AnotherData.MyManager.get())
			pManager->StopCommand();
	}
	return 0;
}


DEFINE_HOOK(0x69252D, ScrollClass_ProcessClickCoords_VirtualUnit, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt && pExt->VirtualUnit ? 0x6925E6 : 0x0;
}
#endif
DEFINE_HOOK(0x5F45A0, TechnoClass_Selectable_DP, 0x7)
{
	GET(TechnoClass*, pThis, EDI);

	auto pUnit = specific_cast<UnitClass*>(pThis);
	const bool bNotSlectable = pUnit && pUnit->DeatFrameCounter > 0;

	bool Slectable = true;

	#ifdef COMPILE_PORTED_DP_FEATURES
	if (auto pExt = TechnoExt::ExtMap.Find(pThis))
		Slectable = !pExt->VirtualUnit.Get();
	#endif

	return Slectable || !bNotSlectable ? 0x0 : 0x5F45A9;
}


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
namespace CalculatePinch
{
	static void Calc(TechnoClass* pFirer, int nWeaponIdx)
	{
		const auto pWeapon = pFirer->GetWeapon(nWeaponIdx);
		{
			if (!pFirer->IsVoxel())
				return;

			auto ext = WeaponTypeExt::ExtMap.Find(pWeapon->WeaponType);

			if (ext && (ext->RockerPitch.Get() > 0.0f))
			{
				double halfPI = Math::PI / 2;
				double theta = 0;

				if (pFirer->HasTurret())
				{
					double turretRad = pFirer->GetRealFacing().current().radians() - halfPI;
					double bodyRad = pFirer->PrimaryFacing.current().radians() - halfPI;
					const Matrix3D& matrix3D = Matrix3D { };
					matrix3D.MakeIdentity();
					matrix3D.RotateZ((float)turretRad);
					matrix3D.RotateZ((float)-bodyRad);
					theta = matrix3D.GetZRotation();
				}

				double gamma = (double)ext->RockerPitch.Get();
				int lrSide = 1;
				int fbSide = 1;
				if (theta < 0)
				{
					lrSide *= -1;
				}
				if (theta >= halfPI || theta <= -halfPI)
				{
					fbSide *= -1;
				}

				double pitch = gamma;
				double roll = 0.0;
				if (theta != 0)
				{
					if (Math::sin(halfPI - theta) == 0.0)
					{
						pitch = 0.0;
						roll = gamma * lrSide;
					}
					else
					{
						double l = Math::cos(gamma);
						double y = l / Math::sin(halfPI - theta);
						double z = Math::sin(gamma);
						double lyz = Math::sqrt(std::pow(y, 2) + std::pow(z, 2));
						pitch = Math::acos(std::abs(y) / lyz) * fbSide;
						roll = (gamma - std::abs(pitch)) * lrSide;
					}
				}
				pFirer->RockingForwardsPerFrame = -(float)pitch;
				pFirer->RockingSidewaysPerFrame = (float)roll;
			}
		}
	}
}

DEFINE_HOOK(0x6FDD61, TechnoClass_Fire_OverrideWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(AbstractClass*, pTarget, 0x4);

	if (auto pExt = TechnoExt::ExtMap.Find(pThis)) {
		R->EBX(pThis->GetWeapon(pExt->CurrentWeaponIdx)->WeaponType);
		return 0x6FDD71;
	}

	return 0;
}

DEFINE_HOOK(0x6FDD50, TechnoClass_Fire_DP, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(const int, nWeapon, 0x8);
	//GET(AbstractClass*, pTarget, EDI);

	CalculatePinch::Calc(pThis, nWeapon);
	ExtraFirefunctional::GetWeapon(pThis, pTarget, nWeapon);
	//FireSWFunctional::OnFire(pThis, pTarget, nWeapon);
	SpawnSupportFunctional::OnFire(pThis);
	if (auto pExt = TechnoExt::ExtMap.Find(pThis)) {
		pExt->CurrentWeaponIdx = nWeapon;
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
			AircraftDiveFunctional::OnFire(pExt, pTypeExt, pTarget, nWeapon);
			//AttackBeaconFunctional::OnFire(pExt, pTarget, nWeapon);
		}
	}
	return 0x0;
}

DEFINE_HOOK(0x702050, TechnoClass_ReceiveDamage_Destroyed, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pExt = TechnoExt::ExtMap.Find(pThis)) {
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
			GiftBoxFunctional::Destroy(pExt, pTypeExt);
		 }
	}

	return 0;
}

DEFINE_HOOK(0x6F6CA0, TechnoClass_Unlimbo_DP, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pCoord, (0x4));
	GET_STACK(DirType, faceDir, (0x8));

	if (auto pExt = TechnoExt::ExtMap.Find<true>(pThis)) {
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
			GiftBoxFunctional::Init(pExt, pTypeExt);
			AircraftPutDataFunctional::OnPut(pExt, pTypeExt, pCoord);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FC016, TechnoClass_Select_SkipVoice, 0x8)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find<true>(pThis);
	return pExt && pExt->SkipVoice ? 0x6FC01E :0x0;
}

//DEFINE_HOOK(0x701DFF, TechnoClass_TakeDamage_AfterObjectClassCall, 0x7) //9
//{
//	GET(TechnoClass*, pThis, ESI);
//	//GET(int*, pRealDamage , EBX);
//	GET(WarheadTypeClass*, pWH , EBP);
//	GET(DamageState, damageState , EDI);
//
//	const auto pExt = TechnoExt::ExtMap.Find(pThis);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
//
//	if(pExt && pTypeExt) {
//		GiftBoxFunctional::TakeDamage(pExt, pTypeExt, pWH, damageState);
//	}
//
//	return 0x0;
//}

//9
//DEFINE_HOOK(0x6F9039, TechnoClass_Greatest_Threat_GuardRange, 0x9)
//{
//	GET(TechnoClass*, pTechno, ESI);
//	auto nGuarRange = pTechno->GetTechnoType()->GuardRange;
//
//	if (auto pPri = pTechno->GetWeapon(0)) {
//		if (auto pPriW = pPri->WeaponType)
//			if (pPriW->Range > nGuarRange)
//				nGuarRange = pPriW->Range;
//	}
//
//	if(auto pSec = pTechno->GetWeapon(1))
//	{
//		if (auto pSecW = pSec->WeaponType)
//			if (pSecW->Range > nGuarRange)
//				nGuarRange = pSecW->Range;
//	}
//
//
//	R->EDI(nGuarRange ? nGuarRange : 512);
//	return 0x6F903E;
//}
#endif