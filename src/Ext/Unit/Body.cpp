#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/UnitType/Body.h>

#include <Locomotor/Cast.h>

#include <SlaveManagerClass.h>
#include <GameModeOptionsClass.h>
#include <Ext/Anim/Body.h>
#include <ScenarioClass.h>
#include <GameOptionsClass.h>

UnitExtContainer UnitExtContainer::Instance;
std::vector<UnitExtData*> Container<UnitExtData>::Array;

void Container<UnitExtData>::Clear()
{
	Array.clear();
}

bool UnitExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool UnitExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

bool UnitExtContainer::HasDeployingAnim(UnitTypeClass* pUnitType) {
	return pUnitType->DeployingAnim || !TechnoTypeExtContainer::Instance.Find(pUnitType)->DeployingAnims.empty();
}

bool UnitExtContainer::CheckDeployRestrictions(FootClass* pUnit, bool isDeploying)
{
	// Movement restrictions.
	if (isDeploying && pUnit->Locomotor->Is_Moving_Now())
		return true;

	FacingClass* currentDir = &pUnit->PrimaryFacing;
	bool isJumpjet = false;

	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
	{
		// Jumpjet rotating is basically half a guarantee it is also moving and
		// may not be caught by the Is_Moving_Now() check.
		if (isDeploying && pJJLoco->Facing.Is_Rotating())
			return true;

		currentDir = &pJJLoco->Facing;
		isJumpjet = true;
	}

	// Facing restrictions.
	auto const pType = pUnit->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const defaultFacing = (FacingType)(RulesClass::Instance->DeployDir >> 5);
	auto const facing = pTypeExt->DeployDir.Get(defaultFacing);

	if (facing == FacingType::None)
		return false;

	if (facing != (FacingType)currentDir->Current().GetFacing<8>())
	{
		auto dir = DirStruct();
		dir.SetFacing<8>((size_t)facing);

		if (isDeploying)
		{
			static_cast<UnitClass*>(pUnit)->Deploying = true;

			if (isJumpjet)
				currentDir->Set_Desired(dir);

			pUnit->Locomotor->Do_Turn(dir);

			return true;
		}
		else
		{
			currentDir->Set_Desired(dir);
		}
	}

	return false;
}

void UnitExtContainer::CreateDeployingAnim(UnitClass* pUnit, bool isDeploying)
{
	if (!pUnit->DeployAnim)
	{
		auto const pType = pUnit->Type;
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pAnimType = !pTypeExt->DeployingAnims.empty() ?
			GeneralUtils::GetItemForDirection<AnimTypeClass*>(pTypeExt->DeployingAnims, pUnit->PrimaryFacing.Current())
			: pUnit->Type->DeployingAnim;

		auto const pAnim = GameCreate<AnimClass>(pAnimType, pUnit->Location, 0, 1, 0x600, 0,
			!isDeploying && pTypeExt->DeployingAnim_ReverseForUndeploy);

		pUnit->DeployAnim = pAnim;
		pAnim->SetOwnerObject(pUnit);
		AnimExtData::SetAnimOwnerHouseKind(pAnim, pUnit->Owner, nullptr, pUnit, false, true);
		auto const pExt = TechnoExtContainer::Instance.Find(pUnit);

		if (pTypeExt->DeployingAnim_UseUnitDrawer)
		{
			pAnim->LightConvert = pUnit->GetRemapColour();
			pAnim->IsBuildingAnim = true; // Hack to make it use tint in drawing code.
		}

		// Set deploy animation timer. Required to be independent from animation lifetime due
		// to processing order / pointer invalidation etc. adding additional delay - simply checking
		// if the animation is still there wouldn't work well as it'd lag one frame behind I believe.
		const int rate = pAnimType->Normalized ?
			GameOptionsClass::Instance->GetAnimSpeed(pAnimType->Rate) :
			pAnimType->Rate;

		auto& timer = pExt->SimpleDeployerAnimationTimer;

		if (pAnimType->Reverse || pAnim->Reverse)
			timer.Start(pAnim->Animation.Stage * rate);
		else
			timer.Start(pAnimType->End * rate);
	}
}

