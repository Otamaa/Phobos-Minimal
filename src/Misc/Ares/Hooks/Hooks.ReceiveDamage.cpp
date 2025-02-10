#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <RadarEventClass.h>
#include <InfantryClass.h>
#include <BombClass.h>
#include <TacticalClass.h>
#include <SlaveManagerClass.h>
#include <CaptureManagerClass.h>

#include <Utilities/Debug.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/TerrainType/Body.h>

#include <Ext/SWType/NewSuperWeaponType/Firewall.h>

#include <Conversions.h>

#include "Header.h"

#include <Misc/DamageArea.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include <ExtraHeaders/StackVector.h>

DWORD Crashable(FootClass* pThis, TechnoTypeClass* pType, ObjectClass* pKiller)
{

	if (pType->Crashable)
	{
		if (pThis->Crash(pKiller))
		{
			return 0x518D52;
		}
	}

	pThis->UnInit();
	return 0x518D52;
}

static COMPILETIMEEVAL TypeList<AnimTypeClass*>* GetDebrisAnim(TechnoTypeClass* pType)
{

	if (pType->DebrisAnims.Count <= 0)
	{
		if (!pType->DebrisTypes.Count && !RulesClass::Instance->MetallicDebris.Count)
			return nullptr;

		return &RulesClass::Instance->MetallicDebris;
	}

	return &pType->DebrisAnims;
}

//TODO : update , add the new tags problaby
//the newer implementation is seems weird
//https://github.com/Phobos-developers/Phobos/pull/1313
static bool AllowedToCombatAlert(TechnoClass* pThis, args_ReceiveDamage* args)
{
	const auto pWH = args->WH;
	const auto pHouse = pThis->Owner;
	const auto pSourceHouse = args->SourceHouse;
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);
	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (pHouseExt->CombatAlertTimer.HasTimeLeft())
		return false;

	if (pType->Insignificant || pType->Spawned || !pThis->IsInPlayfield)
		return false;

	if (!pTypeExt->CombatAlert.Get(RulesExtData::Instance()->CombatAlert))
		return false;

	if (!pThis->IsOwnedByCurrentPlayer || !pHouse->IsControlledByHuman())
		return false;

	if (*args->Damage <= 1 || pWHExt->CombatAlert_Suppress.Get(!pWHExt->Malicious || pWHExt->Nonprovocative))
		return false;

	if (pSourceHouse && RulesExtData::Instance()->CombatAlert_SuppressIfAllyDamage && pHouse->IsAlliedWith(pSourceHouse))
		return false;

	const auto pBuilding = cast_to<BuildingClass*, false>(pThis);
	if (pBuilding && RulesExtData::Instance()->CombatAlert_IgnoreBuilding && !pTypeExt->CombatAlert_NotBuilding.Get(((BuildingClass*)pThis)->Type->IsVehicle()))
		return false;

	if (RulesExtData::Instance()->CombatAlert_SuppressIfInScreen && pThis->IsOnMyView())
		return false;

	return true;
}

static void applyCombatAlert(TechnoClass* pThis, args_ReceiveDamage* args)
{

	if (AllowedToCombatAlert(pThis, args))
	{
		const auto pHouse = pThis->Owner;
		const auto pType = pThis->GetTechnoType();
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pRules = RulesExtData::Instance();
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);
		const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
		pHouseExt->CombatAlertTimer.Start(pRules->CombatAlert_Interval);
		RadarEventClass::Create(RadarEventType::Combat, CellClass::Coord2Cell(pThis->GetCoords()));
		if (pRules->CombatAlert_MakeAVoice)
		{
			if (pTypeExt->CombatAlert_UseFeedbackVoice.Get(pRules->CombatAlert_UseFeedbackVoice) && pType->VoiceFeedback.Count > 0) // Use VoiceFeedback first
			{
				int idxvoice = pType->VoiceFeedback.Count > 1 ? ScenarioClass::Instance->Random.RandomFromMax(pType->VoiceFeedback.Count - 1) : 0;
				VocClass::PlayGlobal(idxvoice, Panning::Center, 1.0);
			}
			else if (pTypeExt->CombatAlert_UseAttackVoice.Get(pRules->CombatAlert_UseAttackVoice) && pType->VoiceAttack.Count > 0) // Use VoiceAttack then
			{
				int idxvoice = pType->VoiceAttack.Count > 1 ? ScenarioClass::Instance->Random.RandomFromMax(pType->VoiceAttack.Count - 1) : 0;
				VocClass::PlayGlobal(idxvoice, Panning::Center, 1.0);
			}
		}

		if (pTypeExt->CombatAlert_UseEVA.Get(pRules->CombatAlert_EVA))
		{
			VoxClass::PlayIndex(pTypeExt->EVA_Combat);
		}
	}
}

#pragma region Terrain

DEFINE_HOOK(0x71B920, TerrainClass_ReceiveDamage_Handled, 7)
{
	GET(TerrainClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;

	if (auto pWH = args.WH)
	{
		if (args.IgnoreDefenses || (pWH->Wood && !pThis->Type->Immune))
		{

			auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
			auto pExt = TerrainExtContainer::Instance.Find(pThis);
			double PriorHealthRatio = pThis->GetHealthPercentage();

			_res = pThis->ObjectClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, pWH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

			if (!pThis->IsBurning && *args.Damage > 0 && args.WH->Sparky)
			{
				const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

				if (!pWarheadExt->Flammability.isset() || ScenarioClass::Instance->Random.PercentChance(Math::abs(pWarheadExt->Flammability.Get())))
					pThis->Ignite();
			}

			double condYellow = RulesExtData::Instance()->ConditionYellow_Terrain;

			if (!pThis->Type->IsAnimated && pTypeExt->HasDamagedFrames && PriorHealthRatio > condYellow && pThis->GetHealthPercentage() <= condYellow)
			{
				pThis->TimeToDie = true; // Dirty hack to get game to redraw the art reliably.
				LogicClass::Instance->AddObject(pThis, false);
			}

			if (_res == DamageState::PostMortem)
			{
				R->EAX(_res);
				return 0x71BB84;
			}

			if (_res == DamageState::NowDead)
			{
				if (auto& pAttached = pExt->AttachedAnim)
				{
					pAttached->RemainingIterations = 0;
					pAttached.reset(nullptr);
				}

				if (pThis->Type->SpawnsTiberium)
				{
					const auto _damagingDamage = pTypeExt->Damage.Get(100);
					const auto _adamagingWarhead = pTypeExt->Warhead.Get(RulesClass::Instance->C4Warhead);
					const auto _thisCell = pThis->GetCell();

					if (auto const pAnim = MapClass::SelectDamageAnimation(_damagingDamage, _adamagingWarhead, _thisCell->LandType, pThis->Location))
					{
						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, 0),
							nullptr,
							nullptr,
							false
						);
					}

					if (pTypeExt->AreaDamage)
					{
						auto pCoord = &pThis->Location;
						DamageArea::Apply(pCoord, _damagingDamage, nullptr, _adamagingWarhead, true, nullptr);
						MapClass::FlashbangWarheadAt(_damagingDamage, _adamagingWarhead, pThis->Location);
					}

					_thisCell->ChainReaction();
				}
				else if (pThis->IsBurning)
				{
					if (auto& pFire = pExt->AttachedFireAnim)
					{
						pFire->RemainingIterations = 0;
						pFire.reset(nullptr);
					}
				}
				else if (!pThis->TimeToDie)
				{
					pThis->TimeToDie = 1;
					pThis->Animation.Start(2);
				}

				const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
				// Skip over the removal of the tree as well as destroy sound/anim (for now) if the tree has crumble animation.
				if (pThis->TimeToDie && pTerrainExt->HasCrumblingFrames)
				{
					// Needs to be added to the logic layer for the anim to work.
					LogicClass::Instance->AddObject(pThis, false);
					VocClass::PlayIndexAtPos(pTerrainExt->CrumblingSound, pThis->GetCoords());
					pThis->UpdatePlacement(PlacementType::Redraw);
					pThis->Disappear(true);
					return 0x71BB79;
				}

				auto const nCoords = pThis->GetCenterCoords();
				VocClass::PlayIndexAtPos(pTerrainExt->DestroySound, nCoords);
				const auto pAttackerHoue = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

				if (auto const pAnimType = pTerrainExt->DestroyAnim)
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, nCoords),
						args.SourceHouse,
						pThis->GetOwningHouse(),
						args.Attacker,
						false
					);
				}

				if (const auto nBounty = pTerrainExt->Bounty.Get())
				{
					if (pAttackerHoue && pAttackerHoue->CanTransactMoney(nBounty))
					{
						pAttackerHoue->TransactMoney(nBounty);
						FlyingStrings::AddMoneyString(true, nBounty, pAttackerHoue, AffectedHouse::All, nCoords);
					}
				}

				RectangleStruct _drawDim {};
				pThis->GetRenderDimensions(&_drawDim);
				TacticalClass::Instance->RegisterDirtyArea(_drawDim, false);
				pThis->Disappear(true);
				pThis->UnInit();
			}
		}
	}

	R->EAX(_res);
	return 0x71BB84;
}

#pragma endregion

#pragma region Object

