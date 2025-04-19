#include "SWStateMachine.h"

#include "NuclearMissile.h"
#include "Dominator.h"
#include "LightningStorm.h"

#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/Entity/ElectricBoltClass.h>

#pragma region defines

HelperedVector<MemoryPoolUniquePointer<SWStateMachine>> SWStateMachine::Array;

#pragma endregion

ChronoWarpStateMachine::~ChronoWarpStateMachine() { }
CloneableLighningStormStateMachine::~CloneableLighningStormStateMachine()
{
	for (auto& CP : CloudsPresent)
	{
		if (CP && CP->IsAlive && CP->Type)
		{
			CP->UnInit();
		}

		CP = nullptr;
	}

	for (auto& CM : CloudsManifest)
	{
		if (CM && CM->IsAlive && CM->Type)
		{
			CM->UnInit();
		}
		CM = nullptr;
	}

	for (auto& BP : BoltsPresent)
	{
		if (BP && BP->IsAlive && BP->Type)
		{
			BP->UnInit();
		}

		BP = nullptr;
	}

}
DroppodStateMachine::~DroppodStateMachine() { }
GeneticMutatorStateMachine::~GeneticMutatorStateMachine() { }
GenericWarheadStateMachine::~GenericWarheadStateMachine() { }
ParaDropStateMachine::~ParaDropStateMachine() { }
PsychicDominatorStateMachine::~PsychicDominatorStateMachine() { }
RevealStateMachine::~RevealStateMachine() { }
SonarPulseStateMachine::~SonarPulseStateMachine() { }
SpyPlaneStateMachine::~SpyPlaneStateMachine() { }
UnitDeliveryStateMachine::~UnitDeliveryStateMachine() { }
IonCannonStateMachine::~IonCannonStateMachine()
{
	if (this->Anim)
	{
		this->Anim->TimeToDie = true;
		this->Anim->UnInit();
		this->Anim = nullptr;
	}
}
LaserStrikeStateMachine::~LaserStrikeStateMachine() { }

void SWStateMachine::UpdateAll()
{
	SWStateMachine::Array.remove_all_if([](MemoryPoolUniquePointer<SWStateMachine>& pMachine) {
		 if (!pMachine)
			 return true;

		 pMachine->Update();
		 return pMachine->Finished();
	});
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
