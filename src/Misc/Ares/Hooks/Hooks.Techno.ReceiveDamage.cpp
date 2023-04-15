#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Conversions.h>

DEFINE_OVERRIDE_HOOK(0x701A5C, TechnoClass_ReceiveDamage_IronCurtainFlash, 0x7)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	if (!WarheadTypeExt::ExtMap.Find(pWh)->IC_Flash.Get(RulesExt::Global()->IC_Flash.Get()))
		return 0x701A98;

	return (pThis->ForceShielded == 1) ? 0x701A65 : 0x701A69;
}

DEFINE_OVERRIDE_HOOK(0x701914, TechnoClass_ReceiveDamage_Damaging, 0x7)
{
	R->Stack(0xE, R->EAX() > 0);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7021F5, TechnoClass_ReceiveDamage_OverrideDieSound, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->DieSound_Override;

	if (nSound.isset())
	{
		VocClass::PlayIndexAtPos(nSound, pThis->Location);
		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702185, TechnoClass_ReceiveDamage_OverrideVoiceDie, 0x6)
{
	GET_STACK(WarheadTypeClass*, pWh, 0xD0);
	GET(TechnoClass*, pThis, ESI);

	auto const& nSound = WarheadTypeExt::ExtMap.Find(pWh)->VoiceSound_Override;

	if (nSound.isset())
	{
		VocClass::PlayIndexAtPos(nSound, pThis->Location);
		return 0x702200;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x702CFE, TechnoClass_ReceiveDamage_PreventScatter, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	// only allow to scatter if not prevented
	if (!pExt->PreventScatter)
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
	}

	return 0x702D11;
}

// #1283653: fix for jammed buildings and attackers in open topped transports
DEFINE_OVERRIDE_HOOK(0x702A38, TechnoClass_ReceiveDamage_OpenTopped, 0x7)
{
	REF_STACK(TechnoClass*, pAttacker, STACK_OFFS(0xC4, -0x10));

	// decide as if the transporter fired at this building
	if (pAttacker && pAttacker->InOpenToppedTransport && pAttacker->Transporter)
	{
		pAttacker = pAttacker->Transporter;
	}

	R->EDI(pAttacker);
	return 0x702A3F;
}

DEFINE_OVERRIDE_HOOK(0x702669, TechnoClass_ReceiveDamage_SuppressDeathWeapon, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (!WarheadTypeExt::ExtMap.Find(pWarhead)->ApplySuppressDeathWeapon(pThis))
	{
		pThis->FireDeathWeapon(0);
	}

	return 0x702672;
}

DEFINE_OVERRIDE_HOOK(0x517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 0x6)
{

	GET(InfantryClass*, I, ESI);
	const bool IgnoreDefenses = R->BL() != 0;

	if (!I->IsDeployed() || IgnoreDefenses)
	{
		return 0;
	}

	GET(WarheadTypeClass*, pWH, EBP);
	GET(int*, pDamage, EDI);

	// yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
	if (pWH)
	{
		*pDamage = static_cast<int>(*pDamage * WarheadTypeExt::ExtMap.Find(pWH)->DeployedDamage);
		return 0x517FF9u;
	}

	return 0x518016u;
}

DEFINE_OVERRIDE_HOOK(0x702050, TechnoClass_ReceiveDamage_SuppressUnitLost, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);

	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	auto pTechExt = TechnoExt::ExtMap.Find(pThis);

	if (pWarheadExt->Supress_LostEva.Get())
		pTechExt->SupressEVALost = true;

	return 0x0;
}

#define Is_MaliciousWH(wh) (*(bool*)(((char*)wh->unused_1CC) + 0x75))

// not this
/*
 * Fixing issue #722
 */
 DEFINE_OVERRIDE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
 {
 	GET_STACK(WarheadTypeClass*, WH, STACK_OFFS(0x44, -0xC));
 	return WH && !Is_MaliciousWH(WH) ? 0x738535u : 0u;
 }