DEFINE_HOOK(0x5F5390, ObjectClass_ReveiveDamage_Handled, 0x5)
{
	GET(ObjectClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	int oldstrength = pThis->Health;

	// WWP's shit code! Wrong check.
	// To avoid units dying when they are already dead.
	if (oldstrength <= 0 || !pThis->IsAlive)
	{
		R->EAX(DamageState::PostMortem);
		return 0x5F584A;
	}

	DamageState _res = DamageState::Unaffected;

	auto pObjType = pThis->GetType();

	if (oldstrength <= 0 || !*args.Damage || !args.IgnoreDefenses && pObjType->Immune)
	{
		R->EAX(_res);
		return 0x5F584A;
	}

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);
	int maxstrength = pThis->GetType()->Strength;

	OwnFunc::ApplyHitAnim(pThis, &args);
	pWHExt->applyRelativeDamage(pThis, &args);

	if (!args.IgnoreDefenses)
	{
		MapClass::GetTotalDamage(&args, TechnoExtData::GetTechnoArmor(pThis, args.WH));
		//this already calculate distance damage from epicenter
		pWHExt->ApplyRecalculateDistanceDamage(pThis, &args);
	}

	if (auto pBld = cast_to<BuildingClass*, false>(pThis))
	{
		auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
		if (!pBld->Type->CanC4 && *args.Damage == 0 && !pBldTypeExt->CanC4_AllowZeroDamage)
		{
			*args.Damage = 1;
		}
	}

	if (!args.IgnoreDefenses && args.Attacker && *args.Damage > 0)
	{
		if (pWHExt->applyCulling(args.Attacker, pThis))
			*args.Damage = pThis->Health;
	}

	if (!*args.Damage)
	{
		R->EAX(_res);//unaffected
		return 0x5F584A;
	}

	if (*args.Damage > 0)
	{
		_res = DamageState::Unchanged;
		const auto curstr = int(maxstrength * RulesClass::Instance->ConditionYellow);

		if (oldstrength <= *args.Damage)
		{
			*args.Damage = oldstrength;

		}
		else if (oldstrength >= curstr && (oldstrength - *args.Damage) < curstr)
		{
			_res = DamageState::NowYellow;
		}

		double v15 = (double)maxstrength * RulesClass::Instance->ConditionRed;
		if ((double)oldstrength > v15 && (double)(oldstrength - *args.Damage) < v15)
		{
			_res = DamageState::NowRed;
		}

		int adjust = oldstrength - *args.Damage;
		pThis->Health = adjust;
		auto pInf = cast_to<InfantryClass*, false>(pThis);

		if (!adjust && !args.IgnoreDefenses && pInf && pInf->Type->Cyborg && !pInf->Crawling)
		{

			auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			auto pInvoker = args.Attacker
				? args.Attacker->Owner
				: args.SourceHouse;

			AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pInf->Owner, args.Attacker, true);

			pThis->Health = int(pInf->Type->Strength * 0.25);

			if (pThis->Health <= 1)
			{
				pThis->Health = 1;
			}

			pInf->Crawling = 1;
			pInf->PlayAnim(DoType::Crawl, true, false);
			_res = DamageState::NowRed;
		}

		int arg10b = pThis->Health;
		if (_res == DamageState::NowYellow)
		{
			if (args.Attacker)
			{
				if (pThis->AttachedTag)
				{
					pThis->AttachedTag->SpringEvent(TriggerEvent::HalfHealth_combatonly, pThis, CellStruct::Empty, false, nullptr);
				}
			}

			if (!pThis->IsAlive)
			{
				R->EAX(DamageState::PostMortem);
				return 0x5F584A;
			}

			if (pThis->AttachedTag)
			{
				pThis->AttachedTag->SpringEvent(TriggerEvent::HalfHealth_anysource, pThis, CellStruct::Empty, false, nullptr);

			}
		}

		if (pThis->IsAlive)
		{
			if (_res == DamageState::NowRed)
			{
				if (args.Attacker && pThis->AttachedTag)
				{
					pThis->AttachedTag->SpringEvent(TriggerEvent::QuarterHealth_combatonly, pThis, CellStruct::Empty, false, nullptr);
				}

				if (!pThis->IsAlive)
				{
					R->EAX(DamageState::PostMortem);
					return 0x5F584A;
				}

				if (pThis->AttachedTag)
				{
					pThis->AttachedTag->SpringEvent(TriggerEvent::QuarterHealth_anysource, pThis, CellStruct::Empty, false, nullptr);
				}
			}

			if (pThis->IsAlive)
			{
				if (pThis->Health != oldstrength && oldstrength == pObjType->Strength)
				{
					if (args.Attacker && pThis->AttachedTag)
					{
						pThis->AttachedTag->SpringEvent(TriggerEvent::FirstDamaged_combatonly, pThis, CellStruct::Empty, false, nullptr);
					}

					if (pThis->AttachedTag && pThis->IsAlive)
					{
						pThis->AttachedTag->SpringEvent(TriggerEvent::FirstDamaged_anysource, pThis, CellStruct::Empty, false, nullptr);
					}

					if (!pThis->IsAlive)
					{
						R->EAX(DamageState::PostMortem);
						return 0x5F584A;
					}

					if (pThis->AttachedTag && args.Attacker)
					{
						pThis->AttachedTag->SpringEvent(TriggerEvent::FirstDamaged_anysource, pThis, CellStruct::Empty, false, args.Attacker);
					}
				}

				if (pThis->IsAlive && (arg10b <= 0 || pThis->Health > 0))
				{
					HouseClass* v27 = 0;
					if (!pThis->Health)
					{
						if (args.Attacker)
						{
							v27 = args.Attacker->Owner;
						}

						if (!args.SourceHouse || args.Attacker && args.SourceHouse == v27)
						{
							pThis->RegisterDestruction(args.Attacker);
						}
						else
						{
							pThis->RegisterKill(args.SourceHouse);
						}

						pThis->Disappear(true);
						_res = DamageState::NowDead;
					}

					if (_res != DamageState::NowDead && !pWHExt->Nonprovocative)
					{
						if (pThis->IsAlive && args.Attacker && pThis->AttachedTag)
						{
							pThis->AttachedTag->SpringEvent(TriggerEvent::AttackedByAnybody, pThis, CellStruct::Empty, false, args.Attacker);
						}

						if (pThis->IsAlive && args.Attacker && pThis->AttachedTag)
						{
							pThis->AttachedTag->SpringEvent(TriggerEvent::AttackedByHouse, pThis, CellStruct::Empty, false, args.Attacker);
						}

						if (pThis->IsAlive && pThis->AttachedTag)
						{
							pThis->AttachedTag->RaiseEvent((TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByHouse,
								pThis,
								CellStruct::Empty,
								false,
								args.Attacker
							);
						}

						if (pThis->IsAlive && pThis->AttachedTag)
						{
							pThis->AttachedTag->RaiseEvent((TriggerEvent)AresTriggerEvents::AttackedOrDestroyedByAnybody,
								pThis,
								CellStruct::Empty,
								false,
								args.Attacker
							);
						}

						if (pThis->IsAlive && pThis->IsSelected)
						{
							pThis->UpdatePlacement(PlacementType::Redraw);
						}
					}

					R->EAX(_res);
					return 0x5F584A;
				}
			}
		}

		R->EAX(DamageState::PostMortem);
		return 0x5F584A;
	}

	int _oldStr = pThis->Health;
	int _adj = _oldStr - *args.Damage;
	pThis->Health = MinImpl(_adj, maxstrength);

	if (_oldStr != pThis->Health)
	{
		pThis->Flash(7);
	}

	R->EAX(_res);
	return 0x5F584A;
}

#pragma endregion

#pragma region Techno

static bool IsTechnoImmuneToAffects(TechnoClass* pTechno, Rank rank, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->ImmunityType.isset() && TechnoExtData::HasImmunity(rank, pTechno, pWHExt->ImmunityType))
	{
		return true;
	}

	if (pWH->Radiation && TechnoExtData::IsRadImmune(rank, pTechno))
	{
		return true;
	}

	if (pWH->PsychicDamage && TechnoExtData::IsPsionicsWeaponImmune(rank, pTechno))
	{
		return true;
	}

	if (pWH->Poison && TechnoExtData::IsPoisonImmune(rank, pTechno))
	{
		return true;
	}

	return false;
}

DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_Handle, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;
	int damage_ = *args.Damage;
	bool _isNegativeDamage = *args.Damage < 0;
	auto pType = pThis->GetTechnoType();
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	pWHExt->ApplyDamageMult(pThis, &args);
	applyCombatAlert(pThis, &args);
	TechnoExtData::ApplyKillWeapon(pThis, args.Attacker, args.WH);

	if (args.Attacker && (!args.Attacker->IsAlive || args.Attacker->Health <= 0) && !args.Attacker->Owner)
		args.Attacker = nullptr; //clean up;

	if (!pThis || !pThis->IsAlive || pThis->Health <= 0)
	{
		R->EAX(DamageState::NowDead);
		return 0x702D1F;
	}

	if (!args.IgnoreDefenses)
	{
		if (auto pShieldData = pExt->GetShield())
		{
			pShieldData->OnReceiveDamage(&args);
		}
	}

	if (!args.IgnoreDefenses && *args.Damage >= 0)
	{

		*args.Damage = (int)TechnoExtData::GetDamageMult(pThis, (double)(*args.Damage));

		if (pExt->SkipLowDamageCheck)
		{
			pExt->SkipLowDamageCheck = false;
		}
		else
		{

			// Restore overridden instructions
			if (*args.Damage < 1)
				*args.Damage = 1;
		}

		if (args.Attacker)
		{
			if (pType->TypeImmune)
			{
				auto pAttackerType = args.Attacker->GetTechnoType();
				if (pType == pAttackerType && pThis->Owner == args.Attacker->Owner)
				{
					R->EAX(DamageState::Unaffected);
					return 0x702D1F;
				}
			}
		}
	}

	if (pThis->IsIronCurtained() && !args.IgnoreDefenses && !_isNegativeDamage)
	{

		if (!pWHExt->CanAffectInvulnerable(pThis))
		{
			if (pThis->ProtectType == ProtectTypes::ForceShield)
				MapClass::FlashbangWarheadAt(2 * (*args.Damage), args.WH, pThis->Location, true, SpotlightFlags::NoRed | SpotlightFlags::NoGreen);
			else if (pWHExt->IC_Flash.Get(RulesExtData::Instance()->IC_Flash.Get()))
				MapClass::FlashbangWarheadAt(2 * (*args.Damage), args.WH, pThis->Location, true, SpotlightFlags::NoColor);

			*args.Damage = 0;
			R->EAX(DamageState::Unaffected);
			return 0x702D1F;
		}
	}

	if (pThis->IsBeingWarpedOut() || TechnoExtData::IsChronoDelayDamageImmune(flag_cast_to<FootClass*, false>(pThis)) && !args.IgnoreDefenses)
	{
		*args.Damage = 0;
		R->EAX(DamageState::Unaffected);
		return 0x702D1F;
	}

	if (pType->DamageReducesReadiness)
	{
		const double v111 = pType->ReadinessReductionMultiplier * ((double)*args.Damage / (double)pType->Strength);
		const int _ammo = (int)((double)pThis->Ammo - (double)pType->Ammo * v111);
		pThis->Ammo = MaxImpl(_ammo, 0);
		pThis->StartReloading();
	}

	const auto nRank = pThis->Veterancy.GetRemainingLevel();

	if (pThis->BunkerLinkedItem && !args.IgnoreDefenses)
	{
		if (auto pBld = cast_to<BuildingClass*, false>(pThis))
		{
			if (args.WH->PenetratesBunker)
			{
				*args.Damage = 0;
				R->EAX(DamageState::Unaffected);
				return 0x702D1F;
			}
		}
		else
		{
			if (!args.WH->PenetratesBunker && pThis->GetCell()->GetBuilding() == pThis->BunkerLinkedItem)
			{
				*args.Damage = 0;
				R->EAX(DamageState::Unaffected);
				return 0x702D1F;
			}
		}
	}

	if (IsTechnoImmuneToAffects(pThis, nRank, args.WH))
	{
		*args.Damage = 0;
		R->EAX(DamageState::Unaffected);
		return 0x702D1F;
	}

	if (!args.WH->AffectsAllies && args.Attacker && args.Attacker->Owner->IsAlliedWith(pThis->Owner))
	{
		*args.Damage = 0;
		R->EAX(DamageState::Unaffected);
		return 0x702D1F;
	}

	const auto pSourceHouse = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

	if (args.WH->Psychedelic)
	{
		if (TechnoExtData::IsPsionicsImmune(nRank, pThis) || TechnoExtData::IsBerserkImmune(nRank, pThis))
		{
			R->EAX(DamageState::Unaffected);
			return 0x702D1F;
		}

		if (pThis->Owner->IsAlliedWith(args.SourceHouse))
		{
			R->EAX(DamageState::Unaffected);
			return 0x702D1F;
		}

		if (pThis->WhatAmI() == AbstractType::Building)
		{
			R->EAX(DamageState::Unaffected);
			return 0x702D1F;
		}

		if (!pWHExt->GoBerzerkFor((FootClass*)pThis, args.Damage))
		{
			R->EAX(DamageState::Unchanged);
			return 0x702D1F;
		}
	}
	else
	{
		// restoring TS berzerk cyborg
		//this will happen regardless the immunity i guess
		if (auto pInf = cast_to<InfantryClass*, false>(pThis))
		{
			if (RulesClass::Instance->BerzerkAllowed && pInf->Type->Cyborg && pThis->IsYellowHP())
			{
				if (!pInf->Berzerk)
				{
					pInf->Berzerk = true;
					pInf->GoBerzerkFor(10);
				}
			}
		}
	}

	PhobosAEFunctions::ApplyReflectDamage(pThis, args.Damage, args.Attacker, pSourceHouse, args.WH);

	_res = pThis->ObjectClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	const bool Show = Phobos::Otamaa::IsAdmin || *args.Damage;

	if (Phobos::Debug_DisplayDamageNumbers && Show)
		FlyingStrings::DisplayDamageNumberString(*args.Damage, DamageDisplayType::Regular, pThis->GetRenderCoords(), TechnoExtContainer::Instance.Find(pThis)->DamageNumberOffset);

	GiftBoxFunctional::TakeDamage(TechnoExtContainer::Instance.Find(pThis), TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType()), args.WH, _res);

	if (_res != DamageState::PostMortem && !pThis->IsAlive)
	{
		R->EAX(DamageState::NowDead);
		return 0x702D1F;
	}

	if (args.Attacker)
	{
		pThis->Owner->UpdateAngerNodes(pType->GetCost() * ((double)*args.Damage / pType->Strength), args.SourceHouse);
	}

	if (_res != DamageState::NowDead)
	{
		if (_res == DamageState::PostMortem)
		{
			R->EAX(DamageState::PostMortem);
			return 0x702D1F;
		}

		if (_res == DamageState::Unaffected)
		{
			goto LABEL_101;
		}

		goto LABEL_94;
	}


	if (pThis->WhatAmI() != AbstractType::Building || !args.WH->CausesDelayKill)
	{

	LABEL_94:

		pThis->RadarFlashTimer.Start(RulesClass::Instance->RadarCombatFlashTime);

		if (_res != DamageState::Unaffected && _res != DamageState::NowDead
			 && pType->CanDisguise && !pType->PermaDisguise)
		{
			if (pThis->IsDisguised())
			{
				pThis->ClearDisguise();
			}

			pThis->InfantryBlinkTimer.Start(2 * *args.Damage);
		}

	LABEL_101:
		if (!pThis->Health)
		{
			_res = DamageState::NowDead;
		}

		switch (_res)
		{
		case DamageState::Unaffected:
		case DamageState::NowRed:
			goto LABEL_191;
		case DamageState::Unchanged:
			goto LABEL_181;
		case DamageState::NowYellow:
			if (pType->VoiceFeedback.Count > 0
				&& Random2Class::NonCriticalRandomNumber->RandomRanged(0, 99) < 30
					  && pThis->Owner->ControlledByCurrentPlayer())
			{
				VocClass::PlayIndexAtPos(Random2Class::NonCriticalRandomNumber->Random() % pType->VoiceFeedback.Count, pThis->Location, 0);
			}
			goto LABEL_191;
		case DamageState::NowDead:

			if (pWHExt->Supress_LostEva.Get())
				pExt->SupressEVALost = true;

			GiftBoxFunctional::Destroy(pExt, pTypeExt);

			if (pThis->IsAlive)
			{
				std::set<PhobosAttachEffectTypeClass*> cumulativeTypes {};
				std::vector<WeaponTypeClass*> expireWeapons {};
				PhobosAEFunctions::ApplyExpireWeapon(expireWeapons, cumulativeTypes, pThis);

				for (auto const& pWeapon : expireWeapons)
				{

					TechnoClass* pTarget = pThis;
					if (!pThis->IsAlive)
						pTarget = nullptr;

					WeaponTypeExtData::DetonateAt(pWeapon, pThis->Location, pTarget, false, pThis->Owner);
				}
			}

			if (auto pManager = pThis->SlaveManager)
			{
				pManager->Killed(args.Attacker);
			}

			if (pThis->DrainTarget)
			{
				if (auto DrainAnim = std::exchange(pThis->DrainAnim, nullptr))
				{
					DrainAnim->RemainingIterations = 0;
					DrainAnim->UnInit();
					DrainAnim = nullptr;;
				}

				pThis->DrainTarget->DrainingMe = 0;
				if (auto pDrainingTargetOwn = pThis->DrainTarget->Owner)
				{
					pDrainingTargetOwn->RecheckPower = true;
				}

				pThis->DrainTarget = 0;
			}

			if (auto pDrainer = pThis->DrainingMe)
			{
				if (auto DrainAnim = std::exchange(pDrainer->DrainAnim, nullptr))
				{
					DrainAnim->RemainingIterations = 0;
					DrainAnim->UnInit();
					DrainAnim = nullptr;;
				}

				if (pDrainer->DrainTarget)
				{
					pDrainer->DrainTarget->DrainingMe = 0;
					if (auto pDrainingTargetOwn = pDrainer->DrainTarget->Owner)
					{
						pDrainingTargetOwn->RecheckPower = true;
					}

					pDrainer->DrainTarget = 0;
				}
			}

			if (auto pManager = pThis->CaptureManager)
			{
				pManager->FreeAll();
			}

			if (pType->VoiceDie.Count > 0 && pThis->Owner->ControlledByCurrentPlayer())
			{
				auto const& nSound = pWHExt->DieSound_Override;

				if (nSound.isset())
				{
					VocClass::PlayIndexAtPos(nSound, pThis->Location);
				}
				else
				{
					if (pType->VoiceDie.Count == 1)
					{
						VocClass::PlayIndexAtPos(pType->VoiceDie[0], pThis->Location);
					}
					else
					{
						int rand = Random2Class::NonCriticalRandomNumber->Random();
						VocClass::PlayIndexAtPos(pType->VoiceDie[rand % pType->VoiceDie.Count], pThis->Location);
					}
				}
			}

			if (pType->DieSound.Count > 0)
			{
				auto const& nSound = pWHExt->VoiceSound_Override;

				if (nSound.isset())
				{
					VocClass::PlayIndexAtPos(nSound, pThis->Location);
				}
				else
				{
					if (pType->DieSound.Count == 1)
					{
						VocClass::PlayIndexAtPos(pType->DieSound[0], pThis->Location);
					}
					else
					{
						int rand = Random2Class::NonCriticalRandomNumber->Random();
						VocClass::PlayIndexAtPos(pType->DieSound[rand % pType->DieSound.Count], pThis->Location);

					}
				}
			}

			if (TechnoTypeExtContainer::Instance.Find(pType)->TiberiumSpill)
			{
				const auto pBld = cast_to<BuildingClass*, false>(pThis);

				if (pBld && !pBld->Type->Weeder)
				{

					auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
					double stored = storage->GetAmounts();

					if (stored > 0.0
						&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune)
					{
						// don't spill more than we can hold
						double max = 9.0;
						if (max > pType->Storage)
						{
							max = pType->Storage;
						}

						const int nIdx = storage->GetHighestStorageIdx();

						// assume about half full, recalc if possible
						int value = static_cast<int>(max / 2);
						if (pType->Storage > 0)
						{
							value = int(stored / pType->Storage * max);
						}

						// get the spill center
						TechnoClass::SpillTiberium(value, nIdx, MapClass::Instance->GetCellAt(pThis->GetCoords()), { 0,2 });
					}
				}
			}

			pThis->SendToEachLink(RadioCommand::NotifyUnlink);
			pThis->Stun();

			if (pTypeExt->TiberiumRemains.Get(pType->TiberiumHeal && RulesExtData::Instance()->Tiberium_HealEnabled))
			{
				int nIdx = pExt->TiberiumStorage.GetHighestStorageIdx();
				const CellClass* pCenter = MapClass::Instance->GetCellAt(pThis->Location);

				// increase the tiberium for the four neighbours and center.
				// center is retrieved by getting a neighbour cell index >= 8
				for (int i = 0; i < 8; i += 2)
				{
					pCenter->GetNeighbourCell((FacingType)i)->IncreaseTiberium(nIdx, ScenarioClass::Instance->Random.RandomFromMax(2));
				}
			}

			if (auto& pParticleZero = pThis->FireParticleSystem)
			{
				pParticleZero->UnInit();
				pParticleZero = nullptr;
			}

			if (pThis->GetHeight() > 0 || !pThis->IsABomb || pThis->GetCell()->LandType != LandType::Water)
			{
				if (pType->MaxDebris > 0)
				{
					auto totalSpawnAmount = ScenarioClass::Instance->Random.RandomRanged(pType->MinDebris, pType->MaxDebris);
					auto nCoords = pThis->GetCoords();

					if (totalSpawnAmount && pType->DebrisTypes.Count > 0 && pType->DebrisMaximums.Count > 0)
					{
						for (int currentIndex = 0; currentIndex < pType->DebrisTypes.Count; ++currentIndex)
						{
							if (currentIndex >= pType->DebrisMaximums.Count)
								break;

							if (!pType->DebrisMaximums[currentIndex] || !pType->DebrisTypes.Items[currentIndex])
								continue;

							//this never goes to 0
							int amountToSpawn = (Math::abs(int(ScenarioClass::Instance->Random.Random())) % pType->DebrisMaximums[currentIndex]) + 1;
							amountToSpawn = LessOrEqualTo(amountToSpawn, totalSpawnAmount);
							totalSpawnAmount -= amountToSpawn;

							for (; amountToSpawn > 0; --amountToSpawn)
							{

								auto pVoxAnim = GameCreate<VoxelAnimClass>(pType->DebrisTypes.Items[currentIndex],
								&nCoords, pThis->Owner);

								VoxelAnimExtContainer::Instance.Find(pVoxAnim)->Invoker = pThis;
							}

							if (totalSpawnAmount <= 0)
							{
								totalSpawnAmount = 0;
								break;
							}
						}
					}

					if (totalSpawnAmount > 0)
					{
						if (const auto pArray = GetDebrisAnim(pType))
						{
							auto debrisAnim_Coord = nCoords;
							debrisAnim_Coord.Z += 20;

							for (int b = 0; b < totalSpawnAmount; ++b)
							{
								if (auto pDebrisAnimType = pArray->Items[ScenarioClass::Instance->Random.RandomFromMax(pArray->Count - 1)])
								{
									AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pDebrisAnimType, debrisAnim_Coord, 0, 1, AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_400, 0, 0), args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
									pThis->GetOwningHouse(), false);
								}
							}
						}
					}
				}

				if (!(pThis->WhatAmI() == AbstractType::Building && !BuildingTypeExtContainer::Instance.Find(((BuildingClass*)pThis)->Type)->Explodes_DuringBuildup && (pThis->CurrentMission == Mission::Construction || pThis->CurrentMission == Mission::Selling)))
				{
					auto pWeapon = pThis->GetWeapon(pThis->CurrentWeaponNumber)->WeaponType;

					if (pType->Explodes || pThis->HasAbility(AbilityType::Explodes) || (pWeapon && pWeapon->Suicide))
					{
						if (TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->Explodes_KillPassengers)
						{
							while (pThis->Passengers.FirstPassenger)
							{
								auto pPassenger = pThis->Passengers.GetFirstPassenger();
								if (auto pTeam = pPassenger->Team)
								{
									pTeam->RemoveMember(pPassenger);
								}

								if (auto pPassengerremoved = pThis->Passengers.RemoveFirstPassenger())
								{
									pPassengerremoved->KillCargo(args.Attacker);
									pPassengerremoved->RegisterDestruction(args.Attacker);
								}
							}
						}

						if (!pWHExt->ApplySuppressDeathWeapon(pThis))
							pThis->FireDeathWeapon(0);
					}
				}
			}

			if (args.Attacker)
			{
				auto SourCoords = args.Attacker->Location;

				if (!pWHExt->SuppressRevengeWeapons)
				{
					if (pTypeExt->RevengeWeapon &&
						EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, args.Attacker->Owner) &&
						!pWHExt->SuppressRevengeWeapons_Types.empty() && !pWHExt->SuppressRevengeWeapons_Types.Contains(pTypeExt->RevengeWeapon))
					{
						WeaponTypeExtData::DetonateAt(pTypeExt->RevengeWeapon.Get(), args.Attacker, pThis, true, nullptr);
					}

					if (pThis->IsAlive)
					{
						for (const auto& weapon : pExt->RevengeWeapons)
						{
							if (EnumFunctions::CanTargetHouse(weapon.ApplyToHouses, pThis->Owner, args.Attacker->Owner) && !pWHExt->SuppressRevengeWeapons_Types.empty() && !pWHExt->SuppressRevengeWeapons_Types.Contains(weapon.Value))
								WeaponTypeExtData::DetonateAt(weapon.Value, args.Attacker, pThis, true, nullptr);
						}
					}
				}

				if (pThis->IsAlive)
				{
					PhobosAEFunctions::ApplyRevengeWeapon(pThis, args.Attacker, args.WH);
				}
			}

			if (auto pBomb = pThis->AttachedBomb)
			{
				pBomb->Detonate();
			}
			R->EAX(_res);
			return 0x702D1F;
		default:
		{
		LABEL_181:
			if (pType->DamageSound != -1)
			{
				VocClass::PlayIndexAtPos(pType->DamageSound, pThis->Location, 0);
			}

			if ((pType->ToProtect || pThis->__ProtectMe_3CF) && !pThis->Owner->IsControlledByHuman() && args.Attacker)
			{
				pThis->BaseIsAttacked(args.Attacker);
			}


			if (_res == DamageState::NowDead)
			{
				R->EAX(_res);
				return 0x702D1F;
			}

		LABEL_191:

			if (args.Attacker && !pThis->Owner->IsAlliedWith(args.Attacker))
			{
				pThis->IsTickedOff = 1;
			}

			bool IsAffected = _res != DamageState::Unaffected;
			bool bAffected = false;

			if (IsAffected || args.IgnoreDefenses || _isNegativeDamage || *args.Damage)
			{
				if (IsAffected && _isNegativeDamage)
				{
					const auto rank = pThis->Veterancy.GetRemainingLevel();
					const auto fromTechno = pTypeExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank);
					const int amount = pWHExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank)
						->Get(fromTechno);

					//the timer will always restart
					//not accumulated
					if (amount > 0)
					{
						pExt->SelfHealing_CombatDelay.Start(amount);
					}
				}

			}
			else { bAffected = true; }

			const auto pHouse = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

			if (IsAffected && pWHExt->DecloakDamagedTargets.Get())
				pThis->Reveal();

			const auto bCond1 = (!bAffected || !pWHExt->EffectsRequireDamage);
			const auto bCond2 = (!pWHExt->EffectsRequireVerses || (pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(pThis, args.WH)).Verses >= 0.0001));

			if (bCond1 && bCond2)
			{
				AresWPWHExt::applyKillDriver(args.WH, args.Attacker, pThis);

				if (pWHExt->Sonar_Duration > 0)
				{
					auto& nSonarTime = TechnoExtContainer::Instance.Find(pThis)->CloakSkipTimer;
					if (pWHExt->Sonar_Duration > nSonarTime.GetTimeLeft())
					{
						nSonarTime.Start(pWHExt->Sonar_Duration);

						if (pThis->CloakState != CloakState::Uncloaked)
						{
							pThis->Uncloak(true);
							pThis->NeedsRedraw = true;
						}
					}
				}

				if (pWHExt->DisableWeapons_Duration > 0)
				{
					auto& nTimer = TechnoExtContainer::Instance.Find(pThis)->DisableWeaponTimer;
					if (pWHExt->DisableWeapons_Duration > nTimer.GetTimeLeft())
					{
						nTimer.Start(pWHExt->DisableWeapons_Duration);
					}
				}

				if (pWHExt->Flash_Duration > 0 && pWHExt->Flash_Duration > pThis->Flashing.DurationRemaining)
				{
					pThis->Flash(pWHExt->Flash_Duration);
				}

				if (pWHExt->RemoveDisguise)
				{
					pWHExt->ApplyRemoveDisguise(pHouse, pThis);
				}

				if (pWHExt->RemoveMindControl)
				{
					pWHExt->ApplyRemoveMindControl(pHouse, pThis);
				}
			}

			if (pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
			{
				if (_res == DamageState::NowYellow || _res == DamageState::NowRed)
				{
					StackVector<ParticleSystemTypeClass*, 15u> _Particles {};
					const auto allowAny = pTypeExt->ParticleSystems_DamageSmoke.HasValue();

					for (const auto pSystem : pTypeExt->ParticleSystems_DamageSmoke.GetElements(pType->DamageParticleSystems))
					{
						if (allowAny || pSystem->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						{
							_Particles->push_back(pSystem);
						}
					}

					if (!pThis->NaturalParticleSystem && !_Particles->empty() && pThis->GetHeight() > -10)
					{
						CoordStruct _offs = pThis->Location + pType->GetParticleSysOffset();
						pThis->NaturalParticleSystem =
							GameCreate<ParticleSystemClass>(_Particles[ScenarioClass::Instance->Random.RandomFromMax(_Particles->size() - 1)], _offs, nullptr, pThis);
					}
				}
			}
			else
			{
				if (auto& pPart = pThis->NaturalParticleSystem)
				{
					pPart->UnInit();
					pPart = nullptr;
				}

			}

			if (_isNegativeDamage)
			{
				R->EAX(_res);
				return 0x702D1F;
			}

			const auto pFoot = flag_cast_to<FootClass*, false>(pThis);
			auto retalitate = args.Attacker;
			if (args.Attacker && args.Attacker->InOpenToppedTransport && args.Attacker->Transporter)
			{
				retalitate = args.Attacker->Transporter;
			}

			bool retaliate = false;
			if (pThis->AllowToRetaliate(retalitate, args.WH))
			{
				if (retalitate)
				{

					if (pThis->IsCloseEnough(args.Attacker, pThis->SelectWeapon(args.Attacker))
						|| !pThis->Owner->IsControlledByHuman()
						|| (((pType->Sight + 0.5) * 256.0) >= (retalitate->Location - pThis->Location).Length()))
					{
						pThis->Override_Mission(Mission::Attack, retalitate);
					}

				}

				retaliate = true;
			}

			if (!pWHExt->PreventScatter)
			{
				if (auto pFoot = flag_cast_to<FootClass*, false>(pThis))
				{
					if (!pFoot->Target && !pFoot->Destination)
					{
						if (retaliate && (RulesClass::Instance->PlayerScatter || pFoot->HasAbility(AbilityType::Scatter)))
						{
							pFoot->Scatter(CoordStruct::Empty, true, false);
						}
						else if (!pFoot->IsTethered && pFoot->WhatAmI() != AircraftClass::AbsID && pThis->GetCurrentMissionControl()->Scatter)
						{
							const bool IsMoving = pFoot->Locomotor.GetInterfacePtr()->Is_Moving();

							if (!IsMoving)
							{
								const bool IsHuman = pFoot->Owner->IsControlledByHuman();
								const bool IsScatter = RulesClass::Instance->PlayerScatter;
								const bool IscatterAbility = pFoot->HasAbility(AbilityType::Scatter);
								if (!IsHuman || IsScatter || IscatterAbility)
								{
									pFoot->Scatter(CoordStruct::Empty, true, false);
								}
							}
						}
					}
				}
			}

			R->EAX(_res);
			return 0x702D1F;
		}


		if (auto pBld = cast_to<BuildingClass*, false>(pThis))
		{
			if (!pBld->Type->EligibleForDelayKill)
			{
				goto LABEL_94;
			}

			const int v22 = (int)(((double)args.WH->DelayKillAtMax * (double)args.WH->DelayKillFrames - (double)args.WH->DelayKillFrames)
			  / (double)((int)args.WH->CellSpread << 8)
			  * (double)args.DistanceToEpicenter
			  + (double)args.WH->DelayKillFrames);

			if (!pBld->IsGoingToBlow || pBld->GoingToBlowTimer.Expired())
			{
				pBld->GoingToBlowTimer.Start(v22);
				pBld->IsGoingToBlow = true;
			}
		}
		}
	}

	pThis->IsAlive = true;
	pThis->Health = 1;
	R->EAX(DamageState::PostMortem);
	return 0x702D1F;
}
#pragma endregion

