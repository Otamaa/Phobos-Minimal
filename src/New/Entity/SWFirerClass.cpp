#include "SWFirerClass.h"

#include <SuperClass.h>

#include <Utilities/SavegameDef.h>

SWFirerManagerClass SWFirerManagerClass::Instance;

SWFirerClass::SWFirerClass(SuperClass* SW, int deferment, CellStruct cell, bool playerControl, int oldstart, int oldleft) :
	SW { SW },
	deferment {},
	cell { cell },
	playerControl { playerControl },
	oldstart { oldstart },
	oldleft { MaxImpl(oldleft - deferment, 0) }
{
	this->SW->Reset();
	this->deferment.Start(deferment);
}

void SWFirerManagerClass::Update()
{
	Array.remove_all_if([](auto& item) {
		if (item.deferment.Completed()) {
			item.SW->SetReadiness(true);
			item.SW->Launch(item.cell, item.playerControl);
			item.SW->Reset();
			item.SW->RechargeTimer.StartTime = item.oldstart;
			item.SW->RechargeTimer.TimeLeft = item.oldleft;
			return true;
		}

		return false;
	});
}

void SWFirerManagerClass::Clear()
{
	Array.clear();
}
