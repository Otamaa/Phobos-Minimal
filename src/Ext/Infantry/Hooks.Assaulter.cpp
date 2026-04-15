#include "Body.h"

//https://blueprints.launchpad.net/ares/+spec/assaulter-veterancy
//522A09
ASMJIT_PATCH(0x522A09, InfantryClass_EnteredThing_Assaulter, 0x6)
{
	enum { retTrue = 0x522A11, retFalse = 0x522A45 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F580
ASMJIT_PATCH(0x51F580, InfantryClass_Mission_Hunt_Assaulter, 0x6)
{
	enum { retTrue = 0x51F58A, retFalse = 0x51F5C0 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F493
ASMJIT_PATCH(0x51F493, InfantryClass_Mission_Attack_Assaulter, 0x6)
{
	enum { retTrue = 0x51F49D, retFalse = 0x51F4D3 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51968E
ASMJIT_PATCH(0x51968E, InfantryClass_UpdatePosition_Assaulter, 0x6)
{
	enum { retTrue = 0x5196A6, retFalse = 0x519698 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//4D4BA0
ASMJIT_PATCH(0x4D4BA0, InfantryClass_Mission_Capture_Assaulter, 0x6)
{
	enum { retTrue = 0x4D4BB4, retFalse = 0x4D4BAA };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

ASMJIT_PATCH(0x457DAD, BuildingClass_CanBeOccupied_Assaulter, 0x6)
{
	enum { retTrue = 0x457DD5, retFalse = 0x457DA3 };

	GET(BuildingClass* const, pThis, ESI);
	GET(InfantryClass* const, pInfantry, EDI);

	if (TechnoExtData::IsAssaulter(pInfantry))
	{
		if (!pThis->Owner->IsAlliedWith(pInfantry) && pThis->GetOccupantCount() > 0)
		{
			const auto pBldExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

			// buildings with negative level are not assaultable
			if (pBldExt->AssaulterLevel >= 0)
			{
				// assaultable if infantry has same level or more
				if (pBldExt->AssaulterLevel <= TechnoTypeExtContainer::Instance.Find(pInfantry->Type)->AssaulterLevel)
				{
					return retTrue;
				}
			}
		}
	}

	return retFalse;
}
