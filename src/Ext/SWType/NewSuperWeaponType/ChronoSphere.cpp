#include "ChronoSphere.h"

#include <Ext/SWType/Body.h>

bool SW_ChronoSphere::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ChronoSphere);
}

SuperWeaponFlags SW_ChronoSphere::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEVA | SuperWeaponFlags::NoMoney
		| SuperWeaponFlags::NoEvent | SuperWeaponFlags::NoCleanup | SuperWeaponFlags::NoMessage
		| SuperWeaponFlags::PreClick;
}

bool SW_ChronoSphere::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	auto const pSW = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pSW);

	if (pThis->IsCharged)
	{
		auto const pTarget = MapClass::Instance->GetCellAt(Coords);

		// remember the current source position
		pThis->ChronoMapCoords = Coords;

		// position to play the animation at
		auto coords = pTarget->GetCoordsWithBridge();
		coords.Z += pData->SW_AnimHeight;

		// recoded to support customizable anims
		// and visibility for allies, too.
		if (auto const pAnimType = GetAnim(pData))
		{
			SWTypeExtData::CreateChronoAnim(pThis, coords, pAnimType);
		}

		if (IsPlayer)
		{
			// find the corresponding warp SW type.
			int idxWarp = SuperWeaponTypeClass::FindIndexById(pData->SW_PostDependent);

			// fallback to use the first warp if there is no specific one
			auto const& Types = *SuperWeaponTypeClass::Array;

			if (!Types.ValidIndex(idxWarp) || Types[idxWarp]->Type != SuperWeaponType::ChronoWarp)
			{
				for (auto const& pWarp : Types)
				{
					if (pWarp->Type == SuperWeaponType::ChronoWarp)
					{
						idxWarp = Types.GetItemIndex(&pWarp);
						break;
					}
				}
			}

			if (idxWarp == -1) {
				Debug::LogInfo("[ChronoSphere::Activate] There is no SuperWeaponType with Type=ChronoWarp. Aborted.");
			}

			Unsorted::CurrentSWType = idxWarp;
		}
	}

	return true;
}

void SW_ChronoSphere::Initialize(SWTypeExtData* pData)
{
	pData->SW_AnimVisibility = AffectedHouse::Team;
	pData->SW_AnimHeight = 5;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_ChronosphereReady);
	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_ChronosphereDetected);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_ChronosphereActivated);

	pData->SW_AffectsTarget = SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit;
	pData->CursorType = (int)MouseCursorType::Chronosphere;
	pData->AttachedToObject->Action = Action::ChronoSphere;
}

void SW_ChronoSphere::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);

	pData->Chronosphere_KillOrganic.Read(exINI, section, "Chronosphere.KillOrganic");
	pData->Chronosphere_KillTeleporters.Read(exINI, section, "Chronosphere.KillTeleporters");
	pData->Chronosphere_AffectIronCurtain.Read(exINI, section, "Chronosphere.AffectsIronCurtain");
	pData->Chronosphere_AffectUnwarpable.Read(exINI, section, "Chronosphere.AffectsUnwarpable");
	pData->Chronosphere_AffectUndeployable.Read(exINI, section, "Chronosphere.AffectsUndeployable");
	pData->Chronosphere_BlowUnplaceable.Read(exINI, section, "Chronosphere.BlowUnplaceable");
	pData->Chronosphere_ReconsiderBuildings.Read(exINI, section, "Chronosphere.ReconsiderBuildings");
	pData->Chronosphere_Delay.Read(exINI, section, "Chronosphere.Delay");

	pData->Chronosphere_BlastSrc.Read(exINI, section, "Chronosphere.BlastSrc");
	pData->Chronosphere_BlastDest.Read(exINI, section, "Chronosphere.BlastDest");
	pData->Chronosphere_KillCargo.Read(exINI, section, "Chronosphere.KillCargo");
	// reconstruct the original value, then re-read (otherwise buildings will be affected if
	// the SW section is defined in game mode inis or maps without restating SW.AffectsTarget)
	if (!pData->Chronosphere_AffectBuildings) {
		pData->SW_AffectsTarget = (pData->SW_AffectsTarget & ~SuperWeaponTarget::Building);
	}

	pData->SW_AffectsTarget.Read(exINI, section, "SW.AffectsTarget");

	// we handle the distinction between buildings and deployed vehicles ourselves
	pData->Chronosphere_AffectBuildings = ((pData->SW_AffectsTarget & SuperWeaponTarget::Building) != SuperWeaponTarget::None);
	pData->SW_AffectsTarget = (pData->SW_AffectsTarget | SuperWeaponTarget::Building);
}

AnimTypeClass* SW_ChronoSphere::GetAnim(const SWTypeExtData* pData) const
{
	return pData->SW_Anim.Get(RulesClass::Instance->ChronoPlacement);
}

SWRange SW_ChronoSphere::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange{3, 3} : pData->SW_Range;
}

bool SW_ChronoSphere::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}
