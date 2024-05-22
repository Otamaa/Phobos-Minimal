#include "SWStateMachine.h"

#include "NuclearMissile.h"
#include "Dominator.h"
#include "LightningStorm.h"

#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

std::vector<std::unique_ptr<SWStateMachine>> SWStateMachine::Array;

void SWStateMachine::UpdateAll()
{
	auto iter = std::remove_if(SWStateMachine::Array.begin() , SWStateMachine::Array.end(),[](std::unique_ptr<SWStateMachine>& pMachine){
		if(!pMachine)
			return true;

		pMachine->Update();
		return pMachine->Finished();
	});

	SWStateMachine::Array.erase(iter , SWStateMachine::Array.end());
}

void SWStateMachine::PointerGotInvalid(AbstractClass* ptr, bool remove)
{
	for (auto& Machine : SWStateMachine::Array) {
		if(Machine) {
			Machine->InvalidatePointer(ptr, remove);
		}
	}

	AnnounceInvalidPointer(SW_PsychicDominator::CurrentPsyDom, ptr);
	AnnounceInvalidPointer(SW_LightningStorm::CurrentLightningStorm, ptr);
}

void SWStateMachine::Clear()
{
	// hard-reset any super weapon related globals
	SWStateMachine::Array.clear();
	SW_NuclearMissile::CurrentNukeType = nullptr;
	SW_PsychicDominator::CurrentPsyDom = nullptr;
	SW_LightningStorm::CurrentLightningStorm = nullptr;
}

bool SWStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Clock)
		.Process(this->Super, RegisterForChange)
		.Process(this->Type, RegisterForChange)
		.Process(this->Coords)
		.Success();
}

bool SWStateMachine::Save(PhobosStreamWriter& Stm) const
{
	// used to instantiate in ObjectFactory
	Stm.Save(this->GetIdentifier());

	return Stm
		.Process(this->Clock)
		.Process(this->Super)
		.Process(this->Type)
		.Process(this->Coords)
		.Success();
}

bool SWStateMachine::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(Array)
		.Process(SW_NuclearMissile::CurrentNukeType)
		.Process(SW_PsychicDominator::CurrentPsyDom)
		.Process(SW_LightningStorm::CurrentLightningStorm)
		.Success();
}

bool SWStateMachine::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(Array)
		.Process(SW_NuclearMissile::CurrentNukeType)
		.Process(SW_PsychicDominator::CurrentPsyDom)
		.Process(SW_LightningStorm::CurrentLightningStorm)
		.Success();
}
