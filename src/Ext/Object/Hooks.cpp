#include "Body.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

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
