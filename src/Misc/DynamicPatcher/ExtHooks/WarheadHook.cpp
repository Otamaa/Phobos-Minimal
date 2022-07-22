#include <Ext/Techno/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

bool CanAffectHouse(WarheadTypeExt::ExtData* warheadTypeExt,HouseClass* pOwnerHouse, HouseClass* pTargetHouse)
{
	if (pOwnerHouse && pTargetHouse)
	{
		if (pOwnerHouse == pTargetHouse)
		{
			return warheadTypeExt->AffectsAllies.Get() || warheadTypeExt->AffectsOwner.Get();
		}
		else if (pOwnerHouse->IsAlliedWith(pTargetHouse))
		{
			return  warheadTypeExt->AffectsAllies.Get();
		}
		else
		{
			return warheadTypeExt->AffectsEnemies.Get();
		}
	}
	return true;
}

bool AffectMe(ObjectClass* pVictim , ObjectClass* pAttacker, WarheadTypeClass* pWH, HouseClass* pTargetHouse, WarheadTypeExt::ExtData* warheadTypeExt)
{
	if (pAttacker && warheadTypeExt)
	{
		auto pOwnerHouse = pVictim->GetOwningHouse();
		return CanAffectHouse(warheadTypeExt,pOwnerHouse,pTargetHouse);
	}
	return true;
}


bool DamageMe(ObjectClass* pVictim, int damage, int distanceFromEpicenter, WarheadTypeExt::ExtData* warheadTypeExt, int& realDamage, bool effectsRequireDamage = false)
{
	// 计算实际伤害
	if (damage > 0)
	{
		realDamage = MapClass::GetTotalDamage(damage, warheadTypeExt->Get(), pVictim->GetType()->Armor, distanceFromEpicenter);
	}
	else
	{
		realDamage = -MapClass::GetTotalDamage(-damage, warheadTypeExt->Get(), pVictim->GetType()->Armor, distanceFromEpicenter);
	}

	{
		if (damage == 0)
		{
			return warheadTypeExt->AllowZeroDamage;
		}
		else
		{
			if (warheadTypeExt->EffectsRequireVerses)
			{
				if (MapClass::GetTotalDamage(RulesGlobal->MaxDamage, warheadTypeExt->Get(), pVictim->GetType()->Armor, 0) == 0)
				{
					return false;
				}

				if (effectsRequireDamage || warheadTypeExt->EffectsRequireDamage)
				{
					return realDamage != 0;
				}
			}
		}
	}
	return true;
}

void AttachEffect(ObjectClass* pVictim, AttachEffectType* aeType, ObjectClass* pAttacker, HouseClass* pAttackingHouse)
{
	auto pExt = TechnoExt::GetExtData((TechnoClass*)pVictim);
	auto pManager = pExt->AnotherData.MyManager.get();

	if (!pManager)
		return;

	if (!aeType->PenetratesIronCurtain && pVictim->IsIronCurtained())
	{
		return;
	}

	auto pHouse = aeType->OwnerTarget ? pVictim->GetOwningHouse() : pAttackingHouse;
	auto pAttackerT = generic_cast<TechnoClass*>(pAttacker);

	if (pAttacker && pAttacker->IsAlive && pAttackerT)
	{
		if (aeType->FromTransporter && pAttackerT->Transporter)
		{
			pManager->Attach(aeType, pVictim, pHouse, pAttackerT->Transporter);
		}
		else
		{
			pManager->Attach(aeType, pVictim, pHouse, pAttackerT);
		}
	}
	else
	{
		pManager->Attach(aeType, pVictim, pHouse, nullptr);
	}
}

void ReceiveDamage_AttachEffect(ObjectClass* pVictim, int* pDamage, int distanceFromEpicenter, WarheadTypeClass* pWH,
	ObjectClass* pAttacker, bool ignoreDefenses, bool preventPassengerEscape, HouseClass* pAttackingHouse,
	WarheadTypeExt::ExtData* whExt, int realDamage)
{
	auto pExt = TechnoExt::GetExtData((TechnoClass*)pVictim);
	auto pManager = pExt->AnotherData.MyManager.get();

	if (!pManager)
		return;

	if (!whExt->AnotherData.MyData.Types.empty())
	{
		bool attached = false;
			for(auto const& nAeD : whExt->AnotherData.MyData.Types)
			{
				if (nAeD->AttachWithDamage)
				{
					AttachEffect(pVictim,nAeD, pAttacker, pAttackingHouse);
					attached = true;
				}
			}

		if (attached && whExt->AnotherData.MyData.CabinLength > 0) {
			pManager->SetLocationSpace(whExt->AnotherData.MyData.CabinLength);
		}
	}

	pManager->ReceiveDamage(pVictim, pDamage, distanceFromEpicenter, pWH, pAttacker, ignoreDefenses, preventPassengerEscape, pAttackingHouse);
}

DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage, 0x7)
{
	GET(TechnoClass*, pTechno, ECX);
	GET_STACK(int* , pDamage ,(0x4));
	GET_STACK(int, distanceFromEpicenter ,(0x8));
	GET_STACK(WarheadTypeClass*, pWH ,(0xC));
	GET_STACK(ObjectClass*, pAttacker ,(0x10));
	GET_STACK(bool, ignoreDefenses ,(0x14));
	GET_STACK(bool, preventPassengerEscape,(0x18));
	GET_STACK(HouseClass*, pAttackingHouse ,(0x1C));

	if (!pTechno->IsActive())
		return 0x0;

	auto pWhExt = WarheadTypeExt::ExtMap.Find(pWH);
	int nDamage;
	if (AffectMe(pTechno,pAttacker, pWH, pAttackingHouse, pWhExt) &&
		DamageMe(pTechno, *pDamage, distanceFromEpicenter, pWhExt, nDamage))
	{
		ReceiveDamage_AttachEffect(
			pTechno,
			pDamage,
			distanceFromEpicenter,
			pWH,
			pAttacker,
			ignoreDefenses,
			preventPassengerEscape,
			pAttackingHouse,
			pWhExt,
			nDamage
		);
	}

	return 0x0;
}


