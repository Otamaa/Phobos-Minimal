#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <WarheadTypeClass.h>
#include <ScenarioClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Cell/Body.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>

#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>
#include <Misc/Hooks.Otamaa.h>

#include <SpawnManagerClass.h>

/*
	Custom Radiations
	Worked out from old uncommented Ares RadSite Hook , adding some more hook
	and rewriting some in order to make this working perfectly
	Credit : Ares Team , for unused/uncommented source of Hook.RadSite
						,RulesData_LoadBeforeTypeData Hook
			 Alex-B : GetRadSiteAt ,Helper that used at FootClass_AI & BuildingClass_AI
					  Radiate , Uncommented
			 me(Otamaa) adding some more stuffs and rewriting hook that cause crash

*/

ASMJIT_PATCH(0x469150, BulletClass_Logics_ApplyRadiation, 0x5)
{
	enum { Handled = 0x46920B, Continue = 0x0 };

	GET(BulletClass* const, pThis, ESI);
	GET_BASE(CoordStruct*, pCoords, 0x8);
	GET(WeaponTypeClass*, pWeapon, ECX);
	GET(int, nAmount, EDI);

	if (!MapClass::Instance->IsWithinUsableArea(*pCoords))
		return Handled;

	const auto pCell = MapClass::Instance->TryGetCellAt(*pCoords);

	if (!pCell) {
		return Handled;
	}


	BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(pCell, static_cast<int>(pWeapon->Warhead->CellSpread), nAmount);

	return Handled;
}

//unused function , safeguard
ASMJIT_PATCH(0x46ADE0, BulletClass_ApplyRadiation_NoBullet, 0x5)
{
	enum { Handled = 0x46AE5E, Continue = 0x0 };

	GET(BulletClass* const, pThis, ECX);
	GET_STACK(CellStruct, location, 0x4);
	GET_STACK(int, spread, 0x8);
	GET_STACK(int, amount, 0xC);

	const auto pCell = MapClass::Instance->TryGetCellAt(location);

	if (!pCell)
		return Handled;

	if (!pThis)
		Debug::FatalError(__FUNCTION__" require BulletClass !\n");

	BulletExtContainer::Instance.Find(pThis)->ApplyRadiationToCell(pCell, spread , amount);
	return Handled;

}

// Fix for desolator
ASMJIT_PATCH(0x5213B4, InfantryClass_AIDeployment_CheckRad, 0x7)
{
	enum { FireCheck = 0x5213F4, SetMissionRate = 0x521484, Continue = 0x0 };

		GET(InfantryClass*, pThis, ESI);
		int radLevel = 0;
		int weaponRadLevel = 0;

		if (const auto pWeaponStruct = pThis->GetDeployWeapon())
		{
			if (const auto pWeapon = pWeaponStruct->WeaponType)
			{
				const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
				const auto currentCoord = pThis->InlineMapCoords();
				if (const auto pCell = MapClass::Instance->TryGetCellAt(currentCoord))
				{
					auto pCellExt = CellExtContainer::Instance.Find(pCell);

					auto const it = pCellExt->RadSites.find_if([=](auto const pPair) {

						auto const pRadExt = RadSiteExtContainer::Instance.Find(pPair);

						if (pRadExt->Type != pWeaponExt->RadType)
							return false;

						if (static_cast<int>(pWeapon->Warhead->CellSpread) != pPair->Spread)
							return false;

						if (pWeapon != pRadExt->Weapon)
							return false;

						if (pRadExt->TechOwner)
							return pRadExt->TechOwner == pThis;

						return true;

					});

					if (it != pCellExt->RadSites.end()) {
						radLevel = static_cast<int>((*it)->GetCurrentRadLevel());
					}
				}

				weaponRadLevel = pWeapon->RadLevel;
			}
		}

		return (!radLevel || (radLevel < (weaponRadLevel / 3))) ? FireCheck : SetMissionRate;
}

// Fix for desolator unable to fire his deploy weapon when cloaked
ASMJIT_PATCH(0x521478, InfantryClass_AIDeployment_FireNotOKCloakFix, 0x6) // 4
{
	GET(InfantryClass* const, pThis, ESI);

	AbstractClass* pTarget = nullptr; //default WWP nullptr

	const auto pWeaponstr = pThis->GetDeployWeapon();

	if (pWeaponstr)
	{
		const auto pWeapon = pWeaponstr->WeaponType;

		if (pWeapon
			&& pWeapon->DecloakToFire
			&& pThis->IsInCloakState())
		{
			// FYI this are hack to immediately stop the Cloaking
			// since this function is always failing to decloak and set target when cell is occupied
			// something is wrong somewhere  # Otamaa
			pThis->CloakDelayTimer.Start(
				static_cast<int>(pThis->Type->Sequence->GetSequence(DoType::DeployedFire).CountFrames * 900.0));

			pTarget = pThis->GetCell();
		}
	}

	pThis->SetTarget(pTarget); //Here we go

	return 0x521484;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0858,  FakeRadSiteClass::__GetAltCoords);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0868,  FakeRadSiteClass::__GetAltCoords);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F086C, FakeRadSiteClass::__AI);
DEFINE_FUNCTION_JUMP(LJMP, 0x65B9C0, FakeRadSiteClass::__Increase_In_Area);
DEFINE_FUNCTION_JUMP(LJMP, 0x65BB50, FakeRadSiteClass::__Reduce_In_Area);
DEFINE_FUNCTION_JUMP(LJMP, 0x65B8F0, FakeRadSiteClass::__Radiation_At);
DEFINE_FUNCTION_JUMP(LJMP, 0x65BD00, FakeRadSiteClass::__Reduce_Radiation);