void FakeUnitClass::_Deploy()
{
	auto pThis = this;
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return;

	if (!pThis->Deployed)
	{
		if (!pThis->InAir && pType->DeployToLand && pThis->GetHeight() > 0)
			pThis->InAir = true;

		if (pThis->Deploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = true;
				pThis->Deploying = false;
			}
		}
		else if (!pThis->InAir)
		{
			if (UnitExtContainer::CheckDeployRestrictions(pThis, true))
				return;

			if (UnitExtContainer::HasDeployingAnim(pType))
			{
				UnitExtContainer::CreateDeployingAnim(pThis, true);
				pThis->Deploying = true;
			}
			else
			{
				pThis->Deployed = true;
				pThis->Deploying = false; // DeployDir != -1 + no DeployingAnim case needs this reset here.
			}
		}
	}

	if (pThis->Deployed)
	{
		int maxAmmo = -1;

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

		if (pTypeExt->Convert_Deploy)
			maxAmmo = pTypeExt->Convert_Deploy->Ammo;

		TechnoExtData::HandleOnDeployAmmoChange(pThis, maxAmmo);

		VocClass::SafeImmedietelyPlayAt(pType->DeploySound, pThis->Location);
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x739AC0 , FakeUnitClass::_Deploy)

void FakeUnitClass::_UnDeploy()
{
	auto pThis = this;
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return;

	if (pThis->Deployed)
	{
		if (pThis->Undeploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = false;
				pThis->Undeploying = false;
				auto cell = CellStruct::Empty;
				pThis->NearbyLocation(&cell, pThis);
				auto const pCell = MapClass::Instance->GetCellAt(cell);
				pThis->SetDestination(pCell, true);
			}
		}
		else
		{
			if (UnitExtContainer::HasDeployingAnim(pType))
			{
				UnitExtContainer::CreateDeployingAnim(pThis, false);
				pThis->Undeploying = true;
			}
			else
			{
				pThis->Deployed = false;
			}
		}

		if (pThis->IsDisguised())
			pThis->Disguised = false;

		if (!pThis->Deployed)
		{
			TechnoExtData::HandleOnDeployAmmoChange(pThis);

			VocClass::SafeImmedietelyPlayAt(pType->UndeploySound, pThis->Location);
		}
	}

}

DEFINE_FUNCTION_JUMP(LJMP, 0x739CD0, FakeUnitClass::_UnDeploy)

int FakeUnitClass::_Mission_AreaGuard()
{
	auto pTypeExt = this->_GetTypeExtData();
	auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	if (this->SlaveManager
			&& !(nFrame < 0 || nFrame + this->CurrentMissionStartTime >= Unsorted::CurrentFrame)) {

		this->SlaveManager->Guard();
		return static_cast<int>((this->GetCurrentMissionControl()->Rate
				* 900) + ScenarioClass::Instance->Random(1, 5));

	} else {

		if (TechnoExtData::CannotMove(this)) {

			if (this->CanPassiveAcquireTargets() && this->TargetingTimer.Completed())
				this->TargetAndEstimateDamage(&this->Location, ThreatType::Range);

			int delay = 1;

			if (!this->Target) {
				this->UpdateIdleAction();
				delay = static_cast<int>(this->GetCurrentMissionControl()->Rate
					* 900) + ScenarioClass::Instance->Random(1, 5);
			}

			return delay;
		}
	}

	return FootClass::Mission_AreaGuard();
}

DEFINE_FUNCTION_JUMP(VTABLE , 0x7F5E90 , FakeUnitClass::_Mission_AreaGuard)
DEFINE_FUNCTION_JUMP(CALL , 0x744100, FakeUnitClass::_Mission_AreaGuard)

ASMJIT_PATCH(0x73544D, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x7359DC, UnitClass_DTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeUnitClass::_Detach(AbstractClass* target, bool all)
{
	UnitExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->UnitClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5C98, FakeUnitClass::_Detach)