#pragma region Building

#ifdef _TODO
DEFINE_HOOK(0x442230, BuildingClass_ReceiveDamage_Handle, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;

	if (pThis == args.Attacker && !pThis->Type->DamageSelf)
	{
		R->EAX(_res);
		return 0x442C14;
	}

	auto pShape = pThis->GetShapeNumber();
	auto foundation = pThis->GetFoundationData();

	if (args.Attacker && !pThis->IsStrange())
	{
		pThis->Owner->LAEnemy = args.Attacker->Owner->ArrayIndex;
		pThis->Owner->LATime = Unsorted::CurrentFrame;
		pThis->BaseIsAttacked(args.Attacker);
	}

	StackVector<TechnoClass*, 0xAu> CachedRadio { };

	for (auto i = 0; i < pThis->RadioLinks.Capacity; i++)
	{
		if (pThis->RadioLinks.Items[i])
		{
			CachedRadio->emplace_back(pThis->RadioLinks.Items[i]);
		}
	}

	if (pThis->Type->LaserFence && !args.IgnoreDefenses)
	{
		R->EAX(DamageState::Unaffected);
		return 0x442C14;
	}

	if (pThis->Type->BridgeRepairHut && pThis->Type->Immune)
	{
		R->EAX(DamageState::Unaffected);
		return 0x442C14;
	}

	if (pThis->Health)
	{

		_res = pThis->TechnoClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

		if (!pThis->IsAlive)
		{
			R->EAX(_res);
			return 0x442C14;
		}

		switch (_res)
		{
		case DamageState::NowYellow:
		{
			v31 = (float*)this->t.ParticleSystems[2];
			if (v31)
			{
				v31[58] = v31[58] * 1.5;
			}

			if (this->t.r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(&this->t)->DamageSound == -1)
			{
				coord = this->t.r.m.o.Coord;
				VocClass::Add(Rule->BlowupSound, &coord, 0);
			}

			while (a2->X != REFRESH_EOL || a2->Y != REFRESH_EOL)
			{
				v32 = *a2;
				vftable = this->t.r.m.o.a.vftable;
				++a2;
				v64 = (int)v32;
				nCoord = ((CellStruct * (__thiscall*)(ObjectClass*, CellStruct*))vftable->t.r.m.o.Coord_Cell)(&this->t.r.m.o, &v69);
				v32.X += nCoord->X;
				LOWORD(nCoord) = HIWORD(v64) + nCoord->Y;
				v66.Z = 0;
				v66.X = (v32.X << 8) + 128;
				v66.Y = ((__int16)nCoord << 8) + 128;
				v66.Z = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &v66);
				// /*
				// **   Show pieces of fire to indicate that a significant change in
				// **   damage level has occurred.
				// */
				if (warhead->Sparky)
				{
					v35 = BuildingTypeClass::Height(this->Class, 0);
					v36 = BuildingTypeClass::Width(this->Class);
					switch (Random2Class::operator()(&Scen->RandomNumber, 0, v35 + v36 + 5))
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						v37 = (AnimClass*)operator new(0x1C8u);
						if (v37)
						{
							v38 = Random2Class::operator()(&Scen->RandomNumber, 1, 3);
							Vector_Item = (int*)Rule->OnFire.dvc.Vector_Item;
							v60 = v38;
							v57 = Coord_Scatter(&retval, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v37, *Vector_Item, v57, 0, v60, AnimFlag_200 | AnimFlag_400, 0, 0);
							goto LABEL_75;
						}
						break;
					case 6:
					case 7:
					case 8:
						v41 = (AnimClass*)operator new(0x1C8u);
						if (v41)
						{
							v61 = Random2Class::operator()(&Scen->RandomNumber, 1, 3);
							v42 = (int*)(Rule->OnFire.dvc.Vector_Item + 1);
							v58 = Coord_Scatter(&v71, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v41, *v42, v58, 0, v61, AnimFlag_200 | AnimFlag_400, 0, 0);
							goto LABEL_75;
						}
						break;
					case 9:
						v43 = (AnimClass*)operator new(0x1C8u);
						if (v43)
						{
							v44 = (int*)(Rule->OnFire.dvc.Vector_Item + 2);
							v59 = Coord_Scatter(&v73, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v43, *v44, v59, 0, 1, AnimFlag_200 | AnimFlag_400, 0, 0);
						LABEL_75:
							// /*
							// **   If the animation was created, then attach it to the building.
							// */
							if (v40)
							{
								AnimClass::Attach_To(v40, &this->t.r.m.o);
							}
						}
						break;
					default:
						continue;
					}
				}
			}

			break;
		}
		case DamageState::NowRed:
		{
			if (this->t.r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(&this->t)->DamageSound == -1)
			{
				coord = this->t.r.m.o.Coord;
				VocClass::Add(Rule->BlowupSound, &coord, 0);
			}

			while (a2->X != REFRESH_EOL || a2->Y != REFRESH_EOL)
			{
				v32 = *a2;
				vftable = this->t.r.m.o.a.vftable;
				++a2;
				v64 = (int)v32;
				nCoord = ((CellStruct * (__thiscall*)(ObjectClass*, CellStruct*))vftable->t.r.m.o.Coord_Cell)(&this->t.r.m.o, &v69);
				v32.X += nCoord->X;
				LOWORD(nCoord) = HIWORD(v64) + nCoord->Y;
				v66.Z = 0;
				v66.X = (v32.X << 8) + 128;
				v66.Y = ((__int16)nCoord << 8) + 128;
				v66.Z = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &v66);
				// /*
				// **   Show pieces of fire to indicate that a significant change in
				// **   damage level has occurred.
				// */
				if (warhead->Sparky)
				{
					v35 = BuildingTypeClass::Height(this->Class, 0);
					v36 = BuildingTypeClass::Width(this->Class);
					switch (Random2Class::operator()(&Scen->RandomNumber, 0, v35 + v36 + 5))
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						v37 = (AnimClass*)operator new(0x1C8u);
						if (v37)
						{
							v38 = Random2Class::operator()(&Scen->RandomNumber, 1, 3);
							Vector_Item = (int*)Rule->OnFire.dvc.Vector_Item;
							v60 = v38;
							v57 = Coord_Scatter(&retval, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v37, *Vector_Item, v57, 0, v60, AnimFlag_200 | AnimFlag_400, 0, 0);
							goto LABEL_75;
						}
						break;
					case 6:
					case 7:
					case 8:
						v41 = (AnimClass*)operator new(0x1C8u);
						if (v41)
						{
							v61 = Random2Class::operator()(&Scen->RandomNumber, 1, 3);
							v42 = (int*)(Rule->OnFire.dvc.Vector_Item + 1);
							v58 = Coord_Scatter(&v71, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v41, *v42, v58, 0, v61, AnimFlag_200 | AnimFlag_400, 0, 0);
							goto LABEL_75;
						}
						break;
					case 9:
						v43 = (AnimClass*)operator new(0x1C8u);
						if (v43)
						{
							v44 = (int*)(Rule->OnFire.dvc.Vector_Item + 2);
							v59 = Coord_Scatter(&v73, &v66, 96, 0);
							v40 = AnimClass::AnimClass(v43, *v44, v59, 0, 1, AnimFlag_200 | AnimFlag_400, 0, 0);
						LABEL_75:
							// /*
							// **   If the animation was created, then attach it to the building.
							// */
							if (v40)
							{
								AnimClass::Attach_To(v40, &this->t.r.m.o);
							}
						}
						break;
					default:
						continue;
					}
				}
			}

			break;
		}
		case DamageState::NowDead:
		{

			if (this->t.__BunkerLinkedItem)
			{
				v19 = v63.vftble->ID1((int)&v63, &this->t.__BunkerLinkedItem);
				if (v19 != -1 && v19 < v63.ActiveCount)
				{
					--v63.ActiveCount;
					for (; v19 < v63.ActiveCount; v63.Vector_Item[v19 - 1] = v63.Vector_Item[v19])
					{
						++v19;
					}
				}
				BuildingClass_bunker_code(this);
			}

			CaptureManager = this->t.__CaptureManager;
			if (CaptureManager)
			{
				CaptureManagerClass::Free_All(CaptureManager);
			}

			if (this->t.__LocomotorTarget)
			{
				TechnoClass_70FEE0(&this->t, 1);
			}

			for (i = 0; i < v63.ActiveCount; ++i)
			{
				v22 = (TechnoClass*)v63.Vector_Item[i];
				v23 = v22->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(v22, &v74);
				v24 = this->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(this, &v75);
				v66 = *Operator_Negate(&a1, v24->X - v23->X, v24->Y - v23->Y, v24->Z - v23->Z);
				// /*
				// ** If we were in contact with a landed plane, blow the plane up too.
				// */
				if (CoordStruct::Length(&v66) < 0x100 || this->Class->Helipad)
				{
					v25 = v22->r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(v22);
					v26 = v22->r.m.o.a.vftable;
					v64 = 10 * v25->ot.MaxStrength;
					v26->t.r.m.o.Take_Damage((ObjectClass*)v22, (int)&v64, 0, Rule->C4Warhead, 0, 1, 1, 0);
				}
				else
				{
					this->t.r.m.o.a.vftable->t.r.Transmit_Message__MSG__PTR((RadioClass*)this, RADIO_RUN_AWAY, (FootClass*)v22);
					v22->__LinkedBuilding = 0;
				}
			}

			v27 = v63.VectorMax;
			v63.vftble->Clear((VectorClass*)&v63);
			((void(__stdcall*)(int, _DWORD))v63.vftble->Resize)(v27, 0);

			if (this->Class->CanBeOccupied)
			{
				BuildingClass_UnloadOccupants_AllOccupantsHaveLeft(this, 0, 0);
			}

			LightSource = (LightSourceClass*)this->__LightSource;
			if (LightSource)
			{
				LightSourceClass::Deactivate(LightSource, 0);
			}

			((void(__thiscall*)(BuildingClass*, int, TechnoClass*, char, CellStruct*))this->t.r.m.o.a.vftable->sensors_capture_4EC)(
				this,
				0,
				source,
				forced,
				a2);

			Started = this->CountDown.Started;
			DelayTime = this->CountDown.DelayTime;

			if (Started == -1)
			{
				goto LABEL_59;
			}

			if (Frame - Started < DelayTime)
			{
				DelayTime -= Frame - Started;
			LABEL_59:
				if (DelayTime > 0)
				{
					this->t.r.m.o.a.vftable->t.r.m.o.Remove_This_deletethis((AbstractClass*)&this->t.r.m.o);
					BuildingClass::Leave_Rubble(this);
				}
			}
			break;
		}
		case DamageState::PostMortem:
		{
			R->EAX(_res);
			return 0x442C14;
		}
		default:
			break;
		}
	}

	if (this->t.r.m.o.IsActive)
	{
		if (source && res != RESULT_NONE)
		{

			/*
			// **   If any damage occurred, then inform the house of this fact. If it is the player's
			// **   house, it might announce this fact.
			// */
			if (!this->Class->tt.ot.IsInsignificant && !this->t.r.m.o.a.vftable->t.r.m.o.Is_Vehicle_Can_Undeploy(this))
			{
				HouseClass::Attacked(this->t.House, this);
			}

			// /*
			// ** Save the type of the house that's doing the damage, so if the building burn
			// ** to death credit can still be given for the kill
			// */
			this->WhoLastHurtMe = source->r.m.o.a.vftable->t.r.m.o.a.Owner__Owning_House(source)->ID;

			// /*
			// **   When certain buildings are hit, they "snap out of it" and
			// **   return fire if they are able and allowed.
			// */
			if (this->t.r.m.Mission != MISSION_DECONSTRUCTION && !HouseClass::Is_Ally(this->t.House, source))
			{
				this->t.r.m.o.a.vftable->t.Get_Weapon((TechnoClass*)this, 0);
				if (*v45)
				{
					if (!this->t.r.m.o.a.vftable->t.Get_Weapon(&this->t, 0)->WeaponType->Bullet->IsAntiAircraft
					  && (!this->t.TarCom || !this->t.r.m.o.a.vftable->t.In_Range((TechnoClass*)this, (TechnoClass*)this->t.TarCom)))
					{
						if (source->r.m.o.a.vftable->t.r.m.o.a.Kind_Of((AbstractClass*)source) == RTTI_AIRCRAFT
						  || HouseClass::Is_Player_Control(this->t.House) && !Rule->IsSmartDefense)
						{
							// /*
							// **   Generate a random rotation effect since there is nothing else that this
							// **   building can do.
							// */
							if (!FacingClass::Is_Rotating(&this->t.PrimaryFacing) && this->t.r.m.o.a.vftable->t.mTechnoClass_activestate_701190(this))
							{
								LOWORD(v64) = 0;
								LOBYTE(v46) = 0;
								HIBYTE(v46) = Random2Class::operator()(&Scen->RandomNumber);
								v47 = v64;
								BYTE1(v47) = 0;
								a2 = (CellStruct*)(v47 | v46);
								FacingClass::Set_Desired(&this->t.PrimaryFacing, (DirStruct*)&a2);
							}
						}
						else
						{
							this->t.r.m.o.a.vftable->t.Assign_Target((ObjectClass*)this, (int)source);
						}
					}
				}
			}
		}

		if (res != RESULT_NONE)
		{
			v48 = ObjectClass::Health_Ratio(&this->t.r.m.o) <= Rule->ConditionYellow;
			if (this->IsDamaged != v48)
			{
				v49 = BANIM_UPGRADE_ONE;
				this->IsDamaged = v48;
				v50 = 0;
				a2 = (CellStruct*)this->Anims;
				do
				{
					if (*a2)
					{
						v51 = v48 ? this->Class->BuildingAnim[v50].Damaged : (char*)&this->Class->BuildingAnim[v50];
						if (v51 && *v51)
						{
							BuildingClass::Anim_Logic_0(this, v51, v49, v48, 0, 0);
						}
					}
					++v50;
					++v49;
					++a2;
				}
				while (v50 < BANIM_COUNT);
			}
		}

		if (_ShapeNum != BuildingClass::Shape_Number(this))
		{
			this->t.r.m.o.IsToDisplay = 1;
			v53 = ObjectClass::Health_Ratio(&this->t.r.m.o) <= Rule->ConditionYellow;
			if (this->IsDamaged != v53)
			{
				v54 = BANIM_UPGRADE_ONE;
				this->IsDamaged = v53;
				v55 = 0;
				a2 = (CellStruct*)this->Anims;
				do
				{
					if (*a2)
					{
						v56 = v53 ? this->Class->BuildingAnim[v55].Damaged : (char*)&this->Class->BuildingAnim[v55];
						if (v56 && *v56)
						{
							BuildingClass::Anim_Logic_0(this, v56, v54, v53, 0, 0);
						}
					}
					++v55;
					++v54;
					++a2;
				}
				while (v55 < 21);
			}
		}

		return res;
	}


	R->EAX(DamageState::NowDead);
	return 0x442C14;
}
#endif

