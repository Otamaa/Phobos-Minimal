#include "Body.h"

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
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
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/House/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

ASMJIT_PATCH(0x65DD4E, TeamTypeClass_CreateGroub_MissingOwner, 0x7)
{
	//GET(TeamClass*, pCreated, ESI);
	GET(TeamTypeClass*, pType, EBX);

	const auto pHouse = pType->GetHouse();
	if (!pHouse)
	{
		Debug::FatalErrorAndExit("Creating Team[%s] groub without proper Ownership may cause crash , Please check !", pType->ID);
	}

	R->EAX(pHouse);
	return 0x65DD55;
}

ASMJIT_PATCH(0x6F09C0, TeamTypeClass_CreateOneOf_Handled, 0x9)
{
	GET(TeamTypeClass*, pThis, ECX);
	GET_STACK(DWORD, caller, 0x0);
	GET_STACK(HouseClass*, pHouse, 0x4);

	if (!pHouse)
	{
		pHouse = pThis->Owner;
		if (!pHouse)
		{
			if (HouseClass::Index_IsMP(pThis->idxHouse))
			{
				pHouse = HouseClass::FindByPlayerAt(pThis->idxHouse);
			}
		}
	}

	if (!pHouse)
	{
		R->EAX<TeamClass*>(nullptr);
		return 0x6F0A2C;
	}

	if (!Unsorted::ScenarioInit())
	{
		if (pThis->Max >= 0)
		{
			if (SessionClass::Instance->GameMode != GameMode::Campaign)
			{
				if (pHouse->GetTeamCount(pThis) >= pThis->Max)
				{
					R->EAX<TeamClass*>(nullptr);
					return 0x6F0A2C;
				}
			}
			else if (pThis->cntInstances >= pThis->Max)
			{
				R->EAX<TeamClass*>(nullptr);
				return 0x6F0A2C;
			}
		}
	}

	const auto pTeam = GameCreate<TeamClass>(pThis, pHouse, false);
	Debug::LogInfo("[{0} - {1}] Creating a new team named [{2} -{3}] caller [{4:x}].",
		pHouse->get_ID(), (void*)pHouse, pThis->ID, (void*)pTeam, caller);
	R->EAX(pTeam);
	return 0x6F0A2C;
}

ASMJIT_PATCH(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
{
	GET(FootClass*, pFoot, EBP);
	R->ECX(HouseExtData::GetParadropPlane(pFoot->Owner));
	++Unsorted::ScenarioInit();
	return 0x65DBD0;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
ASMJIT_PATCH(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if (pHouse &&
		(pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated))
	{
		return 0;
	}

	// no.
	return (R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
}ASMJIT_PATCH_AGAIN(0x65EC4A, TeamTypeClass_ValidateHouse, 6)

