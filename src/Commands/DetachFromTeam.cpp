#include "DetachFromTeam.h"

#include <SessionClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/Ares/Hooks/AresNetEvent.h>

const char* DetachFromTeamCommandClass::GetName() const
{
	return "Detach team of selected Object(s)";
}

const wchar_t* DetachFromTeamCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DETACHTEAM", L"Detach team of Selected Object");
}

const wchar_t* DetachFromTeamCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* DetachFromTeamCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DETACHTEAM_DESC", L"Detach team of any selected objects.");
}

void DetachFromTeamCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated() || !ObjectClass::CurrentObjects->Count)
		return;

	ObjectClass::CurrentObjects->for_each([](ObjectClass* const object) {
	    if (FootClass* techno = flag_cast_to<FootClass*>(object)) {
		  if (techno->BelongsToATeam()) {
			  auto pTeam = techno->Team;
			  pTeam->RemoveMember(techno);
			  pTeam->Reacalculate();
		  }
	    }
	});
}