//these are all for cleaning up when a prism tower becomes unavailable
DEFINE_HOOK(0x4424EF, BuildingClass_ReceiveDamage_PrismForward, 6)
{
	GET(FakeBuildingClass* const, pThis, ESI);
	auto pExt = pThis->_GetExtData();

	if (auto& pPrism = pExt->MyPrismForwarding)
		pPrism->RemoveFromNetwork(true);

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268, Continue = 0x0 };

	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x9C, -0x4));

	return  WarheadTypeExtContainer::Instance.Find(args.WH)->AllowDamageOnSelf ? SkipCheck : Continue;
}

DEFINE_HOOK(0x4426C8, BuildingClass_ReceiveDamage_Handle, 0xA)
{
	GET(FakeBuildingClass* const, pThis, ESI);
	GET_STACK(CellStruct*, pFoundationArray, 0x10);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x9C, -0x4));

	if (!pThis->_GetTypeExtData()->DisableDamageSound && pThis->Type->DamageSound == -1)
	{
		VocClass::PlayIndexAtPos(RulesClass::Instance->BuildingDamageSound, pThis->Location, nullptr);
	}

	if (args.WH->Sparky)
	{

		auto const pTypeExt = pThis->_GetTypeExtData();
		auto const pBldExt = pThis->_GetExtData();
		const bool Onfire = pTypeExt->HealthOnfire.Get(pThis->GetHealthStatus());
		auto const pFireType = pTypeExt->OnFireTypes.GetElements(RulesClass::Instance->OnFire);

		if (Onfire && pFireType.size() >= 3)
		{

			if (Unsorted::CurrentFrame < pBldExt->LastFlameSpawnFrame + RulesExtData::Instance()->BuildingFlameSpawnBlockFrames)
				return 0x4428FE;

			pBldExt->LastFlameSpawnFrame = Unsorted::CurrentFrame;
			const auto rand_ = pThis->Type->GetFoundationWidth() + pThis->Type->GetFoundationHeight(false) + 5;

			for (; (pFoundationArray->X != 0x7FFF || pFoundationArray->Y != 0x7FFF); ++pFoundationArray)
			{
				auto const& [nCellX, nCellY] = pThis->InlineMapCoords() + *pFoundationArray;
				CoordStruct nDestCoord { (nCellX * 256) + 128, (nCellY * 256) + 128, 0 };
				nDestCoord.Z = MapClass::Instance->GetCellFloorHeight(nDestCoord);

				auto PlayFireAnim = [&](int nLoop = 1, int nFireTypeAt = 2)
					{
						if (auto pAnimType = pFireType[nFireTypeAt])
						{
							nDestCoord = MapClass::GetRandomCoordsNear(nDestCoord, 96, false);
							auto const pAnim = GameCreate<AnimClass>(pAnimType, nDestCoord, 0, nLoop);
							pAnim->SetOwnerObject(pThis);
							const auto pKiller = args.Attacker;
							const auto Invoker = (pKiller) ? pKiller->Owner : args.SourceHouse;
							AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, pKiller, false);
						}
					};

				switch (ScenarioClass::Instance->Random.RandomFromMax(rand_))
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					PlayFireAnim(ScenarioClass::Instance->Random.RandomFromMax(pFireType.size()), 0);
					break;
				case 6:
				case 7:
				case 8:
					PlayFireAnim(ScenarioClass::Instance->Random.RandomFromMax(pFireType.size()), 1);
					break;
				case 9:
					PlayFireAnim();
					break;
				default:
					break;
				}
			}
		}
	}

	return 0x4428FE;
}

DEFINE_HOOK(0x442974, BuildingClass_ReceiveDamage_Malicious, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWH, 0xA8);

	if (WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative)
		return 0x442980;

	BuildingExtContainer::Instance.Find(pThis)->ReceiveDamageWarhead = pWH;
	pThis->BuildingUnderAttack();

	return 0x442980;
}

DEFINE_HOOK(0x44227E, BuildingClass_ReceiveDamage_Nonprovocative_DonotSetLAT, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EBP);
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFSET(0x9C, 0xC));

	if (WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative)
		return 0x4422C1;

	if (!pSource || pThis->IsStrange())
		return 0x4422C1;

	auto pSourceHouse = pSource->Owner;

	if (!pSourceHouse)
	{
		Debug::Log("Building [%s - %s] Attacked by dead [%x - %s] with null owner!\n", pThis, pThis->Type->ID, pSource, pSource->GetThisClassName());
		return 0x4422C1;
	}

	pThis->Owner->LATime = Unsorted::CurrentFrame;
	pThis->Owner->LAEnemy = pSourceHouse->ArrayIndex;
	pThis->BaseIsAttacked(pSource);
	return 0x4422C1;
}

