#include "Body.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

ASMJIT_PATCH(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	GET(ObjectClass* const, pThis, ECX);
	GET_STACK(TechnoClass* const, pTechno, 0x4);
	R->AL(TechnoExtData::IsCrushable(pThis, pTechno));
	return 0x5F6D90;
}

ASMJIT_PATCH(0x5F5A56, ObjectClass_ParachuteAnim, 0x7)
{
	GET(CoordStruct*, pCoord, EDI);
	GET(ObjectClass*, pThis, ESI);

	AnimClass* pParach = nullptr;
	bool IsBullet = false;

	if (auto pBullet = cast_to<BulletClass*, false>(pThis))
	{
		IsBullet = true;
		auto pParach_type = ((FakeBulletClass*)pBullet)->_GetTypeExtData()->Parachute.Get(RulesClass::Instance->BombParachute);

		pParach = GameCreate<AnimClass>(pParach_type, pCoord, 0, 1, AnimFlag::AnimFlag_600, 0, false);

	}
	else
	{

		auto coord = *pCoord;
		coord.Z += 75;
		auto pParach_type = RulesClass::Instance->Parachute;

		if (const auto pTechno = flag_cast_to<TechnoClass*, false>(pThis))
		{
			auto pType = GET_TECHNOTYPE(pTechno);
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pTypeExt->IsBomb)
				pThis->IsABomb = true;

			pParach_type = pTypeExt->ParachuteAnim ? pTypeExt->ParachuteAnim : HouseExtData::GetParachuteAnim(pTechno->Owner);
		}

		pParach = GameCreate<AnimClass>(pParach_type, coord);
	}

	pThis->Parachute = pParach;

	if (pParach)
	{
		bool AllowRemap = !IsBullet;
		HouseClass* pOwn = pThis->GetOwningHouse();

		pParach->SetOwnerObject(pThis);

		if (IsBullet)
		{
			auto pTypeExt = BulletTypeExtContainer::Instance.Find(((BulletClass*)pThis)->Type);
			AllowRemap = pTypeExt->Parachuted_Remap;

			if (AllowRemap)
			{
				auto pExt = BulletExtContainer::Instance.Find((BulletClass*)pThis);
				pOwn = ((BulletClass*)pThis)->Owner ? ((BulletClass*)pThis)->Owner->Owner : pExt->Owner;
			}
		}

		const int idx = pOwn ? pOwn->ColorSchemeIndex : RulesExtData::Instance()->AnimRemapDefaultColorScheme;

		if (AllowRemap && idx >= 0)
		{
			pParach->LightConvert = ColorScheme::Array->Items[idx]->LightConvert;
			pParach->TintColor = pThis->GetCell()->Color1.Red;
		}
	}

	return 0x5F5B36;
}

static void __fastcall AnnounceInvalidatePointerWrapper(ObjectClass* pObject, bool removed)
{
	if (!pObject->Limbo()) // when object failed to be unlimbo , immedietely announce them dead
		pObject->AnnounceExpiredPointer(removed);
}
//ObjectClass_RemoveThis -> re-reoute the Invalidation call
DEFINE_FUNCTION_JUMP(CALL, 0x5F6616, AnnounceInvalidatePointerWrapper)
//ObjectClass_RemoveThis -> remove the unlimbo call
DEFINE_JUMP(LJMP, 0x5F661B, 0x5F6625)


DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2460, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3510, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3C8C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4078, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E48A0, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8E50, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB214, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EC414, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EDE7C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF21C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF590, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EFB10, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EFD58, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F06C4, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F34B8, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4B1C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F53E8, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5E2C, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F64D4, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6864, FakeObjectClass::_GetCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6DB0, FakeObjectClass::_GetCell);

int Get_FallDamage(
	double ratio,
	const TechnoClass* pTechno,
	const TechnoTypeClass* pTechnoType)
{
	if (ratio < 0.0)
		return static_cast<int>(pTechno->Health * Math::abs(ratio));

	if (ratio >= 0.0 && ratio <= 1.0)
		return static_cast<int>(pTechnoType->Strength * ratio);

	return static_cast<int>(ratio);
}

