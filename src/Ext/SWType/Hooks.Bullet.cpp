#include "Body.h"

#include "NewSuperWeaponType/SWTypeHandler.h"
#include "NewSuperWeaponType/NuclearMissile.h"

#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

// create a downward pointing missile if the launched one leaves the map.
ASMJIT_PATCH(0x46B371, BulletClass_NukeMaker, 5)
{
	GET(BulletClass* const, pThis, EBP);

	SuperWeaponTypeClass* pNukeSW = nullptr;

	if (auto const pNuke = BulletExtContainer::Instance.Find(pThis)->NukeSW)
	{
		pNukeSW = pNuke;
	}
	else if (auto pLinkedNuke = SuperWeaponTypeClass::Array->
		get_or_default(WarheadTypeExtContainer::Instance.Find(pThis->WH)->NukePayload_LinkedSW))
	{
		pNukeSW = pLinkedNuke;
	}

	if (pNukeSW)
	{
		auto const pSWExt = SWTypeExtContainer::Instance.Find(pNukeSW);

		// get the per-SW nuke payload weapon
		if (WeaponTypeClass const* const pPayload = pSWExt->Nuke_Payload)
		{

			// these are not available during initialisation, so we gotta
			// fall back now if they are invalid.
			auto const damage = pSWExt->GetNewSWType()->GetDamage(pSWExt);
			auto const pWarhead = pSWExt->GetNewSWType()->GetWarhead(pSWExt);

			// put the new values into the registers
			R->Stack(0x30, R->EAX());
			R->ESI(pPayload);
			R->Stack(0x10, 0);
			R->Stack(0x18, pPayload->Speed);
			R->Stack(0x28, pPayload->Projectile);
			R->EAX(pWarhead);
			R->ECX(R->lea_Stack<CoordStruct*>(0x10));
			R->EDX(damage);

			return 0x46B3B7;
		}
		else
		{
			Debug::LogInfo(
				"[%s] has no payload weapon type, or it is invalid.",
				pNukeSW->ID);
		}
	}

	return 0;
}

// just puts the launched SW pointer on the downward aiming missile.
ASMJIT_PATCH(0x46B423, BulletClass_NukeMaker_PropagateSW, 6)
{
	GET(BulletClass* const, pThis, EBP);
	GET(BulletClass* const, pNuke, EDI);

	auto const pThisExt = BulletExtContainer::Instance.Find(pThis);
	auto const pNukeExt = BulletExtContainer::Instance.Find(pNuke);
	pNukeExt->NukeSW = pThisExt->NukeSW;
	pNuke->Owner = pThis->Owner;
	pNukeExt->Owner = pNukeExt->Owner;

	return 0;
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
ASMJIT_PATCH(0x467E59, BulletClass_Update_NukeBall, 5)
{
	// changed the hardcoded way to just do this if the warhead is called NUKE
		// to a more universal approach. every warhead can get this behavior.
	GET(BulletClass* const, pThis, EBP);

	auto const pExt = BulletExtContainer::Instance.Find(pThis);
	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

	enum { Default = 0u, FireNow = 0x467F9Bu, PreImpact = 0x467ED0 };

	auto allowFlash = true;
	// flashDuration = 0;

	// this is a bullet launched by a super weapon
	if (pExt->NukeSW && !pThis->WH->NukeMaker)
	{
		SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

		if (pThis->GetHeight() < 0)
		{
			pThis->SetHeight(0);
		}

		// cause yet another radar event
		auto const pSWTypeExt = SWTypeExtContainer::Instance.Find(pExt->NukeSW);

		if (pSWTypeExt->SW_RadarEvent)
		{
			auto const coords = pThis->GetMapCoords();
			RadarEventClass::Create(
				RadarEventType::SuperweaponActivated, coords);
		}

		if (pSWTypeExt->Lighting_Enabled.isset())
			allowFlash = pSWTypeExt->Lighting_Enabled.Get();
	}

	// does this create a flash?
	auto const duration = pWarheadExt->NukeFlashDuration.Get();

	if (allowFlash && duration > 0)
	{
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = duration;

		SWTypeExtData::ChangeLighting(pExt->NukeSW ? pExt->NukeSW : nullptr);
		MapClass::Instance->RedrawSidebar(1);
	}

	if (auto pPreImpact = pWarheadExt->PreImpactAnim.Get())
	{
		R->EDI(pPreImpact);
		return PreImpact;
	}

	return FireNow;
}

//create nuke pointing down to the target
//ASMJIT_PATCH(0x46B310, BulletClass_NukeMaker_Handle, 6)
//{
//	GET(BulletClass*, pThis, ECX);
//
//	enum { ret = 0x46B53C };
//
//	const auto pTarget = pThis->Target;
//	if (!pTarget)
//	{
//		Debug::LogInfo("Bullet[%s] Trying to Apply NukeMaker but has invalid target !", pThis->Type->ID);
//		return ret;
//	}
//
//	WeaponTypeClass* pPaylod = nullptr;
//	SuperWeaponTypeClass* pNukeSW = nullptr;
//
//	if (auto const pNuke = BulletExtContainer::Instance.Find(pThis)->NukeSW)
//	{
//		pPaylod = SWTypeExtContainer::Instance.Find(pNuke)->Nuke_Payload;
//		pNukeSW = pNuke;
//	}
//	else if (auto pLinkedNuke = SuperWeaponTypeClass::Array->
//		GetItemOrDefault(WarheadTypeExtContainer::Instance.Find(pThis->WH)->NukePayload_LinkedSW))
//	{
//		pPaylod = SWTypeExtContainer::Instance.Find(pLinkedNuke)->Nuke_Payload;
//		pNukeSW = pLinkedNuke;
//	}
//	else
//	{
//		pPaylod = WeaponTypeClass::Find(GameStrings::NukePayload());
//	}
//
//	if (!pPaylod || !pPaylod->Projectile)
//	{
//		Debug::LogInfo("Bullet[%s] Trying to Apply NukeMaker but has invalid Payload Weapon or Payload Weapon Projectile !",
//			pThis->Type->ID);
//		return ret;
//	}
//	auto targetcoord = pTarget->GetCoords();
//
//	R->EAX(SW_NuclearMissile::DropNukeAt(pNukeSW, targetcoord, nullptr, pThis->Owner ? pThis->Owner->Owner : HouseExtData::FindFirstCivilianHouse(), pPaylod));
//	return ret;
//}

