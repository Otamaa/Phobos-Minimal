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

//if (pExt && pTypeExt) {
//	if (pExt->ID != pThis->get_ID()) {
//		pExt->ID = pThis->get_ID();
//	}
//}
//
//if (pThis->TemporalTargetingMe)
//{
//	if (auto const pCell = pThis->GetCell())
//	{
//		if (auto const pBld = pCell->GetBuilding())
//		{
//			if (pBld->Type->BridgeRepairHut)
//			{
//				pThis->TemporalTargetingMe->Detach();
//			}
//		}
//	}
//}

void TechnoExt::ExtData::GattlingDamage()
{
	auto const pThis = this->Get();
	if (!TechnoExt::IsAlive(pThis, false, false, false))
		return;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pThis->GetTechnoType());

	if (!pTypeExt)
		return;

	const auto pType = pTypeExt->Get();

	if (!pType->IsGattling || !pTypeExt->Gattling_Overload.Get())
		return;

	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue)
		{
			GattlingDmageDelay = -1;
			GattlingDmageSound = false;
			return;
		}

		GattlingDmageDelay = pTypeExt->Gattling_Overload_Frames.Get();
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			GattlingDmageSound = false;
		}
		else
		{
			pThis->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, 0, 0, 0, 0);

			if (!GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.Get(-1) >= 0)
					VocClass::PlayAt(pTypeExt->Gattling_Overload_DeathSound, pThis->Location, 0);

				GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get())
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(); i > 0; --i)
				{
					auto const nRandomY = ScenarioGlobal->Random(-200, 200);
					auto const nRamdomX = ScenarioGlobal->Random(-200, 200);
					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
				}
			}

			if (pThis->WhatAmI() == AbstractType::Unit && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioGlobal->Random(0, 1) ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioGlobal->Random(0, 100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--GattlingDmageDelay;
	}
}

void TechnoExt::KillSlave(TechnoClass* pThis)
{
	if (const auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (pInf->Type->Slaved && !pInf->InLimbo && pInf->IsAlive && pInf->Health > 0 && !pInf->TemporalTargetingMe)
		{
			const auto pExt = TechnoTypeExt::ExtMap.Find<false>(pInf->Type);
			if (pExt && !pInf->SlaveOwner && (pExt->Death_WithMaster.Get() || pExt->Slaved_ReturnTo == SlaveReturnTo::Suicide))
				TechnoExt::KillSelf(pInf, pExt->Death_Method);
		}
	}
}

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

	if (!pType)
		return;

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt)
		return;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	//LineTrailExt::ConstructLineTrails(pThis);

	pExt->IsMissisleSpawn = (RulesGlobal->V3Rocket.Type == pType || pType == RulesGlobal->DMisl.Type || pType == RulesGlobal->CMisl.Type || pTypeExt->IsCustomMissile);
	pExt->CurrentShieldType = pTypeExt->ShieldType;

#ifdef COMPILE_PORTED_DP_FEATURES
	pExt->PaintBallState = std::make_unique<PaintBall>();
#endif
	if (pThis->WhatAmI() != AbstractType::Building)
	{
#ifdef ENABLE_HOMING_MISSILE
		if (const auto pFoot = specific_cast<AircraftClass*>(pThis))
		{
			if (auto const pLoco = static_cast<LocomotionClass*>(pFoot->Locomotor.get()))
			{
				CLSID nID { };
				pLoco->GetClassID(&nID);

				if (nID == LocomotionClass::CLSIDs::Rocket && pTypeExt->MissileHoming)
				{
					pExt->MissileTargetTracker = GameCreate<HomingMissileTargetTracker>();
				}
			}
		}
#endif
		if (pTypeExt->LaserTrailData.size() > 0 && !pThis->GetTechnoType()->Invisible)
			pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());

#ifdef ENABLE_HOMING_MISSILE
		pExt->IsMissileHoming = pTypeExt->MissileHoming.Get();
#endif
		TechnoExt::InitializeLaserTrail(pThis, false);

#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(pThis);
#endif
	}
}

void TechnoExt::ApplyMobileRefinery(TechnoClass* pThis)
{
	if (!TechnoExt::IsAlive(pThis, false, false, false))
		return;

	if ((pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find<false>(pThis->GetTechnoType());
	if (!pTypeExt)
		return;

	if (!pTypeExt->MobileRefinery || !pThis->Owner || (pTypeExt->MobileRefinery_TransRate > 0 &&
		Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate))
		return;

	const int cellCount = std::clamp(static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()), 1, static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()));

	CoordStruct flh = { 0,0,0 };

	for (int idx = 0; idx < cellCount; idx++)
	{
		flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()) > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] * Unsorted::LeptonsPerCell : 0;
		flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()) > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] * Unsorted::LeptonsPerCell : 0;
		auto nPos = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false);
		const CellClass* pCell = MapClass::Instance->GetCellAt(nPos);

		if (!pCell)
			continue;

		nPos.Z += pThis->Location.Z;

		if (const int tValue = pCell->GetContainedTiberiumValue())
		{
			const int tibValue = TiberiumClass::Array->GetItem(pCell->GetContainedTiberiumIndex())->Value;
			const int tAmount = static_cast<int>(tValue * 1.0 / tibValue);
			const int amount = pTypeExt->MobileRefinery_AmountPerCell ? Math::min(pTypeExt->MobileRefinery_AmountPerCell.Get(), tAmount) : tAmount;
			pCell->ReduceTiberium(amount);
			const int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier);

			if (pThis->Owner->CanTransactMoney(value))
			{
				pThis->Owner->TransactMoney(value);
				FlyingStrings::AddMoneyString(pTypeExt->MobileRefinery_Display, value, pThis, AffectedHouse::All, nPos, Point2D::Empty, pTypeExt->MobileRefinery_DisplayColor);
			}


			if (!pTypeExt->MobileRefinery_Anims.empty())
			{
				AnimTypeClass* pAnimType = nullptr;
				int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

				if (facing >= 7)
					facing = 0;
				else
					facing++;

				switch (pTypeExt->MobileRefinery_Anims.size())
				{
				case 1:
					pAnimType = pTypeExt->MobileRefinery_Anims[0];
					break;
				case 8:
					pAnimType = pTypeExt->MobileRefinery_Anims[facing];
					break;
				default:
					pAnimType = pTypeExt->MobileRefinery_Anims[
						ScenarioClass::Instance->Random.RandomFromMax(pTypeExt->MobileRefinery_Anims.size() - 1)];
					break;
				}

				if (pAnimType)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nPos))
					{
						AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

						if (pTypeExt->MobileRefinery_AnimMove)
							pAnim->SetOwnerObject(pThis);
					}
				}
			}
		}
	}
}
//auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());