DEFINE_HOOK(0x4423E7, BuildingClass_ReceiveDamage_FSW, 5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(int* const, pDamage, 0xA0);

	if (FirewallFunctions::IsActiveFirestormWall(pThis, nullptr))
	{
		auto const pExt = RulesExtData::Instance();
		auto const& coefficient = pExt->DamageToFirestormDamageCoefficient;
		auto const amount = static_cast<int>(*pDamage * coefficient);

		if (amount > 0)
		{
			auto const index = SW_Firewall::FirewallType;

			if (auto const pSuper = pThis->Owner->FindSuperWeapon(index))
			{
				auto const left = pSuper->RechargeTimer.GetTimeLeft();
				int const reduced = MaxImpl(0, left - amount);
				pSuper->RechargeTimer.Start(reduced);
			}
		}
		return 0x4423B7;
	}

	return 0x4423F2;
}

DEFINE_HOOK(0x44266B, BuildingClass_ReceiveDamage_Destroyed, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pKiller, EBP);
	pThis->Destroyed(pKiller);
	return 0x0;
}

DEFINE_HOOK(0x442991, BuildingClass_ReceiveDamage_ReturnFire_EMPulseCannon, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->EMPulseCannon || pThis->Type->NukeSilo ? 0x442A95 : 0x0;
}

DEFINE_HOOK(0x442A08, BuildingClass_ReceiveDamage_ReturnFire, 0x5)
{
	enum { SetTarget = 0x442A34, RandomFacing = 0x442A41 };

	GET(TechnoClass*, pAttacker, EBP);
	GET(BuildingClass*, pThis, ESI);

	//Was pThis->Owner->ControlledByCurrentPlayer(), got desync ed with that
	const bool def = BuildingTypeExtContainer::Instance.Find(pThis->Type)->PlayerReturnFire.Get(
		pAttacker->WhatAmI() == AircraftClass::AbsID ||
		(pThis->Owner->IsControlledByHuman() && !RulesClass::Instance->PlayerReturnFire)
	);

	return !def ? SetTarget : RandomFacing;
}

#pragma endregion

#pragma region Foot
#include <TeamTypeClass.h>

DEFINE_HOOK(0x4D7330, FootClass_ReceiveDamage_Handle, 0x8)
{
	GET(FootClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);
	DamageState _res = DamageState::Unaffected;

	if (args.WH->Sonic)
	{
		if (auto pParasite = pThis->ParasiteEatingMe)
		{
			pParasite->ParasiteImUsing->ExitUnit();

			if (args.Attacker)
			{
				args.Attacker->SetTarget(nullptr);
			}
		}
	}

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);

	if (pThis->ParasiteEatingMe && args.Attacker != pThis->ParasiteEatingMe && *args.Damage > pThis->ParasiteEatingMe->GetTechnoType()->SuppressionThreshold)
	{
		pThis->ParasiteEatingMe->ParasiteImUsing->SuppressionTimer.Start((2 * *args.Damage) - pThis->ParasiteEatingMe->GetTechnoType()->SuppressionThreshold);
	}

	if (pThis->ParasiteEatingMe && pWHExt->RemoveParasites.Get(*args.Damage < 0))
	{
		pThis->ParasiteEatingMe->ParasiteImUsing->SuppressionTimer.Start(50);
		pThis->ParasiteEatingMe->ParasiteImUsing->ExitUnit();
	}

	_res = pThis->TechnoClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	if (_res != DamageState::PostMortem)
	{
		if ((pThis->IsSinking || (!pThis->IsAttackedByLocomotor && pThis->IsCrashing)))
		{
			_res = DamageState::PostMortem;
		}

		if (_res != DamageState::Unaffected)
		{
			if (auto pTeam = pThis->Team)
			{
				pTeam->TookDamage(pThis, _res, args.Attacker);
				R->EAX(_res);
				return 0x4D74D6;
			}
		}

		if (_res == DamageState::NowDead || _res == DamageState::Unaffected)
		{
			R->EAX(_res);
			return 0x4D74D6;
		}

		if (auto pTeam = pThis->Team)
		{
			if (pTeam->Type->Whiner && !pThis->Owner->IsControlledByHuman())
			{
				if (!args.Attacker)
				{
					R->EAX(_res);
					return 0x4D74D6;
				}

				if (!pWHExt->Nonprovocative)
					pThis->BaseIsAttacked(args.Attacker);
			}
		}

		if (args.Attacker)
		{
			auto pMission = pThis->GetCurrentMissionControl();
			if (pMission->NoThreat && !pMission->Zombie)
			{
				pThis->EnterIdleMode(false, 1);
			}
		}
	}

	R->EAX(_res);
	return 0x4D74D6;
}
#pragma endregion

#pragma region Aircraft

DEFINE_HOOK(0x4165C0, AircraftClass_ReceiveDamage_Handle, 0x7)
{
	GET(AircraftClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	const DamageState _res = pThis->FootClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	if (_res == DamageState::NowDead)
	{
		pThis->Destroyed(args.Attacker);
		if (pThis->Type->Explosion.Count > 0)
		{
			if (auto pExp = pThis->Type->Explosion
				[ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1)])
			{
				auto nCoord = pThis->GetTargetCoords();
				// if (pInvoker && !Is_House(pInvoker))
				// 	pInvoker = nullptr;

				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExp, nCoord),
					args.Attacker ? args.Attacker->Owner : (args.SourceHouse ? args.SourceHouse : nullptr),
					pThis->Owner,
					true
				);
			}
		}

		// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
		const bool bSelected = pThis->IsSelected && pThis->Owner->ControlledByCurrentPlayer();
		TechnoExt_ExtData::SpawnSurvivors(pThis,
			args.Attacker,
			bSelected,
			args.IgnoreDefenses,
			args.PreventsPassengerEscape);

		const auto& crashable = TechnoTypeExtContainer::Instance.Find(pThis->Type)->Crashable;
		if ((crashable.isset() && !crashable.Get()) || !pThis->Crash(args.Attacker))
			pThis->UnInit();

	}

	R->EAX(_res);
	return 0x4166B0;
}

#pragma endregion

#pragma region Infantry

DEFINE_HOOK(0x517FA0, InfantryClass_ReceiveDamage_Handled, 6)
{
	GET(FakeInfantryClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

	if (pThis->IsDeployed() && args.IgnoreDefenses)
	{
		*args.Damage = static_cast<int>(*args.Damage * pWarheadExt->DeployedDamage.Get(pThis));
	}
	else if (pThis->Crawling)
	{
		if (*args.Damage > 0 && !args.IgnoreDefenses)
		{
			*args.Damage = MaxImpl(((double)*args.Damage * args.WH->ProneDamage), 1);
		}
	}

	if (args.WH->InfDeath == InfDeath::Mutate && pThis->GetHeight() > 0)
	{
		*args.Damage = 0;
	}

	DamageState _res = pThis->FootClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	if (_res == DamageState::NowDead)
	{
		if (auto pEnslave = pThis->SlaveOwner)
		{
			if (auto pManager = pEnslave->SlaveManager)
			{
				pManager->Killed(pThis);
			}
		}

		auto MyTransport = pThis->Transporter;

		if (MyTransport
		  && MyTransport->WhatAmI() == UnitClass::AbsID
		  && pThis->Type->Gunner)
		{
			static_cast<UnitClass*>(MyTransport)->RemovePassenger(pThis);
		}

		pThis->Destroyed(args.Attacker);
		pThis->StopMoving();
		pThis->Stun();
		pThis->QueueMission(Mission::None, false);
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();
		pThis->KillPassengers(args.Attacker);

		if (!TechnoExtContainer::Instance.Find(pThis)->GarrisonedIn)
		{

			bool IsForcedCyborg = false;
			if (args.IgnoreDefenses)
			{
				if (pThis->Type->Cyborg)
				{
					IsForcedCyborg = true;
					if (pThis->IsABomb)
					{
						GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
					}
				}
			}

			if (pThis->GetHeight() <= 10)
			{
				if (MapClass::Instance->GetCellAt(pThis->Location)->LandType == LandType::Water)
				{
					if (pThis->IsABomb)
					{
						GameCreate<AnimClass>(RulesClass::Instance->Wake, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

						auto splash_loc = pThis->Location + CoordStruct { 0, 0, 3 };
						GameCreate<AnimClass>(RulesClass::Instance->SplashList[0], splash_loc, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
					}

					R->EAX(DamageState::NowDead);
					return Crashable(pThis, pThis->Type, args.Attacker);
				}
			}

			if (pThis->Type->Cyborg && pThis->Crawling)
			{
				GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
				R->EAX(DamageState::NowDead);
				return Crashable(pThis, pThis->Type, args.Attacker);
			}
			else
			{
				auto infDeath = args.WH->InfDeath;
				if (!pThis->Type->JumpJet)
				{
					if (pThis->SequenceAnim == DoType::Paradrop)
					{
						if (infDeath == InfDeath::Virus)
						{
							auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
							auto pInvoker = args.Attacker
								? args.Attacker->Owner
								: args.SourceHouse;

							AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true);

							if (ParticleSystemClass::Array->ValidIndex(RulesClass::Instance->InfantryVirus->SpawnsParticle))
							{
								auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
								ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
							}

							if (pThis)
							{
								pThis->_DestroyThis(1u);
							}
						}

						infDeath = InfDeath::Explode;
					}

					if (auto pAttackerBld = cast_to<BuildingClass*>(args.Attacker))
					{
						if (pAttackerBld->Type->LaserFence)
						{
							infDeath = InfDeath::Electro;
						}
					}

					if (pThis->Type->DeathAnims.Count <= 0)
					{
						if (pThis->Type->NotHuman)
						{
							bool Succeeded = false;

							if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
							{
								auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location);
								auto pInvoker = args.Attacker ? args.Attacker->GetOwningHouse() : nullptr;
								AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), args.Attacker, true, true);
								pAnim->ZAdjust = pThis->GetZAdjustment();
								Succeeded = true;
							}
							else
							{
								auto const& whSequence = pWarheadExt->NotHuman_DeathSequence;
								// Die1-Die5 sequences are offset by 10
								COMPILETIMEEVAL auto Die = [](int x) { return x + 10; };

								int resultSequence = Die(1);

								if (!whSequence.isset()
									&& TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NotHuman_RandomDeathSequence.Get())
								{
									resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));
								}
								else if (whSequence.isset())
								{
									resultSequence = std::clamp(Die(Math::abs(whSequence.Get())), Die(1), Die(5));
								}

								pThis->_GetExtData()->IsUsingDeathSequence = true;

								//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
								if (pThis->PlayAnim(static_cast<DoType>(resultSequence), true))
								{
									Succeeded = true;
								}
							}

							if (Succeeded)
							{
								if (infDeath == InfDeath::Virus)
								{
									auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true);

									if (ParticleSystemClass::Array->ValidIndex(RulesClass::Instance->InfantryVirus->SpawnsParticle))
									{
										auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
										ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
									}

									if (pThis)
									{
										pThis->_DestroyThis(1u);
									}
								}

								if (!IsForcedCyborg)
								{
									R->EAX(DamageState::NowDead);
									return 0x518D52;
								}

							}
							else
							{
								pThis->UnInit();
								R->EAX(DamageState::NowDead);
								return 0x518D52;
							}

							R->EAX(DamageState::NowDead);
							return Crashable(pThis, pThis->Type, args.Attacker);
						}
						else
						{
							AnimTypeClass* pTypeAnim = pWarheadExt->InfDeathAnim;
							for (auto begin = pWarheadExt->InfDeathAnims.begin(); begin != pWarheadExt->InfDeathAnims.end(); ++begin)
							{
								if (begin->first == pThis->Type)
								{
									pTypeAnim = begin->second;
									break;
								}
							}

							if (pTypeAnim)
							{
								auto pAnim = GameCreate<AnimClass>(pTypeAnim, pThis->Location);
								HouseClass* const Invoker = (args.Attacker)
									? args.Attacker->Owner
									: args.SourceHouse
									;

								AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, args.Attacker, false, true);
							}
							else
							{
								AnimClass* pAnim = nullptr;
								switch (infDeath)
								{
								case InfDeath::Die1:
									if (pThis->PlayAnim(DoType::Die1, true, false))
									{
										if (!IsForcedCyborg)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
									}

									break;
								case InfDeath::Die2:
									if (pThis->PlayAnim(DoType::Die2, true, false))
									{
										if (!IsForcedCyborg)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
									}

									break;
								case InfDeath::Explode:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								case InfDeath::Flames:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->FlamingInfantry, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								case InfDeath::Electro:
								{
									AnimTypeClass* El = RulesExtData::Instance()->ElectricDeath;

									if (!El)
									{
										El = AnimTypeClass::Array->Items[0];
									}

									pAnim = GameCreate<AnimClass>(El, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

								}
								break;
								case InfDeath::HeadPop:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryHeadPop, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								case InfDeath::Nuked:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryNuked, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								case InfDeath::Virus:
								{
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
								}
								break;
								case InfDeath::Mutate:
								{
									auto curLoc = pThis->Location;
									pThis->UnmarkAllOccupationBits(curLoc);
									auto pCell = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(curLoc));
									bool Hasbuilding = false;

									for (auto i = pCell->FirstObject; i; i = i->NextObject)
									{
										if (i && i->WhatAmI() == BuildingClass::AbsID)
											Hasbuilding = true;
									}

									if (GroundType::Get(pCell->LandType)->Cost[0] == 0.0 && !pThis->OnBridge)
									{
										if (!pThis->PlayAnim(DoType::Die2, true, false))
											break;

										if (!IsForcedCyborg)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
									}

									CoordStruct closest {};
									pCell->FindInfantrySubposition(&closest, curLoc, false, false, false);

									if (!closest.IsValid())
									{
										if (!pThis->PlayAnim(DoType::Die2, true, false))
											break;

										if (!IsForcedCyborg)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
									}

									if (Hasbuilding)
									{
										if (!pThis->PlayAnim(DoType::Die2, true, false))
											break;

										if (!IsForcedCyborg)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
									}

									pThis->MarkAllOccupationBits(curLoc);
									auto pAnim_Mutate = GameCreate<AnimClass>(RulesClass::Instance->InfantryMutate, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									AnimExtData::SetAnimOwnerHouseKind(pAnim_Mutate, pInvoker, pThis->Owner, args.Attacker, true);

									pAnim_Mutate->MarkAllOccupationBits(pThis->Location);
								}
								break;
								case InfDeath::Brute:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryBrute, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								default:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								}

								if (pAnim)
								{
									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									const auto& [bChanged, result] = AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true);

									if (infDeath == InfDeath::Mutate && bChanged && result != OwnerHouseKind::Default)
									{
										pAnim->LightConvert = nullptr;
									}
								}
							}

							R->EAX(DamageState::NowDead);
							return Crashable(pThis, pThis->Type, args.Attacker);
						}
					}
					else
					{
						int infDeathInt = (int)infDeath;
						if (infDeathInt < 0
							|| infDeathInt >= pThis->Type->DeathAnims.Count
							|| !pThis->Type->DeathAnims[infDeathInt])
						{
							infDeathInt = 0;
						}

						if (auto pDeathAnim = pThis->Type->DeathAnims[infDeathInt])
						{
							auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

							HouseClass* Invoker = (args.Attacker)
								? args.Attacker->Owner
								: args.SourceHouse
								;

							AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, args.Attacker, false, true);

						}
					}
				}
			}

			GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			R->EAX(DamageState::NowDead);
			return Crashable(pThis, pThis->Type, args.Attacker);
		}


		if (!pThis->Owner->IsControlledByHuman()
		  && pThis->Type->Engineer
		  && (pThis->GetCurrentMission() == Mission::Guard
			  || pThis->GetCurrentMission() == Mission::Area_Guard))
		{
			pThis->QueueMission(Mission::Hunt, false);
		}

		CoordStruct target_ = args.Attacker ? args.Attacker->Location : CoordStruct::Empty;
		pThis->Scatter(target_, false, false);

		if (args.Attacker && pThis->PanicDurationLeft < 100)
		{
			if (pThis->Type->Fraidycat)
			{
				pThis->PanicDurationLeft = 300;
				R->EAX(_res);
				return 0x518D52;
			}

			if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless))
			{
				int PanixMax = 100;
				if (pThis->Type->Doggie && pThis->IsRedHP())
				{
					PanixMax = RulesExtData::Instance()->DoggiePanicMax;
				}

				pThis->PanicDurationLeft = PanixMax;
				R->EAX(_res);
				return 0x518D52;
			}
		}
		else if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless))
		{
			int morefear = 50;
			auto _HPPercent = pThis->GetHealthPercentage();

			if (_HPPercent > RulesClass::Instance->ConditionRed)
			{
				morefear = 25;
			}

			if (_HPPercent > RulesClass::Instance->ConditionYellow)
			{
				morefear /= 2;
			}

			int v66 = pThis->PanicDurationLeft + morefear;

			if (v66 >= 300)
			{
				v66 = 300;
			}

			pThis->PanicDurationLeft = v66;
		}
	}

	R->EAX(_res);
	return 0x518D52;
}

