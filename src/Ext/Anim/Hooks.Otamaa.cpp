#include "Body.h"
#include <Utilities/Macro.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/AnimHelpers.h>
#include <Utilities/Helpers.h>
#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>

#include <Memory.h>

DEFINE_HOOK(0x685078, Generate_OreTwinkle_Anims, 0x7)
{
	GET(CellClass* const, location, ESI);

	if (location->GetContainedTiberiumValue() > 0)
	{
		if (auto const pTibExt = TiberiumExt::ExtMap.Find(TiberiumClass::Array->GetItem(location->GetContainedTiberiumIndex())))
		{
			if (!ScenarioClass::Instance->Random(0, pTibExt->GetTwinkleChance() - 1))
			{
				if (auto pAnimtype = pTibExt->GetTwinkleAnim())
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimtype, location->GetCoords(), 1))
					{
						//AnimExt::AnimCellUpdater::Marked.push_back(location);
						AnimExt::SetAnimOwnerHouseKind(pAnim, nullptr, nullptr, false);
					}
				}
			}
		}
	}

	return 0x6850E5;
}

/*
DEFINE_HOOK(0x423CD5, AnimClass_Expired_Extra_OnWater, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (!pThis->Type)
		return 0x423EFD;

	auto const nLocation = pThis->GetCenterCoord();
	auto pOwner = pThis->Owner;
	auto pType = pThis->Type;
	DWORD flags = 0x600u;
	int ForceZAdjust = 0;

	//Default

	const auto GetOriginalAnimResult = [pType]()
	{
		if (pType->IsMeteor)
		{
			auto const splash = RulesClass::Instance->SplashList;

			if (splash.Count > 0)
			{
				return splash[ScenarioClass::Instance->Random(0, splash.Count - 1)];
			}
		}

		return RulesClass::Instance->Wake;
	};

	AnimTypeClass* pSplashAnim = nullptr;
	TechnoClass* pInvoker = nullptr;

	if (auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pAnimTypeExt->ExplodeOnWater.Get())
		{
			if (auto const pWarhead = pType->Warhead)
			{
				auto const nDamage = Game::F2I(pThis->Accum);
				pInvoker = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
				pOwner = pInvoker ? pInvoker->GetOwningHouse() : pOwner;

				if (pAnimTypeExt->Warhead_Detonate)
				{
					WarheadTypeExt::DetonateAt(pWarhead, nLocation, pInvoker, nDamage);
				}
				else
				{
					MapClass::DamageArea(nLocation, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, nLocation);
					auto nLand = pThis->GetCell() ? pThis->GetCell()->LandType : LandType::Clear;
					pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pWarhead, nLand, nLocation);
					flags = 0x2600u;
					ForceZAdjust = -30;
				}
			}

			if (auto pExpireAnim = pType->ExpireAnim)
			{
				if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nLocation, 0, 1, 0x2600u, -30, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
					if (auto pExt = AnimExt::ExtMap.Find(pAnim))
						pExt->Invoker = pInvoker;
				}
			}
		}
		else
		{
			if (pType->IsMeteor)
			{
				auto const splash = pAnimTypeExt->SplashList.GetElements(RulesClass::Instance->SplashList);

				if (splash.size() > 0)
				{
					auto nIndexR = (splash.size() - 1);
					auto nIndex = pAnimTypeExt->SplashIndexRandom.Get() ?
						ScenarioClass::Instance->Random(0, nIndexR) : pAnimTypeExt->SplashIndex.Get(nIndexR);

					pSplashAnim = splash.at(nIndex);
				}
			}
			else
			{
				pSplashAnim = pAnimTypeExt->WakeAnim.Get(RulesClass::Instance->Wake);
			}
		}

	}
	else
	{
		pSplashAnim = GetOriginalAnimResult();
	}

	if (pSplashAnim)
	{
		if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, nLocation, 0, 1, flags, ForceZAdjust))
		{
			AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, false);
			if (auto pExt = AnimExt::ExtMap.Find(pSplashAnimCreated))
				pExt->Invoker = pInvoker;
		}
	}

	return 0x423EFD;
}

DEFINE_HOOK(0x423DE7, AnimClass_Expired_Extra_OnLand_DamageArea, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pType = pThis->Type)
	{
		auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
		TechnoClass* pTechOwner = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt && pAnimTypeExt->Damage_DealtByInvoker);
		auto const pOwner = pTechOwner ? pTechOwner->GetOwningHouse() : pThis->Owner;

		auto nCoords = pThis->Bounce.GetCoords();

		if (auto pExpireAnim = pType->ExpireAnim)
		{
			if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, nCoords, 0, 1, 0x2600u, -30, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, false);
				if (auto pExt = AnimExt::ExtMap.Find(pAnim))
					pExt->Invoker = pTechOwner;
			}
		}

		if (auto const pWarhead = pType->Warhead)
		{
			auto const nDamage = Game::F2I(pType->Damage);
			MapClass::DamageArea(nCoords, nDamage, pTechOwner, pWarhead, pWarhead->Tiberium, pOwner);
			MapClass::FlashbangWarheadAt(nDamage, pWarhead, nCoords, false);
		}
	}

	return 0x423EFD;
}
*/

