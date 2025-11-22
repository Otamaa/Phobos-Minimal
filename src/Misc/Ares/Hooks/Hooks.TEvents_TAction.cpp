#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Side/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
//#include <Lib/gcem/gcem.hpp>

#include <Notifications.h>

#include "Header.h"

#include <TEventClass.h>
#include <TActionClass.h>

ASMJIT_PATCH(0x6E20D8, TActionClass_DestroyAttached_Loop, 0x5)
{
	GET(int, nLoopVal, EAX);
	return nLoopVal < 4 ? 0x6E20E0 : 0x0;
}

ASMJIT_PATCH(0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA)
{
	GET(HouseClass*, House, EDI);
	GET(int, triggerSide, EAX);

	enum { Eligible = 0x41E8D7, NotEligible = 0x41E8A1 };

	if (!triggerSide) {
		return Eligible;
	}

	return((triggerSide - 1) == House->SideIndex)
		? Eligible
		: NotEligible
		;
}

#include <Ext/TEvent/Body.h>

ASMJIT_PATCH(0x6E3EE0, TActionClass_GetFlags, 5)
{
	GET(AresNewTriggerAction, nAction, ECX);

	std::pair<TriggerAttachType, bool> _result = AresTActionExt::GetTriggetAttach(nAction);

	if (_result.second) {
		R->EAX(_result.first);
		return 0x6E3EFE;
	}

	 return 0;
}

ASMJIT_PATCH(0x6E3B60, TActionClass_GetMode, 8)
{
	GET(AresNewTriggerAction, nAction, ECX);

	std::pair<LogicNeedType, bool> _result = AresTActionExt::GetLogicNeed(nAction);

	if (_result.second)
	{
		R->EAX(_result.first);
		return 0x6E3C4B;
	}

	_result = TEventExtData::GetLogicNeed((PhobosTriggerEvent)nAction);

	if(_result.second) {
		R->EAX(_result.first);
		return 0x6E3C4B;
	}

	R->EAX(((int)nAction) - 1);
	return ((int)nAction) > 0x8F ? 0x6E3C49 : 0x6E3B6E;

}