#pragma endregion

#pragma region Unit

DEFINE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x44, -0xC));
	return !WarheadTypeExtContainer::Instance.Find(pWH)->Malicious || WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative ? 0x738535u : 0u;
}

DEFINE_HOOK(0x744745, UnitClass_RegisterDestruction_Trigger, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis && pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag)
		{
			pTag->RaiseEvent((TriggerEvent)AresTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x737F6D, UnitClass_ReceiveDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x44, -0x4));

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	R->ECX(R->ESI());
	pExt->ReceiveDamage = true;
	AnimTypeExtData::ProcessDestroyAnims(pThis, args.Attacker, args.WH);
	pThis->Destroy();
	return 0x737F74;
}

DEFINE_HOOK(0x738801, UnitClass_Destroy_DestroyAnim, 0x6) //was C
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExtContainer::Instance.Find(pThis);

	if (!Extension->ReceiveDamage)
	{
		AnimTypeExtData::ProcessDestroyAnims(pThis);
	}

	return 0x73887E;
}

// #895584: ships not taking damage when repaired in a shipyard. bug
// was that the logic that prevented units from being damaged when
// exiting a war factory applied here, too. added the Naval check.
DEFINE_HOOK(0x737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
{
	GET(BuildingTypeClass*, pType, ECX);
	return (pType->WeaponsFactory && !pType->Naval)
		? 0x737CEE : 0x737D31;
}

DEFINE_HOOK(0x737F97, UnitClass_ReceiveDamage_Survivours, 0xA)
{
	//GET(UnitTypeClass*, pType, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFSET(0x44, 0x18));

	if (pThis && pThis->Passengers.NumPassengers > 0)
	{
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
		{
			auto pPassenger = pThis->Passengers.GetFirstPassenger();
			auto pExt = TechnoExtContainer::Instance.Find(pPassenger);

			if (pExt->OriginalPassengerOwner)
				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);

			while (pPassenger->NextObject)
			{
				pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
				pExt = TechnoExtContainer::Instance.Find(pPassenger);

				if (pExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
			}
		}
	}

	TechnoExt_ExtData::SpawnSurvivors(pThis, pKiller, select, ignoreDefenses, preventPassangersEscape);

	R->EBX(-1);
	return 0x73838A;
}

DEFINE_HOOK(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(DamageState const, result, EAX);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (result == DamageState::NowDead)
	{

		if (pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
			pThis->IsAttackedByLocomotor = false;

		//this cause desync ?
		if (!pThis->Type->Voxel && pThis->Type->Strength > 0)
		{
			if (pThis->Type->MaxDeathCounter > 0
				&& !pThis->InLimbo
				&& !pThis->IsCrashing
				&& !pThis->IsSinking
				&& !pThis->TemporalTargetingMe
				&& !pThis->IsInAir()
				&& pThis->DeathFrameCounter <= 0
				)
			{

				pThis->Stun();
				const auto loco = pThis->Locomotor.GetInterfacePtr();

				if (loco->Is_Moving_Now())
					loco->Stop_Moving();

				pThis->DeathFrameCounter = 1;
			}
		}
	}

	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
	{
		R->EAX(DamageState::PostMortem);
	}

	return 0;
}

DEFINE_HOOK(0x737CBB, UnitClass_ReceiveDamage_DeathCounter, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->DeathFrameCounter > 0)
	{
		return 0x737D26;
	}

	return 0x0;
}

#pragma endregion

#pragma region Veinhole

#ifdef _TODO

DamageResultType __thiscall VeinholeMonsterClass::Take_Damage(
		VeinholeMonsterClass* this,
		signed int* damage,
		int distance,
		WarheadTypeClass* warhead,
		TechnoClass* source,
		bool forced,
		int a7,
		int house)
{
	DamageResultType res; // eax MAPDST
	__int32 v11; // eax
	FrameTimerClass* p_timer2; // esi
	DamageResultType result; // eax
	int v14; // [esp+Ch] [ebp-8h]

	res = ObjectClass::Take_Damage(&this->o, damage, distance, warhead, source, forced, a7, house);
	if (res == RESULT_NONE)
	{
		return res;
	}
	v11 = res - 4;
	if (!v11)     // RESULT_DESTROYED
	{
		this->DamageState = 3;
		Detach_This_From_All(this, 0);
		return res;
	}
	if (v11 == 1)
	{
		return _RESULT_POST_MORTEM;
	}
	this->DamageState = 2;
	p_timer2 = &this->___timer2;
	p_timer2->Started = Frame;
	result = res;
	p_timer2->Timer = v14;
	p_timer2->Accumulated_DelayTime = 120;
	return result;
}
#endif

#pragma endregion

DEFINE_HOOK(0x718B29, LocomotionClass_SomethingWrong_ReceiveDamage_UseCurrentHP, 0x6)
{
	GET(FootClass* const, pLinked, ECX);
	R->ECX(pLinked->GetType()->Strength);
	return R->Origin() + 0x6;
}

#pragma region placeholder

//DEFINE_HOOK(0x518313, InfantryClass_ReceiveDamage_JumpjetExplode, 0x6)
//{
//	enum { NonJumpJet = 0x518362 ,  ContinueChecks = 0x51831D , SkipPlayExplode = 0x5185F1 };
//	GET(InfantryClass*, pThis, ESI);
//	GET(InfantryTypeClass*, pThisType, EAX);
//
//	if (pThisType->JumpJet) {
//		return pThisType->Explodes || pThis->HasAbility(AbilityType::Explodes)
//			? ContinueChecks : SkipPlayExplode;
//		return ContinueChecks;
//	}
//
//	return NonJumpJet;
//}

// DEFINE_HOOK(0x5184F7, InfantryClass_ReceiveDamage_NotHuman, 0x6)
// {
// 	enum
// 	{
// 		Delete = 0x518619,
// 		DoOtherAffects = 0x518515,
// 		IsHuman = 0x5185C8,
// 		CheckAndReturnDamageResultDestroyed = 0x5185F1,
// 		PlayInfDeaths = 0x5185CE
// 	};
//
// 	GET(InfantryClass* const, pThis, ESI);
// 	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xD0, -0x4));
// 	GET(DWORD, InfDeath, EDI);
//
// 	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);
//
// 	if (!pThis->Type->NotHuman)
// 	{
// 		--InfDeath;
// 		R->EDI(InfDeath);
//
// 		bool Handled = false;
// 		if (pThis->GetHeight() < 10)
// 		{
// 			AnimTypeClass* pTypeAnim = pWarheadExt->InfDeathAnim;
// 			for(auto begin = pWarheadExt->InfDeathAnims.begin(); begin != pWarheadExt->InfDeathAnims.end(); ++begin) {
// 				if(begin->first == pThis->Type){
// 					pTypeAnim = begin->second;
// 					break;
// 				}
// 			}
//
// 			if(pTypeAnim){
// 				auto pAnim = GameCreate<AnimClass>(pTypeAnim, pThis->Location);
// 				HouseClass* const Invoker = (args.Attacker)
// 					? args.Attacker->Owner
// 					: args.SourceHouse
// 					;
//
// 				AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, args.Attacker, false , true);
// 				Handled = true;
// 			}
// 		}
//
// 		return (Handled || InfDeath >= 10)
// 			? CheckAndReturnDamageResultDestroyed
// 			: PlayInfDeaths
// 			;
//
// 		//return IsHuman;
// 	}
//
// 	R->ECX(pThis);
//
// 	if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
// 	{
// 		auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location);
// 		auto pInvoker = args.Attacker ? args.Attacker->GetOwningHouse() : nullptr;
// 		AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), args.Attacker, true, true);
// 		pAnim->ZAdjust = pThis->GetZAdjustment();
// 	}
// 	else
// 	{
// 		auto const& whSequence = pWarheadExt->NotHuman_DeathSequence;
// 		// Die1-Die5 sequences are offset by 10
// 		COMPILETIMEEVAL auto Die = [](int x) { return x + 10; };
//
// 		int resultSequence = Die(1);
//
// 		if (!whSequence.isset()
// 			&& TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NotHuman_RandomDeathSequence.Get())
// 		{
// 			resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));
// 		}
// 		else if (whSequence.isset())
// 		{
// 			resultSequence = std::clamp(Die(Math::abs(whSequence.Get())), Die(1), Die(5));
// 		}
//
// 		InfantryExtContainer::Instance.Find(pThis)->IsUsingDeathSequence = true;
//
// 		//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
// 		if (pThis->PlayAnim(static_cast<DoType>(resultSequence), true))
// 		{
// 			return DoOtherAffects;
// 		}
// 	}
//
// 	return Delete;
// }


/*
DEFINE_HOOK(0x442243, BuildingClass_ReceiveDamage_AddEarly, 0xA)
{
	R->Stack(STACK_OFFS(0x9C, 0x6C), DamageState::Unaffected);

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EBP);

	if (pThis == pSource && !pSource->GetTechnoType()->DamageSelf) {
		return 0x442C06;
	}

	return 0x442268;
}*/
// DEFINE_HOOK(0x51849A, InfantryClass_ReceiveDamage_DeathAnim, 5)
// {
// 	GET(InfantryClass*, I, ESI);
// 	LEA_STACK(args_ReceiveDamage*, Arguments, 0xD4);
// 	GET(DWORD, InfDeath, EDI);
//
// 	// if you got here, a valid DeathAnim for this InfDeath has been defined, and the game has already checked the preconditions
// 	// just allocate the anim and set its owner/remap
//
// 	AnimClass* Anim = GameCreate<AnimClass>(I->Type->DeathAnims[InfDeath], I->Location);
//
// 	HouseClass* Invoker = (Arguments->Attacker)
// 		? Arguments->Attacker->Owner
// 		: Arguments->SourceHouse
// 		;
//
// 	AnimExtData::SetAnimOwnerHouseKind(Anim, Invoker, I->Owner, Arguments->Attacker, false, true);
//
// 	R->EAX<AnimClass*>(Anim);
// 	return 0x5184F2;
// }

// DEFINE_HOOK_AGAIN(0x518575, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
// DEFINE_HOOK(0x5183DE, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	GET(AnimClass*, pAnim, EDI);
// 	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));
//
// 	// Rules->InfantryVirus animation has been created. set the owner and color.
//
// 	auto pInvoker = Arguments.Attacker
// 		? Arguments.Attacker->Owner
// 		: Arguments.SourceHouse;
//
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, true);
//
// 	// bonus: don't require SpawnsParticle to be present
//
// 	if (ParticleSystemClass::Array->ValidIndex(pAnim->Type->SpawnsParticle))
// 	{
// 		return 0;
// 	}
//
// 	return (R->Origin() == 0x5183DE) ? 0x518422 : 0x5185B9;
// }

