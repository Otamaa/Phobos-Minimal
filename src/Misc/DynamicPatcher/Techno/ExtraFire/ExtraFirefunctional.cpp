#ifdef COMPILE_PORTED_DP_FEATURES
#include "ExtraFirefunctional.h"
#include "ExtraFire.h"

#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Misc/DynamicPatcher/CustomWeapon/CustomWeapon.h>

void ExtraFirefunctional::GetWeapon(TechnoClass* pThis, AbstractClass* pTarget, int nWeaponIdx)
{
	// wtf is this , really

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt || !pType || !pTarget)
		return;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pTypeExt)
		return;

	const auto&  nExtraFireData = pTypeExt->MyExtraFireData;

	ExtraFireData::FLHData nSelectedFLH = nExtraFireData.AttachedFLH;
	std::vector<WeaponTypeClass*> nSelectedWeapon;

	if (auto pTransporter = pThis->Transporter)
	{
		if (auto const pTrasTypeExt = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType()))
		{
			const auto& pTransExtraFire = pTrasTypeExt->MyExtraFireData;
			nSelectedFLH = pTransExtraFire.AttachedFLH;
		}
	}

	CoordStruct nFLH = CoordStruct::Empty;

	if (pType->WeaponCount > 0)
	{
		if (pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.empty() :
			nExtraFireData.AttachedWeapon.WeaponX.empty())
			return;

		if ((pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX.size() :
			nExtraFireData.AttachedWeapon.WeaponX.size())
			< (size_t)nWeaponIdx)
			return;

		nSelectedWeapon = (pThis->Veterancy.IsElite() ?
			nExtraFireData.AttachedWeapon.EliteWeaponX :
			nExtraFireData.AttachedWeapon.WeaponX)[(size_t)nWeaponIdx];

		if (!(!pThis->Veterancy.IsElite() ?
			nSelectedFLH.WeaponXFLH.empty() :
			nSelectedFLH.EliteWeaponXFLH.empty()))
		{
			if (!((!pThis->Veterancy.IsElite() ?
				nSelectedFLH.WeaponXFLH.size() :
				nSelectedFLH.EliteWeaponXFLH.size())
				< (size_t)nWeaponIdx))
			{
				nFLH = (!pThis->Veterancy.IsElite() ?
					nSelectedFLH.WeaponXFLH :
					nSelectedFLH.EliteWeaponXFLH)[(size_t)nWeaponIdx];
			}
		}
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
					return;

				nSelectedWeapon = (pThis->Veterancy.IsElite() ?
					nExtraFireData.AttachedWeapon.ElitePrimaryWeapons :
					nExtraFireData.AttachedWeapon.PrimaryWeapons);

				nFLH = (pThis->Veterancy.IsElite() ?
						nSelectedFLH.ElitePrimaryWeaponFLH :
						nSelectedFLH.PrimaryWeaponFLH);

			}
			break;
		case 1:
			{
				if (pThis->Veterancy.IsElite() ?
					nExtraFireData.AttachedWeapon.EliteSecondaryWeapons.empty() :
					nExtraFireData.AttachedWeapon.SecondaryWeapons.empty())
						return;

				nSelectedWeapon = (pThis->Veterancy.IsElite() ?
					nExtraFireData.AttachedWeapon.EliteSecondaryWeapons :
					nExtraFireData.AttachedWeapon.SecondaryWeapons);

				nFLH = (pThis->Veterancy.IsElite() ?
						nSelectedFLH.EliteSecondaryWeaponFLH :
						nSelectedFLH.SecondaryWeaponFLH);
			}
		    break;
		}
	}

	if (!nSelectedWeapon.empty())
	{
		const auto ROF = Helpers_DP::GetROFMult(pThis);

		if (nFLH == CoordStruct::Empty)
			nFLH = pThis->GetWeapon(nWeaponIdx)->FLH;

		for (auto const& pWeapon: nSelectedWeapon)
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
}
#endif