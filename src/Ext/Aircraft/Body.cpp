#include "Body.h"
#include <Ext/WeaponType/Body.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>

//AircraftExt::ExtContainer AircraftExt::ExtMap;

void AircraftExt::TriggerCrashWeapon(AircraftClass* pThis, int nMult)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto pCrashWeapon = pTypeExt->CrashWeapon.GetOrDefault(pThis, pTypeExt->CrashWeapon_s.Get());

	if (!TechnoExt::FireWeaponAtSelf(pThis, pCrashWeapon))
		pThis->FireDeathWeapon(nMult);

	AnimTypeExt::ProcessDestroyAnims(pThis, nullptr);
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	if (!pTarget)
		return;
	
	AircraftExt::FireBurst(pThis, pTarget, shotNumber, pThis->SelectWeapon(pTarget));
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	const auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	const auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	AircraftExt::FireBurst(pThis , pTarget, shotNumber, WeaponIdx, weaponType);
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
{
	if (!pWeapon->Burst)
		return;

	for (int i = 0; i < pWeapon->Burst; i++)
	{
		if (pWeapon->Burst < 2 && WeaponTypeExt::ExtMap.Find(pWeapon)->Strafing_SimulateBurst)
			pThis->CurrentBurstIndex = (int)shotNumber;

		pThis->Fire(pTarget, WeaponIdx);
	}
}
bool AircraftExt::IsValidLandingZone(AircraftClass* pThis)
{
	if (const auto pPassanger = pThis->Passengers.GetFirstPassenger())
	{
		if (const auto pDest = pThis->Destination)
		{
			const auto pDestCell = MapClass::Instance->GetCellAt(pDest->GetCoords());

			return pDestCell->IsClearToMove(pPassanger->GetTechnoType()->SpeedType, false, false, -1, pPassanger->GetTechnoType()->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1
				&& pDestCell->IsValidMapCoords();
		}
	}

	return false;

}
// =============================
// load / save

//template <typename T>
//void AircraftExt::ExtData::Serialize(T& Stm)
//{
//	Stm
//
//
//		;
//}
//
//void AircraftExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
//{
//	TExtension<AircraftClass>::LoadFromStream(Stm);
//	this->Serialize(Stm);
//}
//
//void AircraftExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
//{
//	TExtension<AircraftClass>::SaveToStream(Stm);
//	this->Serialize(Stm);
//}
//
//bool AircraftExt::LoadGlobals(PhobosStreamReader& Stm)
//{
//	return Stm
//		.Success();
//}
//
//bool AircraftExt::SaveGlobals(PhobosStreamWriter& Stm)
//{
//	return Stm
//		.Success();
//}

// =============================
// container

//AircraftExt::ExtContainer::ExtContainer() : TExtensionContainer("AircraftClass") { }
//AircraftExt::ExtContainer::~ExtContainer() = default;

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x413F6A, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);

	AircraftExt::ExtMap.JustAllocate(pItem, !pItem, "Invalid !");

	return 0;
}

DEFINE_HOOK(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);

	AircraftExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AircraftExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
{
	AircraftExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
{
	AircraftExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	if (const auto pExt = AircraftExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return 0x0;
}
#endif