#include "Kratos.h"

#include <Utilities/Stream.h>

#include <AbstractClass.h>

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args)
{
}
void InvalidatePointer(EventSystem* sender, Event e, void* args)
{
}

void DetachAll(EventSystem* sender, Event e, void* args)
{
	auto const& argsArray = reinterpret_cast<void**>(args);
	AbstractClass* pItem = (AbstractClass*)argsArray[0];
	bool all = argsArray[1];

}

void Kratos::ExeRun(EventSystem* sender, Event e, void* args)
{
	EventSystems::General.Broadcast(Events::ExeRun);
}

#include <Misc/Kratos/Ext/Helper/MathEx.h>

void Kratos::LogicUpdate_Early() {
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent);
}

void Kratos::LogicUpdate_End() {
	EventSystems::Logic.Broadcast(Events::LogicUpdateEvent, EventArgsLate);
}

void Kratos::EnsureSeeded(unsigned long seed) {
	Random::SetRandomSeed(seed);

	EventSystems::General.Broadcast(Events::ScenarioStartEvent);
}

void Kratos::ScenarioClearFlagSet() { KratosCommon::IsScenarioClear = true; }

void Kratos::ScenarioClearFlagUnset() {
	EventSystems::General.Broadcast(Events::ScenarioClearClassesEvent);
	KratosCommon::IsScenarioClear = false;
}

void Kratos::DetachFromAll(AbstractClass* const pInvalid, bool const removed) {
	void* args[] = { pInvalid, (void*)removed };

	// 该广播会执行三次
	// (01) DetachAll
	// (01) Limbo
	// (01) DetachAll
	// (02) DetachAll
	// (02) Delete
	EventSystems::General.Broadcast(Events::DetachAll, &args);

}

void Kratos::GScreenRender_Early() {
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent);
}

void Kratos::GSCreenRender_End() {
	EventSystems::Render.Broadcast(Events::GScreenRenderEvent, EventArgsLate);
}
