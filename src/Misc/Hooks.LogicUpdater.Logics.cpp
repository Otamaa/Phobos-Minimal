#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>
#include <Kamikaze.h>

#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/DamageSelf/DamageSelfType.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTargetFunctional.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterAreaGuardFunctional.h>


#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>

void TechnoExtData::InitFunctionEvents()
{
	/*
	GenericFuctions.clear();

	//register desired functions !
	GenericFuctions += TechnoExtData::UpdateMindControlAnim;
	GenericFuctions += TechnoExtData::ApplyMindControlRangeLimit;
	GenericFuctions += TechnoExtData::ApplyInterceptor;
	GenericFuctions += TechnoExtData::ApplySpawn_LimitRange;
	GenericFuctions += TechnoExtData::CheckDeathConditions;
	GenericFuctions += TechnoExtData::EatPassengers;
	GenericFuctions += PassengersFunctional::AI;
	GenericFuctions += SpawnSupportFunctional::AI;
	GenericFuctions += TechnoClass_AI_GattlingDamage;
	*/
}

void TechnoExtData::InitializeItems(TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	pExt->Type = pType;
	pExt->AbsType = pThis->WhatAmI();
	pExt->CurrentShieldType = pTypeExt->ShieldType;

	pExt->IsMissisleSpawn = (RulesClass::Instance->V3Rocket.Type == pExt->Type ||
	 pExt->Type == RulesClass::Instance->DMisl.Type || pExt->Type == RulesClass::Instance->CMisl.Type
	 || pTypeExt->IsCustomMissile);

	pExt->PaintBallState = std::make_unique<PaintBall>();

	if (pExt->AbsType != BuildingClass::AbsID)
	{
		if (!pTypeExt->LaserTrailData.empty() && !pExt->Type->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

		TechnoExtData::InitializeLaserTrail(pThis, false);
		TrailsManager::Construct(pThis);
	}
}
