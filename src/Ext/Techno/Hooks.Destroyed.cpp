#include "Body.h"

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/CaptureManager/Body.h>

#include <Misc/DamageArea.h>

void FakeTechnoClass::__Destroy(TechnoClass* pThis)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const bool isCalledFromRcvDamage = pExt->ReceiveDamage;
	const auto what = pThis->WhatAmI();

	if (what == UnitClass::AbsID)
	{
		auto pThisUnit = (UnitClass*)pThis;

		if (pThisUnit->Type->Explosion.Count > 0)
		{
			if (pThisUnit->Type->Explodes || pThis->HasAbility(AbilityType::Explodes))
			{
				const auto pExplode = pThisUnit->Type->Ammo == -1 || pThis->Ammo > 0 ?
					pThisUnit->Type->Explosion.Items[pThisUnit->Type->Explosion.Count - 1] :
					pThisUnit->Type->Explosion.Items[ScenarioClass::Instance->Random.RandomFromMax(pThisUnit->Type->Explosion.Count - 1)];

				if (pExplode)
				{
					CoordStruct crd = pThis->GetCoords();

					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExplode, crd, 0, 1, AnimFlag(0x2600), -15, false),
						pThis->Owner,
						nullptr,
						false
					);
				}
			}

			auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

			int morePower = 0;

			if (RulesClass::Instance->TiberiumExplosive
				&& !pThisUnit->Type->Weeder
				&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
				&& storage->GetAmounts() > 0.0f)
			{
				// multiply the amounts with their powers and sum them up

				for (int i = 0; i < TiberiumClass::Array->Count; ++i)
				{
					morePower += int(storage->m_values[i] * TiberiumClass::Array->Items[i]->Power);
				}

				if (morePower > 0)
				{

					CoordStruct crd = pThis->GetCoords();
					if (auto pWH = FakeRulesClass::Instance()->Tiberium_ExplosiveWarhead)
					{
						DamageArea::Apply(&crd, morePower, pThisUnit, pWH, pWH->Tiberium, pThis->Owner);
					}

					if (auto pAnim = FakeRulesClass::Instance()->Tiberium_ExplosiveAnim)
					{
						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false),
							pThis->Owner,
							nullptr,
							false
						);
					}
				}

				if (RulesClass::Instance->ShakeScreen && !Phobos::Config::HideShakeEffects)
				{
					if (!pExt->TypeExtData->DontShake.Get())
					{
						if (pThisUnit->Type->Strength > 0)
						{
							TechnoExtData::ShakeScreen(pThis, pThisUnit->Type->Strength, RulesClass::Instance->ShakeScreen);
						}
					}
				}
			}

			if (!isCalledFromRcvDamage)
			{
				AnimTypeExtData::ProcessDestroyAnims(pThis);
			}
		}
	}

	if (auto pCapture = pThis->CaptureManager) { pCapture->FreeAll(); }
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB1C8, FakeTechnoClass::__Destroy); // InfantryClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E402C, FakeTechnoClass::__Destroy); // BuildingClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2414, FakeTechnoClass::__Destroy); // AircraftClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5DE0, FakeTechnoClass::__Destroy); // UnitClass

#ifdef _ded
ASMJIT_PATCH(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExtContainer::Instance.Find(pThis);

	if (!Extension->ReceiveDamage)
	{
		AnimTypeExtData::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

ASMJIT_PATCH(0x7387D1, UnitClass_Destroyed_Shake, 0x6)
{
	GET(UnitClass* const, pUnit, ESI); //forEXT

	if (!pUnit || !pUnit->Type || !RulesClass::Instance->ShakeScreen || Phobos::Config::HideShakeEffects)
		return 0x738801;

	if (!pUnit->Type->Strength)
		return 0x738801;

	if (!TechnoTypeExtContainer::Instance.Find(pUnit->Type)->DontShake.Get())
		TechnoExtData::ShakeScreen(pUnit, pUnit->Type->Strength, RulesClass::Instance->ShakeScreen);

	return 0x738801;
}

ASMJIT_PATCH(0x738749, UnitClass_Destroy_TiberiumExplosive, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

	if (RulesClass::Instance->TiberiumExplosive
		&& !pThis->Type->Weeder
		&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& storage->GetAmounts() > 0.0f)
	{
		// multiply the amounts with their powers and sum them up
		int morePower = 0;

		for (int i = 0; i < TiberiumClass::Array->Count; ++i)
		{
			morePower += int(storage->m_values[i] * TiberiumClass::Array->Items[i]->Power);
		}

		if (morePower > 0)
		{

			CoordStruct crd = pThis->GetCoords();
			if (auto pWH = FakeRulesClass::Instance()->Tiberium_ExplosiveWarhead)
			{
				DamageArea::Apply(&crd, morePower, const_cast<UnitClass*>(pThis), pWH, pWH->Tiberium, pThis->Owner);
			}

			if (auto pAnim = FakeRulesClass::Instance()->Tiberium_ExplosiveAnim)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}

	return 0x7387C4;
}
#endif