DEFINE_HOOK(0x423CC1, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);

	//overriden instruction !
	R->Stack(STACK_OFFS(0x8C, 0x78), R->AL());

	auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	{
		TechnoClass* const pTechOwner = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
		auto const pOwner = pThis->Owner ? pThis->Owner : pTechOwner ? pTechOwner->GetOwningHouse() : nullptr;

		GET8(bool, LandIsWater, BL);
		GET8(bool, EligibleHeight, AL);

		if (!LandIsWater || EligibleHeight)
		{
			Helper::Otamaa::Detonate(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());

			if (auto const pExpireAnim = pThis->Type->ExpireAnim)
			{
				if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, pThis->Bounce.GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTechOwner, false);
				}
			}
		}
		else
		{
			if (!pAnimTypeExt->ExplodeOnWater)
			{
				if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pAnimTypeExt->SplashList, pAnimTypeExt->WakeAnim.Get(RulesGlobal->Wake), pAnimTypeExt->SplashIndexRandom.Get(), pThis->Type->IsMeteor))
				{
					if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, false))
					{
						AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
					}
				}
			}
			else
			{
				auto const [bPlayWHAnim, nDamage] = Helper::Otamaa::Detonate(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());
				if (bPlayWHAnim)
				{
					if (auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pThis->Type->Warhead, pThis->GetCell()->LandType, pThis->GetCoords()))
					{
						if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30))
						{
							AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
						}
					}
				}
			}
		}
	}

	return SkipGameCode;
}

//crash and corrup ESI pointer around
DEFINE_HOOK(0x424FE8, AnimClass_Middle_SpawnParticle, 0x6) //was C
{
	GET(AnimClass*, pThis, ESI);

	{
		const auto pType = pThis->Type;
		const auto pTypeExt = AnimTypeExt::ExtMap.Find(pType);

		{
			auto pAnimTypeExt = pTypeExt;
			const auto pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
			const auto pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : nullptr);
			const auto nCoord = pThis->GetCoords();

			Helper::Otamaa::SpawnMultiple(
				pAnimTypeExt->SpawnsMultiple,
				pAnimTypeExt->SpawnsMultiple_amouts,
				nCoord, pObject, pHouse, pAnimTypeExt->SpawnsMultiple_Random.Get());

			if (pType->SpawnsParticle != -1)
			{
				const auto pParticleType = ParticleTypeClass::Array.get()->GetItem(pType->SpawnsParticle);

				if (pType->NumParticles > 0 && pParticleType)
				{
					for (int i = 0; i < pType->NumParticles; ++i)
					{
						CoordStruct nDestCoord = CoordStruct::Empty;
						if (pAnimTypeExt->ParticleChance.isset() ?
							(ScenarioGlobal->Random(0, 99) < abs(pAnimTypeExt->ParticleChance.Get())) : true)
						{
							nDestCoord = Helper::Otamaa::GetRandomCoordsInsideLoops(pAnimTypeExt->ParticleRangeMin.Get(), pAnimTypeExt->ParticleRangeMax.Get(), nCoord, i);
							ParticleSystemClass::Instance->SpawnParticle(pParticleType, &nDestCoord);
						}
					}
				}
			}

			if (!pTypeExt->Launchs.empty())
			{
				for (auto const& nLauch : pTypeExt->Launchs)
				{
					Helpers::Otamaa::LauchSW(
							nLauch.LaunchWhat, pHouse,
							nCoord, nLauch.LaunchWaitcharge,
							nLauch.LaunchResetCharge,
							nLauch.LaunchGrant,
							nLauch.LaunchGrant_RepaintSidebar,
							nLauch.LaunchGrant_OneTime,
							nLauch.LaunchGrant_OnHold,
							nLauch.LaunchSW_Manual,
							nLauch.LaunchSW_IgnoreInhibitors,
							nLauch.LaunchSW_IgnoreDesignators,
							nLauch.LauchSW_IgnoreMoney
					);
				}
			}
		}
	}

	return 0x42504D;
}

