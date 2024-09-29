#include "MeteorShower.h"

#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/SWType/Body.h>

std::vector<const char*> SW_MeteorShower::GetTypeString() const
{
	return { "MeteorShower" };
}

// TODO : support deferment
bool SW_MeteorShower::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		if (const auto pCell = MapClass::Instance->TryGetCellAt(Coords))
		{
			auto const pData = SWTypeExtContainer::Instance.Find(pThis->Type);
			auto pFirer = this->GetFirer(pThis, Coords, false);

			const auto nCoord = pCell->GetCoordsWithBridge();

			const int count =  ScenarioClass::Instance->Random.RandomFromMax(pData->MeteorCounts);


			AnimTypeClass* large_meteor = pData->MeteorLarge;
			AnimTypeClass* small_meteor = pData->MeteorSmall;
			VoxelAnimTypeClass* large_Impact = pData->MeteorImpactLarge;
			VoxelAnimTypeClass* small_Impact = pData->MeteorImpactSmall;
			const int nMaxForrand = count * 70;

			for (int i = 0; i < count; ++i)
			{
				const int x_adj = ScenarioClass::Instance->Random.Random() % (nMaxForrand);
				const int y_adj = ScenarioClass::Instance->Random.Random() % (nMaxForrand);

				Coordinate nwhere = nCoord;

				nwhere.X += x_adj;
				nwhere.Y += y_adj;

				nwhere.Z = MapClass::Instance->GetCellFloorHeight(nwhere);

				if (AnimTypeClass* anim = ScenarioClass::Instance->Random.PercentChance(pData->MeteorKindChance) ?
					large_meteor : small_meteor)
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(anim, nwhere),
						pThis->Owner,
						nullptr,
						pFirer,
						false
					);
				}
			}

			if (ScenarioClass::Instance->Random.PercentChance(pData->MeteorAddImpactChance))
			{
				for (int a = 0; a < pData->MeteorImactCounts; ++a)
				{
					Coordinate im_where = nCoord;

					const int x_adj = ScenarioClass::Instance->Random.Random() % (pData->MeteorImactCounts);
					const int y_adj = ScenarioClass::Instance->Random.Random() % (pData->MeteorImactCounts);

					im_where.X += x_adj;
					im_where.Y += y_adj;

					im_where.Z = MapClass::Instance->GetCellFloorHeight(im_where);
					if (VoxelAnimTypeClass* impact = ScenarioClass::Instance->Random.PercentChance(pData->MeteorImpactKindChance) ?
						large_Impact : small_Impact) {
						VoxelAnimExtContainer::Instance.Find(GameCreate<VoxelAnimClass>(impact, &im_where, pThis->Owner))->Invoker = pFirer;
					}
				}
			}
		}
	}

	return true;
}

void SW_MeteorShower::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LightningStorm;
	pData->SW_RadarEvent = false;
	pData->MeteorSmall = AnimTypeClass::Find(GameStrings::METSMALL);
	pData->MeteorLarge = AnimTypeClass::Find(GameStrings::METLARGE);

	pData->MeteorImpactSmall = VoxelAnimTypeClass::Find("METEOR02");
	pData->MeteorImpactLarge = VoxelAnimTypeClass::Find("METEOR01");
}

void SW_MeteorShower::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const auto pSection = pData->AttachedToObject->ID;
	INI_EX exINI(pINI);

	pData->MeteorCounts.Read(exINI ,pSection,"Meteor.Count");
	pData->MeteorImactCounts.Read(exINI, pSection, "Meteor.ImpactCount");
	pData->MeteorAddImpactChance.Read(exINI, pSection, "Meteor.AddImpactChance");
	pData->MeteorKindChance.Read(exINI, pSection, "Meteor.KindChance");
	pData->MeteorImpactKindChance.Read(exINI, pSection, "Meteor.ImpactKindChance");

	pData->MeteorSmall.Read(exINI, pSection, "Meteor.SmallAnim");
	pData->MeteorLarge.Read(exINI, pSection, "Meteor.LargeAnim");

	pData->MeteorImpactSmall.Read(exINI, pSection, "Meteor.VoxelAnimImpactSmall");
	pData->MeteorImpactLarge.Read(exINI, pSection, "Meteor.VoxelAnimImpactLarge");
}

bool SW_MeteorShower::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}