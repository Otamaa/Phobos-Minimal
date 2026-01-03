#include "ExtraFirefunctional.h"
#include "ExtraFire.h"

#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Misc/DynamicPatcher/CustomWeapon/CustomWeapon.h>

static Iterator<WeaponTypeClass*> GetWeaponAndFLH(TechnoClass* pThis, const ExtraFireData& nExtraFireData, int nWeaponIdx , CoordStruct& selectedFLh)
{
	auto const pType = GET_TECHNOTYPE(pThis);

	ExtraFireData::FLHData* ptrFlhData = nullptr;

	if (auto pTransporter = pThis->Transporter) {
		ptrFlhData = GET_TECHNOTYPEEXT(pTransporter)->MyExtraFireData.AttachedFLH.AsPointer();
	} else {
		ptrFlhData = nExtraFireData.AttachedFLH.AsPointer();
	}

	const bool IsElite = pThis->Veterancy.IsElite();

	if (pType->WeaponCount > 0)
	{
		if (IsElite ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.empty() :
			nExtraFireData.AttachedWeapon.WeaponX.empty())
			return {};

		if ((IsElite ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.size() :
			nExtraFireData.AttachedWeapon.WeaponX.size())
			< (size_t)nWeaponIdx)
			return {};

		if (!(!IsElite ?
			(ptrFlhData)->WeaponXFLH.empty() :
			(ptrFlhData)->EliteWeaponXFLH.empty()))
		{
			if (!((!IsElite?
				(ptrFlhData)->WeaponXFLH.size() :
				(ptrFlhData)->EliteWeaponXFLH.size())
				< (size_t)nWeaponIdx))
			{
				selectedFLh = *((!IsElite ?
					(ptrFlhData)->WeaponXFLH :
					(ptrFlhData)->EliteWeaponXFLH).begin() + nWeaponIdx);
			}
		}

		const auto selected =  (IsElite ? nExtraFireData.AttachedWeapon.EliteWeaponX : nExtraFireData.AttachedWeapon.WeaponX).begin() + nWeaponIdx;
		return make_iterator(selected->data(), selected->size());
	}
	else
	{
		switch (nWeaponIdx)
		{
		case 0:
		{
			if (IsElite ?
				nExtraFireData.AttachedWeapon.ElitePrimaryWeapons.empty() :
				nExtraFireData.AttachedWeapon.PrimaryWeapons.empty()
				)
				return {};

			selectedFLh = (IsElite ? (ptrFlhData)->ElitePrimaryWeaponFLH : (ptrFlhData)->PrimaryWeaponFLH).Get();
			const std::vector<WeaponTypeClass*>* vec = &(IsElite ? nExtraFireData.AttachedWeapon.ElitePrimaryWeapons : nExtraFireData.AttachedWeapon.PrimaryWeapons);
			return make_iterator(vec->data(), vec->size());
		}
		case 1:
		{
			if (IsElite ?
				nExtraFireData.AttachedWeapon.EliteSecondaryWeapons.empty() :
				nExtraFireData.AttachedWeapon.SecondaryWeapons.empty())
				return {};

			selectedFLh = (pThis->Veterancy.IsElite() ?
				(ptrFlhData)->EliteSecondaryWeaponFLH :
				(ptrFlhData)->SecondaryWeaponFLH);

			const std::vector<WeaponTypeClass*>* vec = &(IsElite ? nExtraFireData.AttachedWeapon.EliteSecondaryWeapons : nExtraFireData.AttachedWeapon.SecondaryWeapons);
			return make_iterator(vec->data(), vec->size());
		}
		break;
		}
	}

	return {};
}

void ExtraFirefunctional::GetWeapon(TechnoClass* pThis, AbstractClass* pTarget, int nWeaponIdx)
{
	if (!pTarget)
		return;

	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto& nExtraFireData = pTypeExt->MyExtraFireData;
	CoordStruct nFLH = CoordStruct::Empty;
	const auto nSelectedWeapon=  GetWeaponAndFLH(pThis, nExtraFireData, nWeaponIdx , nFLH);

	if (nSelectedWeapon.empty())
		return;

	const auto ROF = TechnoExtData::GetROFMult(pThis);

	if (nFLH == CoordStruct::Empty) {
		if(auto const pWPStr = pThis->GetWeapon(nWeaponIdx)) {
			nFLH = pWPStr->FLH;
		}
	}

	nSelectedWeapon.for_each([&](WeaponTypeClass* pWeapon){
		bool bFire = true;
		int nRof = 0;
		if (auto const pWeaponTypeExt = WeaponTypeExtContainer::Instance.Find(pWeapon))
		{
			const auto& fireData = pWeaponTypeExt->MyAttachFireDatas;

			if (fireData.UseROF)
			{
				auto& nTimer = pExt->ExtraWeaponTimers[pWeapon];
				bFire = false;
				nRof = (int)(pWeapon->ROF * ROF);

				if (nTimer.Expired())
				{
					nTimer.Start(nRof);
					pExt->MyWeaponManager.FireCustomWeapon(pThis, pThis, pTarget, pWeapon, nFLH, CoordStruct::Empty, nRof);
				}
			}
		}

		if (bFire)
			pExt->MyWeaponManager.FireCustomWeapon(pThis, pThis, pTarget, pWeapon, nFLH, CoordStruct::Empty, ROF);
	});
}
