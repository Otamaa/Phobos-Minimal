#include "Body.h"

#include <Ares_TechnoExt.h>

#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

// This function controls the availability of super weapons. If a you want to
// add to or change the way the game thinks a building provides a super weapon,
// change the lambda UpdateStatus. Available means this super weapon exists at
// all. Setting it to false removes the super weapon. PowerSourced controls
// whether the super weapon charges or can be used.
void SuperExtData::UpdateSuperWeaponStatuses(HouseClass* pHouse)
{
	// look at every sane building this player owns, if it is not defeated already.
	if (!pHouse->Defeated && !pHouse->IsObserver())
	{
		if (pHouse->Supers.Count > 0) {
			pHouse->Supers.for_each([pHouse](SuperClass* pSuper) {
				auto pExt = SuperExtContainer::Instance.Find(pSuper);
				pExt->Statusses.reset();

				//if AlwaysGranted and SWAvaible
				pExt->Statusses.PowerSourced = !pSuper->IsPowered();
				if (pExt->Type->SW_AlwaysGranted && pExt->Type->IsAvailable(pHouse))
				{
					pExt->Statusses.Available = true;
					pExt->Statusses.Charging = true;
					pExt->Statusses.PowerSourced = true;
				}
			});
		}

		pHouse->Buildings.for_each([=](BuildingClass* pBld) {
			if (pBld->IsAlive && !pBld->InLimbo)
			{
				bool PowerChecked = false;
				bool HasPower = false;

				// check for upgrades. upgrades can give super weapons, too.
				for (const auto type : pBld->GetTypes())
				{
					if (auto pUpgradeExt = BuildingTypeExtContainer::Instance.TryFind(const_cast<BuildingTypeClass*>(type)))
					{
						for (auto i = 0; i < pUpgradeExt->GetSuperWeaponCount(); ++i)
						{
							const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i);

							if (idxSW >= 0)
							{
								const auto pSuperExt = SuperExtContainer::Instance.Find(pHouse->Supers[idxSW]);
								auto& status = pSuperExt->Statusses;

								if (!status.Charging)
								{
									if (pSuperExt->Type->IsAvailable(pHouse))
									{
										status.Available = true;

										if (!PowerChecked)
										{
											HasPower = pBld->HasPower
												&& !pBld->IsUnderEMP()
												&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExt_ExtData::IsOperated(pBld));

											PowerChecked = true;
										}

										if (!status.Charging && HasPower)
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
		});

		// kill off super weapons that are disallowed and
		// factor in the player's power status
		const bool hasPower = pHouse->HasFullPower();
		const bool bIsSWShellEnabled = Unsorted::SWAllowed || SessionClass::Instance->GameMode == GameMode::Campaign;

		if (!hasPower || !bIsSWShellEnabled) {
			pHouse->Supers.for_each([&](SuperClass* pSuper) {

				const auto pExt = SuperExtContainer::Instance.Find(pSuper);
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
			});
		}
	}
}


// =============================
// load / save

template <typename T>
void SuperExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Initialized)
		.Process(this->Type, true)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)
		.Process(this->CameoFirstClickDone)
		.Process(this->FirstClickAutoFireDone)
		//.Process(this->Firer)
		.Process(this->Statusses)
		;
}

// =============================
// container
SuperExtContainer SuperExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x6CAF32 , SuperClass_CTOR, 0x6)
DEFINE_HOOK(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);

	if (auto pExt = SuperExtContainer::Instance.FindOrAllocate(pItem)) {
		pExt->Type = SWTypeExtContainer::Instance.TryFind(pItem->Type);
	}

	return 0;
}

DEFINE_HOOK(0x6CB1BD, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CDEF0, SuperClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6CDFD0, SuperClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SuperClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CDFC4, SuperClass_Load_Suffix, 0x7)
{
	SuperExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CDFE8, SuperClass_Save_Suffix, 0x5)
{
	SuperExtContainer::Instance.SaveStatic();
	return 0;
}

// DEFINE_HOOK(0x6CE001 , SuperClass_Detach , 0x5)
// {
// 	GET(SuperClass*, pThis, ESI);
// 	GET(void*, target, EAX);
// 	GET_STACK(bool, all, STACK_OFFS(0x4, -0x8));
//
// 	SuperExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//
// 	return target == pThis->Type ? 0x6CE006 : 0x6CE009;
// }

//void __fastcall SuperClass_Detach_Wrapper(SuperClass* pThis ,DWORD , AbstractClass* target , bool all)\
//{
//	SuperExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//	pThis->SuperClass::PointerExpired(target , all);
//}
//DEFINE_JUMP(VTABLE, 0x7F4010, GET_OFFSET(SuperClass_Detach_Wrapper))
