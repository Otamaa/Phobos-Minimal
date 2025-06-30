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

#include <Misc/PhobosGlobal.h>

static DWORD Crashable(FootClass* pThis, TechnoTypeClass* pType, ObjectClass* pKiller)
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
	//const auto pWH = args->WH;
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
		//const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);
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

ASMJIT_PATCH(0x71B920, TerrainClass_ReceiveDamage_Handled, 7)
{
	GET(TerrainClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;
	auto pWH = args.WH;

	if (pWH->Wood && !pThis->Type->Immune)
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
					pFire.release();
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
				VocClass::SafeImmedietelyPlayAt(pTerrainExt->CrumblingSound, &pThis->GetCoords());
				pThis->Mark(MarkType::Redraw);
				pThis->Disappear(true);
				return 0x71BB79;
			}

			auto const nCoords = pThis->GetCenterCoords();
			VocClass::SafeImmedietelyPlayAt(pTerrainExt->DestroySound, &nCoords);
			const auto pAttackerHoue = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

			if (auto const pAnimType = pTerrainExt->DestroyAnim)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, nCoords),
					args.SourceHouse,
					pThis->GetOwningHouse(),
					args.Attacker,
					false, false
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

	R->EAX(_res);
	return 0x71BB84;
}

#pragma endregion

#pragma region Object

ASMJIT_PATCH(0x5F5390, ObjectClass_ReveiveDamage_Handled, 0x5)
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

	if (!*args.Damage || (!args.IgnoreDefenses && pObjType->Immune))
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
		if (*args.Damage == 0 && !pBld->Type->CanC4 && !pBldTypeExt->CanC4_AllowZeroDamage)
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

		int v15 = int(maxstrength * RulesClass::Instance->ConditionRed);
		if (oldstrength > v15 && (oldstrength - *args.Damage) < v15)
		{
			_res = DamageState::NowRed;
		}

		int adjust = oldstrength - *args.Damage;
		pThis->Health = adjust;
		auto pInf = cast_to<InfantryClass*, false>(pThis);

		if (adjust > 0 || !pInf || args.IgnoreDefenses || !pInf->Type->Cyborg || pInf->Crawling)
		{
			_res = _res;
		}
		else
		{

			auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			auto pInvoker = args.Attacker
				? args.Attacker->Owner
				: args.SourceHouse;

			AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pInf->Owner, args.Attacker, true, false);

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

						PhobosGlobal::Instance()->Disappear_removed = true;
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
							pThis->Mark(MarkType::Redraw);
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

#include <Utilities/DebrisSpawners.h>

