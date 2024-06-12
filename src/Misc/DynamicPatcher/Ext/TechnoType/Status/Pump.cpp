#include "../TechnoStatus.h"

#include <Locomotor/JumpjetLocomotionClass.h>

#include <Misc/DynamicPatcher/Ext/Helper/Physics.h>
#include <Misc/DynamicPatcher/Ext/Helper/Weapon.h>
#include <Misc/DynamicPatcher/Ext/Helper/Scripts.h>

#include <Misc/DynamicPatcher/Extension/WarheadTypeExt.h>

bool TechnoStatus::PumpAction(CoordStruct targetPos, bool isLobber, DoType flySequence)
{
	if (!IsBuilding() && !pTechno->IsFallingDown && !AmIStand())
	{
		return Pump->Jump(targetPos, isLobber, flySequence);
	}
	return false;
}

void TechnoStatus::HumanCannon(CoordStruct sourcePos, CoordStruct targetPos, int height, bool isLobber, DoType flySequence)
{
	if (pTechno->Passengers.NumPassengers > 0)
	{
		// 人间大炮一级准备
		FootClass* pPassenger = pTechno->Passengers.RemoveFirstPassenger();
		pPassenger->Transporter = nullptr;
		DirStruct facing = pTechno->GetRealFacing().Current();
		++Unsorted::ScenarioInit;
		pPassenger->Unlimbo(sourcePos, ToDirType(facing));
		--Unsorted::ScenarioInit;
		// 人间大炮二级准备
		if (TechnoStatus* status = GetStatus<TechnoExt, TechnoStatus>(dynamic_cast<TechnoClass*>(pPassenger)))
		{
			// 人间大炮发射
			targetPos += CoordStruct{ 0, 0, height };
			status->Pump->Jump(targetPos, isLobber, flySequence, true);
		}
	}
}

