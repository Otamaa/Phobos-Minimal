#include "Body.h"

#include <Ext/House/Body.h>

#include <ColorScheme.h>

std::vector<CellClass*> AnimExt::AnimCellUpdater::Marked;

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

	if (!pTypeExt || !pTypeExt->CreateUnit.Get())
	{
		if(!pAnim->Owner && pInvoker)
			pAnim->SetHouse(pInvoker);

		return false;
	}

	if (auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
	{
		pAnim->SetHouse(newOwner);

		if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

		return true;
	}

	return false;
}

TechnoClass* AnimExt::GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	auto pExt = AnimExtAlt::GetExtData(pThis);
	return pExt && pExt->Invoker ? pExt->Invoker : abstract_cast<TechnoClass*>(pThis->OwnerObject);
}

bool AnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(AnimCellUpdater::Marked)
		.Success();
}

bool AnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(AnimCellUpdater::Marked)
		.Success();
}