DEFINE_HOOK(0x42504D, AnimClass_Middle_SpawnCreater, 0xA) //was 4
{
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CellClass*, pCell, STACK_OFFS(0x30, 0x14));
	GET(int, nX, EBP);
	GET_STACK(int, nY, STACK_OFFS(0x30, 0x20));

	auto pType = pThis->Type;
	auto pTypeExt = AnimTypeExt::ExtMap.Find(pType);


	if (pTypeExt->SpawnCrater.Get(pThis->GetHeight() < 30))
	{
		auto nCoord = pThis->GetCoords();
		if (!pType->Scorch || (pType->Crater && ScenarioGlobal->Random.RandomDouble() >= pTypeExt->CraterChance.Get()))
		{
			if (pType->Crater)
			{
				if (pTypeExt->CraterDecreaseTiberiumAmount.Get() > 0)
					pCell->ReduceTiberium(pTypeExt->CraterDecreaseTiberiumAmount.Get());

				if (pType->ForceBigCraters)
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, 300, 300, true);
				else
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, nX, nY, false);
			}
		}
		else
		{
			const bool bSpawn = (pTypeExt->ScorchChance.isset()) ? (ScenarioGlobal->Random.RandomDouble() >= pTypeExt->ScorchChance.Get()) : true;
			if (bSpawn)
				SmudgeTypeClass::CreateRandomSmudge(nCoord, nX, nY, false);
		}
	}

	return 0x42513F;
}

DEFINE_HOOK(0x42264D, AnimClass_Init, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET_BASE(CoordStruct*, pCoord, 0xC);

	auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	//auto pExt = AnimExt::ExtMap.Find(pThis);

	if (pTypeExt->ConcurrentChance.Get() >= 1.0 && !pTypeExt->ConcurrentAnim.empty())
	{
		if (ScenarioGlobal->Random.RandomDouble() <= pTypeExt->ConcurrentChance.Get())
		{
			auto const nIdx = ScenarioGlobal->Random.RandomFromMax(pTypeExt->ConcurrentAnim.size() - 1);
			if (auto pType = pTypeExt->ConcurrentAnim.at(nIdx))
			{

				if (pType == pThis->Type)
					return 0x0;

				if (auto pAnim = GameCreate<AnimClass>(pType, pCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
					pAnim->Owner = pThis->GetOwningHouse();
			}
		}
	}

	//if (auto const& pSpawns = pExt->SpawnData) {
	//	pSpawns->OnInit(pCoord);
	//}

	return 0x0;
}

//the anim trying to play `StopSound` altho it has no type ? , wtf
DEFINE_JUMP(LJMP, 0x4220DE, 0x42211F)
#ifdef ENABLE_NEWHOOKS
TODO : retest for desync


DEFINE_HOOK(0x4242BA, AnimClass_AI_SetCoord, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pExt = AnimExt::ExtMap.Find(pThis))
	{
		if (pExt->Something)
		{
			auto nCoord = pThis->GetCoords() + pExt->Something;
			pThis->SetLocation(nCoord);
		}
	}
	return 0x0;
}
#endif