bool IsTechnoFalling(ObjectClass* pThis)
{
	pThis->FallRate = 0;

	if (const auto pTechno = flag_cast_to<TechnoClass*, true>(pThis))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pTechno);
		const auto pType = pTechno->GetTechnoType();
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const bool onParachuted = pExt->OnParachuted;
		pExt->OnParachuted = false;

		if (pThis->IsABomb && pThis->IsAlive)
		{
			const bool hoverShutdown = pExt->HoverShutdown;
			pExt->HoverShutdown = false;

			if (hoverShutdown)
			{
				if (pTypeExt->HoverDrownable)
				{
					int damage = pThis->Health;
					pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}

				pThis->IsABomb = false;
				return true;
			}

			const auto pCell = pTechno->GetCell();
			const bool onBridge = pCell->ContainsBridge();

			int damage = 0;

			if (!pCell->IsClearToMove(pType->SpeedType, true, true, ZoneType::None, pType->MovementZone, -1, onBridge))
			{
				damage = pThis->Health;
				pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);

				return true;
			}

			const LandType landType = pCell->LandType;
			const bool inWater = !onBridge && (landType == LandType::Water || landType == LandType::Beach);

			if (!onParachuted)
			{
				if (!pTypeExt->FallingDownDamage_AllowEMP && pTechno->EMPLockRemaining > 0)
				{
					damage = pThis->Health;
					pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);

					return true;
				}

				double ratio = pCell->LandType == LandType::Water && !pTechno->OnBridge ?
					pTypeExt->FallingDownDamage_Water.Get(pTypeExt->FallingDownDamage.Get())
					: pTypeExt->FallingDownDamage.Get();

				damage = Get_FallDamage(ratio, pTechno, pType);
			}

			if (damage == 0
				|| pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr) != DamageState::NowDead)
			{
				pThis->IsABomb = false;
				const auto abs = pThis->WhatAmI();

				if (abs == AbstractType::Infantry)
				{
					const auto pInf = static_cast<InfantryClass*>(pTechno);
					const auto sequenceAnim = pInf->SequenceAnim;
					pInf->ShouldDeploy = false;

					if (inWater)
					{
						if (sequenceAnim != DoType::Swim)
							pInf->PlayAnim(DoType::Swim, true, false);
					}
					else if (sequenceAnim != DoType::Guard)
					{
						pInf->PlayAnim(DoType::Ready, true, false);
					}

					ObjectClass* pObject = pCell->GetContent();

					while (pObject->NextObject)
					{
						pObject = pObject->NextObject;
					}

					if (pObject != pInf)
						pInf->Scatter(pInf->GetCoords(), true, false);
				}
				else if (abs == AbstractType::Unit)
				{
					static_cast<UnitClass*>(pTechno)->UpdatePosition(PCPType::During);
				}
			}
		}

		return true;
	}

	return false;
}

ASMJIT_PATCH(0x5F3FB2, ObjectClass_Update_MaxFallRate, 6)
{
	GET(ObjectClass*, pThis, ESI);
	GET(Layer, curLayer, EBP);

	const auto pTechnoType = pThis->GetTechnoType();
	const bool bAnimAttached = pTechnoType ? pThis->Parachute != 0 : pThis->HasParachute;

	int nFallRate = 1;
	int nMaxFallRate = bAnimAttached ? RulesClass::Instance->ParachuteMaxFallRate : RulesClass::Instance->NoParachuteMaxFallRate;

	if (pTechnoType)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
		nFallRate = (!bAnimAttached ? pExt->FallRate_NoParachute : pExt->FallRate_Parachute).Get();
		auto& nCustomMaxFallRate = (!bAnimAttached ? pExt->FallRate_NoParachuteMax : pExt->FallRate_ParachuteMax);

		if (nCustomMaxFallRate.isset())
			nMaxFallRate = nCustomMaxFallRate;
	}

	if (pThis->FallRate - nFallRate >= nMaxFallRate)
		nMaxFallRate = pThis->FallRate - nFallRate;

	pThis->FallRate = nMaxFallRate;

	if (curLayer != pThis->InWhichLayer())
	{
		DisplayClass::Instance->SubmitObject(pThis);
	}

	if (pThis->IsFallingDown)
		return 0x5F4151;

	if (IsTechnoFalling(pThis))
		return pThis->IsAlive ? 0x5F405B : 0x5F4151;

	if (pThis->IsABomb && pThis->Health > 0 && pThis->IsAlive)
	{
		int damage = pThis->Health;
		pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);

		if (!pThis->IsAlive)
			return 0x5F4151;
	}

	return 0x5F405B;
}

ASMJIT_PATCH(0x5F5965, ObjectClass_SpawnParachuted_Track, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if (RulesExtData::Instance()->FallingDownTargetingFix && (pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = true;
	}
	return 0;
}

ASMJIT_PATCH(0x5F4160, ObjectClass_DropAsBomb_Track, 0x6)
{
	GET(ObjectClass*, pThis, ECX);

	if (RulesExtData::Instance()->FallingDownTargetingFix && (pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = true;
	}

	return 0;
}

ASMJIT_PATCH(0x5F3F86, ObjectClass_Update_Track, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if (RulesExtData::Instance()->FallingDownTargetingFix && (pThis->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		ScenarioExtData::Instance()->FallingDownTracker.emplace((TechnoClass*)pThis);
		TechnoExtContainer::Instance.Find((TechnoClass*)pThis)->FallingDownTracked = false;
	}

	return 0;
}
