#include "Body.h"

#include <Misc/AresData.h>
#include <Ext/Building/Body.h>

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
void SuperExt::UpdateSuperWeaponStatuses(HouseClass* pHouse)
{
	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && !pHouse->IsObserver())
	{
		if (pHouse->Supers.Count > 0)
		{
			for (int i = 0; i < pHouse->Supers.Count; ++i)
			{
				const auto pSuper = pHouse->Supers[i];
				auto pExt = SuperExt::ExtMap.Find(pSuper);
				pExt->Statusses.reset();

				//if AlwaysGranted and SWAvaible

				if (pExt->Type->SW_AlwaysGranted && pExt->Type->IsAvailable(pHouse)) {

					pExt->Statusses.Available = true;
					pExt->Statusses.Charging = true;
					pExt->Statusses.PowerSourced = !pSuper->IsPowered();
				}
			}
		}

		for (auto pBld : pHouse->Buildings)
		{
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				bool Operatored = false;
				bool IsPowered = false;
				BuildingTypeExt::ExtData* Types[4] = {
					BuildingTypeExt::ExtMap.Find(pBld->Type)  ,
					BuildingTypeExt::ExtMap.TryFind(pBld->Upgrades[0]) ,
					BuildingTypeExt::ExtMap.TryFind(pBld->Upgrades[1]) ,
					BuildingTypeExt::ExtMap.TryFind(pBld->Upgrades[2])
				};

				// check for upgrades. upgrades can give super weapons, too.
				for (const auto& pUpgradeExt : Types) {
					if (pUpgradeExt) {

						for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i) {
							const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i);

							if (idxSW >= 0)
							{
								const auto pSuperExt = SuperExt::ExtMap.Find(pHouse->Supers[idxSW]);
								auto& status = pSuperExt->Statusses;

								if (!status.Charging)
								{
									if (pSuperExt->Type->IsAvailable(pHouse))
									{
										status.Available = true;

										if (!Operatored)
										{
											IsPowered = pBld->HasPower;
											if (!pBld->IsUnderEMP() && (Is_Operated(pBld) || AresData::IsOperated(pBld)))
												Operatored = true;
										}

										if (!status.Charging && IsPowered)
										{
											status.PowerSourced = true;

											if (!pBld->IsBeingWarpedOut()
												&& (pBld->CurrentMission != Mission::Construction)
												&& (pBld->CurrentMission != Mission::Selling)
												&& (pBld->QueuedMission != Mission::Construction)
												&& (pBld->QueuedMission != Mission::Selling))
											{
												status.Charging = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		const bool hasPower = pHouse->HasFullPower();
		const bool bIsSWShellEnabled = Unsorted::SWAllowed || SessionClass::Instance->GameMode == GameMode::Campaign;

		if (!hasPower || !bIsSWShellEnabled)
		{
			for (auto const& pSuper : pHouse->Supers)
			{
				const auto pExt = SuperExt::ExtMap.Find(pSuper);
				auto& nStatus = pExt->Statusses;

				// turn off super weapons that are disallowed.
				if (!bIsSWShellEnabled && pSuper->Type->DisableableFromShell)
				{
					nStatus.Available = false;
				}

				// if the house is generally on low power,
				// powered super weapons aren't powered
				if (!hasPower && pSuper->IsPowered())
				{
					nStatus.PowerSourced &= hasPower;
				}
			}
		}
	}
}


// =============================
// load / save

template <typename T>
void SuperExt::ExtData::Serialize(T& Stm) { 

	Stm
		.Process(this->Initialized)
		.Process(this->Type)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)
		.Process(this->Firer)
		.Process(this->Statusses)
		.Process(this->LauchDatas)
		;
}

// =============================
// container
SuperExt::ExtContainer SuperExt::ExtMap;

SuperExt::ExtContainer::ExtContainer() : Container("SuperClass") { }
SuperExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks


DEFINE_HOOK_AGAIN(0x6CAF32 , SuperClass_CTOR, 0x6)
DEFINE_HOOK(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);

	if (auto pExt = SuperExt::ExtMap.FindOrAllocate(pItem)) {
		pExt->Type = SWTypeExt::ExtMap.TryFind(pItem->Type);
	}

	return 0;
}

DEFINE_HOOK(0x6CB1BD, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CDEF0, SuperClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6CDFD0, SuperClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SuperClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CDFC4, SuperClass_Load_Suffix, 0x7)
{
	SuperExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CDFE8, SuperClass_Save_Suffix, 0x5)
{
	SuperExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x6CE001 , SuperClass_Detach , 0x5)
{
	GET(SuperClass*, pThis, ESI);
	GET(void*, target, EAX);
	GET_STACK(bool, all, STACK_OFFS(0x4, -0x8));

	SuperExt::ExtMap.InvalidatePointerFor(pThis , target , all);

	return target == pThis->Type ? 0x6CE006 : 0x6CE009;
}