// DEFINE_HOOK_AGAIN(0x518B93, InfantryClass_ReceiveDamage_Anims, 5) // InfantryBrute
// DEFINE_HOOK_AGAIN(0x518821, InfantryClass_ReceiveDamage_Anims, 5) // InfantryNuked
// DEFINE_HOOK_AGAIN(0x5187BB, InfantryClass_ReceiveDamage_Anims, 5) // InfantryHeadPop
// DEFINE_HOOK_AGAIN(0x518755, InfantryClass_ReceiveDamage_Anims, 5) // InfantryElectrocuted
// DEFINE_HOOK_AGAIN(0x5186F2, InfantryClass_ReceiveDamage_Anims, 5) // FlamingInfantry
// DEFINE_HOOK(0x518698, InfantryClass_ReceiveDamage_Anims, 5) // InfantryExplode
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	GET(AnimClass*, pAnim, EAX);
// 	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));
//
// 	// animation has been created. set the owner and color.
// 	const auto pInvoker = Arguments.Attacker
// 		? Arguments.Attacker->Owner
// 		: Arguments.SourceHouse;
//
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);
//
// 	return 0x5185F1;
// }

// DEFINE_HOOK(0x51887B, InfantryClass_ReceiveDamage_InfantryVirus2, 0xA)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	GET(AnimClass*, pAnim, EAX);
// 	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));
//
// 	// Rules->InfantryVirus animation has been created. set the owner, but
// 	auto pInvoker = Arguments.Attacker
// 		? Arguments.Attacker->Owner
// 		: Arguments.SourceHouse;
//
// 	const auto& [bChanged, result] =
// 		AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);
//
// 	// reset the color for default (invoker).
// 	if (bChanged && result != OwnerHouseKind::Default)
// 	{
// 		pAnim->LightConvert = nullptr;
// 	}
//
// 	return 0x5185F1;
// }
//
// DEFINE_HOOK(0x518A96, InfantryClass_ReceiveDamage_InfantryMutate, 7)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	GET(AnimClass*, pAnim, EDI);
// 	REF_STACK(args_ReceiveDamage, Arguments, STACK_OFFS(0xD0, -0x4));
//
// 	// Rules->InfantryMutate animation has been created. set the owner and color.
// 	auto pInvoker = Arguments.Attacker
// 		? Arguments.Attacker->Owner
// 		: Arguments.SourceHouse;
//
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, Arguments.Attacker, false, true);
//
// 	return 0x518AFF;
// }

//DEFINE_HOOK(0x489A01, MapClass_DamageArea_LoopDamageGroups, 0x6)
//{
//	enum { AdvanceLoop = 0x489AC1 , SetStack1FTrue = 0x489ABC , Continue = 0x0};
//
//	GET(ObjectClass*, pTarget, ESI);
//	GET(int , nDistance , EDI);
//	GET_BASE(TechnoClass*, pSource, 0x8);
//	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);
//	GET_BASE(HouseClass*, pHouse, 0x14);
//	GET_STACK(bool , IsIroncurtained , 0x17);
//	GET_STACK(int , cellSpread , 0x68);
//	GET_STACK(int , damage , 0x24);
//
//
//	char dummy[0x70];
//	if(pSource) {
//		IMPL_SNPRNINTF(dummy,sizeof(dummy),"%x - %s" , (size_t)pSource , pSource->get_ID());
//	}else{
//		std::strcpy(dummy,"Unknown");
//	}
//
//	if(!pTarget->IsAlive)
//		return AdvanceLoop;
//
//	const auto pWhat = pTarget->WhatAmI();
//
//	if(pWhat == BulletClass::AbsID)
//		Debug::Log("Bullet[%x - %s] Getting hit by [%s] Warhead [%s] !\n" , pTarget,pTarget->get_ID() ,dummy , pWarhead->ID);
//
//	{
//
//		if(pWhat == BuildingClass::AbsID && static_cast<BuildingClass*>(pTarget)->Type->InvisibleInGame)
//			return AdvanceLoop;
//
//		if(!IsIroncurtained || pTarget->IsIronCurtained())
//		{
//			if(pWhat == AircraftClass::AbsID && pTarget->IsInAir())
//				nDistance /= 2;
//
//			if(pTarget->Health > 0 && pTarget->IsOnMap && !pTarget->InLimbo && nDistance <= cellSpread)
//			{
//				pTarget->ReceiveDamage(&damage ,nDistance ,pWarhead ,pSource , false ,false ,pHouse);
//				return SetStack1FTrue;
//			}
//		}
//	}
//
//	return AdvanceLoop;
//	//return Continue;
//}

//DEFINE_HOOK(0x4D7431, FootClass_TakeDamage_ProbeResult, 0x5)
//{
//	GET(DamageState, result, EAX);
//	GET(WarheadTypeClass*, pWH, EBP);
//	GET(FootClass*, pThis, ESI);
//
//	if (IS_SAME_STR_("EradiationWH", pWH->ID) && IS_SAME_STR_("PENTGENX", pThis->get_ID())) {
//		Debug::Log("Affected [%d] by[%s]\n", (int)result, pWH->ID);
//	}
//
//	return 0x0;
//}

// Has to be done here, before Ares survivor hook to take effect.
//DEFINE_HOOK(0x737F80, TechnoClass_ReceiveDamage_Cargo_SyncOwner, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis && pThis->Passengers.NumPassengers > 0)
//	{
//		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
//
//		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
//		{
//			auto pPassenger = pThis->Passengers.GetFirstPassenger();
//			auto pExt = TechnoExtContainer::Instance.Find(pPassenger);
//
//			if (pExt->OriginalPassengerOwner)
//				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
//
//			while (pPassenger->NextObject)
//			{
//				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
//				pExt = TechnoExtContainer::Instance.Find(pPassenger);
//
//				if (pExt->OriginalPassengerOwner)
//					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
//			}
//		}
//	}
//
//	return 0;
//}

/*
		to do :
		- idle anim
		- Re-Draw function , to show damaged part

DEFINE_HOOK(0x71B98B, TerrainClass_ReceiveDamage_Add, 0x7)
{
	enum { PostMortemReturn = 0x71B994, CheckNowDead = 0x71B9A7, SetReturn = 0x71BB79 };

	GET(DamageState, nState, EAX);
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x3C, -0x4));

	R->EAX(nState);
	R->Stack(0x10, nState);

	// ignite this terrain object

	if (!pThis->IsBurning && *args.Damage > 0 && args.WH->Sparky)
	{
		const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

		if (!pWarheadExt->Flammability.isset() || ScenarioClass::Instance->Random.PercentChance
		   (Math::abs(pWarheadExt->Flammability.Get())))
			pThis->Ignite();
	}

	//return handle !
	if (nState == DamageState::PostMortem)
		return PostMortemReturn;
	if (nState == DamageState::NowDead)
		return CheckNowDead;

	return SetReturn;
}*/

//DEFINE_HOOK(0x4D7330, FootClass_ReceiveDamage_probe, 0x8) {
//	GET(FootClass*, pThis, ECX);
//
//	auto id = pThis->get_ID();
//	if (IS_SAME_STR_("MDUMMY7", id))
//		Debug::Log(__FUNCTION__" Executed\n");
//
//	return 0;
//}

//DEFINE_HOOK(0x737C90 , UnitClass_TakeDamage_probe ,0x5)
//{
//	GET(UnitClass*, pThis, ECX);
//	REF_STACK(args_ReceiveDamage, args, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (IS_SAME_STR_(pThis->Type->ID, "MDUMMY7"))
//		Debug::Log("%s [%s]DamageResult %d , HP %d/%d Called %x.\n", pThis->Type->ID, args.WH->ID, *args.Damage, pThis->Health, pThis->Type->Strength , caller);
//
//	return 0x0;
//}


// DEFINE_HOOK(0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6)
// {
// 	GET(TechnoClass* const, pRadio, EDI);
//
// 	pRadio->ReceiveDamage(&pRadio->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, true, nullptr);
//
// 	return 0x4425F4;
// }

// DEFINE_HOOK(0x70253F, TechnoClass_ReceiveDamage_Metallic_AnimDebris, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(AnimClass*, pAnim, EDI);
// 	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x30));
// 	GET(int, nIdx, EAX);
// 	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));
//
// 	//well , the owner dies , so taking Invoker is not nessesary here ,..
// 	pAnim->AnimClass::AnimClass(RulesClass::Instance->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
// 	pThis->GetOwningHouse(), false);
//
// 	return 0x70256B;
// }
//
// DEFINE_HOOK(0x702484, TechnoClass_ReceiveDamage_AnimDebris, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(TechnoTypeClass* const, pType, EAX);
// 	GET(AnimClass*, pAnim, EBX);
// 	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x3C));
// 	GET(int, nIdx, EDI);
// 	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));
//
// 	//well , the owner dies , so taking Invoker is not nessesary here ,..
// 	pAnim->AnimClass::AnimClass(pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim,
// 		args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
// 		pThis->GetOwningHouse(),
// 		false
// 	);
//
// 	return 0x7024AF;
// }

//ObjectClass TakeDamage , 5F559C
//UnitClass TakeDamage , 737F0E

// DEFINE_HOOK(0x518744, InfantryClass_ReceiveDamage_ElectricDeath, 6)
// {
// 	AnimTypeClass* El = RulesExtData::Instance()->ElectricDeath;
//
// 	if (!El) {
// 		El = AnimTypeClass::Array->Items[0];
// 	}
//
// 	R->EDX(El);
// 	return 0x51874D;
// }


// DEFINE_HOOK(0x518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	//GET_STACK(ObjectClass *, pAttacker, 0xE0);
//
// 	// there is not InfantryExt ExtMap yet!
// 	// too much space would get wasted since there is only four bytes worth of data we need to store per object
// 	// so those four bytes get stashed in Techno Map instead. they will get their own map if there's ever enough data to warrant it
//
// 	return TechnoExtContainer::Instance.Find(pThis)->GarrisonedIn ? 0x5185F1 : 0;
// }

#pragma endregion