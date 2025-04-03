#include "SWFirerClass.h"

void SWFirerClass::Update()
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
