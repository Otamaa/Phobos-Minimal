#include "Trails.h"
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>

void UniversalTrail::DrawAnimTrail(CoordStruct& sourcePos, HouseClass* pHouse, TechnoClass* pAnimInvoker, HouseClass* pHouseVictim)
{
	auto animType = Type->AnimTrailType.WhileDrivingAnim;

	switch (drivingState)
	{
	case DrivingState::Start:
		animType = Type->AnimTrailType.StartDrivingAnim;
		break;
	case DrivingState::Stop:
		animType = Type->AnimTrailType.StopDrivingAnim;
		break;
	}

	if (animType)
	{
		auto pAnim = GameCreate<AnimClass>(animType, sourcePos);
		auto const pTypeExt = AnimTypeExtContainer::Instance.Find(animType);

		if(!pTypeExt->NoOwner){
			const auto Owner = pTypeExt->GetAnimOwnerHouseKind();

			if ((Owner == OwnerHouseKind::Invoker && pAnimInvoker) || (Owner == OwnerHouseKind::Victim && pHouseVictim))
			{
				const auto newOwner = HouseExtData::GetHouseKind(Owner, true, HouseExtData::FindFirstCivilianHouse() ? pHouseVictim : nullptr, pHouse, pHouseVictim);

				if (!pAnim->Owner || pAnim->Owner != newOwner)
				{
					pAnim->SetHouse(newOwner);
					if (pTypeExt->RemapAnim || pAnim->Type->MakeInfantry > -1 || (pTypeExt->CreateUnitType && pTypeExt->CreateUnitType->RemapAnim.Get(pTypeExt->RemapAnim))) {
						pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
					}


					pAnim->Owner = pHouse;
					((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pAnimInvoker;
				}
			}
		}
	}
}