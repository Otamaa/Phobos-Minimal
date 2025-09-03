#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/UnitType/Body.h>

#include <SlaveManagerClass.h>

UnitExtContainer UnitExtContainer::Instance;
std::vector<UnitExtData*> Container<UnitExtData>::Array;

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
				this->TargetAndEstimateDamage(&this->Location, ThreatType::Area);

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