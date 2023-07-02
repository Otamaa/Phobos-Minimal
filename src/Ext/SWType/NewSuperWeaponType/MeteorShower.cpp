#include "MeteorShower.h"

#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Misc/AresData.h>

std::vector<const char*> SW_MeteorShower::GetTypeString() const
{
	return { "MeteorShower" };
}

SuperWeaponFlags SW_MeteorShower::Flags() const
{
	return  SuperWeaponFlags::NoEvent;
}

bool SW_MeteorShower::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		if (const auto pCell = MapClass::Instance->TryGetCellAt(Coords))
		{
			auto const pData = SWTypeExt::ExtMap.Find(pThis->Type);
			BuildingClass* pFirer = this->GetFirer(pThis, Coords, false);

			const auto nCoord = pCell->GetCoordsWithBridge();
			const auto _meteor_counts = { 6, 8, 15 };
			const auto _impact_counts = { 2 , 4 , 5 }; // too much of this , causing game to freeze , lmao
			const auto Chances = { 30 , 10 , 50 };

			const int count = *(_meteor_counts.begin() + ScenarioClass::Instance->Random.RandomFromMax(_meteor_counts.size() - 1));
			const int chance = *Chances.begin();
			const int Shower_addImpactChance = *(Chances.begin() + 1);
			const int impact_chance = *(Chances.begin() + 2);
			const int impact_count = *(_impact_counts.begin() + ScenarioClass::Instance->Random.RandomFromMax(_impact_counts.size() - 1));

			AnimTypeClass* large_meteor = AnimTypeClass::Find("METLARGE");
			AnimTypeClass* small_meteor = AnimTypeClass::Find("METSMALL");
			VoxelAnimTypeClass* large_Impact = VoxelAnimTypeClass::Find("METEOR01");
			VoxelAnimTypeClass* small_Impact = VoxelAnimTypeClass::Find("METEOR02");
			const int nMaxForrand = count * 70;

			for (int i = 0; i < count; ++i)
			{
				const int x_adj = ScenarioClass::Instance->Random.Random() % (nMaxForrand);
				const int y_adj = ScenarioClass::Instance->Random.Random() % (nMaxForrand);

				Coordinate nwhere = nCoord;

				nwhere.X += x_adj;
				nwhere.Y += y_adj;
				nwhere.Z = MapClass::Instance->GetCellFloorHeight(nwhere);

				if (AnimTypeClass* anim = ScenarioClass::Instance->Random.PercentChance(chance) ?
					large_meteor : small_meteor)
				{
					if (auto pAnim = GameCreate<AnimClass>(anim, nwhere))
						AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, pFirer, false);
				}
			}

			if (ScenarioClass::Instance->Random.PercentChance(Shower_addImpactChance))
			{
				for (int a = 0; a < impact_count; ++a)
				{
					Coordinate im_where = nCoord;

					const int x_adj = ScenarioClass::Instance->Random.Random() % (impact_count);
					const int y_adj = ScenarioClass::Instance->Random.Random() % (impact_count);

					im_where.X += x_adj;
					im_where.Y += y_adj;
					im_where.Z = MapClass::Instance->GetCellFloorHeight(im_where);
					if (VoxelAnimTypeClass* impact = ScenarioClass::Instance->Random.PercentChance(impact_chance) ?
						large_Impact : small_Impact)
					{
						if (auto pVxl = GameCreate<VoxelAnimClass>(impact, &im_where, pThis->Owner))
						{
							VoxelAnimExt::ExtMap.Find(pVxl)->Invoker = pFirer;
						}
					}
				}
			}
		}
	}

	return true;
}

void SW_MeteorShower::Initialize(SWTypeExt::ExtData* pData)
{
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LightningStorm;
}

void SW_MeteorShower::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	//const auto pSection = pData->Get()->ID;
	//INI_EX exINI(pINI);
}