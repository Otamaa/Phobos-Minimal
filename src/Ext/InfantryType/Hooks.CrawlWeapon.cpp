#include "Body.h"

template<bool IsElite>
static WeaponStruct* GetCrawlWeapon(InfantryClass* pThis, int nIdx)
{
	if (pThis->Crawling)
	{
		auto pTypeExt = InfantryTypeExt::ExtMap.Find(pThis->Type);

		switch (nIdx)
		{
		case 0:
		{
			if constexpr (IsElite)
				return &pTypeExt->CrawlingWeaponDatas[1];
			else
				return &pTypeExt->CrawlingWeaponDatas[0];
		}
		break;
		case 1:
		{
			if constexpr (IsElite)
				return &pTypeExt->CrawlingWeaponDatas[3];
			else
				return &pTypeExt->CrawlingWeaponDatas[2];
		}
		break;
		}
	}
	return nullptr;
}

DEFINE_HOOK(0x70E163, TechnoClass_GetWeapon_EliteCrawlWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nSlot, EDI);

	if (auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (auto pCrawlWeaponStruct = GetCrawlWeapon<true>(pInf, nSlot))
		{
			if (pCrawlWeaponStruct->WeaponType)
			{
				R->EAX(pCrawlWeaponStruct);
				return 0x70E190;
			}
		}
	}

	R->EAX(pThis->GetTechnoType()->GetEliteWeapon(nSlot));
	return 0x70E190;
}

DEFINE_HOOK(0x70E17E, TechnoClass_GetWeapon_CrawlWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nSlot, EDI);

	if (auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (auto pCrawlWeaponStruct = GetCrawlWeapon<false>(pInf, nSlot)){
			if (pCrawlWeaponStruct->WeaponType)
			{
				R->EAX(pCrawlWeaponStruct);
				return 0x70E190;
			}
		}
	}

	R->EAX(pThis->GetTechnoType()->GetWeapon(nSlot));
	return 0x70E190;
}