ASMJIT_PATCH(0x701900, TechnoClass_ReceiveDamage_Handle, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;
	//int damage_ = *args.Damage;
	bool _isNegativeDamage = *args.Damage < 0;
	auto pType = pThis->GetTechnoType();
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	pWHExt->ApplyDamageMult(pThis, &args);
	applyCombatAlert(pThis, &args);

	if (pWHExt->CanTargetHouse(args.SourceHouse, pThis))
		pExt->LastHurtFrame = Unsorted::CurrentFrame;

	if (args.Attacker && (!args.Attacker->IsAlive || args.Attacker->Health <= 0) && !args.Attacker->Owner)
		args.Attacker = nullptr; //clean up;

	if (!pThis || !pThis->IsAlive || pThis->Health <= 0)
	{
		R->EAX(DamageState::NowDead);
		return 0x702D1F;
	}

	const bool unkillable =
		!pWHExt->CanKill || pExt->AE.Unkillable;

	if (!args.IgnoreDefenses)
	{
		if (auto pShieldData = pExt->GetShield())
		{
			pShieldData->OnReceiveDamage(&args);
		}
	}

	if (!args.IgnoreDefenses && *args.Damage >= 0)
	{
		*args.Damage = (int)TechnoExtData::GetArmorMult(pThis, (double)(*args.Damage), args.WH);

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

		if (args.Attacker && pType->TypeImmune)
		{
			auto pAttackerType = args.Attacker->GetTechnoType();
			if (pType == pAttackerType && pThis->Owner == args.Attacker->Owner)
			{
				R->EAX(DamageState::Unaffected);
				return 0x702D1F;
			}
		}
	}

	if (pThis->IsIronCurtained() && !args.IgnoreDefenses && !_isNegativeDamage)
	{
		if (!(pThis->ProtectType == ProtectTypes::ForceShield ? pWHExt->PenetratesForceShield.Get(pWHExt->PenetratesIronCurtain) : pWHExt->PenetratesIronCurtain))
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

	if ((pThis->IsBeingWarpedOut() && !args.IgnoreDefenses && TechnoExtData::IsChronoDelayDamageImmune(flag_cast_to<FootClass*, false>(pThis))))
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

	const auto pSourceHouse = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

	if (!pWHExt->CanAffectHouse(pThis->Owner, pSourceHouse))
	{
		*args.Damage = 0;
		R->EAX(DamageState::Unaffected);
		return 0x702D1F;
	}

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

	// Check if the warhead can not kill targets
	//we dont want to kill the thing , dont return
	bool isActuallyAffected = false;
	if (!args.IgnoreDefenses && pThis->Health > 0 && unkillable && args.Damage != 0 && *args.Damage >= pThis->Health)
	{
		*args.Damage = 0;
		pThis->Health = 1;
		pThis->EstimatedHealth = 1;
		isActuallyAffected = true;
	}
	_res = pThis->ObjectClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	const bool Show = Phobos::Otamaa::IsAdmin || *args.Damage;

	if (Phobos::Debug_DisplayDamageNumbers && Show)
		FlyingStrings::DisplayDamageNumberString(*args.Damage, DamageDisplayType::Regular, pThis->GetRenderCoords(), TechnoExtContainer::Instance.Find(pThis)->DamageNumberOffset);

	if (!pThis->Health)
	{
		int nSelectedPowerup = -1;

		if (pExt->DropCrate >= 0 && pExt->DropCrate == 1)
		{
			nSelectedPowerup = static_cast<int>(pExt->DropCrateType);
		}
		else if (pTypeExt->DropCrate.isset())
		{
			nSelectedPowerup = pTypeExt->DropCrate.Get();
		}

		if (nSelectedPowerup >= 0) {
			TechnoExtData::TryToCreateCrate(pThis->Location, static_cast<PowerupEffects>(nSelectedPowerup));
		}
	}

	GiftBoxFunctional::TakeDamage(pExt, pTypeExt, args.WH, _res);

	if (args.Attacker && !pWHExt->Nonprovocative)
	{
		pThis->Owner->UpdateAngerNodes((int)(pType->GetCost() * ((double)*args.Damage / pType->Strength)), args.SourceHouse);
	}

	if (_res != DamageState::PostMortem && !pThis->IsAlive)
	{
		R->EAX(DamageState::NowDead);
		return 0x702D1F;
	}

	if (_res == DamageState::PostMortem)
	{
		R->EAX(DamageState::PostMortem);
		return 0x702D1F;
	}

	if (_res != DamageState::NowDead && _res != DamageState::Unaffected)
	{
		pThis->RadarFlashTimer.Start(RulesClass::Instance->RadarCombatFlashTime);
		auto pBld = cast_to<BuildingClass*, false>(pThis);

		if (args.WH->CausesDelayKill && pBld && pBld->Type->EligibleForDelayKill)
		{
			const int v22 = (int)(((double)args.WH->DelayKillAtMax * (double)args.WH->DelayKillFrames - (double)args.WH->DelayKillFrames)
				 / (double)((int)args.WH->CellSpread << 8)
				 * (double)args.DistanceToEpicenter
				 + (double)args.WH->DelayKillFrames);

			if (pBld->IsGoingToBlow)
			{
				int Started = pBld->GoingToBlowTimer.StartTime;
				int DelayTime = pBld->GoingToBlowTimer.TimeLeft;

				if (Started != -1)
				{

					if (Unsorted::CurrentFrame - Started >= DelayTime)
						DelayTime = 0;
					else
						DelayTime -= Unsorted::CurrentFrame - Started;

				}

				if ((int)v22 < DelayTime)
				{
					pBld->IsGoingToBlow = 1;
					pBld->GoingToBlowTimer.Start(v22);
				}
			}
			else
			{
				pBld->IsGoingToBlow = 1;
				pBld->GoingToBlowTimer.Start(v22);
			}

			_res = DamageState::PostMortem;
		}

		if (pType->CanDisguise && !pType->PermaDisguise )
		{

			if (pThis->IsDisguised()) {
				pThis->ClearDisguise();
			}

			pThis->InfantryBlinkTimer.Start(2 * *args.Damage);
		}
	}

	if (!pThis->Health) {
		_res = DamageState::NowDead;
	}

	switch (_res)
	{
	case DamageState::Unaffected:
	case DamageState::NowRed:
		break;
	case DamageState::Unchanged:
	{
		if (pType->DamageSound != -1) {
			VocClass::SafeImmedietelyPlayAt(pType->DamageSound, &pThis->Location, 0);
		}

		if (!pWHExt->Malicious && args.Attacker && args.Attacker->IsAlive && !pWHExt->Nonprovocative) {
			if ((pType->ToProtect || pThis->__ProtectMe_3CF) && !pThis->Owner->IsControlledByHuman()) {
				pThis->BaseIsAttacked(args.Attacker);
			}
		}

		break;
	}
	case DamageState::NowYellow:
	{
		if (pType->VoiceFeedback.Count > 0
			&& Random2Class::NonCriticalRandomNumber->RandomRanged(0, 99) < 30
			 && pThis->Owner->ControlledByCurrentPlayer())
		{
			const int feedbackIndex = pType->VoiceFeedback.Count > 1 ? Random2Class::NonCriticalRandomNumber->RandomRanged(0, pType->VoiceFeedback.Count - 1) : 0;
			VocClass::SafeImmedietelyPlayAt(pType->VoiceFeedback.Items[feedbackIndex], &pThis->Location, 0);
		}

		break;
	}
	case DamageState::NowDead:
	{
		if (pWHExt->Supress_LostEva.Get())
			pExt->SupressEVALost = true;

		GiftBoxFunctional::Destroy(pExt, pTypeExt);

		if(!pExt->PhobosAE.empty()){
			std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons {};
			std::set<PhobosAttachEffectTypeClass*> cumulativeTypes {};

			for (auto const& attachEffect : pExt->PhobosAE) {

				auto const pAEType = attachEffect->GetType();

				if (pAEType->ExpireWeapon && (pAEType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Death) != ExpireWeaponCondition::None)
				{
					if (!pAEType->Cumulative || !pAEType->ExpireWeapon_CumulativeOnlyOnce || !cumulativeTypes.contains(pAEType))
					{
						if (pAEType->Cumulative && pAEType->ExpireWeapon_CumulativeOnlyOnce)
							cumulativeTypes.insert(pAEType);

						if (pAEType->ExpireWeapon_UseInvokerAsOwner)
						{
							if (auto const pInvoker = attachEffect->GetInvoker())
								expireWeapons.emplace_back(pAEType->ExpireWeapon, pInvoker);
						}
						else
						{
							expireWeapons.emplace_back(pAEType->ExpireWeapon, pThis);
						}
					}
				}
			}

			PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons);
		}


		if (!pThis->IsAlive)
			break;

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
				VocClass::SafeImmedietelyPlayAt(nSound, &pThis->Location);
			}
			else
			{
				VocClass::SafeImmedietelyPlayAt(pType->VoiceDie[pType->VoiceDie.Count == 1 ? 0 : Random2Class::NonCriticalRandomNumber->RandomFromMax(pType->VoiceDie.Count - 1)], &pThis->Location);
			}
		}

		if (pType->DieSound.Count > 0)
		{
			auto const& nSound = pWHExt->VoiceSound_Override;

			if (nSound.isset())
			{
				VocClass::SafeImmedietelyPlayAt(nSound, &pThis->Location);
			}
			else
			{
				VocClass::SafeImmedietelyPlayAt(pType->DieSound[pType->DieSound.Count == 1 ? 0 : Random2Class::NonCriticalRandomNumber->RandomFromMax(pType->DieSound.Count - 1)], &pThis->Location);
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

		if (auto& pParticleZero = pThis->FireParticleSystem) {
			pParticleZero->UnInit();
		}

		if (pThis->GetHeight() > 0 || !pThis->IsABomb || pThis->GetCell()->LandType != LandType::Water)
		{
			std::optional<bool> limited {};
			if (pTypeExt->DebrisTypes_Limit.isset()) {
				limited = pTypeExt->DebrisTypes_Limit.Get();
			}

			DebrisSpawners::Spawn(pType->MinDebris,pType->MaxDebris,
			pThis->GetCoords() , pType->DebrisTypes,
			pType->DebrisAnims ,pType->DebrisMaximums, pTypeExt->DebrisMinimums, limited, args.Attacker , args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse, pThis->Owner);

			auto pWeapon = pThis->GetWeapon(pThis->CurrentWeaponNumber)->WeaponType;
			if (pType->Explodes || pThis->HasAbility(AbilityType::Explodes) || (pWeapon && pWeapon->Suicide))
			{
				const bool refuseToExplode = pThis->WhatAmI() == AbstractType::Building
					&& !BuildingTypeExtContainer::Instance.Find(((BuildingClass*)pThis)->Type)->Explodes_DuringBuildup
					&& (pThis->CurrentMission == Mission::Construction || pThis->CurrentMission == Mission::Selling);

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
							pPassengerremoved->RegisterDestruction(args.Attacker);
							TechnoExtData::HandleRemove(pPassengerremoved, args.Attacker, false, false);
						}
					}
				}

				if (!refuseToExplode && !pWHExt->ApplySuppressDeathWeapon(pThis))
					pThis->FireDeathWeapon(0);
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
					TechnoExtData::ApplyKillWeapon(pThis, args.Attacker, args.WH);

				if (pThis->IsAlive)
					PhobosAEFunctions::ApplyRevengeWeapon(pThis, args.Attacker, args.WH);
			}

			if (auto pBomb = pThis->AttachedBomb)
			{
				pBomb->Detonate();
			}

			R->EAX(_res);
			return 0x702D1F;
		} else {
			if (pType->DamageSound != -1)
			{
				VocClass::SafeImmedietelyPlayAt(pType->DamageSound, &pThis->Location, 0);
			}

			if (args.Attacker && args.Attacker->IsAlive && (pType->ToProtect || pThis->__ProtectMe_3CF) && !pThis->Owner->IsControlledByHuman())
			{
				pThis->BaseIsAttacked(args.Attacker);
			}
		}
	}
		break;
	case DamageState::PostMortem:
	{
		pThis->IsAlive = true;
		pThis->Health = 1;
		R->EAX(DamageState::PostMortem);
		return 0x702D1F;
	}
	default:
		break;
	}

	if (args.Attacker && !pThis->Owner->IsAlliedWith(args.Attacker)) {
		pThis->IsTickedOff = 1;
	}

	bool IsAffected = _res != DamageState::Unaffected || isActuallyAffected;
	bool bAffected = false;
	if (IsAffected || args.IgnoreDefenses || _isNegativeDamage || *args.Damage) {
		if (IsAffected && !_isNegativeDamage) {
			const auto rank = pThis->Veterancy.GetRemainingLevel();
			const auto fromTechno = pTypeExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank);
			const int amount = pWHExt->SelfHealing_CombatDelay.GetFromSpecificRank(rank)
				->Get(fromTechno);

			//the timer will always restart
			//not accumulated
			if (amount > 0) {
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

			if (!pThis->DamageParticleSystem && !_Particles->empty() && pThis->GetHeight() > -10)
			{
				CoordStruct _offs = pThis->Location + pType->GetParticleSysOffset();
				pThis->DamageParticleSystem =
					GameCreate<ParticleSystemClass>(
						_Particles[ScenarioClass::Instance->Random.RandomFromMax(_Particles->size() - 1)],
						_offs,
						nullptr,
						pThis);
			}
		}
	}
	else
	{
		if (auto& pPart = pThis->DamageParticleSystem)
		{
			pPart->UnInit();
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
		if (pFoot)
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

#pragma endregion

#pragma region Building

DamageState FakeBuildingClass::_ReceiveDamage(int* Damage, int DistanceToEpicenter, WarheadTypeClass* WH, TechnoClass* Attacker, bool IgnoreDefenses, bool PreventsPassengerEscape, HouseClass* SourceHouse)
{
	BuildingClass* pThis = this;
	DamageState _res = DamageState::Unaffected;
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(WH);
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	auto pBldExt = BuildingExtContainer::Instance.Find(pThis);

	if (pThis == Attacker && (!pWHExt->AllowDamageOnSelf && !pThis->Type->DamageSelf))
	{
		return DamageState::Unaffected;
	}

	auto pShape = pThis->GetShapeNumber();
	auto foundation = pThis->GetFoundationData();

	if (pThis->Owner && !pWHExt->Nonprovocative && Attacker && Attacker->IsAlive && !pThis->IsStrange())
	{
		pThis->Owner->LAEnemy = Attacker->Owner->ArrayIndex;
		pThis->Owner->LATime = Unsorted::CurrentFrame;
		pThis->BaseIsAttacked(Attacker);
	}

	StackVector<TechnoClass*, 0xAu> CachedRadio { };

	for (auto i = 0; i < pThis->RadioLinks.Capacity; i++)
	{
		if (pThis->RadioLinks.Items[i])
		{
			CachedRadio->emplace_back(pThis->RadioLinks.Items[i]);
		}
	}

	if (pThis->Type->LaserFence && !IgnoreDefenses)
	{
		return DamageState::Unaffected;
	}

	if (pThis->Type->BridgeRepairHut && pThis->Type->Immune)
	{
		return DamageState::Unaffected;
	}

	if (FirewallFunctions::IsActiveFirestormWall(pThis, nullptr))
	{
		auto const pExt = RulesExtData::Instance();
		auto const& coefficient = pExt->DamageToFirestormDamageCoefficient;
		auto const amount = static_cast<int>(*Damage * coefficient);

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

		return DamageState::Unaffected;
	}
	else
	{
		_res = pThis->TechnoClass::ReceiveDamage(Damage, DistanceToEpicenter, WH, Attacker, IgnoreDefenses, PreventsPassengerEscape, SourceHouse);

		if (!pThis->IsAlive)
		{
			return _res;
		}

		switch (_res)
		{
		case DamageState::NowYellow:
		{
			if (auto pParticle = pThis->NaturalParticleSystem)
			{
				pParticle->SpawnFrames *= 1.5;
			}
			[[fallthrough]];
		}
		case DamageState::NowRed:
		{
			if (!pTypeExt->DisableDamageSound && pThis->Type->DamageSound == -1)
			{
				VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->BuildingDamageSound, &pThis->Location, 0);
			}

			if (WH->Sparky)
			{

				const bool Onfire = pTypeExt->HealthOnfire.Get(pThis->GetHealthStatus());
				auto const pFireType = pTypeExt->OnFireTypes.GetElements(RulesClass::Instance->OnFire);

				if (Onfire && pFireType.size() >= 3 && Unsorted::CurrentFrame > pBldExt->LastFlameSpawnFrame + RulesExtData::Instance()->BuildingFlameSpawnBlockFrames)
				{
					pBldExt->LastFlameSpawnFrame = Unsorted::CurrentFrame;
					const auto rand_ = pThis->Type->GetFoundationWidth() + pThis->Type->GetFoundationHeight(false) + 5;

					for (auto fnd = foundation; (fnd->X != 0x7FFF || fnd->Y != 0x7FFF); ++fnd)
					{
						auto const& [nCellX, nCellY] = pThis->InlineMapCoords() + *fnd;
						CoordStruct nDestCoord { (nCellX * 256) + 128, (nCellY * 256) + 128, 0 };
						nDestCoord.Z = MapClass::Instance->GetCellFloorHeight(nDestCoord);

						auto PlayFireAnim = [&](int nLoop = 1, int nFireTypeAt = 2)
							{
								if (auto pAnimType = pFireType[nFireTypeAt])
								{
									nDestCoord = MapClass::GetRandomCoordsNear(nDestCoord, 96, false);
									auto const pAnim = GameCreate<AnimClass>(pAnimType, nDestCoord, 0, nLoop);
									pAnim->SetOwnerObject(pThis);
									const auto pKiller = Attacker;
									const auto Invoker = (pKiller) ? pKiller->Owner : SourceHouse;
									AnimExtData::SetAnimOwnerHouseKind(pAnim, Invoker, pThis->Owner, pKiller, false, false);
								}
							};

						switch (ScenarioClass::Instance->Random.RandomFromMax(rand_))
						{
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
							PlayFireAnim(ScenarioClass::Instance->Random.RandomRanged(1, pFireType.size()), 0);
							break;
						case 6:
						case 7:
						case 8:
							PlayFireAnim(ScenarioClass::Instance->Random.RandomRanged(1, pFireType.size()), 1);
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

			break;
		}
		case DamageState::NowDead:
		{

			if (auto pLinked = pThis->BunkerLinkedItem)
			{
				for (int i = 0; i < (int)CachedRadio->size(); ++i)
				{
					if (CachedRadio[i] == pLinked)
					{
						CachedRadio->erase(CachedRadio->begin() + i);
					}
				}

				pThis->UnloadBunker();
			}

			pBldExt->MyPrismForwarding->RemoveFromNetwork(true);

			if (auto pManager = pThis->CaptureManager)
			{
				pManager->FreeAll();
			}

			if (auto pLocoTarget = pThis->LocomotorTarget)
			{
				pLocoTarget->LocomotorImblued(true);
			}

			//if ((int)CachedRadio->size() != pThis->RadioLinks.Capacity && pThis->Type->Helipad)
			//	Debug::LogInfo("Building {} - {} has inconsistent cache size of RadioLinks [O : {} ,  C : {}" , pThis->Type->ID, pThis->Owner->Type->ID , pThis->RadioLinks.Capacity , (int)CachedRadio->size());

			for (int i = 0; i < (int)CachedRadio->size(); ++i)
			{
				if ((pThis->GetCoords() - CachedRadio[i]->GetCoords()).Length() < 0x100 || pThis->Type->Helipad)
				{
					int _damage = CachedRadio[i]->GetTechnoType()->Strength;
					CachedRadio[i]->ReceiveDamage(&_damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
				}
				else
				{
					pThis->SendCommand(RadioCommand::NotifyLeave, CachedRadio[i]);
					CachedRadio[i]->QueueUpToEnter = nullptr;
				}
			}

			CachedRadio->clear();

			if (pThis->Type->CanBeOccupied)
			{
				pThis->KickAllOccupants(false, false);
			}

			if (auto pSource = pThis->LightSource)
			{
				pSource->Deactivate();
			}

			pThis->Destroy(0u, Attacker, IgnoreDefenses, foundation);

			auto Started = pThis->GoingToBlowTimer.StartTime;
			auto DelayTime = pThis->GoingToBlowTimer.TimeLeft;

			if (Started != -1 && Unsorted::CurrentFrame - Started < DelayTime)
			{
				DelayTime -= Unsorted::CurrentFrame - Started;
			}

			if (DelayTime > 0)
			{
				pThis->UnInit();
				pThis->AfterDestruction();
			}
			break;
		}
		case DamageState::PostMortem:
		{
			return _res;
		}
		default:
			break;
		}
	}

	if (pThis->IsAlive)
	{
		if (_res != DamageState::Unaffected)
		{
			if (!pWHExt->Nonprovocative && Attacker)
			{
				if (!pThis->Type->Insignificant && !pThis->IsStrange())
				{
					pBldExt->ReceiveDamageWarhead = WH;
					pThis->BuildingUnderAttack();
				}

				pThis->OwnerCountryIndex = Attacker->Owner->ArrayIndex;

				if (!pThis->Type->EMPulseCannon
					&& !pThis->Type->NukeSilo
					&& pThis->CurrentMission != Mission::Selling
					&& !pThis->Owner->IsAlliedWith(Attacker->Owner))
				{
					if (Attacker->IsAlive)
					{
						auto pWPS = pThis->GetWeapon(0);
						if (pWPS && pWPS->WeaponType)
						{
							if (!pWPS->WeaponType->Projectile->AA
								&& (!pThis->Target || !pThis->IsCloseEnoughToAttack(Attacker)))
							{
								const bool def = BuildingTypeExtContainer::Instance.Find(pThis->Type)->PlayerReturnFire.Get(
												Attacker->WhatAmI() == AircraftClass::AbsID ||
												(pThis->Owner->IsControlledByHuman() && !RulesClass::Instance->PlayerReturnFire)
								);

								if (def)
								{
									auto& pri = pThis->PrimaryFacing;

									if (!pri.Is_Rotating() && pThis->IsPowerOnline())
									{
										DirStruct _rand { ScenarioClass::Instance->Random.Random() };
										pri.Set_Desired(_rand);
									}
								}
								else
								{
									pThis->SetTarget(Attacker);
								}
							}
						}
					}
				}
			}

			pThis->ToggleDamagedAnims(pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow);
		}

		if (pShape != pThis->GetShapeNumber())
		{
			pThis->NeedsRedraw = true;
			pThis->ToggleDamagedAnims(pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow);
		}
	}

	return _res;
}


DEFINE_FUNCTION_JUMP(LJMP, 0x442230, FakeBuildingClass::_ReceiveDamage)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4028, FakeBuildingClass::_ReceiveDamage)
#pragma endregion

#pragma region Foot
#include <TeamTypeClass.h>

ASMJIT_PATCH(0x4D7330, FootClass_ReceiveDamage_Handle, 0x8)
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

	if (_res == DamageState::NowDead || _res == DamageState::Unaffected)
	{
		R->EAX(_res);
		return 0x4D74D6;

	}
	else if (_res != DamageState::PostMortem)
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

		if (auto pTeam = pThis->Team)
		{
			if (pTeam->Type->Whiner && !pThis->Owner->IsControlledByHuman())
			{
				if (!args.Attacker || !args.Attacker->IsAlive)
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

ASMJIT_PATCH(0x4165C0, AircraftClass_ReceiveDamage_Handle, 0x7)
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

ASMJIT_PATCH(0x517FA0, InfantryClass_ReceiveDamage_Handled, 6)
{
	GET(FakeInfantryClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

	if (pThis->IsDeployed() && args.IgnoreDefenses) {
		*args.Damage = static_cast<int>(*args.Damage * pWarheadExt->DeployedDamage.Get(pThis));
	}
	else if (pThis->Crawling && *args.Damage > 0 && !args.IgnoreDefenses) {
		*args.Damage = (int)MaxImpl(((double)*args.Damage * args.WH->ProneDamage), 1);
	}

	if (args.WH->InfDeath == InfDeath::Mutate && pThis->GetHeight() > 0) {
		*args.Damage = 0;
	}

	DamageState _res = pThis->FootClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);
	if (_res == DamageState::Unaffected || _res == DamageState::PostMortem) {
		R->EAX(_res);
		return 0x518D52;
	}

	if (_res == DamageState::NowDead)
	{
		if (auto pEnslave = pThis->SlaveOwner) {
			if (auto pManager = pEnslave->SlaveManager) {
				pManager->Killed(pThis);
			}
		}

		auto MyTransport = pThis->Transporter;

		if (MyTransport
		  && MyTransport->WhatAmI() == UnitClass::AbsID
		  && pThis->Type->Gunner) {
			static_cast<UnitClass*>(MyTransport)->RemovePassenger(pThis);
		}

		pThis->Destroyed(args.Attacker);
		pThis->StopMoving();
		pThis->Stun();
		pThis->QueueMission(Mission::None, false);
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();
		pThis->KillPassengers(args.Attacker);

		if (!TechnoExtContainer::Instance.Find(pThis)->GarrisonedIn) {

			bool IsForcedCyborg = false;
			if (args.IgnoreDefenses) {
				if (pThis->Type->Cyborg) {
					IsForcedCyborg = true;
					if (pThis->IsABomb) {
						GameCreate<AnimClass>(RulesClass::Instance->InfantryExplode, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
					}
				}
			}

			if (pThis->GetHeight() <= 10 && pThis->GetCell()->LandType == LandType::Water)
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

							AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true, false);

							if (ParticleSystemClass::Array->ValidIndex(RulesClass::Instance->InfantryVirus->SpawnsParticle))
							{
								auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
								ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
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
								Succeeded = pThis->PlayAnim(static_cast<DoType>(resultSequence), true);
							}

							if (Succeeded)
							{
								if (infDeath == InfDeath::Virus)
								{
									auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryVirus, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true, false);

									if (ParticleSystemClass::Array->ValidIndex(RulesClass::Instance->InfantryVirus->SpawnsParticle))
									{
										auto pParticleType = ParticleTypeClass::Array->Items[RulesClass::Instance->InfantryVirus->SpawnsParticle];
										ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
									}
								}

								if (!IsForcedCyborg) {
									R->EAX(DamageState::NowDead);
									return 0x518D52;
								}
							}
							else //infdeath fail
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
									const bool Hasbuilding = pCell->GetBuilding();

									if (GroundType::Get(pCell->LandType)->Cost[0] == 0.0 && !pThis->OnBridge) {
										bool fail = false;
										if (!pThis->PlayAnim(DoType::Die2, true, false)) {
											pThis->UnInit();//fail
											fail = true;
										}

										if (!IsForcedCyborg || fail) {
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
										else break;
									}

									CoordStruct closest {};
									pCell->FindInfantrySubposition(&closest, curLoc, false, false, false);

									if (!closest.IsValid()) {
										bool fail = false;
										if (!pThis->PlayAnim(DoType::Die2, true, false)) {
											pThis->UnInit();//fail
											fail = true;
										}

										if (!IsForcedCyborg || fail) {
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
										else break;
									}

									if (Hasbuilding)
									{
										bool fail = false;

										if (!pThis->PlayAnim(DoType::Die2, true, false)) {
											pThis->UnInit();//fail
											fail = true;
										}

										if (!IsForcedCyborg || fail)
										{
											R->EAX(DamageState::NowDead);
											return 0x518D52;
										}
										else break;

									}

									pThis->MarkAllOccupationBits(curLoc);
									auto pAnim_Mutate = GameCreate<AnimClass>(RulesClass::Instance->InfantryMutate, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);

									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									AnimExtData::SetAnimOwnerHouseKind(pAnim_Mutate, pInvoker, pThis->Owner, args.Attacker, true, false);

									pAnim_Mutate->MarkAllOccupationBits(pThis->Location);
								}
								break;
								case InfDeath::Brute:
									pAnim = GameCreate<AnimClass>(RulesClass::Instance->InfantryBrute, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
									break;
								default:
									break;
								}

								if (pAnim)
								{
									auto pInvoker = args.Attacker
										? args.Attacker->Owner
										: args.SourceHouse;

									const auto& [bChanged, result] = AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, args.Attacker, true, false);

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
		}

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

	if (args.Attacker && pThis->PanicDurationLeft < 100) {

		if (pThis->Type->Fraidycat) {
			pThis->PanicDurationLeft = 300;
		}else if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless)) {
			int PanixMax = 100;
			if (pThis->Type->Doggie && pThis->IsRedHP()) {
				PanixMax = RulesExtData::Instance()->DoggiePanicMax;
			}

			pThis->PanicDurationLeft = PanixMax;
		}
	} else if (!pThis->Type->Fearless && !pThis->HasAbility(AbilityType::Fearless)) {

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

		pThis->PanicDurationLeft = MinImpl(300 , pThis->PanicDurationLeft + morefear);
	}

	R->EAX(_res);
	return 0x518D52;
}

#pragma endregion

#pragma region Unit

namespace RemoveCellContentTemp
{
	bool CheckBeforeUnmark = false;
}

ASMJIT_PATCH(0x737C90, UnitClass_ReceiveDamage_Handled, 5)
{
	GET(UnitClass*, pThis, ECX);
	REF_STACK(args_ReceiveDamage, args, 0x4);

	DamageState _res = DamageState::Unaffected;
	if (pThis->DeathFrameCounter > 0)
	{
		R->EAX(_res);
		return 0x738679;
	}

	auto pWHExt = WarheadTypeExtContainer::Instance.Find(args.WH);
	bool isPlayerControlled = pThis->Owner->ControlledByCurrentPlayer();
	bool selected = pThis->IsSelected && isPlayerControlled;

	if (!args.IgnoreDefenses)
	{
		if (auto pRadio = pThis->GetRadioContact())
		{
			if (auto pBld = cast_to<BuildingClass*, false>(pRadio))
			{
				// #895584: ships not taking damage when repaired in a shipyard. bug
				// was that the logic that prevented units from being damaged when
				// exiting a war factory applied here, too. added the Naval check.
				if (pBld->Type->WeaponsFactory
					&& !pBld->Type->Naval
					&& MapClass::Instance->TryGetCellAt(pThis->Location)->GetBuilding() == pBld)
				{
					R->EAX(_res);
					return 0x738679;
				}
			}
		}
	}

	_res = pThis->FootClass::ReceiveDamage(args.Damage, args.DistanceToEpicenter, args.WH, args.Attacker, args.IgnoreDefenses, args.PreventsPassengerEscape, args.SourceHouse);

	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
	if (_res == DamageState::NowDead)
	{
		if (pThis->IsAttackedByLocomotor && pThis->Type->Crashable)
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

	if (_res != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
	{
		R->EAX(DamageState::PostMortem);
		return 0x738679;
	}

	auto _CurCoord = pThis->GetCoords();
	auto _CurCell = CellClass::Coord2Cell(_CurCoord);
	auto pType = pThis->Type;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (_res != DamageState::PostMortem)
	{
		if (_res != DamageState::NowDead)
		{
			if (_res != DamageState::Unaffected)
			{

				if (pThis->Type->Harvester
					&& pWHExt->Malicious
					&& !pWHExt->Nonprovocative
					&& pThis->Owner == HouseClass::CurrentPlayer()
					&& !pThis->Owner->IsObserver()) {
					if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, _CurCell)) {
						VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack());
					}
				}

				if (!pThis->HaveAttackMoveTarget
					&& args.Attacker
					&& args.Attacker->IsAlive
					&& !args.Attacker->IsSinking
					&& !args.Attacker->IsCrashing
					&& !args.Attacker->TemporalTargetingMe
					&& !pThis->IsTethered
					&& pThis->Owner->IsAlliedWith(args.Attacker->Owner)
					&& isPlayerControlled)
				{

					if (pThis->ShouldCrushIt(args.Attacker))
					{
						pThis->SetDestination(args.Attacker, true);
						pThis->QueueMission(Mission::Move, false);
						R->EAX(_res);
						return 0x738679;
					}

					if ((pType->Harvester || pType->Weeder)
						&& pThis->GetPipFillLevel() > 0
						&& pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
					{
						TechnoClass* pDock = nullptr;
						for (int i = 0; i < pType->Dock.Count; ++i)
						{

							pDock = pThis->FindDockingBay(pType->Dock.Items[i], 0, false);
							if (pDock)
								break;
						}

						if (pType->Dock.Count > 0 && !pDock)
						{
							R->EAX(_res);
							return 0x738679;
						}

						if (!pThis->ContainsLink(pDock) && pThis->SendCommand(RadioCommand::RequestLink, pDock) == RadioCommand::AnswerPositive)
						{
							pThis->QueueMission(Mission::Enter, false);
						}
					}
				}
			}

			R->EAX(_res);
			return 0x738679;
		}

		if (auto pBunker = cast_to<BuildingClass*>(pThis->BunkerLinkedItem))
		{
			pBunker->ClearBunker();
		}

		if (pType->DeathFrames <= 0)
		{
			bool ShouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;

			if (!pTypeExt->Sinkable.Get(ShouldSink)
			   || pThis->GetCell()->LandType != LandType::Water
			   || pThis->WarpingOut)
			{
				pThis->Destroyed(args.Attacker);

				if (pThis->GetHeight() <= 10
				  && pThis->IsABomb
				  && (pThis->GetCell()->LandType == LandType::Water))
				{
					GameCreate<AnimClass>(RulesClass::Instance->Wake, pThis->Location, 0, 1, AnimFlag(0x600), 0, 0);
					auto coord_splash = pThis->Location;
					coord_splash += CoordStruct(0, 0, 5);
					GameCreate<AnimClass>(RulesClass::Instance->SplashList.Items[RulesClass::Instance->SplashList.Count - 1], pThis->Location, 0, 1, AnimFlag(0x600), 0, 0);
				}
				else
				{

					pExt->ReceiveDamage = true;
					AnimTypeExtData::ProcessDestroyAnims(pThis, args.Attacker, args.WH);
					pThis->Explode();
				}
			}
			else
			{
				pThis->Destroyed(args.Attacker);
				pThis->Health = 1;
				pThis->IsAlive = 1;
				pThis->IsSinking = 1;
				pThis->Stun();
			}
		}
		else
		{
			if (pThis->DeathFrameCounter == -1)
			{
				pThis->DeathFrameCounter = 0;
				pThis->Destroyed(args.Attacker);
			}

			pThis->Health = 1;
			pThis->IsAlive = 1;
		}

		RemoveCellContentTemp::CheckBeforeUnmark = true;
		pThis->Mark(MarkType::Remove);
		RemoveCellContentTemp::CheckBeforeUnmark = false;

		if (pThis->Passengers.NumPassengers > 0 && pThis->Passengers.GetFirstPassenger())
		{
			if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
			{
				auto pPassenger = pThis->Passengers.GetFirstPassenger();
				auto pPassengerExt = TechnoExtContainer::Instance.Find(pPassenger);

				if (pPassengerExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pPassengerExt->OriginalPassengerOwner, false);

				while (pPassenger->NextObject)
				{
					pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
					pPassengerExt = TechnoExtContainer::Instance.Find(pPassenger);

					if (pExt->OriginalPassengerOwner)
						pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
				}
			}
		}

		if (pType->OpenTopped)
			pThis->MarkPassengersAsExited();

		TechnoExt_ExtData::SpawnSurvivors(pThis, args.Attacker, selected, args.IgnoreDefenses, args.PreventsPassengerEscape);

		if (GameModeOptionsClass::Instance->Crates)
		{
			if (pType->CrateGoodie
			  && (ScenarioClass::Instance->TruckCrate && !pType->IsTrain
				  || ScenarioClass::Instance->TrainCrate && pType->IsTrain))
			{

				const auto crate_cell = MapClass::Instance->NearByLocation(pThis->GetMapCoords(), SpeedType::Track, ZoneType::None, MovementZone::Normal, 0, 1, 1, true, false, false, true, CellStruct::Empty, false, false);
				if (crate_cell.IsValid())
				{
					MapClass::Instance->Place_Crate(crate_cell, PowerupEffects(0x14));
				}
			}
		}

		if ((!pType->Crashable || !pThis->Crash(args.Attacker)) && !pThis->IsSinking)
			pThis->UnInit();

	}

	R->EAX(_res);
	return 0x738679;
}

ASMJIT_PATCH(0x47EAF7, CellClass_RemoveContent_BeforeUnmarkOccupationBits, 0x7)
{
	enum { ContinueCheck = 0x47EAFE, DontUnmark = 0x47EB8F };

	GET(CellClass*, pCell, EDI);
	GET_STACK(bool, onBridge, STACK_OFFSET(0x14, 0x8));

	if (RemoveCellContentTemp::CheckBeforeUnmark && onBridge ? pCell->AltObject : pCell->FirstObject)
		return DontUnmark;

	GET(ObjectClass*, pContent, ESI);
	R->EAX(pContent->WhatAmI());
	return ContinueCheck;
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

ASMJIT_PATCH(0x718B29, LocomotionClass_SomethingWrong_ReceiveDamage_UseCurrentHP, 0x6)
{
	GET(FootClass* const, pLinked, ECX);
	R->ECX(pLinked->GetType()->Strength);
	return R->Origin() + 0x6;
}

#pragma region placeholder
//ASMJIT_PATCH(0x737F97, UnitClass_ReceiveDamage_Survivours, 0xA)
//{
//	//GET(UnitTypeClass*, pType, EAX);
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(TechnoClass*, pKiller, 0x54);
//	GET_STACK(bool, select, 0x13);
//	GET_STACK(bool, ignoreDefenses, 0x58);
//	GET_STACK(bool, preventPassangersEscape, STACK_OFFSET(0x44, 0x18));
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
//				pPassenger = flag_cast_to<FootClass*, false>(pPassenger->NextObject);
//				pExt = TechnoExtContainer::Instance.Find(pPassenger);
//
//				if (pExt->OriginalPassengerOwner)
//					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
//			}
//		}
//	}
//
//	TechnoExt_ExtData::SpawnSurvivors(pThis, pKiller, select, ignoreDefenses, preventPassangersEscape);
//
//	R->EBX(-1);
//	return 0x73838A;
//}
//
//ASMJIT_PATCH(0x737F6D, UnitClass_ReceiveDamage_Destroy, 0x7)
//{
//	GET(UnitClass* const, pThis, ESI);
//	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x44, -0x4));
//
//	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
//	R->ECX(R->ESI());
//	pExt->ReceiveDamage = true;
//	AnimTypeExtData::ProcessDestroyAnims(pThis, args.Attacker, args.WH);
//	pThis->Destroy();
//	return 0x737F74;
//}
//
//ASMJIT_PATCH(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
//{
//	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x44, -0xC));
//	return !WarheadTypeExtContainer::Instance.Find(pWH)->Malicious || WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative ? 0x738535u : 0u;
//}
//
//ASMJIT_PATCH(0x737DE2, UnitClass_ReceiveDamage_Sinkable, 0x6)
//{
//	enum { GoOtherChecks = 0x737E18, NoSink = 0x737E63 };
//
//	GET(UnitTypeClass*, pType, EAX);
//
//	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
//	bool ShouldSink = pType->Weight > RulesClass::Instance->ShipSinkingWeight && pType->Naval && !pType->Underwater && !pType->Organic;
//
//	return pTypeExt->Sinkable.Get(ShouldSink) ? GoOtherChecks : NoSink;
//}
//
//ASMJIT_PATCH(0x737D57, UnitClass_ReceiveDamage_DyingFix, 0x7)
//{
//	GET(UnitClass* const, pThis, ESI);
//	GET(DamageState const, result, EAX);
//
//	// Immediately release locomotor warhead's hold on a crashable unit if it dies while attacked by one.
//	if (result == DamageState::NowDead)
//	{
//
//		if (pThis->IsAttackedByLocomotor && pThis->GetTechnoType()->Crashable)
//			pThis->IsAttackedByLocomotor = false;
//
//		//this cause desync ?
//		if (!pThis->Type->Voxel && pThis->Type->Strength > 0)
//		{
//			if (pThis->Type->MaxDeathCounter > 0
//				&& !pThis->InLimbo
//				&& !pThis->IsCrashing
//				&& !pThis->IsSinking
//				&& !pThis->TemporalTargetingMe
//				&& !pThis->IsInAir()
//				&& pThis->DeathFrameCounter <= 0
//				)
//			{
//
//				pThis->Stun();
//				const auto loco = pThis->Locomotor.GetInterfacePtr();
//
//				if (loco->Is_Moving_Now())
//					loco->Stop_Moving();
//
//				pThis->DeathFrameCounter = 1;
//			}
//		}
//	}
//
//	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
//	{
//		R->EAX(DamageState::PostMortem);
//	}
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x737CBB, UnitClass_ReceiveDamage_DeathCounter, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	if (pThis->DeathFrameCounter > 0)
//	{
//		return 0x737D26;
//	}
//
//	return 0x0;
//}
//
//// #895584: ships not taking damage when repaired in a shipyard. bug
//// was that the logic that prevented units from being damaged when
//// exiting a war factory applied here, too. added the Naval check.
//ASMJIT_PATCH(0x737CE4, UnitClass_ReceiveDamage_ShipyardRepair, 6)
//{
//	GET(BuildingTypeClass*, pType, ECX);
//	return (pType->WeaponsFactory && !pType->Naval)
//		? 0x737CEE : 0x737D31;
//}

//ASMJIT_PATCH(0x518313, InfantryClass_ReceiveDamage_JumpjetExplode, 0x6)
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

// ASMJIT_PATCH(0x5184F7, InfantryClass_ReceiveDamage_NotHuman, 0x6)
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
ASMJIT_PATCH(0x442243, BuildingClass_ReceiveDamage_AddEarly, 0xA)
{
	R->Stack(STACK_OFFS(0x9C, 0x6C), DamageState::Unaffected);

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EBP);

	if (pThis == pSource && !pSource->GetTechnoType()->DamageSelf) {
		return 0x442C06;
	}

	return 0x442268;
}*/
// ASMJIT_PATCH(0x51849A, InfantryClass_ReceiveDamage_DeathAnim, 5)
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

// ASMJIT_PATCH_AGAIN(0x518575, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
// ASMJIT_PATCH(0x5183DE, InfantryClass_ReceiveDamage_InfantryVirus1, 6)
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

// ASMJIT_PATCH_AGAIN(0x518B93, InfantryClass_ReceiveDamage_Anims, 5) // InfantryBrute
// ASMJIT_PATCH_AGAIN(0x518821, InfantryClass_ReceiveDamage_Anims, 5) // InfantryNuked
// ASMJIT_PATCH_AGAIN(0x5187BB, InfantryClass_ReceiveDamage_Anims, 5) // InfantryHeadPop
// ASMJIT_PATCH_AGAIN(0x518755, InfantryClass_ReceiveDamage_Anims, 5) // InfantryElectrocuted
// ASMJIT_PATCH_AGAIN(0x5186F2, InfantryClass_ReceiveDamage_Anims, 5) // FlamingInfantry
// ASMJIT_PATCH(0x518698, InfantryClass_ReceiveDamage_Anims, 5) // InfantryExplode
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

// ASMJIT_PATCH(0x51887B, InfantryClass_ReceiveDamage_InfantryVirus2, 0xA)
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
// ASMJIT_PATCH(0x518A96, InfantryClass_ReceiveDamage_InfantryMutate, 7)
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

//ASMJIT_PATCH(0x489A01, MapClass_DamageArea_LoopDamageGroups, 0x6)
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
//		Debug::LogInfo("Bullet[%x - %s] Getting hit by [%s] Warhead [%s] !" , pTarget,pTarget->get_ID() ,dummy , pWarhead->ID);
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

//ASMJIT_PATCH(0x4D7431, FootClass_TakeDamage_ProbeResult, 0x5)
//{
//	GET(DamageState, result, EAX);
//	GET(WarheadTypeClass*, pWH, EBP);
//	GET(FootClass*, pThis, ESI);
//
//	if (IS_SAME_STR_("EradiationWH", pWH->ID) && IS_SAME_STR_("PENTGENX", pThis->get_ID())) {
//		Debug::LogInfo("Affected [%d] by[%s]", (int)result, pWH->ID);
//	}
//
//	return 0x0;
//}

// Has to be done here, before Ares survivor hook to take effect.
//ASMJIT_PATCH(0x737F80, TechnoClass_ReceiveDamage_Cargo_SyncOwner, 0x6)
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

ASMJIT_PATCH(0x71B98B, TerrainClass_ReceiveDamage_Add, 0x7)
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

//ASMJIT_PATCH(0x4D7330, FootClass_ReceiveDamage_probe, 0x8) {
//	GET(FootClass*, pThis, ECX);
//
//	auto id = pThis->get_ID();
//	if (IS_SAME_STR_("MDUMMY7", id))
//		Debug::LogInfo(__FUNCTION__" Executed");
//
//	return 0;
//}

//ASMJIT_PATCH(0x737C90 , UnitClass_TakeDamage_probe ,0x5)
//{
//	GET(UnitClass*, pThis, ECX);
//	REF_STACK(args_ReceiveDamage, args, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (IS_SAME_STR_(pThis->Type->ID, "MDUMMY7"))
//		Debug::LogInfo("%s [%s]DamageResult %d , HP %d/%d Called %x.", pThis->Type->ID, args.WH->ID, *args.Damage, pThis->Health, pThis->Type->Strength , caller);
//
//	return 0x0;
//}


// ASMJIT_PATCH(0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6)
// {
// 	GET(TechnoClass* const, pRadio, EDI);
//
// 	pRadio->ReceiveDamage(&pRadio->GetType()->Strength, 0, RulesClass::Instance->C4Warhead,
// 		nullptr, true, true, nullptr);
//
// 	return 0x4425F4;
// }

// ASMJIT_PATCH(0x70253F, TechnoClass_ReceiveDamage_Metallic_AnimDebris, 0x6)
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
// ASMJIT_PATCH(0x702484, TechnoClass_ReceiveDamage_AnimDebris, 0x6)
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

// ASMJIT_PATCH(0x518744, InfantryClass_ReceiveDamage_ElectricDeath, 6)
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


// ASMJIT_PATCH(0x518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
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