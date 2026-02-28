 #include "Body.h"
#include <SpecificStructures.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Locomotor/Cast.h>
#include <Ext/Script/Body.h>
#include <Ext/Anim/Body.h>

#include <RadarEventClass.h>

//TODO : evaluate this
//interesting mechanic to replace negative damage always remove parasite
//https://github.com/Phobos-developers/Phobos/pull/1126
static void applyRemoveParasite(TechnoClass* pThis, args_ReceiveDamage* args)
{
	if (ScriptExtData::IsUnitAvailable(pThis, false))
	{
		if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
		{
			// Ignore other cases that aren't useful for this logic
			if (pFoot->ParasiteEatingMe)
			{
				const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);
				auto parasyte = pFoot->ParasiteEatingMe;

				if (!pWHExt->CanRemoveParasytes.isset() || !pWHExt->CanTargetHouse(parasyte->Owner, pThis))
					return;

				if (pWHExt->CanRemoveParasytes.Get())
				{
					if (pWHExt->CanRemoveParasytes_ReportSound.isset()) {
						auto _sound_coord = parasyte->GetCoords();
						VocClass::SafeImmedietelyPlayAt(pWHExt->CanRemoveParasytes_ReportSound.Get(), &_sound_coord, nullptr);
					}

					// Kill the parasyte
					CoordStruct coord = TechnoExtData::PassengerKickOutLocation(pThis, parasyte, 10);

					if (!pWHExt->CanRemoveParasytes_KickOut.Get() || coord == CoordStruct::Empty)
					{
						//Debug::LogInfo(__FUNCTION__);
						TechnoExtData::HandleRemove(parasyte, args->Attacker, false, false);
					}
					else
					{
						// Kick the parasyte outside
						pFoot->ParasiteEatingMe = nullptr;

						if (!parasyte->Unlimbo(coord, parasyte->PrimaryFacing.Current().GetDir()))
						{
							//Debug::LogInfo(__FUNCTION__);
							TechnoExtData::HandleRemove(parasyte, nullptr, false, false);
							return;
						}

						parasyte->Target = nullptr;
						int paralysisCountdown = pWHExt->CanRemoveParasytes_KickOut_Paralysis.Get() < 0 ? 15 : pWHExt->CanRemoveParasytes_KickOut_Paralysis.Get();

						if (paralysisCountdown > 0)
						{
							parasyte->ParalysisTimer.Start(paralysisCountdown);
							parasyte->RearmTimer.Start(paralysisCountdown);
						}

						if (pWHExt->CanRemoveParasytes_KickOut_Anim.isset() && pWHExt->CanRemoveParasytes_KickOut_Anim)
						{
							auto const pAnim = GameCreate<AnimClass>(pWHExt->CanRemoveParasytes_KickOut_Anim.Get(), parasyte->GetCoords());
							AnimExtData::SetAnimOwnerHouseKind(pAnim, args->SourceHouse ? args->SourceHouse : parasyte->Owner, pThis->Owner, parasyte, false, false);
							pAnim->SetOwnerObject(parasyte);
						}
					}
				}
			}
		}
	}
}
#undef REPLACE_ARMOR

#include <Ext/Super/Body.h>
#include <New/PhobosAttachedAffect/Functions.h>

ASMJIT_PATCH(0x6F6AC4, TechnoClass_Limbo_AfterRadioClassRemove, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pThis->Owner && pThis->Owner->CountOwnedAndPresent(pTypeExt->This()) <= 0 && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsOwned();

	if (const auto pShieldData = pExt->GetShield())
		pShieldData->OnRemove();

	bool markForRedraw = false;
	bool altered = false;

	// Do not remove attached effects from undeploying buildings.
	if (auto const pBuilding = cast_to<BuildingClass*, false>(pThis)) {
		if ((pBuilding->Type->UndeploysInto && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus == 2)) {
			return 0;
		}
	}

	pExt->PhobosAE.remove_all_if([&](auto& it){

		if(!it)
			return true;

		if ((it->GetType()->DiscardOn & DiscardCondition::Entry) != DiscardCondition::None) {
			altered = true;

			if (it->GetType()->HasTint())
				markForRedraw = true;

			if (it->ResetIfRecreatable()) {
				return false;
			}

			return true;
		}

		return false;
	});

	if (altered)
		AEProperties::Recalculate(pThis);

	if (markForRedraw)
		pThis->MarkForRedraw();

	return 0;
}