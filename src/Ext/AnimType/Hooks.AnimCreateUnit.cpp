// Anim-to--Unit
// Author: Otamaa

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFS(0x44, -0x4));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	R->ECX(R->ESI());
	pExt->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
	pThis->Destroy();
	return 0x737F74;
}

DEFINE_HOOK(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage) {
		AnimTypeExt::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

DEFINE_HOOK(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
//DEFINE_HOOK(0x4226F0, AnimClass_CTOR_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();

		if (auto pCell = pThis->GetCell())
			Location = pCell->GetCoordsWithBridge();
		else
			Location.Z = MapClass::Instance->GetCellFloorHeight(Location);

		pThis->MarkAllOccupationBits(Location);
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;

}

DEFINE_HOOK(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	{
		if (const auto unit = pTypeExt->CreateUnit.Get())
		{
			HouseClass* decidedOwner = (pThis->Owner)
				? pThis->Owner :HouseExt::FindCivilianSide();
		

			//if (!AnimExt::ExtMap.Find(pThis)->OwnerSet)
			//	decidedOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, nullptr, decidedOwner, nullptr);

			const auto pCell = pThis->GetCell();
			CoordStruct location = pThis->GetCoords();

			if (pCell)
				location = pCell->GetCoordsWithBridge();
			else
				location.Z = MapClass::Instance->GetCellFloorHeight(location);

			pThis->UnmarkAllOccupationBits(location);

			if (const auto pTechno = static_cast<TechnoClass*>(unit->CreateObject(decidedOwner)))
			{
				bool success = false;
				const auto pExt = AnimExt::ExtMap.Find(pThis);
				{
					const short resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() && pExt->DeathUnitFacing.has_value())
						? pExt->DeathUnitFacing.get() : pTypeExt->CreateUnit_RandomFacing.Get()
						? ScenarioClass::Instance->Random.RandomRangedSpecific<unsigned short>(0, 255) : pTypeExt->CreateUnit_Facing.Get();

					if (pCell)
						pTechno->OnBridge = pCell->ContainsBridge();

					const BuildingClass* pBuilding = pCell ?
					pCell->GetBuilding() : MapClass::Instance->GetCellAt(location)->GetBuilding();

					if (!pBuilding && !pTypeExt->CreateUnit_ConsiderPathfinding.Get())
					{
						++Unsorted::IKnowWhatImDoing;
						success = pTechno->Unlimbo(location, static_cast<DirType>(resultingFacing));
						--Unsorted::IKnowWhatImDoing;
					}
					else
					{
						success = pTechno->Unlimbo(location, static_cast<DirType>(resultingFacing));
					}

					if (success)
					{
						if (const auto pCreateUnitAnimType = pTypeExt->CreateUnit_SpawnAnim.Get(nullptr)) {
							if (auto const pCreateUnitAnim = GameCreate<AnimClass>(pCreateUnitAnimType, location)) {
								pCreateUnitAnim->Owner = decidedOwner;
								if (auto pCreateUnitAnimExt = AnimExt::ExtMap.Find(pCreateUnitAnim))
									pCreateUnitAnimExt->Invoker = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
							}
						}

						//if (const auto pFoot = generic_cast<FootClass*>(pTechno))
							//if (const auto pLoco = pFoot->Locomotor.get())
								//pLoco->Process();

						if (!decidedOwner->IsNeutral() && !unit->Insignificant) {
							decidedOwner->RegisterGain(pTechno, false);
							decidedOwner->AddTracking(pTechno);
							decidedOwner->RecheckTechTree = 1;
						}

						if (pTechno->HasTurret() && pExt->DeathUnitTurretFacing.has_value()){
							pTechno->SecondaryFacing.Set_Desired(pExt->DeathUnitTurretFacing.get());
						}

						pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
					}
					else
					{
						if (pTechno) {
							TechnoExt::HandleRemove(pTechno);
						}
					}
				}
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}

#include <Ext/Bullet/Body.h>

// this set after ares set their ownership
DEFINE_HOOK(0x469C98, BulletClass_Logics_DamageAnimSelected, 0x9) //was 0
{
	enum { Continue = 0x469D06, NukeWarheadExtras = 0x469CAF };

	GET(BulletClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	if (pAnim) {
		HouseClass* pInvoker =  nullptr;
		HouseClass* pVictim = nullptr;

		if(auto pTech = pThis->Owner) {
			pInvoker = pThis->Owner->GetOwningHouse();
			AnimExt::ExtMap.Find(pAnim)->Invoker = pTech;
		}
		else
		{
			pInvoker = BulletExt::ExtMap.Find(pThis)->Owner;
		}

		if (TechnoClass* Target = generic_cast<TechnoClass*>(pThis->Target))
			pVictim = Target->Owner;

		AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pVictim, pInvoker);

	} else if (pWarheadExt->IsNukeWarhead.Get()) {
		return NukeWarheadExtras;
	}

	return Continue;
}

DEFINE_HOOK(0x6E2368, ActionClass_PlayAnimAt, 0x7)
{
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x4));

	if (pAnim) {
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pHouse,false);
	}

	return 0;
}