#include "FighterAreaGuardFunctional.h"
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

void FighterAreaGuardFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->OwnerObject()->Spawned)
		return;

	if (!pTypeExt->MyFighterData.AreaGuard)
		return;

	auto &nData = pExt->MyFighterData;
	auto& nDataType = pTypeExt->MyFighterData;

	if (pExt->OwnerObject()->CurrentMission == Mission::Move)
	{
		nData.isAreaProtecting = false;
		nData.isAreaGuardReloading = false;
		return;
	}

	if (auto pFoot = (FootClass*)(pExt->OwnerObject()))
	{
		if (!nData.isAreaProtecting)
		{
			if (pExt->OwnerObject()->CurrentMission == Mission::Area_Guard)
			{
				nData.isAreaProtecting = true;
				CoordStruct dest = pFoot->Locomotor.get()->Destination();
				nData.areaProtectTo = dest;
			}
		}

		if (nData.isAreaProtecting)
		{
			//没弹药的情况下返回机场
			if (pExt->OwnerObject()->Ammo == 0 && !nData.isAreaGuardReloading)
			{
				pExt->OwnerObject()->SetTarget(nullptr);
				pExt->OwnerObject()->SetDestination(nullptr, false);
				pExt->OwnerObject()->ForceMission(Mission::Stop);
				nData.isAreaGuardReloading = true;
				return;
			}

			//填弹完毕后继续巡航
			if (nData.isAreaGuardReloading)
			{
				if (pExt->OwnerObject()->Ammo >= nDataType.MaxAmmo)
				{
					nData.isAreaGuardReloading = false;
					pExt->OwnerObject()->ForceMission(Mission::Area_Guard);
				}
				else
				{
					if (pExt->OwnerObject()->CurrentMission != Mission::Sleep &&
						pExt->OwnerObject()->CurrentMission != Mission::Enter)
					{
						if (pExt->OwnerObject()->CurrentMission == Mission::Guard)
						{
							pExt->OwnerObject()->ForceMission(Mission::Sleep);
						}
						else
						{
							pExt->OwnerObject()->ForceMission(Mission::Enter);
						}
						return;
					}
				}
			}

			if (pExt->OwnerObject()->CurrentMission == Mission::Move)
			{
				nData.isAreaProtecting = false;
				return;
			}
			else if (pExt->OwnerObject()->CurrentMission == Mission::Attack)
			{
				return;
			}
			else if (pExt->OwnerObject()->CurrentMission == Mission::Enter)
			{
				if (nData.isAreaGuardReloading)
				{
					return;
				}
				else
				{
					pExt->OwnerObject()->ForceMission(Mission::Stop);
				}
			}
			else if (pExt->OwnerObject()->CurrentMission == Mission::Sleep)
			{
				if (nData.isAreaGuardReloading)
				{
					return;
				}
			}

			if (nData.areaProtectTo)
			{
				auto dest = nData.areaProtectTo;
				auto house = pExt->OwnerObject()->Owner;

				if (pTypeExt->MyFighterData.AutoFire)
				{
					if (nData.areaProtectTo.DistanceFrom(pExt->OwnerObject()->GetCoords()) <= 2000)
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
										pTech == pExt->OwnerObject() ||
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
								pExt->OwnerObject()->SetTarget(pTarget);
								pExt->OwnerObject()->ForceMission(Mission::Stop);
								pExt->OwnerObject()->ForceMission(Mission::Attack);
								return;
							}
						}
					}
				}

				if (nData.areaProtectTo.DistanceFrom(pExt->OwnerObject()->GetCoords()) <= 2000)
				{
					if (nData.currentAreaProtectedIndex > (int)(nData.areaGuardCoords.size() - 1))
					{
						nData.currentAreaProtectedIndex = 0;
					}
					dest += nData.areaGuardCoords[nData.currentAreaProtectedIndex];
					nData.currentAreaProtectedIndex++;
				}

				pFoot->Locomotor.get()->Move_To(dest);
				if (auto const pCell = Map[CellClass::Coord2Cell(dest)]) {
					pExt->OwnerObject()->SetDestination(pCell, false);
				}
			}
		}
	}
}
#endif