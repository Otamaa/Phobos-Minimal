#include "SWStateMachine.h"

std::vector<std::unique_ptr<SWStateMachine>> SWStateMachine::Array;

void SWStateMachine::UpdateAll()
{
	for (auto& Machine : SWStateMachine::Array) {
		Machine->Update();
	}

	Array.erase(std::remove_if(Array.begin(), Array.end(), 
	[](const std::unique_ptr<SWStateMachine>& ptr) {
		return ptr->Finished();
	}), Array.end());
}

void SWStateMachine::PointerGotInvalid(void* ptr, bool remove) {
	for (auto& Machine : SWStateMachine::Array) {
		Machine->InvalidatePointer(ptr, remove);
	}
}

void SWStateMachine::Clear()
{
	// hard-reset any super weapon related globals
	SWStateMachine::Array.clear();
}

void UnitDeliveryStateMachine::Update()
{
	if (this->Finished()) {
		this->PlaceUnits();
	}
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
		.Success();
}

bool SWStateMachine::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(Array)
		.Success();
}