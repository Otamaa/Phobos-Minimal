#ifdef COMPILE_PORTED_DP_FEATURES
#include "ExtraFirefunctional.h"
#include "ExtraFire.h"

#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Misc/DynamicPatcher/CustomWeapon/CustomWeapon.h>

static std::pair<std::vector<WeaponTypeClass*>, CoordStruct> GetWeaponAndFLH(TechnoClass* pThis, ExtraFireData nExtraFireData, int nWeaponIdx)
{
	auto const pType = pThis->GetTechnoType();

	if (auto pTransporter = pThis->Transporter)
	{
		if (auto const pTrasTypeExt = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType()))
		{
			const auto& pTransExtraFire = pTrasTypeExt->MyExtraFireData;
			nExtraFireData.AttachedFLH = pTransExtraFire.AttachedFLH;
		}
	}

	if (pType->WeaponCount > 0)
	{
		if (pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.empty() :
			nExtraFireData.AttachedWeapon.WeaponX.empty())
			return { {} , CoordStruct::Empty };

		if ((pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.size() :
			nExtraFireData.AttachedWeapon.WeaponX.size())
			< (size_t)nWeaponIdx)
			return { {} , CoordStruct::Empty };

		CoordStruct nFLH {};

		if (!(!pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedFLH.WeaponXFLH.empty() :
			nExtraFireData.AttachedFLH.EliteWeaponXFLH.empty()))
		{
			if (!((!pThis->Veterancy.IsElite() ?
				nExtraFireData.AttachedFLH.WeaponXFLH.size() :
				nExtraFireData.AttachedFLH.EliteWeaponXFLH.size())
				< (size_t)nWeaponIdx))
			{
				nFLH = (!pThis->Veterancy.IsElite() ?
					nExtraFireData.AttachedFLH.WeaponXFLH :
					nExtraFireData.AttachedFLH.EliteWeaponXFLH).at((size_t)nWeaponIdx);
			}
		}

		return { (pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX :
			nExtraFireData.AttachedWeapon.WeaponX).at((size_t)nWeaponIdx),nFLH };
	}
	else
	{
		switch (nWeaponIdx)
		{
		case 0:
		{
			if (pThis->Veterancy.IsElite() ?
				nExtraFireData.AttachedWeapon.ElitePrimaryWeapons.empty() :
				nExtraFireData.AttachedWeapon.PrimaryWeapons.empty()
				)
				return { {} , CoordStruct::Empty };

			return { (pThis->Veterancy.IsElite() ?
				nExtraFireData.AttachedWeapon.ElitePrimaryWeapons :
				nExtraFireData.AttachedWeapon.PrimaryWeapons) ,(pThis->Veterancy.IsElite() ?
					nExtraFireData.AttachedFLH.ElitePrimaryWeaponFLH :
					nExtraFireData.AttachedFLH.PrimaryWeaponFLH)
			};
		}
		break;
		case 1:
		{
			if (pThis->Veterancy.IsElite() ?
				nExtraFireData.AttachedWeapon.EliteSecondaryWeapons.empty() :
				nExtraFireData.AttachedWeapon.SecondaryWeapons.empty())
				return { {} , CoordStruct::Empty };

			return { (pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteSecondaryWeapons :
			nExtraFireData.AttachedWeapon.SecondaryWeapons) ,(pThis->Veterancy.IsElite() ?
				nExtraFireData.AttachedFLH.EliteSecondaryWeaponFLH :
				nExtraFireData.AttachedFLH.SecondaryWeaponFLH)
			};
		}
		break;
		}
	}

	return { {} , CoordStruct::Empty };
}

void ExtraFirefunctional::GetWeapon(TechnoClass* pThis, AbstractClass* pTarget, int nWeaponIdx)
{
	if (!pTarget)
		return;

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto& nExtraFireData = pTypeExt->MyExtraFireData;
	auto [nSelectedWeapon, nFLH] = GetWeaponAndFLH(pThis, nExtraFireData, nWeaponIdx);

	if (nSelectedWeapon.empty())
		return;

	const auto ROF = TechnoExt::GetROFMult(pThis);

	if (nFLH == CoordStruct::Empty) { 
		if(auto const pWPStr = pThis->GetWeapon(nWeaponIdx)) { 
			nFLH = pWPStr->FLH;
		}
	}

	for (auto const& pWeapon : nSelectedWeapon)
	{
		bool bFire = true;
		int nRof = 0;
		if (auto const pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon))
		{
			const auto& fireData = pWeaponTypeExt->MyAttachFireDatas;

			if (fireData.UseROF)
			{
				bFire = false;
				nRof = (int)(pWeapon->ROF * ROF);
				auto& nTimer = pExt->ExtraWeaponTimers[pWeapon];

				if (nTimer.Expired())
				{
					nTimer.Start(nRof);
					pExt->MyWeaponManager.FireCustomWeapon(pThis, pThis, pTarget, pWeapon, nFLH, CoordStruct::Empty, nRof);
				}
			}
		}

		if (bFire)
			pExt->MyWeaponManager.FireCustomWeapon(pThis, pThis, pTarget, pWeapon, nFLH, CoordStruct::Empty, ROF);

	}
}
#endif