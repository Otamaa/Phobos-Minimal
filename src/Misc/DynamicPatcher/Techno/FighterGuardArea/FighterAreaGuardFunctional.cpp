#include "FighterAreaGuardFunctional.h"
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

static constexpr std::array<CoordStruct, 6> areaGuardCoords
{
	{
		  {-300,-300,0}
		, { -300 ,0,0 }
		, { 0,0,0 }
		, { 300,0,0 }
		, {300,300,0 }
		, {0 , 300 ,0 }
	}
};

void FighterAreaGuardFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->Get()->Spawned)
		return;

	if (!pTypeExt->MyFighterData.AreaGuard)
		return;

	auto &nData = pExt->MyFighterData;
	auto& nDataType = pTypeExt->MyFighterData;

	if (pExt->Get()->CurrentMission == Mission::Move)
	{
		nData.isAreaProtecting = false;
		nData.isAreaGuardReloading = false;
		return;
	}

	if (auto pFoot = (FootClass*)(pExt->Get()))
	{
		if (!nData.isAreaProtecting)
		{
			if (pExt->Get()->CurrentMission == Mission::Area_Guard)
			{
				nData.isAreaProtecting = true;
				CoordStruct dest = pFoot->Locomotor.get()->Destination();
				nData.areaProtectTo = dest;
			}
		}

		if (nData.isAreaProtecting)
		{
			//没弹药的情况下返回机场
			if (pExt->Get()->Ammo == 0 && !nData.isAreaGuardReloading)
			{
				pExt->Get()->SetTarget(nullptr);
				pExt->Get()->SetDestination(nullptr, false);
				pExt->Get()->ForceMission(Mission::Stop);
				nData.isAreaGuardReloading = true;
				return;
			}

			//填弹完毕后继续巡航
			if (nData.isAreaGuardReloading)
			{
				if (pExt->Get()->Ammo >= nDataType.MaxAmmo)
				{
					nData.isAreaGuardReloading = false;
					pExt->Get()->ForceMission(Mission::Area_Guard);
				}
				else
				{
					if (pExt->Get()->CurrentMission != Mission::Sleep &&
						pExt->Get()->CurrentMission != Mission::Enter)
					{
						if (pExt->Get()->CurrentMission == Mission::Guard)
						{
							pExt->Get()->ForceMission(Mission::Sleep);
						}
						else
						{
							pExt->Get()->ForceMission(Mission::Enter);
						}
						return;
					}
				}
			}

			if (pExt->Get()->CurrentMission == Mission::Move)
			{
				nData.isAreaProtecting = false;
				return;
			}
			else if (pExt->Get()->CurrentMission == Mission::Attack)
			{
				return;
			}
			else if (pExt->Get()->CurrentMission == Mission::Enter)
			{
				if (nData.isAreaGuardReloading)
				{
					return;
				}
				else
				{
					pExt->Get()->ForceMission(Mission::Stop);
				}
			}
			else if (pExt->Get()->CurrentMission == Mission::Sleep)
			{
				if (nData.isAreaGuardReloading)
				{
					return;
				}
			}

			if (nData.areaProtectTo)
			{
				auto dest = nData.areaProtectTo;
				auto house = pExt->Get()->Owner;

				if (pTypeExt->MyFighterData.AutoFire)
				{
					if (nData.areaProtectTo.DistanceFrom(pExt->Get()->GetCoords()) <= 2000)
					{
						if (nData.areaGuardTargetCheckRof-- <= 0)
						{
							nData.areaGuardTargetCheckRof = 20;

							auto FindOneTechno = [&](HouseClass* pOwner)
							{
								TechnoClass* pDummy = nullptr;
								auto const nArr = TechnoClass::Array();
								for (int i = nArr->Count - 1; i >= 0; i--)
								{
									auto pTech = nArr->GetItem(i);
									if (!pTech ||
										!pTech->GetOwningHouse() ||
										pTech->GetOwningHouse()->IsAlliedWith(pOwner) ||
										pTech->GetOwningHouse() == pOwner ||
										pTech->GetOwningHouse()->IsNeutral()||
										pTech == pExt->Get() ||
										Helpers_DP::IsDeadOrInvisibleOrCloaked(pTech) ||
										pTech->GetTechnoType()->Immune ||
										pTech->GetTechnoType()->Invisible
										)
										continue;

									auto coords = pTech->GetCoords();
									auto height = pTech->GetHeight();
									auto type = pTech->WhatAmI();

									if (pTech->InLimbo)
										continue;

									auto bounsRange = 0;
									if (pTech->GetHeight() > 10)
										bounsRange = nDataType.GuardRange;

									auto nDummy = CoordStruct { 0, 0, height };
									if ((coords - nDummy).DistanceFrom(dest) <= (nDataType.GuardRange * 256 + bounsRange) && type != AbstractType::Building)
									{
										pDummy = pTech;
										break;
									}
								}

								return pDummy;
							};

							if (TechnoClass* pTarget = FindOneTechno(house))
							{
								pExt->Get()->SetTarget(pTarget);
								pExt->Get()->ForceMission(Mission::Stop);
								pExt->Get()->ForceMission(Mission::Attack);
								return;
							}
						}
					}
				}

				if (nData.areaProtectTo.DistanceFrom(pExt->Get()->GetCoords()) <= 2000)
				{
					if (nData.currentAreaProtectedIndex > (int)(areaGuardCoords.size() - 1))
					{
						nData.currentAreaProtectedIndex = 0;
					}
					dest += areaGuardCoords[nData.currentAreaProtectedIndex];
					nData.currentAreaProtectedIndex++;
				}

				pFoot->Locomotor.get()->Move_To(dest);
				if (auto const pCell = Map[CellClass::Coord2Cell(dest)]) {
					pExt->Get()->SetDestination(pCell, false);
				}
			}
		}
	}
}
#endif