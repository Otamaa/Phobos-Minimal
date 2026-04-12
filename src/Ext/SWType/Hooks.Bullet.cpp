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

