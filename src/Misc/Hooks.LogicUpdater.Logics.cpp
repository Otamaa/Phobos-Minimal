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
	GenericFuctions += PassengersFunctional::AI;
	GenericFuctions += SpawnSupportFunctional::AI;
	GenericFuctions += TechnoClass_AI_GattlingDamage;
	*/
}

void TechnoExt::InitializeItems(TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->Type = pType;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);
	pExt->CurrentShieldType = pTypeExt->ShieldType;
	//if(IS_SAME_STR_(pType->ID , "YHARV"))
	//	Debug::Log("Inited YHARD with shield [%s] ! \n" , pTypeExt->ShieldType);

	//LineTrailExt::ConstructLineTrails(pThis);

	pExt->IsMissisleSpawn = (RulesClass::Instance->V3Rocket.Type == pExt->Type ||
	 pExt->Type == RulesClass::Instance->DMisl.Type || pExt->Type == RulesClass::Instance->CMisl.Type
	 || pTypeExt->IsCustomMissile);


	pExt->PaintBallState = std::make_unique<PaintBall>();

	if (!Is_Building(pThis))
	{
		if (!pTypeExt->LaserTrailData.empty() && !pExt->Type->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

		TechnoExt::InitializeLaserTrail(pThis, false);


		TrailsManager::Construct(pThis);

	}
}
