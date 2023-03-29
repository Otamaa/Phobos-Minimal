#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>
#include <Kamikaze.h>

#ifdef COMPILE_PORTED_DP_FEATURES
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

#endif

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>
#include <Phobos_ECS.h>

void TechnoExt::ExtData::InitFunctionEvents()
{
	/*
	GenericFuctions.clear();

	//register desired functions !
	GenericFuctions += TechnoExt::UpdateMindControlAnim;
	GenericFuctions += TechnoExt::ApplyMindControlRangeLimit;
	GenericFuctions += TechnoExt::ApplyInterceptor;
	GenericFuctions += TechnoExt::ApplySpawn_LimitRange;
	GenericFuctions += TechnoExt::CheckDeathConditions;
	GenericFuctions += TechnoExt::EatPassengers;
#ifdef COMPILE_PORTED_DP_FEATURES
	GenericFuctions += PassengersFunctional::AI;
	GenericFuctions += SpawnSupportFunctional::AI;
#endif
	GenericFuctions += TechnoClass_AI_GattlingDamage;
	*/
}

void TechnoExt::InitializeItems(TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->Type = pType;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	//LineTrailExt::ConstructLineTrails(pThis);

	pExt->IsMissisleSpawn = (RulesClass::Instance->V3Rocket.Type == pType || pType == RulesClass::Instance->DMisl.Type || pType == RulesClass::Instance->CMisl.Type || pTypeExt->IsCustomMissile);
	pExt->CurrentShieldType = pTypeExt->ShieldType;

#ifdef COMPILE_PORTED_DP_FEATURES
	pExt->PaintBallState = std::make_unique<PaintBall>();
#endif
	if (!Is_Building(pThis))
	{
		if (pTypeExt->LaserTrailData.size() > 0 && !pThis->GetTechnoType()->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

		TechnoExt::InitializeLaserTrail(pThis, false);

#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(pThis);
#endif
	}
}
