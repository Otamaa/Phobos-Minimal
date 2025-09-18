#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>

#include <BitFont.h>

#include <New/Entity/FlyingStrings.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/Scenario/Body.h>

#include <Locomotor/Cast.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Enum.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/LocomotionCast.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/Ares/Hooks/Header.h>
#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

#include <memory>
#include <TerrainTypeClass.h>

#pragma region defines
UnitClass* TechnoExtData::Deployer { nullptr };
#pragma endregion

void TechnoExtData::InitPassiveAcquireMode()
{
	auto pType = This()->GetTechnoType();
	this->PassiveAquireMode = TechnoTypeExtContainer::Instance.Find(pType)->PassiveAcquireMode.Get();
}

PassiveAcquireMode TechnoExtData::GetPassiveAcquireMode() const
{
	// if this is a passenger then obey the configuration of the transport
	if (auto pTransport = This()->Transporter)
		return TechnoExtContainer::Instance.Find(pTransport)->GetPassiveAcquireMode();

	return this->PassiveAquireMode;
}

void TechnoExtData::TogglePassiveAcquireMode(PassiveAcquireMode newMode)
{
	auto previousMode = this->PassiveAquireMode;
	this->PassiveAquireMode = newMode;

	if (newMode == previousMode)
		return;

	const auto pThis = This();
	const auto pTechnoType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	int voiceIndex;

	if (newMode == PassiveAcquireMode::Normal)
	{
		if (previousMode == PassiveAcquireMode::Ceasefire)
		{
			voiceIndex = pTypeExt->VoiceExitCeasefireMode.Get();

			if (voiceIndex < 0)
			{
				const auto& voiceList = pTechnoType->VoiceAttack.Count ? pTechnoType->VoiceAttack : pTechnoType->VoiceMove;

				if (const auto count = voiceList.Count)
					voiceIndex = voiceList.GetItem(Random2Class::Global->Random() % count);
			}
		}
		else
		{
			pThis->SetTarget(nullptr);
			voiceIndex = pTypeExt->VoiceExitAggressiveMode.Get();

			if (voiceIndex < 0)
			{
				const auto& voiceList = pTechnoType->VoiceMove.Count ? pTechnoType->VoiceMove : pTechnoType->VoiceSelect;

				if (const auto count = voiceList.Count)
					voiceIndex = voiceList.GetItem(Random2Class::Global->Random() % count);
			}
		}
	}
	else if (newMode == PassiveAcquireMode::Ceasefire)
	{
		pThis->SetTarget(nullptr);
		voiceIndex = pTypeExt->VoiceEnterCeasefireMode.Get();

		if (voiceIndex < 0)
		{
			const auto& voiceList = pTechnoType->VoiceSelect.Count ? pTechnoType->VoiceSelect : pTechnoType->VoiceMove;

			if (const auto count = voiceList.Count)
				voiceIndex = voiceList.GetItem(Random2Class::Global->Random() % count);
		}
	}
	else
	{
		voiceIndex = pTypeExt->VoiceEnterAggressiveMode.Get();

		if (voiceIndex < 0)
		{
			const auto& voiceList = pTechnoType->VoiceAttack.Count ? pTechnoType->VoiceAttack : pTechnoType->VoiceMove;

			if (const auto count = voiceList.Count)
				voiceIndex = voiceList.GetItem(Random2Class::Global().Random() % count);
		}
	}

	pThis->QueueVoice(voiceIndex);
}

bool TechnoExtData::CanTogglePassiveAcquireMode()
{
	if (!RulesExtData::Instance()->EnablePassiveAcquireMode)
		return false;

	auto pType = This()->GetTechnoType();
	return TechnoTypeExtContainer::Instance.Find(pType)->PassiveAcquireMode_Togglable;
}

bool TechnoExtData::IsOnBridge(FootClass* pUnit)
{
	auto const pCell = MapClass::Instance->GetCellAt(pUnit->GetCoords());
	auto const pCellAjd = pCell->GetNeighbourCell(FacingType::North);
	bool containsBridge = pCell->ContainsBridge();
	bool containsBridgeDir = static_cast<bool>(pCell->Flags & CellFlags::BridgeDir);

	if ((containsBridge || containsBridgeDir || pCellAjd->ContainsBridge()) && (!containsBridge || pCell->GetNeighbourCell(FacingType::West)->ContainsBridge()))
		return true;

	return false;
}

int TechnoExtData::GetJumpjetIntensity(FootClass* pThis)
{
	int level = ScenarioClass::Instance->NormalLighting.Level;

	if (LightningStorm::IsActive())
		level = ScenarioClass::Instance->IonLighting.Level;
	else if (PsyDom::IsActive())
		level = ScenarioClass::Instance->DominatorLighting.Level;
	else if (NukeFlash::IsFadingIn())
		level = ScenarioClass::Instance->NukeLighting.Level;

	int levelIntensity = 0;
	int cellIntensity = 1000;
	GetLevelIntensity(pThis, level, levelIntensity, cellIntensity, RulesExtData::Instance()->JumpjetLevelLightMultiplier, IsOnBridge(pThis));

	return levelIntensity + cellIntensity;
}

void TechnoExtData::GetLevelIntensity(TechnoClass* pThis, int level, int& levelIntensity, int& cellIntensity, double levelMult, double cellMult, bool applyBridgeBonus)
{
	double currentLevel = pThis->GetHeight() / static_cast<double>(Unsorted::LevelHeight);
	levelIntensity = static_cast<int>(level * currentLevel * levelMult);
	int bridgeBonus = applyBridgeBonus ? 4 * level : 0;
	cellIntensity = MapClass::Instance()->GetCellAt(pThis->GetMapCoords())->Intensity_Normal + bridgeBonus;

	if (cellMult > 0.0)
		cellIntensity = std::clamp(cellIntensity + static_cast<int>((1000 - cellIntensity) * currentLevel * cellMult), 0, 1000);
	else if (cellMult < 0.0)
		cellIntensity = 1000;
}

int TechnoExtData::GetDeployingAnimIntensity(FootClass* pThis)
{
	int intensity = 0;

	if (locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
		intensity = GetJumpjetIntensity(pThis);
	else
		intensity = pThis->GetCell()->Intensity_Normal;

	intensity = pThis->GetFlashingIntensity(intensity);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (TechnoExtContainer::Instance.Find(pThis)->AirstrikeTargetingMe)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	return intensity;
}

int __fastcall FakeTechnoClass::__AdjustDamage(TechnoClass* pThis, discard_t,TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	int damage = 0;
	if (pTarget && !pWeapon->IsSonic && !pWeapon->UseFireParticles && pWeapon->Damage > 0)
	{

		double _damage = TechnoExtData::GetDamageMult(pThis, (double)pWeapon->Damage);
		int _damage_int = (int)TechnoExtData::GetArmorMult(pTarget, _damage, pWeapon->Warhead);
		if (_damage_int < 1)
			_damage_int = 1;

		damage = FakeWarheadTypeClass::ModifyDamage(_damage_int, pWeapon->Warhead, TechnoExtData::GetTechnoArmor(pTarget, pWeapon->Warhead), 0);
	}

	return damage;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6FDB80, FakeTechnoClass::__AdjustDamage);
DEFINE_FUNCTION_JUMP(CALL, 0x6FE61D, FakeTechnoClass::__AdjustDamage);
DEFINE_FUNCTION_JUMP(CALL, 0x7099B0, FakeTechnoClass::__AdjustDamage);

bool __fastcall FakeTechnoClass::__TargetSomethingNearby(TechnoClass* pThis, discard_t, CoordStruct* coord, ThreatType threat)
{
	pThis->__creationframe_4FC = Unsorted::CurrentFrame();
	auto pType = pThis->GetTechnoType();

	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);

	const auto pRules = RulesClass::Instance();
	const bool IsHuman = pThis->Owner->IsHumanPlayer || pThis->Owner->IsControlledByHuman();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pThis->MegaMissionIsAttackMove())
	{
		delay += IsHuman
			? pTypeExt->PlayerAttackMoveTargetingDelay.Get(RulesExtData::Instance()->PlayerAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AIAttackMoveTargetingDelay.Get(RulesExtData::Instance()->AIAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}
	else if (pThis->CurrentMission == Mission::Area_Guard)
	{
		delay +=
			IsHuman
			? pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExtData::Instance()->PlayerGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay))
			: pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExtData::Instance()->AIGuardAreaTargetingDelay.Get(pRules->GuardAreaTargetingDelay));

	}
	else
	{
		delay += IsHuman
			? pTypeExt->PlayerNormalTargetingDelay.Get(RulesExtData::Instance()->PlayerNormalTargetingDelay.Get(pRules->NormalTargetingDelay))
			: pTypeExt->AINormalTargetingDelay.Get(RulesExtData::Instance()->AINormalTargetingDelay.Get(pRules->NormalTargetingDelay));
	}

	pThis->TargetingTimer.Start(delay);

	if (pTypeExt->AutoFire) {

		pThis->SetTarget(pTypeExt->AutoFire_TargetSelf ? pThis :
			static_cast<AbstractClass*>(pThis->GetCell()));
		return pThis->Target != nullptr;
	}

	// Check current target
	if (pThis->Target && pThis->ShouldLoseTargetNow)
	{
		const int weaponIndex = pThis->SelectWeapon(pThis->Target);
		const auto fire = pThis->GetFireError(pThis->Target, weaponIndex, 1);

		if (fire == FireError::CANT) {

			if(!pThis->SpawnManager) {
				pThis->SetTarget(nullptr);
			} else {
				pThis->SpawnManager->ResetTarget();
			}
		}

		if (fire == FireError::ILLEGAL || fire == FireError::RANGE) {
			pThis->SetTarget(nullptr);
		}
	}

	if (!pThis->Target) {
		if (pType->DistributedFire) {
			pThis->DistributedFire();
		} else if (const auto potentialTarget = pThis->GreatestThreat((threat & (ThreatType::Range | ThreatType::Area)).operator ThreatType(), coord, 0)) {

			pThis->SetTarget(potentialTarget);

			if (auto pPotentT = flag_cast_to<TechnoClass* ,false>(potentialTarget)) {
				const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));

				if (pWeapon && pWeapon->WeaponType && !pWeapon->WeaponType->Projectile->Inaccurate) {
					pPotentT->EstimatedHealth -= FakeTechnoClass::__AdjustDamage(pThis , discard_t() , pPotentT, pWeapon->WeaponType);

				}
			}
		}
	}

	return pThis->Target != nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x709820, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2640, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4258, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E9030, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB3F4, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4CFC, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F600C, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x6FA6DC, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x4D6F06, FakeTechnoClass::__TargetSomethingNearby);
DEFINE_FUNCTION_JUMP(CALL6, 0x4D5392, FakeTechnoClass::__TargetSomethingNearby);

int __fastcall FakeTechnoClass::_EvaluateJustCell(TechnoClass* pThis , discard_t,CellStruct* where)
{

	// /*
	// **  First, only computer objects are allowed to automatically scan for walls.
	// */
	if (pThis->Owner->IsControlledByHuman())
	{
		return 0;
	}

	// /*
	// **  Even then, if the difficulty indicates that it shouldn't search for wall
	// **  targets, then don't allow it to do so.
	// */
	if (!RulesClass::Instance->AIDiffs[(int)pThis->Owner->AIDifficulty].DestroyWalls)
	{
		return 0;
	}

	auto pCell = MapClass::Instance->GetCellAt(where);

	if (pCell->OverlayTypeIndex == -1 || !OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex]->Wall)
		return 0;

	auto pSelectedWeapon = pThis->SelectWeapon(pCell);

	if (!pThis->IsCloseEnough(pCell, pSelectedWeapon))
		return 0;

	auto pSelectedWeapon_ = pThis->GetWeapon(pSelectedWeapon);

	if (!pSelectedWeapon_ || !pSelectedWeapon_->WeaponType || !pSelectedWeapon_->WeaponType->Warhead)
		return 0;

	// /*
	// **  If the weapon cannot deal with ground based targets, then don't consider
	// **  this a valid cell target.
	// */
	if (pSelectedWeapon_->WeaponType->Projectile && !pSelectedWeapon_->WeaponType->Projectile->AG)
		return 0;

	// /*
	// **  If the primary weapon cannot destroy a wall, then don't give the cell any
	// **  value as a target.
	// */
	if (!pSelectedWeapon_->WeaponType->Warhead->Wall)
	{
		return 0;
	}

	// /*
	// **  If this is a friendly wall, then don't attack it.
	// */
	if (pCell->WallOwnerIndex == -1 || pThis->Owner->IsAlliedWith(HouseClass::Array->Items[pCell->WallOwnerIndex]))
	{
		return 0;
	}

	const double distance = (pThis->GetCoords() - CellClass::Cell2Coord(*where)).Length();

	// /*
	// **  Since a wall was found, then return a value adjusted according to the range the wall
	// **  is from the object. The greater the range, the lesser the value returned.
	// */

	return int((double)pThis->GetWeaponRange(pSelectedWeapon) - distance);
}

int TechnoExtData::CalculateBlockDamage(TechnoClass* pThis, args_ReceiveDamage* args)
{
	int damage = *args->Damage;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);

	if (pWHExt->ImmuneToBlock)
		return damage;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!pTypeExt->CanBlock)
		return damage;

	const auto pBlockType = pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	const auto pOtherBlock = !pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	std::vector<double>& blockChances = pBlockType->Block_Chances;
	std::vector<double>& blockDamageMultipliers = pBlockType->Block_DamageMultipliers;

	if (pWHExt->Block_AllowOverride)
	{
		blockChances = !pOtherBlock->Block_Chances.empty() ? pOtherBlock->Block_Chances : blockChances;
		blockDamageMultipliers = !pOtherBlock->Block_DamageMultipliers.empty() ? pOtherBlock->Block_DamageMultipliers : blockDamageMultipliers;
	}

	if (!pWHExt->Block_IgnoreChanceModifier)
		blockChances = TechnoExtData::GetBlockChance(pThis, blockChances);

	if ((blockChances.size() == 1 && blockChances[0] + pWHExt->Block_ExtraChance > 0.0) || blockChances.size() > 1)
	{
		// handle block conditions first
		auto blockAffectBelowPercents = pBlockType->Block_AffectBelowPercents;
		auto blockAffectsHouses = pBlockType->Block_AffectsHouses.Get(AffectedHouse::All);
		bool blockCanActiveZeroDamage = pBlockType->Block_CanActive_ZeroDamage.Get(false);
		bool blockCanActiveNegativeDamage = pBlockType->Block_CanActive_NegativeDamage.Get(false);
		bool blockCanActivePowered = pBlockType->Block_CanActive_Powered.Get(false);
		bool blockCanActiveNoFirer = pBlockType->Block_CanActive_NoFirer.Get(true);
		bool blockCanActiveShieldActive = pBlockType->Block_CanActive_ShieldActive.Get(true);
		bool blockCanActiveShieldInactive = pBlockType->Block_CanActive_ShieldInactive.Get(true);
		bool blockCanActiveMove = pBlockType->Block_CanActive_Move.Get(true);
		bool blockCanActiveStationary = pBlockType->Block_CanActive_Stationary.Get(true);

		if (pWHExt->Block_AllowOverride)
		{
			blockAffectBelowPercents = !pOtherBlock->Block_AffectBelowPercents.empty() ? pOtherBlock->Block_AffectBelowPercents : blockAffectBelowPercents;
			blockAffectsHouses = pOtherBlock->Block_AffectsHouses.isset() ? pOtherBlock->Block_AffectsHouses.Get() : blockAffectsHouses;
			blockCanActiveZeroDamage = pOtherBlock->Block_CanActive_ZeroDamage.isset() ? pOtherBlock->Block_CanActive_ZeroDamage : blockCanActiveZeroDamage;
			blockCanActiveNegativeDamage = pOtherBlock->Block_CanActive_NegativeDamage.isset() ? pOtherBlock->Block_CanActive_NegativeDamage : blockCanActiveNegativeDamage;
			blockCanActivePowered = pOtherBlock->Block_CanActive_Powered.isset() ? pOtherBlock->Block_CanActive_Powered : blockCanActivePowered;
			blockCanActiveNoFirer = pOtherBlock->Block_CanActive_NoFirer.isset() ? pOtherBlock->Block_CanActive_NoFirer : blockCanActiveNoFirer;
			blockCanActiveShieldActive = pOtherBlock->Block_CanActive_ShieldActive.isset() ? pOtherBlock->Block_CanActive_ShieldActive : blockCanActiveShieldActive;
			blockCanActiveShieldInactive = pOtherBlock->Block_CanActive_ShieldInactive.isset() ? pOtherBlock->Block_CanActive_ShieldInactive : blockCanActiveShieldInactive;
			blockCanActiveMove = pOtherBlock->Block_CanActive_Move.isset() ? pOtherBlock->Block_CanActive_Move : blockCanActiveMove;
			blockCanActiveStationary = pOtherBlock->Block_CanActive_Stationary.isset() ? pOtherBlock->Block_CanActive_Stationary : blockCanActiveStationary;
		}

		if (blockAffectBelowPercents.size() > 0 && pThis->GetHealthPercentage() > blockAffectBelowPercents[0])
			return damage;

		if (damage == 0 && !blockCanActiveZeroDamage)
			return 0;
		else if (damage < 0 && !blockCanActiveNegativeDamage)
			return damage;

		unsigned int level = 0;

		if (blockAffectBelowPercents.size() > 0)
		{
			for (; level < blockAffectBelowPercents.size() - 1; level++)
			{
				if (pThis->GetHealthPercentage() > blockAffectBelowPercents[level + 1])
					break;
			}
		}

		double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (blockChances.size() == 1)
		{
			if (blockChances[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
				return damage;
		}
		else if (blockChances.size() <= level || blockChances[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
		{
			return damage;
		}

		if (blockCanActivePowered)
		{
			bool isActive = !(pThis->Deactivated || pThis->IsUnderEMP());

			if (isActive && pThis->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass const*>(pThis);
				isActive = pBuilding->IsPowerOnline();
			}

			if (!isActive)
				return damage;
		}

		if (auto const pFoot = flag_cast_to<FootClass*>(pThis))
		{
			if (pFoot->Locomotor->Is_Really_Moving_Now())
			{
				if (!blockCanActiveMove)
					return damage;
			}
			else if (!blockCanActiveStationary)
			{
				return damage;
			}
		}

		const auto pFirer = args->Attacker;

		if (pFirer)
		{
			if (pFirer->Owner && !EnumFunctions::CanTargetHouse(blockAffectsHouses, pFirer->Owner, pThis->Owner))
				return damage;
		}
		else if (!blockCanActiveNoFirer)
		{
			return damage;
		}

		const auto pShieldData = pExt->Shield.get();

		if (pShieldData && pShieldData->IsActive())
		{
			if (!blockCanActiveShieldActive || !pShieldData->GetType()->CanBlock)
				return damage;
		}
		else if (!blockCanActiveShieldInactive)
		{
			return damage;
		}

		// a block is triggered
		auto blockAnims = pBlockType->Block_Anims;
		auto blockWeapon = pBlockType->Block_Weapon.Get();
		bool blockFlash = pBlockType->Block_Flash.Get(false);
		bool blockReflectDamage = pBlockType->Block_ReflectDamage.Get(false);
		double blockReflectDamageChance = pBlockType->Block_ReflectDamage_Chance.Get(1.0);

		if (pWHExt->Block_AllowOverride)
		{
			blockAnims = !pOtherBlock->Block_Anims.empty() ? pOtherBlock->Block_Anims : blockAnims;
			blockWeapon = pOtherBlock->Block_Weapon.isset() ? pOtherBlock->Block_Weapon.Get() : blockWeapon;
			blockFlash = pOtherBlock->Block_Flash.isset() ? pOtherBlock->Block_Flash.Get() : blockFlash;
			blockReflectDamage = pOtherBlock->Block_ReflectDamage.isset() ? pOtherBlock->Block_ReflectDamage.Get() : blockReflectDamage;
			blockReflectDamageChance = pOtherBlock->Block_ReflectDamage_Chance.isset() ? pOtherBlock->Block_ReflectDamage_Chance.Get() : blockReflectDamageChance;
		}

		if (blockAnims.size() > 0)
		{
			const int idx = blockAnims.size() > 1 ?
				ScenarioClass::Instance->Random.RandomRanged(0, blockAnims.size() - 1) : 0;

			TechnoExtData::PlayAnim(blockAnims[idx], pThis);
		}

		if (blockFlash)
		{
			int size = pBlockType->Block_Flash_FixedSize.Get(damage * 2);
			SpotlightFlags flags = SpotlightFlags::NoColor;
			bool blockFlashRed = pBlockType->Block_Flash_Red.Get(true);
			bool blockFlashGreen = pBlockType->Block_Flash_Green.Get(true);
			bool blockFlashBlue = pBlockType->Block_Flash_Blue.Get(true);
			bool blockFlashBlack = pBlockType->Block_Flash_Black.Get(false);

			if (pWHExt->Block_AllowOverride)
			{
				size = pOtherBlock->Block_Flash_FixedSize.isset() ? pOtherBlock->Block_Flash_FixedSize.Get() : size;
				blockFlashRed = pOtherBlock->Block_Flash_Red.isset() ? pOtherBlock->Block_Flash_Red.Get() : blockFlashRed;
				blockFlashGreen = pOtherBlock->Block_Flash_Green.isset() ? pOtherBlock->Block_Flash_Green.Get() : blockFlashGreen;
				blockFlashBlue = pOtherBlock->Block_Flash_Blue.isset() ? pOtherBlock->Block_Flash_Blue.Get() : blockFlashBlue;
				blockFlashBlack = pOtherBlock->Block_Flash_Black.isset() ? pOtherBlock->Block_Flash_Black.Get() : blockFlashBlack;
			}

			if (blockFlashBlack)
			{
				flags = SpotlightFlags::NoColor;
			}
			else
			{
				if (!blockFlashRed)
					flags = SpotlightFlags::NoRed;
				if (!blockFlashGreen)
					flags |= SpotlightFlags::NoGreen;
				if (!blockFlashBlue)
					flags |= SpotlightFlags::NoBlue;
			}

			MapClass::FlashbangWarheadAt(size, args->WH, pThis->Location, true, flags);
		}

		if (blockReflectDamage && blockReflectDamageChance >= ScenarioClass::Instance->Random.RandomDouble()
			&& damage > 0 && pFirer && !pWHExt->SuppressReflectDamage && !pWHExt->Reflected)
		{
			auto pWHRef = pBlockType->Block_ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
			auto blockReflectDamageAffectsHouses = pBlockType->Block_ReflectDamage_AffectsHouses.Get(blockAffectsHouses);
			Nullable<int> blockReflectDamageOverride = pBlockType->Block_ReflectDamage_Override;
			double blockReflectDamageMultiplier = pBlockType->Block_ReflectDamage_Multiplier.Get(1.0);
			bool blockReflectDamageWHDetonate = pBlockType->Block_ReflectDamage_Warhead_Detonate.Get(false);

			if (pWHExt->Block_AllowOverride)
			{
				pWHRef = pOtherBlock->Block_ReflectDamage_Warhead.isset() ? pOtherBlock->Block_ReflectDamage_Warhead.Get() : pWHRef;
				blockReflectDamageOverride = pOtherBlock->Block_ReflectDamage_Override.isset() ? pOtherBlock->Block_ReflectDamage_Override : blockReflectDamageOverride;
				blockReflectDamageAffectsHouses = pOtherBlock->Block_ReflectDamage_AffectsHouses.isset() ? pOtherBlock->Block_ReflectDamage_AffectsHouses.Get() : blockReflectDamageAffectsHouses;
				blockReflectDamageMultiplier = pOtherBlock->Block_ReflectDamage_Multiplier.isset() ? pOtherBlock->Block_ReflectDamage_Multiplier.Get() : blockReflectDamageMultiplier;
				blockReflectDamageWHDetonate = pOtherBlock->Block_ReflectDamage_Warhead_Detonate.isset() ? pOtherBlock->Block_ReflectDamage_Warhead_Detonate.Get() : blockReflectDamageWHDetonate;
			}

			int damageRef = blockReflectDamageOverride.Get(static_cast<int>(damage * blockReflectDamageMultiplier));

			if (EnumFunctions::CanTargetHouse(blockReflectDamageAffectsHouses, pThis->Owner, pFirer->Owner))
			{
				auto const pWHExtRef = WarheadTypeExtContainer::Instance.Find(pWHRef);
				pWHExtRef->Reflected = true;

				if (blockReflectDamageWHDetonate)
					WarheadTypeExtData::DetonateAt(pWHRef, pFirer, pThis, damageRef, pThis->Owner);
				else
					pFirer->ReceiveDamage(&damage, 0, pWHRef, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}
		}

		if (blockDamageMultipliers.size() == 1)
			damage = static_cast<int>(damage * (blockDamageMultipliers[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));
		else if (blockDamageMultipliers.size() > level)
			damage = static_cast<int>(damage * (blockDamageMultipliers[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));

		if (blockWeapon)
			TechnoExtData::FireWeaponAtSelf(pThis, blockWeapon);
	}

	return damage;
}

std::vector<double> TechnoExtData::GetBlockChance(TechnoClass* pThis, std::vector<double>& blockChance)
{
	std::vector<double> result = blockChance;
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (result.empty())
		result.push_back(0.0);

	double extraChance = 0.0;

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (pType->Block_ChanceMultiplier == 1.0 && pType->Block_ExtraChance == 0.0)
			continue;

		for (auto& chance : result) {
			chance = chance * MaxImpl(pType->Block_ChanceMultiplier, 0);
		}

		extraChance += pType->Block_ExtraChance;
	}

	for (auto& chance : result) {
		chance += extraChance;
	}

	return result;
}

void TechnoExtData::AddFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker)
{
	if (Attacker->InLimbo || !Attacker->IsAlive || Attacker->IsCrashing || Attacker->IsSinking)
		return;

	const int index = this->FindFirer(Weapon);
	const OnlyAttackStruct Data { Weapon ,Attacker };

	if (index < 0)
	{
		this->OnlyAttackData.push_back(Data);
	}
	else
	{
		this->OnlyAttackData[index] = Data;
	}
}

bool TechnoExtData::ContainFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker) const
{
	const int index = this->FindFirer(Weapon);

	if (index >= 0)
		return this->OnlyAttackData[index].Attacker == Attacker;

	return true;
}

int TechnoExtData::FindFirer(WeaponTypeClass* const Weapon) const
{
	const auto& AttackerDatas = this->OnlyAttackData;
	if (!AttackerDatas.empty())
	{
		for (int index = 0; index < int(AttackerDatas.size()); index++)
		{
			const auto pWeapon = AttackerDatas[index].Weapon;

			if (pWeapon == Weapon && AttackerDatas[index].Attacker)
				return index;
		}
	}

	return -1;
}

bool TechnoExtData::MultiWeaponCanFire(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pWeaponType || pWeaponType->NeverUse
		|| (pThis->InOpenToppedTransport && !pWeaponType->FireInTransport))
	{
		return false;
	}

	const auto rtti = pTarget->WhatAmI();
	const bool isBuilding = rtti == AbstractType::Building;
	const auto pWH = pWeaponType->Warhead;
	const auto pBulletType = pWeaponType->Projectile;

	const auto pTechno = flag_cast_to<TechnoClass*, true>(pTarget);
	const bool isInAir = pTechno ? pTechno->IsInAir() : false;

	const auto pOwner = pThis->Owner;
	const auto pTechnoOwner = pTechno ? pTechno->Owner : nullptr;
	const bool isAllies = pTechnoOwner ? pOwner->IsAlliedWith(pTechnoOwner) : false;

	if (isInAir)
	{
		if (!pBulletType->AA)
			return false;
	}
	else
	{
		if (BulletTypeExtContainer::Instance.Find(pBulletType)->AAOnly.Get())
		{
			return false;
		}
		else if (pWH->ElectricAssault)
		{
			if (!isBuilding || !isAllies
				|| !static_cast<BuildingClass*>(pTarget)->Type->Overpowerable)
			{
				return false;
			}
		}
		else if (pWH->IsLocomotor)
		{
			if (isBuilding)
				return false;
		}
	}

	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTechno || !isInAir)
	{
		if (auto const pObject = flag_cast_to<ObjectClass*, true>(pTarget))
			pTargetCell = pObject->GetCell();
		else if (auto const pCell = cast_to<CellClass*, true>(pTarget))
			pTargetCell = pCell;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pWeaponExt->SkipWeaponPicking)
	{
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true, true))
			return false;

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget)
				|| !EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTechnoOwner)
				|| !TechnoExtData::ObjectHealthAllowFiring(pTechno, pWeaponType)
				|| !pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
			{
				return false;
			}
		}
	}

	if (PassengersFunctional::CanFire(pThis))
		return false;

	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeaponType->Warhead))
		return false;

	if (!TechnoExtData::InterceptorAllowFiring(pThis, pTechno))
		return false;

	if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
		if (GeneralUtils::GetWarheadVersusArmor(pWH, TechnoExtData::GetTechnoArmor(pObj, pWH)) == 0.0)
			return false;
	}

	if (pTechno)
	{
		auto pTechnoType = pTechno->GetTechnoType();

		if (pTechnoType->Immune && !pWHExt->IsFakeEngineer) {
			return false;
		}

		if (pThis->Berzerk && !EnumFunctions::CanTargetHouse(RulesExtData::Instance()->BerzerkTargeting, pThis->Owner, pTechno->Owner))
			return false;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pTechno, pWeaponType))
			return false;

		if (pTechno->AttachedBomb ? pWH->IvanBomb : pWH->BombDisarm)
			return false;

		if (!pWH->Temporal && pTechno->BeingWarpedOut)
			return false;

		if (pWH->Parasite
			&& (isBuilding || static_cast<FootClass*>(pTechno)->ParasiteEatingMe))
		{
			return false;
		}

		if (pWH->MindControl
			&& (pTechnoType->ImmuneToPsionics || pTechno->IsMindControlled() || pOwner == pTechnoOwner))
		{
			return false;
		}

		if (pWeaponType->DrainWeapon
			&& (!pTechnoType->Drainable || pTechno->DrainingMe || isAllies))
		{
			return false;
		}

		if (pWH->Airstrike)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, WarheadTypeExtContainer::Instance.Find(pWH)->AirstrikeTargets))
				return false;

			const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

			if (pTechno->AbstractFlags & AbstractFlags::Foot)
			{
				if (!pTechnoTypeExt->AllowAirstrike.Get(true))
					return false;
			}
			else if (pTechnoTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTechnoType)->CanC4)
				&& (!pTechnoType->ResourceDestination || !pTechnoType->ResourceGatherer))
			{
				return false;
			}
		}
	}
	else if (rtti == AbstractType::Cell)
	{
		if (pTargetCell->OverlayTypeIndex >= 0)
		{
			const auto pOverlayType = OverlayTypeClass::Array->Items[pTargetCell->OverlayTypeIndex];

			if (pOverlayType->Wall && !pWH->Wall && (!pWH->Wood || pOverlayType->Armor != Armor::Wood))
				return false;
		}
	}
	else if (rtti == AbstractType::Terrain)
	{
		if (!pWH->Wood)
			return false;
	}

	return true;
}

bool TechnoExtData::IsHealthInThreshold(ObjectClass* pObject, double min, double max) {

	if (!pObject->Health && !pObject->GetType()->Strength)
		return true;

	double hp = pObject->GetHealthPercentage();
	return (hp > 0 ? hp > min : hp >= min) && hp <= max;
}

std::tuple<bool, bool, bool> TechnoExtData::CanBeAffectedByFakeEngineer(TechnoClass* pThis, TechnoClass* pTarget, bool checkBridge, bool checkCapturableBuilding, bool checkAttachedBombs) {

	const int nWeaponIndex = pThis->SelectWeapon(pTarget);

	if (nWeaponIndex < 0)
		return { false , false , false };

	const auto pWeapon = pThis->GetWeapon(nWeaponIndex)->WeaponType;

	if (!pWeapon)
		return { false , false , false };

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
	bool canAffectCapturableBuildings = false;
	bool canAffectBridges = false;
	bool canAffectAttachedBombs = false;

	//Check if an attached bomb can be disarmed
	if (checkAttachedBombs
		&& pWHExt->FakeEngineer_BombDisarm
		&& pTarget->AttachedBomb)
	{
		canAffectAttachedBombs = true;
	}

	const auto pBuilding = cast_to<BuildingClass* , false>(pTarget);
	bool isBuilding = pBuilding && pBuilding->IsAlive && pBuilding->Health > 0;

	// Check if a Bridge Repair Hut can be affected
	if (checkBridge && isBuilding && pBuilding->Type->BridgeRepairHut)
	{
		CellStruct bridgeRepairHutCell = CellClass::Coord2Cell(pBuilding->GetCenterCoords());
		bool isBridgeDamaged = MapClass::Instance->IsLinkedBridgeDestroyed(bridgeRepairHutCell);

		if ((isBridgeDamaged && pWHExt->FakeEngineer_CanRepairBridges)
		||  (!isBridgeDamaged && pWHExt->FakeEngineer_CanDestroyBridges)) {
			canAffectBridges = true;
		}
	}

	// Check if a capturable building can be affected
	if (checkCapturableBuilding
		&& isBuilding
		&& pWHExt->FakeEngineer_CanCaptureBuildings
		&& (pBuilding->Type->Capturable || pBuilding->Type->NeedsEngineer)
		&& !pThis->Owner->IsAlliedWith(pBuilding)) // Anti-crash check
	{
		canAffectCapturableBuildings = true;
	}

	return  { canAffectCapturableBuildings , canAffectBridges , canAffectAttachedBombs };
}

bool TechnoExtData::CannotMove(UnitClass* pThis)
{
	const auto pType = pThis->Type;

	if (pType->Speed <= 0)
		return true;

	if (!locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		LandType landType = pThis->GetCell()->LandType;
		const LandType movementRestrictedTo = pType->MovementRestrictedTo;

		if (pThis->OnBridge
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			landType = LandType::Road;
		}

		if (movementRestrictedTo != LandType::None && movementRestrictedTo != landType && landType != LandType::Tunnel)
			return true;
	}

	return false;
}

// Check adjacent cells from the center
// The current MapClass::Instance->PlacePowerupCrate(...) doesn't like slopes and maybe other cases
bool TechnoExtData::TryToCreateCrate(CoordStruct location, PowerupEffects selectedPowerup, int maxCellRange)
{
	CellStruct centerCell = CellClass::Coord2Cell(location);
	short currentRange = 0;
	bool placed = false;

	do
	{
		short x = -currentRange;
		short y = -currentRange;

		CellStruct checkedCell;
		checkedCell.Y = centerCell.Y + y;

		// Check upper line
		for (short i = -currentRange; i <= currentRange; i++)
		{
			checkedCell.X = centerCell.X + i;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.Y = centerCell.Y + Math::abs(y);

		// Check lower line
		for (short i = -currentRange; i <= currentRange; i++)
		{
			checkedCell.X = centerCell.X + i;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.X = centerCell.X + x;

		// Check left line
		for (short j = -currentRange + 1; j < currentRange; j++)
		{
			checkedCell.Y = centerCell.Y + j;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		if (placed)
			break;

		checkedCell.X = centerCell.X + Math::abs(x);

		// Check right line
		for (short j = -currentRange + 1; j < currentRange; j++)
		{
			checkedCell.Y = centerCell.Y + j;
			placed = MapClass::Instance->Place_Crate(checkedCell, selectedPowerup);

			if (placed)
				break;
		}

		currentRange++;
	}
	while (!placed && currentRange < maxCellRange);

	if (!placed)
		Debug::LogInfo(__FUNCTION__": Failed to place a crate in the cell ({},{}) and around that location.", centerCell.X, centerCell.Y, maxCellRange);

	return placed;
}

void TechnoExtData::UpdateRecountBurst() {
	const auto pThis = This();
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->CurrentBurstIndex && !pThis->Target && pTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst)) {
		const auto pWeapon = this->LastWeaponType;
		if (pWeapon && pWeapon->Burst && pThis->LastFireBulletFrame + std::max(pWeapon->ROF, 30) <= Unsorted::CurrentFrame) {


			const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pWeapon->Burst;
			const auto rof = static_cast<int>(ratio * pWeapon->ROF * this->AE.ROFMultiplier) - std::max(pWeapon->ROF, 30);

			if (rof > 0) {
				pThis->ROF = rof;
				pThis->RearmTimer.Start(rof);
			}

			pThis->CurrentBurstIndex = 0;
		}
	}
}

void TechnoExtData::UpdateRearmInEMPState()
{
	const auto pThis = This();
	const bool underEMP = pThis->IsUnderEMP();

	if (!underEMP && !pThis->Deactivated)
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_UnderEMP.Get(RulesExtData::Instance()->NoRearm_UnderEMP))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_UnderEMP.Get(RulesExtData::Instance()->NoReload_UnderEMP))
		pThis->ReloadTimer.StartTime++;

		// Pause building factory production under EMP / Deactivated (AI only)
	if (auto const pBuilding = cast_to<BuildingClass*,false>(pThis)) {
		if (pBuilding->Owner && !pBuilding->Owner->IsControlledByHuman()) {
			if (auto const pFactory = pBuilding->Factory) {
				if (pFactory->Production.Timer.InProgress()) {
					pFactory->Production.Timer.StartTime++;
				}
			}
		}
	}
}

void TechnoExtData::UpdateRearmInTemporal()
{
	const auto pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_Temporal.Get(RulesExtData::Instance()->NoRearm_Temporal))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_Temporal.Get(RulesExtData::Instance()->NoReload_Temporal))
		pThis->ReloadTimer.StartTime++;
}

void TechnoExtData::ResetDelayedFireTimer()
{
	this->DelayedFireTimer.Stop();
	this->DelayedFireWeaponIndex = -1;

	if (this->CurrentDelayedFireAnim) {
		if (AnimExtContainer::Instance.Find(this->CurrentDelayedFireAnim)->DelayedFireRemoveOnNoDelay)
			this->CurrentDelayedFireAnim.reset(nullptr);
	}
}

void TechnoExtData::CreateDelayedFireAnim(AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool useOffsetOverride, CoordStruct offsetOverride)
{
	if (pAnimType)
	{
		auto coords = This()->GetCenterCoords();

		if (useOffsetOverride)
			this->CustomFiringOffset = offsetOverride;

		if (!center)
			This()->GetFLH(&coords, weaponIndex, coords);

		if (useOffsetOverride)
			this->CustomFiringOffset.reset();

		auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);

		if (attach)
			pAnim->SetOwnerObject(This());

		auto const pAnimExt = AnimExtContainer::Instance.Find(pAnim);
		pAnim->Owner = This()->Owner;
		pAnimExt->Invoker = This();

		if (attach)
		{
			pAnimExt->DelayedFireRemoveOnNoDelay = removeOnNoDelay;
			this->CurrentDelayedFireAnim.reset(pAnim);
		}
	}
}

bool TechnoExtData::HandleDelayedFireWithPauseSequence(TechnoClass* pThis, int weaponIndex, int firingFrame)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto& timer = pExt->DelayedFireTimer;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
	{
		pExt->ResetDelayedFireTimer();
		pExt->DelayedFireSequencePaused = false;
	}

	if (pWeaponExt->DelayedFire_PauseFiringSequence && pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
	{
		if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
		{
			if (pThis->Animation.Stage == firingFrame)
				pExt->DelayedFireSequencePaused = true;

			if (!timer.HasStarted())
			{
				pExt->DelayedFireWeaponIndex = weaponIndex;
				timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
				auto pAnimType = pWeaponExt->DelayedFire_Animation;

				if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
					pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation;

				auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

				if (pWeaponExt->DelayedFire_AnimOffset.isset())
					firingCoords = pWeaponExt->DelayedFire_AnimOffset;

				pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

				if (pWeaponExt->DelayedFire_InitialBurstSymmetrical)
					pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
						pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, {firingCoords.X, -firingCoords.Y, firingCoords.Z});

				return true;
			}
			else if (timer.InProgress())
			{
				return true;
			}

			if (timer.Completed())
				pExt->ResetDelayedFireTimer();
		}

		pExt->DelayedFireSequencePaused = false;
	}

	return false;
}

void TechnoExtData::UpdateGattlingRateDownReset()
{
	if (this->Type->IsGattling)
	{
		const auto pThis = This();

		if (TechnoTypeExtContainer::Instance.Find(this->Type)->RateDown_Reset
				&& (!pThis->Target || this->LastTargetID != pThis->Target->UniqueID))
		{
			int oldStage = pThis->CurrentGattlingStage;
			this->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			this->AccumulatedGattlingValue = 0;
			this->ShouldUpdateGattlingValue = false;

			if (oldStage != 0)
			{
				pThis->GattlingRateDown(0);
			}
		}
	}
}

void TechnoExtData::SetChargeTurretDelay(TechnoClass* pThis, int rearmDelay, WeaponTypeClass* pWeapon)
{
	pThis->ROF = rearmDelay;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!pWeaponExt->ChargeTurret_Delays.empty())
	{
		size_t burstIndex = pWeapon->Burst > 1 ? pThis->CurrentBurstIndex - 1 : 0;
		size_t index = burstIndex < pWeaponExt->ChargeTurret_Delays.size() ? burstIndex : pWeaponExt->ChargeTurret_Delays.size() - 1;
		int delay = pWeaponExt->ChargeTurret_Delays[index];

		if (delay <= 0)
			return;

		pThis->ROF = delay;
		TechnoExtContainer::Instance.Find(pThis)->ChargeTurretTimer.Start(delay);
	}
}

void TechnoExtData::ApplyKillWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if ((!pWHExt->KillWeapon  && !pWHExt->KillWeapon_OnFirer) || pTypeExt->SuppressKillWeapons)
		return;

	const auto& filter = pTypeExt->SuppressKillWeapons_Types;

	// KillWeapon can be triggered without the source
	if (pWHExt->KillWeapon && (!pSource || EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_AffectsHouses, pSource->Owner, pThis->Owner))) {
		if ((filter.empty() || !filter.Contains(pWHExt->KillWeapon)) && EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_Affects)) {
			WeaponTypeExtData::DetonateAt(pWHExt->KillWeapon, pThis, pSource, pWHExt->KillWeapon->Damage, false, nullptr);
		}
	}

	// KillWeapon.OnFirer must have a source
	if (pWHExt->KillWeapon_OnFirer && pSource && EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_OnFirer_AffectsHouses, pSource->Owner, pThis->Owner)) {
		if ((filter.empty() || !filter.Contains(pWHExt->KillWeapon_OnFirer)) && EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_Affects)){
			WeaponTypeExtData::DetonateAt(pWHExt->KillWeapon_OnFirer, pThis, pSource, pWHExt->KillWeapon->Damage, false, nullptr);
		}
	}
}

// Checks if vehicle can deploy into a building at its current location. If unit has no DeploysInto set returns noDeploysIntoDefaultValue (def = false) instead.
bool TechnoExtData::CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue)
{
	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return noDeploysIntoDefaultValue;

	bool canDeploy = true;
	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	pThis->Mark(MarkType::Remove);

	pThis->Locomotor.GetInterfacePtr()->Mark_All_Occupation_Bits((int)MarkType::Remove);

	if (!pDeployType->CanCreateHere(mapCoords, pThis->Owner))
		canDeploy = false;

	pThis->Locomotor.GetInterfacePtr()->Mark_All_Occupation_Bits((int)MarkType::Put);
	pThis->Mark(MarkType::Put);

	return canDeploy;
}

bool TechnoExtData::CanDeployIntoBuilding(UnitClass* pThis)
{
	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return true;

	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	// The vanilla game used an inappropriate approach here, resulting in potential risk of desync.
	// Now, through additional checks, we can directly exclude the unit who want to deploy.
	TechnoExtData::Deployer = pThis;
	const bool canDeploy = pDeployType->CanCreateHere(mapCoords, pThis->Owner);
	TechnoExtData::Deployer = nullptr;

	return canDeploy;
}

void TechnoExtData::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (!pTechnoTo || TechnoExtData::IsPsionicsImmune(pTechnoTo))
		return;

	const auto pBld = cast_to<BuildingClass*, false>(pTechnoTo);

	// anim must be transfered before `Free` call , because it will get invalidated !
	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		pTechnoFrom->MindControlRingAnim = nullptr;

		// kill previous anim if any
		if (pTechnoTo->MindControlRingAnim)
		{
			GameDelete<true, false>(pTechnoTo->MindControlRingAnim);
			//pTechnoTo->MindControlRingAnim->TimeToDie = true;
			//pTechnoTo->MindControlRingAnim->UnInit();
		}

		CoordStruct location = pTechnoTo->GetCoords();

		if (pBld)
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		else
			location.Z += pTechnoTo->GetTechnoType()->MindControlRingOffset;

		Anim->SetLocation(location);
		Anim->SetOwnerObject(pTechnoTo);

		if (pBld)
			Anim->ZAdjust = -1024;

		pTechnoTo->MindControlRingAnim = Anim;
	}

	if (const auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}
	else if (pTechnoTo->MindControlledByAUnit && !pTechnoFrom->MindControlledBy)
	{
		pTechnoTo->MindControlledByAUnit = pTechnoFrom->MindControlledByAUnit; //perma MC ed
	}
	else if (auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (auto Manager = Controller->CaptureManager)
		{
			const bool Succeeded =
				CaptureExtData::FreeUnit(Manager, pTechnoFrom, true)
				&& CaptureExtData::CaptureUnit(Manager, pTechnoTo, false, true, nullptr, 0);

			if (Succeeded)
			{
				TechnoExtContainer::Instance.Find(pTechnoTo)->BeControlledThreatFrame = TechnoExtContainer::Instance.Find(pTechnoFrom)->BeControlledThreatFrame;

				if (pBld)
				{
					// Capturing the building after unlimbo before buildup has finished or even started appears to throw certain things off,
					// Hopefully this is enough to fix most of it like anims playing prematurely etc.
					pBld->ActuallyPlacedOnMap = false;
					pBld->DestroyNthAnim(BuildingAnimSlot::All);
					pTechnoTo->QueueMission(Mission::Construction, 0);
					pTechnoTo->Mission_Construction();
				}
			}
		}
	}
}

Point2D TechnoExtData::GetScreenLocation(TechnoClass* pThis)
{
	CoordStruct absolute = pThis->GetCoords();
	Point2D position = TacticalClass::Instance->CoordsToScreen(absolute);
	position -= TacticalClass::Instance->TacticalPos;

	return position;
}

Point2D TechnoExtData::GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor)
{
	int length = 17;
	Point2D position = GetScreenLocation(pThis);

	if (pThis->WhatAmI() == AbstractType::Infantry)
		length = 8;

	RectangleStruct bracketRect =
	{
		position.X - length + (length == 8) + 1,
		position.Y - 28 + (length == 8),
		length * 2,
		length * 3
	};

	return anchor.OffsetPosition(bracketRect);
}

Point2D TechnoExtData::GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition , Point2D offset)
{
	const auto pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
	Point2D position = GetScreenLocation(pThis);
	CoordStruct dim2 = CoordStruct::Empty;
	pBuildingType->Dimension2(&dim2);
	dim2 = { -dim2.X / 2, dim2.Y / 2, dim2.Z };
	Point2D positionFix = TacticalClass::Instance->CoordsToScreen(dim2);

	const int foundationWidth = pBuildingType->GetFoundationWidth();
	const int foundationHeight = pBuildingType->GetFoundationHeight(false);
	const int height = pBuildingType->Height * 12;
	const int lengthW = foundationWidth * 7 + foundationWidth / 2;
	const int lengthH = foundationHeight * 7 + foundationHeight / 2;

	position.X += positionFix.X + 3 + lengthH * 4;
	position.Y += positionFix.Y + 4 - lengthH * 2;

	switch (bracketPosition)
	{
	case BuildingSelectBracketPosition::Top:
		break;
	case BuildingSelectBracketPosition::LeftTop:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2;
		break;
	case BuildingSelectBracketPosition::LeftBottom:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::Bottom:
		position.Y += lengthW * 2 + lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightBottom:
		position.X += lengthW * 4;
		position.Y += lengthW * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightTop:
		position.X += lengthW * 4;
		position.Y += lengthW * 2;
		break;
	default:
		break;
	}

		return position + offset;
	}

std::vector<DigitalDisplayTypeClass*>* TechnoExtData::GetDisplayType(TechnoClass* pThis, TechnoTypeClass* pType, int& length) {
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			return &RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			return &RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Unit:
		{
			return &RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Aircraft:
		{
			return &RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes;
		}
		default:
		{
			return nullptr;
		}

		}
	}

	return &pTypeExt->DigitalDisplayTypes;
}

static bool GetDisplayTypeData(std::vector<DigitalDisplayTypeClass*>* ret , TechnoClass* pThis , TechnoTypeClass* pType, int& length)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			ret = &RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
			return true;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			ret = &(RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Unit:
		{
			ret = &(RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Aircraft:
		{
			ret = &(RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes);
			return true;
		}
		default:
		{
			return false;
		}

		}
	}

	ret = &(pTypeExt->DigitalDisplayTypes);
	return true;
}

void TechnoExtData::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	auto pType = pThis->GetTechnoType();

	if (TechnoTypeExtContainer::Instance.Find(pType)->DigitalDisplay_Disable)
		return;

	int length = 17;
	if (const auto DisplayTypes = TechnoExtData::GetDisplayType(pThis, pType, length)) {

		const auto pExt = TechnoExtContainer::Instance.Find(pThis);
		const auto What = pThis->WhatAmI();
		const bool isBuilding = What == AbstractType::Building;
		const bool isInfantry = What == AbstractType::Infantry;

		for (auto &pDisplayType : *DisplayTypes) {

			if (!pDisplayType->CanShow(pThis))
				continue;

			int value = -1;
			int maxValue = 0;

			TechnoExtData::GetValuesForDisplay(pThis, pDisplayType->InfoType, value, maxValue, pDisplayType->InfoIndex);

			if (value <= -1 || maxValue <= 0)
				continue;

			const auto divisor = pDisplayType->ValueScaleDivisor.Get(pDisplayType->ValueAsTimer ? 15 : 1);

			if (divisor > 1) {
				value = MaxImpl(value / divisor, value ? 1 : 0);
				maxValue = MaxImpl(maxValue / divisor, 1);
			}

			const bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
			Point2D position = isBuilding ? GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
				: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);

			position.Y += pType->PixelSelectionBracketDelta;

			if (pDisplayType->InfoType == DisplayInfoType::Shield)
				position.Y += pExt->CurrentShieldType->BracketDelta;

			pDisplayType->Draw(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
		}
	}
}

void GetDigitalDisplayFakeHealth(TechnoClass* pThis, int& value, int& maxValue) {
	if (TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->DigitalDisplay_Health_FakeAtDisguise) {
		if(auto pType = cast_to<TechnoTypeClass*>(pThis->Disguise)){
			const int newMaxValue = pType->Strength;
			const double ratio = static_cast<double>(value) / maxValue;
			value = static_cast<int>(ratio * newMaxValue);
			maxValue = newMaxValue;
		}
	}
}

// https://github.com/Phobos-developers/Phobos/pull/1287
// TODO : update
void TechnoExtData::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex)
{
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	case DisplayInfoType::Shield:
	{
		if (!pExt->Shield || pExt->Shield->IsBrokenAndNonRespawning())
			return;

		value = pExt->Shield->GetHP();
		maxValue = pExt->Shield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		if (pType->Ammo <= 0)
			return;

		value = pThis->Ammo;
		maxValue = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		if (!pThis->CaptureManager)
			return;

		value = pThis->CaptureManager->ControlNodes.Count;
		maxValue = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		if (infoIndex == 1)
			value = pSpawnManager->CountDockedSpawns();
		else if (infoIndex == 2)
			value = pSpawnManager->CountLaunchingSpawns();
		else
			value = pSpawnManager->CountAliveSpawns();

		maxValue = pType->SpawnsNumber;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		if (pType->Passengers <= 0)
			return;

		value = TechnoTypeExtContainer::Instance.Find(pType)->Passengers_BySize ? pThis->Passengers.NumPassengers : pThis->Passengers.GetTotalSize();
		maxValue = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (pType->Storage <= 0)
			return;

		auto& tib = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

		if (infoIndex && infoIndex <= TiberiumClass::Array->Count)
			value = static_cast<int>(tib.GetAmount(infoIndex - 1));
		else
			value = static_cast<int>(tib.GetAmounts());

		maxValue = pType->Storage;
		break;
	}
	case DisplayInfoType::Experience:
	{
		if (!pType->Trainable)
			return;

		value = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetActualCost(pThis->Owner));
		maxValue = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetActualCost(pThis->Owner));
		break;
	}
	case DisplayInfoType::Occupants:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (!pBuildingType->CanBeOccupied)
			return;

		value = pBuilding->Occupants.Count;
		maxValue = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->GattlingValue ? pThis->CurrentGattlingStage + 1 : 0;
		maxValue = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::IronCurtain:
	{
		if (!pThis->IsIronCurtained())
			return;

		value = pThis->IronCurtainTimer.GetTimeLeft();
		maxValue = pThis->IronCurtainTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::TemporalLife:
	{
		const auto pTemporal = pThis->TemporalTargetingMe;

		if (!pTemporal)
			return;

		value = pTemporal->WarpRemaining;
		maxValue = pType->Strength * 10;
		break;
	}
	case DisplayInfoType::DisableWeapon:
	{
		auto& nTimer = pExt->DisableWeaponTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::SelfHealCombatDelay:
	{
		auto& nTimer = pExt->SelfHealing_CombatDelay;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::CloakDisable :
	{
		auto& nTimer = pExt->CloakSkipTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::GattlingCount:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->GattlingValue;
		maxValue = (pThis->Veterancy.IsElite() ? pType->EliteStage : pType->WeaponStage)[pThis->CurrentGattlingStage];
		break;
	}
	case DisplayInfoType::ROF:
	{
		if (!pThis->IsArmed())
			return;

		value = pThis->RearmTimer.GetTimeLeft();
		maxValue = pThis->ROF;
		break;
	}
	case DisplayInfoType::Reload:
	{
		if (pType->Ammo <= 0)
			return;

		value = (pThis->Ammo >= pType->Ammo) ? 0 : pThis->ReloadTimer.GetTimeLeft();
		maxValue = pThis->ReloadTimer.TimeLeft ? pThis->ReloadTimer.TimeLeft : ((pThis->Ammo || pType->EmptyReload <= 0) ? pType->Reload : pType->EmptyReload);
		break;
	}
	case DisplayInfoType::FactoryProcess:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		auto getFactory = [pThis, pType, infoIndex]() -> FactoryClass*
			{
				const auto pHouse = pThis->Owner;
				const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);

				if (infoIndex == 1)
				{
					if (!pHouse->IsControlledByHuman())
						return static_cast<BuildingClass*>(pThis)->Factory;
					else if (pThis->IsPrimaryFactory)
						return pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);
				}
				else if (infoIndex == 2)
				{
					if (pHouse->IsControlledByHuman() && pThis->IsPrimaryFactory && pBuildingType->Factory == AbstractType::BuildingType)
						return pHouse->Primary_ForDefenses;
				}
				else if (!pHouse->IsControlledByHuman())
				{
					return static_cast<BuildingClass*>(pThis)->Factory;
				}
				else if (pThis->IsPrimaryFactory)
				{
					const auto pFactory = pHouse->GetPrimaryFactory(pBuildingType->Factory, pBuildingType->Naval, BuildCat::DontCare);

					if (pFactory && pFactory->Object)
						return pFactory;
					else if (pBuildingType->Factory == AbstractType::BuildingType)
						return pHouse->Primary_ForDefenses;
				}

				return nullptr;
			};
		if (const auto pFactory = getFactory())
		{
			if (pFactory->Object)
			{
				value = pFactory->GetProgress();
				maxValue = 54;
			}
		}

		break;
	}
	case DisplayInfoType::SpawnTimer:
	{
		const auto pSpawnManager = pThis->SpawnManager;

		if (!pSpawnManager || !pType->Spawns || pType->SpawnsNumber <= 0)
			return;

		if (infoIndex && infoIndex <= pSpawnManager->SpawnedNodes.Count)
		{
			value = pSpawnManager->SpawnedNodes[infoIndex - 1]->NodeSpawnTimer.GetTimeLeft();
		}
		else
		{
			for (int i = 0; i < pSpawnManager->SpawnedNodes.Count; ++i)
			{
				const auto pSpawnNode = pSpawnManager->SpawnedNodes[i];

				if (pSpawnNode->Status == SpawnNodeStatus::Dead)
				{
					const int time = pSpawnNode->NodeSpawnTimer.GetTimeLeft();

					if (!value || time < value)
						value = time;
				}
			}
		}

		maxValue = pSpawnManager->RegenRate;
		break;
	}
	case DisplayInfoType::GattlingTimer:
	{
		if (!pType->IsGattling)
			return;

		const auto thisStage = pThis->CurrentGattlingStage;
		const auto& stage = pThis->Veterancy.IsElite() ? pType->EliteStage : pType->WeaponStage;

		value = pThis->GattlingValue;
		maxValue = stage[thisStage];

		if (thisStage > 0)
		{
			value -= stage[thisStage - 1];
			maxValue -= stage[thisStage - 1];
		}

		break;
	}
	case DisplayInfoType::ProduceCash:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (pBuildingType->ProduceCashAmount <= 0)
			return;

		value = pBuilding->CashProductionTimer.GetTimeLeft();
		maxValue = pBuilding->CashProductionTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::PassengerKill:
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (!pTypeExt->PassengerDeletionType.Enabled)
			return;

		value = pExt->PassengerDeletionTimer.GetTimeLeft();
		maxValue = pExt->PassengerDeletionTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::AutoDeath:
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->Death_Method == KillMethod::None)
			return;

		if (pTypeExt->Death_Countdown > 0)
		{
			value = pExt->Death_Countdown.GetTimeLeft();
			maxValue = pExt->Death_Countdown.TimeLeft;
		}
		else if (pTypeExt->Death_NoAmmo && pType->Ammo > 0)
		{
			value = pThis->Ammo;
			maxValue = pType->Ammo;
		}

		break;
	}
	case DisplayInfoType::SuperWeapon:
	{
		if (pThis->WhatAmI() != AbstractType::Building || !pThis->Owner)
			return;

		auto getSuperTimer = [pThis, pType, infoIndex]() -> CDTimerClass*
			{
				const auto pHouse = pThis->Owner;
				const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
				const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

				if (infoIndex && infoIndex <= pBuildingTypeExt->GetSuperWeaponCount())
				{
					if (infoIndex == 1)
					{
						if (pBuildingType->SuperWeapon != -1)
							return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon)->RechargeTimer;
					}
					else if (infoIndex == 2)
					{
						if (pBuildingType->SuperWeapon2 != -1)
							return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon2)->RechargeTimer;
					}
					else
					{
						const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
						return &pHouse->Supers.GetItem(superWeapons[infoIndex - 3])->RechargeTimer;
					}

					return nullptr;
				}

				if (pBuildingType->SuperWeapon != -1)
					return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon)->RechargeTimer;
				else if (pBuildingType->SuperWeapon2 != -1)
					return &pHouse->Supers.GetItem(pBuildingType->SuperWeapon2)->RechargeTimer;

				const auto& superWeapons = pBuildingTypeExt->SuperWeapons;
				return superWeapons.size() > 0 ? &pHouse->Supers.GetItem(superWeapons[0])->RechargeTimer : nullptr;
			};

			if (const auto pTimer = getSuperTimer())
			{
				value = pTimer->GetTimeLeft();
				maxValue = pTimer->TimeLeft;
			}
		break;
	}
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;

		if (pThis->Disguised && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			GetDigitalDisplayFakeHealth(pThis, value, maxValue);

		break;
	}
	}
}

void TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTarget = std::exchange(pExt->WebbyLastTarget, nullptr);

	if (pTarget)
		pThis->Override_Mission(pExt->WebbyLastMission, pTarget, pTarget);
	else
		pThis->QueueMission(pThis->Owner->IsControlledByHuman() ? Mission::Guard : Mission::Hunt ,true);
}

void TechnoExtData::StoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->WebbyLastMission = pThis->GetCurrentMission();
	pExt->WebbyLastTarget = pThis->Target;
}

//https://blueprints.launchpad.net/ares/+spec/elite-armor
Armor TechnoExtData::GetArmor(ObjectClass* pThis) {
	const auto pType = pThis->GetType();
	Armor res = pType->Armor;

	if(pThis->AbstractFlags & AbstractFlags::Techno){

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)pType);

		if (((TechnoClass*)pThis)->Veterancy.IsVeteran() && pTypeExt->VeteranArmor.isset())
			res = pTypeExt->VeteranArmor;
		else if (((TechnoClass*)pThis)->Veterancy.IsElite() && pTypeExt->EliteArmor.isset())
			res = pTypeExt->EliteArmor;

		if(pThis->WhatAmI() == AbstractType::Infantry) {
			if (((InfantryClass*)pThis)->IsDeployed() && pTypeExt->DeployedArmor.isset()) {
				res = pTypeExt->DeployedArmor;
			}
		}
	}

	//Debug::LogInfo("{} Armor [{} = {}]", pType->ID, res, ArmorTypeClass::Array[(int)res]->Name.data());

	return res;
}

void TechnoExtData::StoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pExt->HijackerLastDisguiseType = (InfantryTypeClass*)pThis->GetDisguise(true);
	pExt->HijackerLastDisguiseHouse = pThis->GetDisguiseHouse(true);
}

void TechnoExtData::RestoreStoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pThis->ClearDisguise();
	pThis->Disguise = pExt->HijackerLastDisguiseType;
	pThis->DisguisedAsHouse = pExt->HijackerLastDisguiseHouse;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary)
{
	if (!pThis)
		return nullptr;

	auto const pType = pThis->GetTechnoType();
	weaponIndex = getSecondary ? 1 : 0;

	if (pType->TurretCount > 0 && !pType->IsGattling)
	{
		if (getSecondary)
		{
			weaponIndex = -1;
			return nullptr;
		}

		weaponIndex = pThis->CurrentWeaponNumber >= 0 ? pThis->CurrentWeaponNumber : 0;
	}
	else if (pType->IsGattling)
	{
		weaponIndex = pThis->CurrentGattlingStage * 2 + weaponIndex;
	}

	//Debug::LogInfo("{} Getting WeaponIndex {} for {}", __FUNCTION__, weaponIndex, pThis->get_ID());
	if(const auto pWpStr = pThis->GetWeapon(weaponIndex))
		return pWpStr->WeaponType;

	return nullptr;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, bool getSecondary)
{
	int weaponIndex = 0;
	return TechnoExtData::GetCurrentWeapon(pThis, weaponIndex, getSecondary);
}

bool TechnoExtData::IsCullingImmune(TechnoClass* pThis)
{
	return HasAbility(pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(TechnoClass* pThis)
{
	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	if (AresEMPulse::IsTypeEMPProne(pThis))
		return true;

	return HasAbility(pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExtData::IsCritImmune(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(FootClass* pThis)
{
	if (!pThis || !pThis->IsWarpingIn())
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExtData::IsPsionicsWeaponImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExtData::IsPoisonImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExtData::IsBerserkImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(pThis, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::IsUntrackable(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Untrackable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Untrackable);
}

bool TechnoExtData::ISC4Holder(InfantryClass* pThis) {

	if (pThis->Type->C4)
		return true;

	return pThis->HasAbility(AbilityType::C4);
}

bool TechnoExtData::IsInterceptor()
{
	auto const pThis = This();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Interceptor)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Interceptor);
}

void TechnoExtData::CreateInitialPayload(bool forced)
{
	auto const pThis = This();
	auto const pType = pThis->GetTechnoType();

	//if (IS_SAME_STR_("FTNKT", pType->ID))
	//	Debug::Log("FTNKT Check\n");

	if (!forced) {
		if (this->PayloadTriggered) {
			return;
		}

		this->PayloadTriggered = true;
	}

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->InitialPayload_Types.empty())
		return;

	const bool re_AppyAcademyBonusses = Unsorted::ScenarioInit && (!pType->UndeploysInto || !pType->DeploysInto);
	auto const pBld = cast_to<BuildingClass*, false>(pThis);

	auto freeSlots = (pBld && pBld->Type->CanBeOccupied)
		? pBld->Type->MaxNumberOccupants - pBld->GetOccupantCount()
		: pType->Passengers - pThis->Passengers.NumPassengers;

	auto const sizePayloadNum = pTypeExt->InitialPayload_Nums.size();
	auto const sizePayloadRank = pTypeExt->InitialPayload_Vet.size();
	auto const sizePyloadAddTeam = pTypeExt->InitialPayload_AddToTransportTeam.size();

	for (size_t i = 0u; i < pTypeExt->InitialPayload_Types.size(); ++i)
	{
		auto const pPayloadType = pTypeExt->InitialPayload_Types[i];

		if (!pPayloadType)
		{
			continue;
		}

		// buildings and aircraft aren't valid payload, and building payload
		// can only be infantry
		auto const absPayload = pPayloadType->WhatAmI();
		if (absPayload == AbstractType::BuildingType
			|| absPayload == AbstractType::AircraftType
			|| (pBld && absPayload != AbstractType::InfantryType))
		{
			continue;
		}

		// if there are no nums, index gets huge and invalid, which means 1
		auto const idxPayloadNum = MinImpl(i + 1, sizePayloadNum) - 1;
		auto const payloadNum = (idxPayloadNum < sizePayloadNum)
			? pTypeExt->InitialPayload_Nums[idxPayloadNum] : 1;

		auto const rank = idxPayloadNum < sizePayloadRank ?
			pTypeExt->InitialPayload_Vet[idxPayloadNum] : Rank::Invalid;

		auto const addtoteam = idxPayloadNum < sizePyloadAddTeam ?
			pTypeExt->InitialPayload_AddToTransportTeam[idxPayloadNum] : false;

		// never fill in more than allowed
		auto const count = MinImpl(payloadNum, freeSlots);
		freeSlots -= count;

		for (auto j = 0; j < count; ++j)
		{
			// clear the mutexes temporally
			// this is really dangerious that can cause issues
			// since Mutex is there to make stuffs go wrong or overlap eachother
			int mutex_old = std::exchange(Unsorted::ScenarioInit(), 0);
			auto const pObject = (TechnoClass*)pPayloadType->CreateObject(pThis->Owner);
			Unsorted::ScenarioInit = mutex_old;

			if (!pObject)
				continue;

			if (rank == Rank::Veteran)
				pObject->Veterancy.SetVeteran();
			else if (rank == Rank::Elite)
				pObject->Veterancy.SetElite();

				if(re_AppyAcademyBonusses) {
					HouseExtContainer::Instance.Find(pThis->Owner)->ApplyAcademyWithoutMutexCheck(pObject, absPayload);
				}

			if (pBld)
			{
				// buildings only allow infantry payload, so this in infantry
				auto const pPayload = static_cast<InfantryClass*>(pObject);

				if (pBld->Type->CanBeOccupied)
				{
					pBld->Occupants.AddItem(pPayload);
					auto const pCell = pThis->GetCell();
					TechnoExtContainer::Instance.Find(pPayload)->GarrisonedIn = pBld;
					pThis->UpdateThreatInCell(pCell);
				}
				else
				{
					pPayload->Limbo();

					if (pBld->Type->InfantryAbsorb)
					{
						pPayload->Absorbed = true;

						if (pPayload->CountedAsOwnedSpecial)
						{
							--pPayload->Owner->OwnedInfantry;
							pPayload->CountedAsOwnedSpecial = false;
						}

						if (pBld->Type->ExtraPowerBonus > 0)
						{
							pBld->Owner->RecheckPower = true;
						}
					}
					else
					{
						pPayload->SendCommand(RadioCommand::RequestLink, pBld);
					}

					pBld->AddPassenger(pPayload);
					pPayload->AbortMotion();
				}
			}
			else
			{
				auto const pPayload = static_cast<FootClass*>(pObject);
				pPayload->SetLocation(pThis->Location);
				pPayload->Limbo();

				if (pType->OpenTopped)
				{
					pThis->EnteredOpenTopped(pPayload);
				}

				pPayload->Transporter = pThis;

				auto const old = std::exchange(VocClass::VoicesEnabled(), false);
				pThis->AddPassenger(pPayload);
				VocClass::VoicesEnabled = old;

				if (addtoteam) {
					if (auto pTeam = ((FootClass*)pThis)->Team){
						pTeam->AddMember(pPayload, true);
					}
				}
			}
		}
	}
}

bool TechnoExtData::HasAbility(TechnoClass* pThis, PhobosAbilityType nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	if (!IsVet && !IsElite)
	{
		return false;
	}

	return HasAbility(IsVet ? Rank::Veteran : Rank::Elite, pThis, nType);
}

bool TechnoExtData::HasImmunity(TechnoClass* pThis, int nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (IsVet)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (IsElite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) ||
			pTypeExt->V_ImmuneToType.Contains((int)nType) ||
			pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

bool TechnoExtData::IsCullingImmune(Rank vet, TechnoClass* pThis)
{
	return HasAbility(vet, pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(Rank vet, TechnoClass* pThis)
{
	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	if (AresEMPulse::IsTypeEMPProne(pThis))
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExtData::IsCritImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(Rank vet, FootClass* pThis)
{
	if (!pThis)
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (!pThis->IsWarpingIn())
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExtData::IsPsionicsWeaponImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExtData::IsPoisonImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExtData::IsBerserkImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(Rank vet, InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(Rank vet, TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(vet, pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(Rank vet, TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::IsUntrackable(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Untrackable)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Untrackable);
}

bool TechnoExtData::HasAbility(Rank vet, TechnoClass* pThis, PhobosAbilityType nType)
{
	if (nType == PhobosAbilityType::None)
		return false;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->Phobos_VeteranAbilities.at((int)nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->Phobos_VeteranAbilities.at((int)nType) || pTypeExt->Phobos_EliteAbilities.at((int)nType);
	}

	return false;
}

bool TechnoExtData::HasImmunity(Rank vet, TechnoClass* pThis, int nType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains((int)nType) || pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

#include <Ext/TerrainType/Body.h>

bool TechnoExtData::IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker)
{
	if (!pVictim || !pVictim->IsAlive || !pAttacker || pVictim->IsBeingWarpedOut())
		return false;

	if (pVictim->IsIronCurtained())
		return false;

	if (pAttacker->Owner && pAttacker->Owner->IsAlliedWith(pVictim))
		return false;

	auto const pAttackerType = pAttacker->GetTechnoType();
	auto const pVictimTechno = flag_cast_to<TechnoClass*, false>(pVictim);
	auto const pAttackerTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pAttackerType);

	if (!pVictimTechno)
	{
		if (auto const pTerrain = cast_to<TerrainClass*, false>(pVictim))
		{
			if (pTerrain->Type->Immune || pTerrain->Type->SpawnsTiberium || !pTerrain->Type->Crushable)
				return false;

			const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);
			if (pTerrainExt->IsPassable)
				return false;

			return pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) > pTerrainExt->CrushableLevel;
		}

		return false;
	}

	auto const pWhatVictim = pVictim->WhatAmI();
	auto const pVictimType = pVictim->GetTechnoType();

	if (pAttackerType->OmniCrusher)
	{
		if (pWhatVictim == BuildingClass::AbsID || pVictimType->OmniCrushResistant)
			return false;
	}
	else
	{
		if (pVictimTechno->Uncrushable || !pVictimType->Crushable)
			return false;
	}

	auto const pVictimTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pVictimType);

	if (pWhatVictim == InfantryClass::AbsID)
	{
		const auto& crushableLevel = static_cast<InfantryClass*>(pVictim)->IsDeployed() ?
			pVictimTechnoTypeExt->DeployCrushableLevel :
			pVictimTechnoTypeExt->CrushableLevel;

		if (pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) < crushableLevel.Get(pVictimTechno))
			return false;
	}

	if (TechnoExtData::IsChronoDelayDamageImmune(flag_cast_to<FootClass*, false>(pVictim)))
	{
		return false;
	}

	//auto const pExt = TechnoExtContainer::Instance.Find(pVictimTechno);
	//if (auto const pShieldData = pExt->Shield.get()) {
	//	auto const pWeaponIDx = pAttacker->SelectWeapon(pVictim);
	//	auto const pWeapon = pAttacker->GetWeapon(pWeaponIDx);

	//	if (pWeapon && pWeapon->WeaponType &&
	//		pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon->WeaponType)) {
	//		return false;
	//	}
	//}

	return true;
}

AreaFireReturnFlag TechnoExtData::ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	switch (pExt->AreaFire_Target.Get())
	{
	case AreaFireTarget::Random:
	{
		std::vector<CellStruct> adjacentCells {};
		GeneralUtils::AdjacentCellsInRange(adjacentCells,
			 static_cast<short>(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) + 0.99));

		size_t const size = adjacentCells.size();

		for (int i = 0; i < (int)size; i++)
		{
			const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			CellStruct const tgtPos = pTargetCell->MapCoords + adjacentCells.at((i + rand) % size);
			CellClass* const tgtCell = MapClass::Instance->GetCellAt(tgtPos);
			bool allowBridges = tgtCell && tgtCell->ContainsBridge() && (pThis->OnBridge || (tgtCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

			if (pExt->SkipWeaponPicking || EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true , allowBridges))
			{
				pTargetCell = tgtCell;
				return AreaFireReturnFlag::Continue;
			}
		}

		return AreaFireReturnFlag::DoNotFire;
	}
	case AreaFireTarget::Self:
	{
		if(pExt->SkipWeaponPicking)
			return AreaFireReturnFlag::SkipSetTarget;

		if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false, pThis->OnBridge))
			return AreaFireReturnFlag::DoNotFire;

		return AreaFireReturnFlag::SkipSetTarget;
	}
	default:
	{
		auto pCell = pTargetCell;
		bool allowBridges = pCell && pCell->ContainsBridge() && (pThis->OnBridge || (pCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

		if (!pExt->SkipWeaponPicking && !EnumFunctions::AreCellAndObjectsEligible(pTargetCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false , allowBridges))
			return AreaFireReturnFlag::DoNotFire;
	}
	}

	return AreaFireReturnFlag::ContinueAndReturn;
}

int TechnoExtData::GetThreadPosed(TechnoClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pShieldData = pExt->GetShield()) {
		if (pShieldData->IsActive()) {
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pShiedType->ThreadPosed.Get();
		}
	}

	return pThis->GetTechnoType()->ThreatPosed;
}

bool TechnoExtData::IsReallyTechno(TechnoClass* pThis)
{
	const auto pAddr = (((DWORD*)pThis)[0]);
	if (pAddr != UnitClass::vtable
		&& pAddr != AircraftClass::vtable
		&& pAddr != InfantryClass::vtable
		&& pAddr != BuildingClass::vtable)
	{
		return false;
	}

	return true;
}

int TechnoExtData::GetDeployFireWeapon(UnitClass* pThis)
{
	if (pThis->Type->DeployFireWeapon == -1)
		return pThis->TechnoClass::SelectWeapon(pThis->Target ? pThis->Target : pThis->GetCell());

	return pThis->Type->DeployFireWeapon;
}

// Gets weapon index for a weapon to use against wall overlay.
int TechnoExtData::GetWeaponIndexAgainstWall(TechnoClass * pThis, OverlayTypeClass * pWallOverlayType)
{
	auto const pTechnoType = pThis->GetTechnoType();
	int weaponIndex = -1;
	auto pWeapon = TechnoExtData::GetCurrentWeapon(pThis, weaponIndex);

	if ((pTechnoType->TurretCount > 0 && !pTechnoType->IsGattling) || !pWallOverlayType || !pWallOverlayType->Wall)
		return weaponIndex;
	else if (weaponIndex == -1)
		return 0;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.TryFind(pWeapon);
	bool aeForbidsPrimary = pWeaponExt && pWeaponExt->AttachEffect_CheckOnFirer
	&& !pWeaponExt->SkipWeaponPicking && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

	if (!pWeapon || (!pWeapon->Warhead->Wall && (!pWeapon->Warhead->Wood || pWallOverlayType->Armor != Armor::Wood)) || TechnoExtData::CanFireNoAmmoWeapon(pThis, 1) || aeForbidsPrimary)
	{
		int weaponIndexSec = -1;
		auto pSecondaryWeapon = TechnoExtData::GetCurrentWeapon(pThis, weaponIndexSec, true);
		auto pSecondaryWeaponExt = WeaponTypeExtContainer::Instance.TryFind(pSecondaryWeapon);
		bool aeForbidsSecondary = pSecondaryWeaponExt && pSecondaryWeaponExt->AttachEffect_CheckOnFirer
		&& !pSecondaryWeaponExt->SkipWeaponPicking && !pSecondaryWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

		if (pSecondaryWeapon && (pSecondaryWeapon->Warhead->Wall || (pSecondaryWeapon->Warhead->Wood && pWallOverlayType->Armor == Armor::Wood)
			&& (!TechnoTypeExtContainer::Instance.Find(pTechnoType)->NoSecondaryWeaponFallback || aeForbidsPrimary)) && !aeForbidsSecondary)
		{
			return weaponIndexSec;
		}

		return weaponIndex;
	}

	return weaponIndex;
}

void TechnoExtData::SetMissionAfterBerzerk(TechnoClass* pThis, bool Immediete)
{
	auto const pType = pThis->GetTechnoType();

	const Mission nEndMission = pThis->IsArmed() ?
		(pThis->Owner->IsHumanPlayer ? Mission::Hunt : Mission::Guard) :
		(!pType->ResourceGatherer ? Mission::Sleep : Mission::Harvest);

	pThis->QueueMission(nEndMission, Immediete);
}

std::pair<TechnoClass*, CellClass*> TechnoExtData::GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget)
{
	TechnoClass* pTargetTechno = nullptr;
	CellClass* targetCell = nullptr;

	//pTarget nullptr check already done above this hook
	if (pTarget && pTarget->WhatAmI() == CellClass::AbsID)
	{
		return { nullptr , targetCell = static_cast<CellClass*>(pTarget) };
	}
	else if (pObjTarget)
	{
		// it is an techno target
		if (((pObjTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
		{
			pTargetTechno = static_cast<TechnoClass*>(pObjTarget);
			if (!pTargetTechno->IsInAir())	// Ignore target cell for airborne technos.
				targetCell = pTargetTechno->GetCell();
		}
		else // non techno target , but still an object
		{
			targetCell = pObjTarget->GetCell();
		}
	}

	return { pTargetTechno , targetCell };
}

bool TechnoExtData::AllowFiring(AbstractClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	//fuckoof
	return true;
}

bool TechnoExtData::ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pTargetObj)
	{
		if(pWeaponExt->Targeting_Health_Percent.isset()){
			auto const pHP = pTargetObj->GetHealthPercentage();

			if (!pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP <= pWeaponExt->Targeting_Health_Percent.Get())
				return false;
			else if (pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP >= pWeaponExt->Targeting_Health_Percent.Get())
				return false;
		}

		if(!pWeaponExt->IsHealthInThreshold(pTargetObj))
			return false;
	}

	return true;
}

bool TechnoExtData::CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon)
{	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if(!pWeaponExt->SkipWeaponPicking) {
		if (pCell && !EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true , true)) {
			return false;
		}
	}

	return true;
}

bool TechnoExtData::TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);;
	if(pWeaponExt->SkipWeaponPicking)
		return true;

	if (!EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) ||
		!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTarget->Owner) ||
		!pWeaponExt->HasRequiredAttachedEffects(pThis, pTarget))
	{
		return false;
	}

	return true;
}

bool TechnoExtData::FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);

	if (auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (!pUnit->Type->IsSimpleDeployer && !pUnit->Deployed && pTarget)
		{
			if (pUnit->Type->DeployFire && pWeapon->FireOnce)
			{
				if (pTechnoExt->DeployFireTimer.GetTimeLeft() > 0)
					return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const int nMoney = pWHExt->TransactMoney;
	if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
		return false;

	return true;
}

bool TechnoExtData::InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget)
{
	//this code is used to remove Techno as auto target consideration , so interceptor can find target faster
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTechnoExt->IsInterceptor() && pTechnoTypeExt->Interceptor_OnlyTargetBullet.Get())
	{
		if (!pTarget || pTarget->WhatAmI() == BulletClass::AbsID)
		{
			return false;
		}
	}

	return true;
}

bool TechnoExtData::TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (const auto pShieldData = pTargetTechnoExt->Shield.get())
	{
		if (pShieldData->IsActive())
		{
			if (!pShieldData->CanBePenetrated(pWeapon->Warhead))
			{
				if (pWHExt->GetVerses(pShieldData->GetType()->Armor).Verses < 0.0 && pShieldData->GetType()->CanBeHealed)
				{
					const bool IsFullHP = pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen;
					if (!IsFullHP)
						return true;
					else
					{
						if (pShieldData->GetType()->PassthruNegativeDamage)
						{
							return !(pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen);
						}
					}
				}

				return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot)
{

	if (!pFoot->IsAlive
		|| pFoot->InLimbo
		|| pFoot->IsIronCurtained()
		|| pFoot->IsSinking
		|| pFoot->IsCrashing
		|| TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled) {
		return false;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//Don't abduct the target if it has more life then the abducting percent

	if (pWeaponExt->Abductor_AbductBelowPercent < pFoot->GetHealthPercentage()) {
		return false;
	}

	if (pWeaponExt->Abductor_MaxHealth > 0 && pWeaponExt->Abductor_MaxHealth < pFoot->Health) {
		return false;
	}

	if (TechnoExtData::IsAbductorImmune(pFoot))
		return false;

	if (!TechnoExtData::IsEligibleSize(pThis, pFoot))
		return false;

	if (!TechnoTypeExtData::PassangersAllowed(pThis->GetTechnoType(), pFoot->GetTechnoType()))
		return false;

	return true;
}

void TechnoExtData::SendPlane(AircraftTypeClass* Aircraft, size_t Amount, HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest)
{
	if (!Aircraft || !pOwner || Amount <= 0)
		return;

	//safeguard
	Mission result = Mission::None;
	switch (SendMission)
	{
	case Mission::Move:
	{
		if (!pDest)
			pDest = pTarget;

		result = SendMission;
	}
	break;
	case Mission::ParadropApproach:
	case Mission::Attack:
	case Mission::SpyplaneApproach:
		result = SendMission;
		break;
	default:
		result = Mission::SpyplaneApproach;
		break;
	}

	const auto edge = pOwner->GetHouseEdge();

	for (size_t i = 0; i < Amount; ++i)
	{
		++Unsorted::ScenarioInit;
		auto const pPlane = static_cast<AircraftClass*>(Aircraft->CreateObject(pOwner));
		--Unsorted::ScenarioInit;

		if (!pPlane)
			continue ;

		pPlane->Spawned = true;
		//randomized
		const auto nCell = MapClass::Instance->PickCellOnEdge(edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
		pPlane->QueueMission(result, false);

		if (SendRank != Rank::Rookie && SendRank != Rank::Invalid && pPlane->CurrentRanking < SendRank)
			pPlane->Veterancy.SetRank(SendRank);

		if (pDest)
			pPlane->SetDestination(pDest, true);

		if (pTarget)
			pPlane->SetTarget(pTarget);

		bool UnLimboSucceeded = AircraftExtData::PlaceReinforcementAircraft(pPlane , nCell);

		if (!UnLimboSucceeded)  {
			GameDelete<true, false>(pPlane);
		}
		else
		{

			// we cant create InitialPayload when mutex atives
			// so here we handle the InitialPayload Creation !
			// this way we can make opentopped airstrike happen !
			TechnoExtContainer::Instance.Find(pPlane)->CreateInitialPayload();
			if ((TechnoTypeExtContainer::Instance.Find(pPlane->Type)->Passengers_BySize
			? pPlane->Passengers.GetTotalSize() : pPlane->Passengers.NumPassengers) > 0)
				pPlane->HasPassengers = true;

			pPlane->NextMission();
		}
	}
}

/*
 * Object should NOT be placed on the map (->Limbo() it or don't Put in the first place)
 * otherwise Bad Things (TM) will happen. Again.
 */
bool TechnoExtData::CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ)
{
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);
	if (Object->IsCellOccupied(MyCell, FacingType::None, -1, nullptr, false) != Move::OK)
	{
		return false;
	}
	else
	{
		LocomotionClass::ChangeLocomotorTo(Object, CLSIDs::DropPod);
		CoordStruct xyz = XYZ;
		xyz.Z = 0;

		Object->SetLocation(xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor->Move_To(XYZ);
		Object->PrimaryFacing.Set_Current(DirStruct()); // TODO : random or let loco set the facing

		if (!Object->InLimbo)
		{
			Object->See(0, 0);
			Object->QueueMission(Object->Owner && Object->Owner->IsControlledByHuman() ? Mission::Area_Guard : Mission::Hunt, true);
			Object->NextMission();
			return true;
		}

		return false;
	}
}

bool TechnoExtData::TargetFootAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	if ((pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = static_cast<FootClass*>(pTarget);
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		if (pWeaponExt->Abductor
			&& pWeaponExt->Abductor_CheckAbductableWhenTargeting
			&& !TechnoExtData::IsAbductable(pThis, pWeapon, pFoot
			))
			return false;

		if (auto const pUnit = cast_to<UnitClass*, false>(pTarget))
		{
			if (pUnit->DeathFrameCounter > 0)
				return false;
		}

		if (TechnoExtData::IsChronoDelayDamageImmune(pFoot))
			return false;
	}

	return true;
}

void TechnoExtData::UpdateMCOverloadDamage(TechnoClass* pOwner)
{
	auto pThis = pOwner->CaptureManager;

	if (!pThis || !pThis->InfiniteMindControl || pOwner->InLimbo)
		return;

	const auto pOwnerTypeExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());

	if (pThis->OverloadPipState > 0)
		--pThis->OverloadPipState;

	if (pThis->OverloadDamageDelay <= 0)
	{

		const auto OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesClass::Instance->OverloadCount);

		if (OverloadCount.empty())
			return;

		int nCurIdx = 0;
		const int nNodeCount = pThis->ControlNodes.Count;

		for (int i = 0; i < (int)(OverloadCount.size()); ++i)
		{
			if (nNodeCount > OverloadCount[i])
			{
				nCurIdx = i + 1;
			}
		}

		const auto nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesClass::Instance->OverloadFrames);
		pThis->OverloadDamageDelay = nOverloadfr.GetItemAtOrMax(nCurIdx);

		const auto nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesClass::Instance->OverloadDamage);
		auto nDamage = nOverloadDmg.GetItemAtOrMax(nCurIdx);

		if (nDamage <= 0)
		{
			pThis->OverloadDeathSoundPlayed = false;
		}
		else
		{
			pThis->OverloadPipState = 10;
			auto const pWarhead = pOwnerTypeExt->Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pOwner->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!pThis->OverloadDeathSoundPlayed)
			{
				VocClass::SafeImmedietelyPlayAt(pOwnerTypeExt->Overload_DeathSound.Get(RulesClass::Instance->MasterMindOverloadDeathSound), &pOwner->Location, 0);
				pThis->OverloadDeathSoundPlayed = true;
			}

			if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem))
			{
				for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto nLoc = pOwner->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { nLoc.X + nRamdomX, nRandomY + nLoc.Y, nLoc.Z };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pOwner->GetCell(), pOwner, CoordStruct::Empty, pOwner->Owner);
				}
			}

			if (nCurIdx > 0 && pOwner->IsAlive)
			{
				double const nBase = (nCurIdx != 1) ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pOwner->RockingSidewaysPerFrame = static_cast<float>(nCopied_base);
			}
		}

	}
	else
	{
		--pThis->OverloadDamageDelay;
	}
}

bool TechnoExtData::AllowedTargetByZone(TechnoClass* pThis, ObjectClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon, std::optional<std::reference_wrapper<const ZoneType>> zone)
{
	if (pThis->WhatAmI() == AircraftClass::AbsID)
		return true;

	const auto pThisType = pThis->GetTechnoType();
	const MovementZone mZone = pThisType->MovementZone;
	const ZoneType currentZone = zone ? zone.value() :
		MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), mZone, pThis->OnBridge);

	if (currentZone != ZoneType::None)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		const ZoneType targetZone =
			MapClass::Instance->GetMovementZoneType(pTarget->InlineMapCoords(), mZone, pTarget->OnBridge);

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (!pWeapon)
			{
				const int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				if (const auto pWpStruct = pThis->GetWeapon(weaponIndex))
					pWeapon = pWpStruct->WeaponType;
				else
					return false;
			}

			auto const speedType = pThisType->SpeedType;
			const auto cellStruct = MapClass::Instance->NearByLocation(pTarget->InlineMapCoords(),
				speedType, ZoneType::None, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);

			auto const pCell = MapClass::Instance->GetCellAt(cellStruct);

			if (!pCell)
				return false;

			const double distance = pCell->GetCoordsWithBridge().DistanceFrom(pTarget->GetCenterCoords());

			if (distance > pWeapon->Range)
				return false;
		}
	}

	return true;
}

//ToDo : Auto regenerate and transferable passengers (Problem : Driver killed and operator stuffs )
void TechnoExtData::PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce)
{
	if (!pTransporter || !pTransporter->Passengers.NumPassengers || !MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	//TODO : check if passenger is actually allowed to go outside
	auto pPassenger = pTransporter->Passengers.RemoveFirstPassenger();
	CoordStruct nDest = nCoord;

	//if (bForce)
	{
		//TechnoTypeClass* pPassengerType = pPassenger->GetTechnoType();
		//auto const pPassengerMZone = pPassengerType->MovementZone;
		//auto const pPassengerSpeedType = pPassengerType->SpeedType;


		//auto const pCellFrom = Map.GetCellAt(nCoord);
		//auto const nZone = Map.Zone_56D230(&pCellFrom->MapCoords, pPassengerMZone, pCellFrom->ContainsBridgeEx());

		//if (!Map[nCoord]->IsClearToMove(pPassengerSpeedType, false, false, nZone, pPassengerMZone, -1, 1))
		{
			nDest = MapClass::Instance->GetRandomCoordsNear(nCoord, ScenarioClass::Instance->Random.RandomFromMax(2000), ScenarioClass::Instance->Random.RandomFromMax(1));
		}
	}

	MapClass::Instance->GetCellAt(nCoord)->ScatterContent(pTransporter->GetCoords(), true, true, false);

	bool Placed = false;
	if (bForce)
	{
		++Unsorted::ScenarioInit;
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		--Unsorted::ScenarioInit;
	}
	else
	{
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		//Placed = CreateWithDroppod(pPassenger, nDest , LocomotionClass::CLSIDs::Teleport);
	}

	//Only remove passengers from the Transporter if it succeeded
	if (Placed)
	{
		pPassenger->Mark(MarkType::Remove);
		pPassenger->OnBridge = MapClass::Instance->GetCellAt(nCoord)->ContainsBridgeEx();
		pPassenger->Mark(MarkType::Put);
		pPassenger->StopMoving();
		pPassenger->SetDestination(nullptr, true);
		pPassenger->SetTarget(nullptr);
		pPassenger->CurrentTargets.Clear();
		pPassenger->SetArchiveTarget(nullptr);
		pPassenger->MissionAccumulateTime = 0; // don't ask
		pPassenger->unknown_5A0 = 0;
		pPassenger->CurrentGattlingStage = 0;
		pPassenger->SetCurrentWeaponStage(0);
		pPassenger->SetLocation(nDest);
		pPassenger->LiberateMember();

		if (pPassenger->SpawnManager)
		{
			pPassenger->SpawnManager->ResetTarget();
		}

		pPassenger->ClearPlanningTokens(nullptr);

		pPassenger->DiscoveredBy(pTransporter->GetOwningHouse());

		if (auto pFoot = flag_cast_to<FootClass*, false>(pTransporter))
		{
			if (pTransporter->GetTechnoType()->Gunner)
			{
				pFoot->RemoveGunner(pPassenger);
			}

			if (pTransporter->GetTechnoType()->OpenTopped)
			{
				pFoot->ExitedOpenTopped(pPassenger);
			}

			pPassenger->Transporter = nullptr;
		}
		else
		{
			auto pBuilding = static_cast<BuildingClass*>(pTransporter);

			if (pBuilding->Absorber())
			{
				pPassenger->Absorbed = false;
				pPassenger->Transporter = nullptr;
				if (pBuilding->Type->ExtraPowerBonus > 0)
				{
					pBuilding->Owner->RecheckPower = true;
				}
			}
		}

		VocClass::SafeImmedietelyPlayAt(nSound, &nDest);

		if (pAnimToPlay)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimToPlay, nDest),
				pTransporter->GetOwningHouse(),
				nullptr,
				pTransporter,
				false, false
			);
		}

		if (pPassenger->CurrentMission != Mission::Guard)
			pPassenger->Override_Mission(Mission::Area_Guard);
	}
	else
	{
		pTransporter->AddPassenger(pPassenger);
	}
}

void TechnoExtData::SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained())
	{
		bool isForceShielded = pFrom->ProtectType == ProtectTypes::ForceShield;
		const auto pTypeExt =  TechnoTypeExtContainer::Instance.Find(pFrom->GetTechnoType());
		const auto bSync = !isForceShielded ?pTypeExt->IronCurtain_KeptOnDeploy
			.Get(RulesExtData::Instance()->IronCurtain_KeptOnDeploy)
			:pTypeExt->ForceShield_KeptOnDeploy.Get(RulesExtData::Instance()->ForceShield_KeptOnDeploy)
			;

		if (bSync) {
			pFrom->IronCurtainTimer = pFrom->IronCurtainTimer;
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExtData::PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker)
{
	if (pAnim && pInvoker) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pInvoker->Location),
			pInvoker->GetOwningHouse(),
			nullptr,
			pInvoker,
			false, false
		);
	}
}

double TechnoExtData::GetArmorMult(TechnoClass* pSource, double damageIn, WarheadTypeClass* pWarhead)
{
	const auto pType = pSource->GetTechnoType();
	double _result = damageIn;

	auto const pExt = TechnoExtContainer::Instance.Find(pSource);

	if (pWarhead && pExt->AE.ArmorMultData.Enabled()) {
		_result /= pExt->AE.ArmorMultData.Get(pExt->AE.ArmorMultiplier, pWarhead);
	}

	if (auto pOwner = pSource->Owner)
		_result /= (pOwner->GetTypeArmorMult(pType) * pSource->ArmorMultiplier);

	if (pSource->HasAbility(AbilityType::Stronger)) {
		_result /= RulesClass::Instance->VeteranArmor;
	}

	return _result;
}

double TechnoExtData::GetDamageMult(TechnoClass* pSource, double damageIn , bool ForceDisable)
{
	if (ForceDisable || !pSource || !pSource->IsAlive)
		return damageIn;

	const auto pType = pSource->GetTechnoType();

	if (!pType)
		return damageIn;

	double _result = damageIn;

	if(pSource->Owner) {
		_result *= pSource->Owner->FirepowerMultiplier;
	}

	_result *= pSource->FirepowerMultiplier;

	if(pSource->HasAbility(AbilityType::Firepower)){
		_result *= RulesClass::Instance->VeteranCombat;
	}

	return _result;
}

const BurstFLHBundle* TechnoExtData::PickFLHs(TechnoClass* pThis, int weaponidx)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	std::span<BurstFLHBundle> res  = pExt->WeaponBurstFLHs;

	if (pThis->WhatAmI() == InfantryClass::AbsID) {
		if (((InfantryClass*)pThis)->IsDeployed() && !pExt->DeployedWeaponBurstFLHs.empty())
			res = pExt->DeployedWeaponBurstFLHs;
		else if (((InfantryClass*)pThis)->Crawling && !pExt->CrouchedWeaponBurstFLHs.empty())
			res = pExt->CrouchedWeaponBurstFLHs;
	}

	if (res.empty() || res.size() <= (size_t)weaponidx)
		return nullptr;

	return &res[weaponidx];
}

std::pair<bool, CoordStruct> TechnoExtData::GetBurstFLH(TechnoClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false ,  CoordStruct::Empty };

	auto pickedFLHs = PickFLHs(pThis ,weaponIndex);

	if(!pickedFLHs)
		return  { false ,  CoordStruct::Empty };

	std::span<const CoordStruct> selected = pThis->Veterancy.IsElite() ? pickedFLHs->EFlh : pickedFLHs->Flh;

	if (!selected.empty() && (size_t)pThis->CurrentBurstIndex < selected.size()) {
		return { true , selected[pThis->CurrentBurstIndex] };
	}

	return { false , CoordStruct::Empty };
}

const Nullable<CoordStruct>* TechnoExtData::GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex)
{
	const auto pTechnoType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedPrimaryFireFLH :
				&pTechnoType->DeployedPrimaryFireFLH;
		}
		else if (weaponIndex == 1)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedSecondaryFireFLH :
				&pTechnoType->DeployedSecondaryFireFLH;
		}
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
			{
				return pThis->Veterancy.IsElite() ?

					pTechnoType->E_PronePrimaryFireFLH.isset() ?
					&pTechnoType->E_PronePrimaryFireFLH :
					&pTechnoType->Elite_PrimaryCrawlFLH
					:

					pTechnoType->PronePrimaryFireFLH.isset() ?
					&pTechnoType->PronePrimaryFireFLH :
					&pTechnoType->PrimaryCrawlFLH
					;
			}
			else if (weaponIndex == 1)
			{
				return pThis->Veterancy.IsElite() ?
					pTechnoType->E_ProneSecondaryFireFLH.isset() ?
					&pTechnoType->E_ProneSecondaryFireFLH :
					&pTechnoType->E_ProneSecondaryFireFLH
					:

					pTechnoType->ProneSecondaryFireFLH.isset() ?
					&pTechnoType->ProneSecondaryFireFLH :
					&pTechnoType->SecondaryCrawlFLH
					;
			}
		}
	}

	return nullptr;
}

const Armor TechnoExtData::GetTechnoArmor(TechnoClass* pThis, WarheadTypeClass* pWarhead)
{
	Armor nArmor = TechnoExtData::GetArmor(pThis);
	TechnoExtData::ReplaceArmor(nArmor, pThis, pWarhead);
	return nArmor;
}

const Armor TechnoExtData::GetTechnoArmor(ObjectClass* pThis, WarheadTypeClass* pWarhead)
{
	if(pThis->AbstractFlags & AbstractFlags::Techno){
		return TechnoExtData::GetTechnoArmor((TechnoClass*)pThis , pWarhead);
	}

	return pThis->GetType()->Armor;
}

std::pair<bool, CoordStruct> TechnoExtData::GetInfantryFLH(InfantryClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false , CoordStruct::Empty };

	const auto pickedFLH = TechnoExtData::GetInfrantyCrawlFLH(pThis, weaponIndex);

	if (pickedFLH && pickedFLH->isset() && pickedFLH->Get().IsValid())
	{
		return { true , pickedFLH->Get() };
	}

	return{ false , CoordStruct::Empty };
}

CoordStruct TechnoExtData::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	//guarantee
	if (maxAttempts < 1)
		maxAttempts = 1;

	CellClass* pCell = pThis->GetCell();
	CellStruct placeCoords = CellStruct::Empty;
	auto pTypePassenger = pPassenger->GetTechnoType();
	SpeedType speedType = SpeedType::Track;
	MovementZone movementZone = MovementZone::Normal;

	if (pPassenger->WhatAmI() != AircraftClass::AbsID)
	{
		speedType = pTypePassenger->SpeedType;
		movementZone = pTypePassenger->MovementZone;
	}

	Point2D ExtDistance = { 1,1 };
	for (int i = 0; i < maxAttempts; ++i)
	{

		placeCoords = pCell->MapCoords - CellStruct { (short)(ExtDistance.X / 2), (short)(ExtDistance.Y / 2) };
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, ZoneType::None, movementZone, false, ExtDistance.X, ExtDistance.Y, true, false, false, false, CellStruct::Empty, false, false);

		pCell = MapClass::Instance->GetCellAt(placeCoords);

		if ((pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) == Move::OK) && MapClass::Instance->IsWithinUsableArea(pCell->GetCoordsWithBridge())) {
			return pCell->GetCoordsWithBridge();
		}

		++ExtDistance;
	}

	return CoordStruct::Empty;
}

/*
void TechnoExtData::DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry, bool sIsDisguised)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->UseCustomSelectBrd.Get(RulesExtData::Instance()->UseSelectBrd.Get(Phobos::Config::EnableSelectBrd)))
		return;

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP
		.Get(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdSHP_INF : RulesExtData::Instance()->SHP_SelectBrdSHP_UNIT);

	if (!SelectBrdSHP)
		return;

	ConvertClass* SelectBrdPAL = (pTypeExt->SHP_SelectBrdPAL ?
		pTypeExt->SHP_SelectBrdPAL :
		(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdPAL_INF : RulesExtData::Instance()->SHP_SelectBrdPAL_UNIT))
		->GetOrDefaultConvert<PaletteManager::Mode::Temperate>(FileSystem::ANIM_PAL);

	if (!SelectBrdPAL)
		return;

	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;

	const Point3D selectbrdFrame = pTypeExt->SelectBrd_Frame.Get((isInfantry ? RulesExtData::Instance()->SelectBrd_Frame_Infantry : RulesExtData::Instance()->SelectBrd_Frame_Unit));

	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExtData::Instance()->SelectBrd_DefaultTranslucentLevel.Get()));
	const auto canSee = sIsDisguised && pThis->DisguisedAsHouse ? pThis->DisguisedAsHouse->IsAlliedWith(HouseClass::CurrentPlayer) :
		pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer)
		|| HouseClass::CurrentPlayer->IsObserver()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExtData::Instance()->SelectBrd_DefaultShowEnemy.Get());

	const Point2D offs = pTypeExt->SelectBrd_DrawOffset.Get((isInfantry ?
		RulesExtData::Instance()->SelectBrd_DrawOffset_Infantry : RulesExtData::Instance()->SelectBrd_DrawOffset_Unit));

	XOffset = offs.X;
	YOffset = pTypeExt->This()->PixelSelectionBracketDelta + offs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;

		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}
*/
#include <New/Type/SelectBoxTypeClass.h>

void TechnoExtData::DrawSelectBox(TechnoClass* pThis,Point2D* pLocation,RectangleStruct* pBounds, bool drawBefore)
{
	const auto whatAmI = pThis->WhatAmI();
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	SelectBoxTypeClass* pSelectBox = nullptr;

	if (pTypeExt->SelectBox.isset())
		pSelectBox = pTypeExt->SelectBox.Get();
	else if (whatAmI == InfantryClass::AbsID)
		pSelectBox = RulesExtData::Instance()->DefaultInfantrySelectBox.Get();
	else if (whatAmI != BuildingClass::AbsID)
		pSelectBox = RulesExtData::Instance()->DefaultUnitSelectBox.Get();

	if (!pSelectBox || pSelectBox->DrawAboveTechno == drawBefore)
		return;

	const bool canSee = HouseClass::IsCurrentPlayerObserver() ? pSelectBox->VisibleToHouses_Observer : EnumFunctions::CanTargetHouse(pSelectBox->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer);

	if (!canSee)
		return;

	const double healthPercentage = pThis->GetHealthPercentage();
	//defaultFrame
	const Point3D defaultFrame = whatAmI == AbstractType::Infantry ? Point3D { 1,1,1 } : Point3D { 0,0,0 };
	const auto pSurface = DSurface::Temp();
	const auto flags = (drawBefore ? BlitterFlags::Flat | BlitterFlags::Alpha : BlitterFlags::Nonzero | BlitterFlags::MultiPass) | BlitterFlags::Centered | pSelectBox->Translucency;
	const int zAdjust = drawBefore ? pThis->GetZAdjustment() - 2 : 0;
	const auto pGroundShape = pSelectBox->GroundShape.Get();

	if ((pGroundShape || pSelectBox->GroundLine) && pSelectBox->Grounded && whatAmI != BuildingClass::AbsID)
	{
		CoordStruct coords = pThis->GetCenterCoords();
		coords.Z = MapClass::Instance->GetCellFloorHeight(coords);

		auto[outClient, visible] = TacticalClass::Instance->GetCoordsToClientSituation(coords);

		if (visible && pGroundShape)
		{
			const auto pPalette = pSelectBox->GroundPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

			const Point3D frames = pSelectBox->GroundFrames.Get(defaultFrame);
			const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;
			auto drawPoint = (outClient + pSelectBox->GroundOffset);
			pSurface->DrawSHP(pPalette, pGroundShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}

		if (pSelectBox->GroundLine)
		{
			Point2D start = *pLocation; // Copy to prevent be modified
			const int color = Drawing::RGB_To_Int(pSelectBox->GroundLineColor.Get(healthPercentage , RulesClass::Instance->ConditionYellow , RulesClass::Instance->ConditionRed));

			if (pSelectBox->GroundLine_Dashed)
				pSurface->Draw_Dashed_Line(start, outClient, color, nullptr, 0);
			else
				pSurface->Draw_Line(start, outClient, color);
		}
	}

	if (const auto pShape = pSelectBox->Shape.Get())
	{
		const auto pPalette = pSelectBox->Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		const Point3D frames = pSelectBox->Frames.Get(defaultFrame);
		const int frame = healthPercentage > RulesClass::Instance->ConditionYellow ? frames.X : healthPercentage > RulesClass::Instance->ConditionRed ? frames.Y : frames.Z;

		const Point2D offset = whatAmI == InfantryClass::AbsID ? Point2D { 8, -3 } : Point2D { 1, -4 };
		Point2D drawPoint = *pLocation + offset + pSelectBox->Offset;

		if (pSelectBox->DrawAboveTechno)
			drawPoint.Y += pType->PixelSelectionBracketDelta;

		pSurface->DrawSHP(pPalette, pShape, frame, &drawPoint, pBounds, flags, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
}

std::pair<TechnoTypeClass*, HouseClass*> TechnoExtData::GetDisguiseType(TechnoClass* pTarget, bool CheckHouse, bool CheckVisibility, bool bVisibleResult)
{
	TechnoTypeClass* pTypeOut = pTarget->GetTechnoType();
	HouseClass* pHouseOut = pTarget->GetOwningHouse();

	//Building cant disguise , so dont bother to check
	if (pTarget->WhatAmI() == BuildingClass::AbsID)
		return { pTypeOut , pHouseOut };

	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible)
	{
		if (CheckHouse)
		{
			if (const auto pDisguiseHouse = pTarget->GetDisguiseHouse(true))
			{
				pHouseOut = pDisguiseHouse;
			}
		}

		if (pTarget->Disguise != pTypeOut)
		{
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise))
			{
				return { pDisguiseType, pHouseOut };
			}
		}
	}

	return { pTypeOut, pHouseOut };
}

TechnoTypeClass* TechnoExtData::GetSimpleDisguiseType(TechnoClass* pTarget, bool CheckVisibility, bool bVisibleResult)
{
	TechnoTypeClass* pTypeOut = pTarget->GetTechnoType();

	//Building cant disguise , so dont bother to check
	if (pTarget->WhatAmI() == BuildingClass::AbsID)
		return pTypeOut;

	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible) {
		if (pTarget->Disguise != pTypeOut) {
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise)) {
				return pDisguiseType;
			}
		}
	}

	return pTypeOut;
}

static FORCEDINLINE std::pair<SHPStruct*, int> GetInsigniaDatas(TechnoClass* pThis, TechnoTypeExtData* pTypeExt)
{
	bool isCustomInsignia = false;
	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;
	const auto nCurRank = pThis->CurrentRanking;

	if (SHPStruct* pCustomShapeFile = pTypeExt->Insignia.GetFromSpecificRank(nCurRank))
	{
		pShapeFile = pCustomShapeFile;
		defaultFrameIndex = 0;
		isCustomInsignia = true;
	}

	auto insigniaFrames = pTypeExt->InsigniaFrames.Get();
	int insigniaFrame = insigniaFrames.X;
	int frameIndex = pTypeExt->InsigniaFrame.GetFromSpecificRank(nCurRank);

	if (pTypeExt->This()->Passengers > 0)
	{
		int passengersIndex = pTypeExt->Passengers_BySize ?
			 pThis->Passengers.GetTotalSize() : pThis->Passengers.NumPassengers;
		passengersIndex = MinImpl(passengersIndex, pTypeExt->This()->Passengers);

		if(!pTypeExt->Insignia_Passengers.empty() && (size_t)passengersIndex < pTypeExt->Insignia_Passengers.size()){
			if (auto const pCustomShapeFile = pTypeExt->Insignia_Passengers[passengersIndex].GetFromSpecificRank(nCurRank)) {
				pShapeFile = pCustomShapeFile;
				defaultFrameIndex = 0;
				isCustomInsignia = true;
			}
		}

		if(!pTypeExt->InsigniaFrame_Passengers.empty() && (size_t)passengersIndex < pTypeExt->InsigniaFrame_Passengers.size()){
			int frame = pTypeExt->InsigniaFrame_Passengers[passengersIndex].GetFromSpecificRank(nCurRank);

			if (frame != -1)
				frameIndex = frame;
		}

		if(!pTypeExt->InsigniaFrames_Passengers.empty() && (size_t)passengersIndex < pTypeExt->InsigniaFrames_Passengers.size()){
			auto const& frames = pTypeExt->InsigniaFrames_Passengers[passengersIndex];

			if (!frames->operator==(Vector3D<int>(-1, -1, -1)))
				insigniaFrames = frames.Get();
		}
	}

	if (pTypeExt->This()->Gunner)
	{
		int weaponIndex = pThis->CurrentWeaponNumber;
		auto weaponInsignia = pTypeExt->Insignia_Weapon.data();

		if (auto const pCustomShapeFile = weaponInsignia[weaponIndex].Shapes.GetFromSpecificRank(nCurRank))
		{
			pShapeFile = pCustomShapeFile;
			defaultFrameIndex = 0;
			isCustomInsignia = true;
		}

		int frame = weaponInsignia[weaponIndex].Frame.GetFromSpecificRank(nCurRank);

		if (frame != -1)
			frameIndex = frame;

		auto const& frames = weaponInsignia[weaponIndex].Frames;

		if (!frames->operator==(Vector3D<int>(-1, -1, -1)))
			insigniaFrames = frames.Get();
	}

	switch (nCurRank)
	{
	case Rank::Elite:
		defaultFrameIndex = !isCustomInsignia ? 15 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Z;
		break;
	case Rank::Veteran:
		defaultFrameIndex = !isCustomInsignia ? 14 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Y;
		break;
	default:
		break;
	}

	frameIndex = frameIndex == -1 ? insigniaFrame : frameIndex;

	if (frameIndex == -1)
		frameIndex = defaultFrameIndex;

	return { pShapeFile  , frameIndex };
}

static FORCEDINLINE void GetAdjustedInsigniaOffset(TechnoClass* pThis , Point2D& offset , const CoordStruct& a_) {

	Point2D a__ { a_.X , a_.Y};
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Infantry->operator+(a__));
		break;
	case AbstractType::Building:
		if (RulesExtData::Instance()->DrawInsignia_AdjustPos_BuildingsAnchor.isset())
				offset = (TechnoExtData::GetBuildingSelectBracketPosition(pThis,
						RulesExtData::Instance()->DrawInsignia_AdjustPos_BuildingsAnchor) +
						RulesExtData::Instance()->DrawInsignia_AdjustPos_Buildings) + a__;
			else
				offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Buildings->operator+(a__));

		break;
	default:
		offset += (RulesExtData::Instance()->DrawInsignia_AdjustPos_Units->operator+(a__));
		break;
	}
}

static FORCEDINLINE TechnoTypeExtData* GetTypeExtData(TechnoClass* pThis , bool isObserver)
{
	TechnoTypeClass* pTechnoType = nullptr;
	auto[pTechnoTyper, pOwner] = TechnoExtData::GetDisguiseType(pThis, true, true, false);
	const bool isDisguised = pTechnoTyper != pThis->GetTechnoType();

	if (isDisguised && isObserver) {
		pTechnoType = pThis->GetTechnoType();
	} else {
		pTechnoType = pTechnoTyper;
	}

	if (!pTechnoType)
		return nullptr;

	return TechnoTypeExtContainer::Instance.Find(pTechnoType);
}

void TechnoExtData::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->CurrentRanking == Rank::Invalid
		|| RulesExtData::Instance()->DrawInsigniaOnlyOnSelected.Get() && !pThis->IsSelected && !pThis->IsMouseHovering)
		return;

	const bool IsObserverPlayer = HouseExtData::IsObserverPlayer();
	auto pTypeExt = GetTypeExtData(pThis , IsObserverPlayer);

	if (!pTypeExt)
		return;

	Point2D offset = *pLocation;
	const bool IsAlly = pThis->Owner && pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer);

	const bool isVisibleToPlayer = IsAlly
		|| IsObserverPlayer
		|| pTypeExt->Insignia_ShowEnemy.Get(RulesExtData::Instance()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	const auto& [pShapeFile, frameIndex] = GetInsigniaDatas(pThis, pTypeExt);

	if (frameIndex != -1 && pShapeFile)
	{
		GetAdjustedInsigniaOffset(pThis , offset , CoordStruct::Empty);
		offset.Y += RulesExtData::Instance()->DrawInsignia_UsePixelSelectionBracketDelta ? pThis->GetTechnoType()->PixelSelectionBracketDelta : 0;
		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

bool TechnoExtData::CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget)
{
	const int wpnIdx = pThis->SelectWeapon(pTarget);
	const FireError fErr = pThis->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)

	{
		return pThis->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExtData::ForceJumpjetTurnToTarget(TechnoClass* pThis)
{
	const auto pFoot = cast_to<UnitClass*, false>(pThis);
	if (!pFoot)
		return;

	const auto pType = pFoot->Type;
	const auto pLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pFoot->Locomotor);

	if (pLoco && pThis->IsInAir()
		&& !pType->TurretSpins)
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->JumpjetTurnToTarget.Get(RulesExtData::Instance()->JumpjetTurnToTarget)
		   && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				if (!pLoco->Facing.Is_Rotating() && TechnoExtData::CheckIfCanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));

					if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
						pLoco->Facing.Set_Desired(tgtDir);
				}
			}
		}
	}
}

// convert UTF-8 string to wstring
static std::wstring Str2Wstr(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void TechnoExtData::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage, WarheadTypeClass* pWH)
{
	if (!pThis || !pThis->IsAlive || pThis->InLimbo || !pThis->IsOnMyView())
		return;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const ColorStruct color = isShieldDamage ? damage > 0 ? Phobos::Defines::ShieldPositiveDamageColor : Phobos::Defines::ShieldPositiveDamageColor :
		damage > 0 ? Drawing::DefaultColors[(int)DefaultColorList::Red] : Drawing::DefaultColors[(int)DefaultColorList::Green];

	static fmt::basic_memory_buffer<wchar_t> damageStr;
	damageStr.clear();

	if (Phobos::Otamaa::IsAdmin)
		fmt::format_to(std::back_inserter(damageStr) ,L"[{}] {} ({})", PhobosCRT::StringToWideString(pThis->get_ID()), PhobosCRT::StringToWideString(pWH->ID), damage);
	else
		fmt::format_to(std::back_inserter(damageStr) ,L"{}", damage);

	damageStr.push_back(L'\0');
	auto coords = pThis->GetCenterCoords();
	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr.data(), &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset == INT32_MIN)
		pExt->DamageNumberOffset = -maxOffset;

	if (auto pBuilding = cast_to<BuildingClass*, false>(pThis))
		coords.Z += 104 * pBuilding->Type->Height;
	else
		coords.Z += 256;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
	{
		if (!pCell->IsFogged() && !pCell->IsShrouded())
		{
			if (pThis->VisualCharacter(0, HouseClass::CurrentPlayer()) != VisualType::Hidden)
			{
				FlyingStrings::Add(damageStr.data(), coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });
			}
		}
	}

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

void TechnoExtData::Stop(TechnoClass* pThis, Mission const& eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetArchiveTarget(nullptr);
	pThis->Stun();
}

bool TechnoExtData::IsOnLimbo(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->InLimbo && !pThis->Transporter;
}

bool TechnoExtData::IsDeactivated(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->Deactivated;
}

bool TechnoExtData::IsUnderEMP(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->IsUnderEMP();
}

bool TechnoExtData::IsActive(TechnoClass* pThis, bool bCheckEMP, bool bCheckDeactivated, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!TechnoExtData::IsAlive(pThis, bIgnoreLimbo, bIgnoreIsOnMap, bIgnoreAbsorb))
		return false;

	if (pThis->BeingWarpedOut || pThis->TemporalTargetingMe || IsUnderEMP(pThis, !bCheckEMP) || IsDeactivated(pThis, !bCheckDeactivated))
		return false;

	return true;
}

bool TechnoExtData::IsAlive(TechnoClass* pThis, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!pThis || !pThis->IsAlive || pThis->Health <= 0)
		return false;

	if ((IsOnLimbo(pThis, !bIgnoreLimbo)) || (pThis->Absorbed && !bIgnoreAbsorb) || (!pThis->IsOnMap && !bIgnoreIsOnMap))
		return false;

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	if (pThis->WhatAmI() == UnitClass::AbsID)
		return (static_cast<UnitClass*>(pThis)->DeathFrameCounter > 0) ? false : true;

	return true;
}

void TechnoExtData::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;

	TechnoClass* pObjectKiller = (pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner ?
		pKiller->SpawnOwner : pKiller;

	if (pObjectKiller)
	{
		TechnoExtData::ObjectKilledBy(pVictim, pObjectKiller->Owner);

		if(pObjectKiller->BelongsToATeam()) {
			if (auto const pFootKiller = flag_cast_to<FootClass*, false>(pObjectKiller)) {
				auto pKillerExt = TechnoExtContainer::Instance.Find(pObjectKiller);

				if (auto const pFocus = flag_cast_to<TechnoClass*>(pFootKiller->Team->ArchiveTarget))
					pKillerExt->LastKillWasTeamTarget =
					pFocus->GetTechnoType() == pVictim->GetTechnoType()
					|| TeamExtData::IsEligible(pFocus ,pVictim->GetTechnoType())
					|| TeamExtData::IsEligible(pVictim, pFocus->GetTechnoType())
						;

				auto pKillerTeamExt = TeamExtContainer::Instance.Find(pFootKiller->Team);

				if (pKillerTeamExt->ConditionalJump_EnabledKillsCount)
				{
					bool isValidKill =
						 pKillerTeamExt->ConditionalJump_Index < 0 ?
						 false :
						ScriptExtData::EvaluateObjectWithMask(pVictim, pKillerTeamExt->ConditionalJump_Index, -1, -1, pKiller);

					if (isValidKill || pKillerExt->LastKillWasTeamTarget)
						pKillerTeamExt->ConditionalJump_Counter++;
				}

				// Special case for interrupting current action
				if (pKillerTeamExt->AbortActionAfterKilling
					&& pKillerExt->LastKillWasTeamTarget)
				{
					pKillerTeamExt->AbortActionAfterKilling = false;
					auto pTeam = pFootKiller->Team;

					const auto&[curAction , curArgs] = pTeam->CurrentScript->GetCurrentAction();
					const auto&[nextAction , nextArgs] = pTeam->CurrentScript->GetNextAction();

					Debug::LogInfo("DEBUG: [{}] [{}] {} = {},{} - Force next script action after successful kill: {} = {},{}"
						, pTeam->Type->ID
						, pTeam->CurrentScript->Type->ID
						, pTeam->CurrentScript->CurrentMission
						, (int)curAction
						, curArgs
						, pTeam->CurrentScript->CurrentMission + 1
						, (int)nextAction
						, nextArgs
					);

					// Jumping to the next line of the script list
					pTeam->StepCompleted = true;

				}

			}
		}
	}
}

void TechnoExtData::ObjectKilledBy(TechnoClass* pVictim, HouseClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;

	if (pKiller != pVictim->Owner) {
		auto pHouseExt = HouseExtContainer::Instance.Find(pKiller);

		if (pHouseExt->AreBattlePointsEnabled()) {
			pHouseExt->UpdateBattlePoints(pHouseExt->CalculateBattlePoints(pVictim));
		}
	}
}

void TechnoExtData::UpdateMCRangeLimit()
{
	auto const pThis = This();
	auto const pCManager = pThis->CaptureManager;

	if (!pCManager)
		return;

	const int Range = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->MindControlRangeLimit.Get();

	if (Range <= 0)
		return;

	for (const auto& node : pCManager->ControlNodes) {
		if (pCManager->Owner->DistanceFrom(node->Unit) > Range) {
			pCManager->FreeUnit(node->Unit);
		}
	}
}

void TechnoExtData::UpdateInterceptor()
{
	auto const pThis = This();

	if (!this->IsInterceptor())
		return;

	if (auto pTarget = pThis->Target) {
		if (pTarget->WhatAmI() != AbstractType::Bullet)
			return;

		const auto pTargetExt = BulletExtContainer::Instance.Find(static_cast<BulletClass*>(pTarget));

		if ((pTargetExt->InterceptedStatus & InterceptedStatus::Locked) == InterceptedStatus::None)
			return;
	}

	const int count = BulletClass::Array->Count;

	if (this->IsBurrowed || !count)
		return;

	if (TechnoExtData::IsInWarfactory(pThis))
		return;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir())
		return;

	if (auto const pTransport = pThis->Transporter)
	{
		if(!pTransport->IsAlive)
			return;

		if (pTransport->WhatAmI() == AircraftClass::AbsID && !pTransport->IsInAir())
			return;

		if (TechnoExtData::IsInWarfactory(pTransport))
			return;

		if (TechnoExtContainer::Instance.Find(pTransport)->IsBurrowed)
			return;
	}


	BulletClass* pTargetBullet = nullptr;

	const auto& guardRange = pTypeExt->Interceptor_GuardRange.Get(pThis);
	const double guardRangeSq = guardRange * guardRange;
	const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);
	const double minguardRangeSq = minguardRange * minguardRange;

	// Interceptor weapon is always fixed
	const auto pWeapon = pThis->GetWeapon(pTypeExt->Interceptor_Weapon)->WeaponType;
	const auto wpnRange = pWeapon->Range;
	const auto wpnRangeS1q = wpnRange * wpnRange;

	const auto wpnminRange = pWeapon->MinimumRange;
	const auto wpnminRangeS1q = wpnminRange * wpnminRange;

	// DO NOT iterate BulletExt::ExtMap here, the order of items is not deterministic
	// so it can differ across players throwing target management out of sync.
	int i = 0;

	for (; i < count; ++i)
	{
		const auto& pBullet = BulletClass::Array->Items[i];
		const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const auto distanceSq = pBullet->Location.DistanceFromSquared(pThis->Location);

		if (pTypeExt->Interceptor_ConsiderWeaponRange.Get() &&
			(distanceSq > wpnRangeS1q || distanceSq < wpnminRangeS1q))
			continue;

		if (distanceSq > guardRangeSq || distanceSq < minguardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			auto const pWhExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (Math::abs(pWhExt->GetVerses(pBulletTypeExt->Armor).Verses)
				//GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead , pBulletTypeExt->Armor))
				< 0.001)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->Owner;

		if (!EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses, pThis->Owner, bulletOwner))
			continue;

		if (pBulletExt->InterceptedStatus & (InterceptedStatus::Targeted | InterceptedStatus::Locked))
		{
			// Set as optional target
			pTargetBullet = pBullet;
			break;
		}

		// Establish target
		pThis->SetTarget(pBullet);
		return;
	}

	// Loop ends and there is no target
	if (!pTargetBullet)
		return;

	// There is an optional target, but it is still possible to continue checking for more suitable target
	for (; i < count; ++i)
	{
		const auto& pBullet = BulletClass::Array->Items[i];
		const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

		if (pBulletExt->InterceptedStatus & (InterceptedStatus::Targeted | InterceptedStatus::Locked))
			continue;

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const auto distanceSq = pBullet->Location.DistanceFromSquared(pThis->Location);

		if (pTypeExt->Interceptor_ConsiderWeaponRange.Get() &&
				(distanceSq > wpnRangeS1q || distanceSq < wpnminRangeS1q))
			continue;

		if (distanceSq > guardRangeSq || distanceSq < minguardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			auto const pWhExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (Math::abs(pWhExt->GetVerses(pBulletTypeExt->Armor).Verses)
				//GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead , pBulletTypeExt->Armor))
				< 0.001)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->Owner;

		if (!EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses, pThis->Owner, bulletOwner))
			continue;

		// Establish target
		pThis->SetTarget(pBullet);
		return;
	}

	// There is no more suitable target, establish optional target
	if (pTargetBullet)
		pThis->SetTarget(pTargetBullet);
}

void TechnoExtData::UpdateTiberiumEater()
{
	const auto pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const auto pEaterType = pTypeExt->TiberiumEaterType.get();

	if (!pEaterType)
		return;

	const int transDelay = pEaterType->TransDelay;

	if (transDelay && this->TiberiumEaterTimer.InProgress())
		return;

	const auto pOwner = pThis->Owner;
	bool active = false;
	const bool displayCash = pEaterType->Display && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer);
	int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

	if (facing >= 7)
		facing = 0;
	else
		facing++;

	const int cellCount = static_cast<int>(pEaterType->Cells.size());

	for (int idx = 0; idx < cellCount; idx++)
	{
		const auto& cellOffset = pEaterType->Cells[idx];
		const auto pos = TechnoExtData::GetFLHAbsoluteCoords(pThis, CoordStruct { cellOffset.X, cellOffset.Y, 0 }, false);
		const auto pCell = MapClass::Instance->TryGetCellAt(pos);

		if (!pCell)
			continue;

		if (const int contained = pCell->GetContainedTiberiumValue())
		{
			const int tiberiumIdx = pCell->GetContainedTiberiumIndex();
			const int tiberiumValue = TiberiumClass::Array->Items[tiberiumIdx]->Value;
			const int tiberiumAmount = static_cast<int>(static_cast<double>(contained) / tiberiumValue);
			const int amount = pEaterType->AmountPerCell > 0 ? MinImpl(pEaterType->AmountPerCell.Get(), tiberiumAmount) : tiberiumAmount;
			pCell->ReduceTiberium(amount);
			const float multiplier = pEaterType->CashMultiplier * (1.0f + pOwner->NumOrePurifiers * RulesClass::Instance->PurifierBonus);
			const int value = static_cast<int>(std::round(amount * tiberiumValue * multiplier));
			pOwner->TransactMoney(value);
			active = true;

			if (displayCash)
			{
				auto cellCoords = pCell->GetCoords();
				cellCoords.Z = std::max(pThis->Location.Z, cellCoords.Z);
				FlyingStrings::AddMoneyString(true , value, pOwner, pEaterType->DisplayToHouse, cellCoords, pEaterType->DisplayOffset);
			}

			const auto& anims = pEaterType->Anims_Tiberiums[tiberiumIdx].GetElements(pEaterType->Anims);
			const int animCount = static_cast<int>(anims.size());

			if (animCount == 0)
				continue;

			AnimTypeClass* pAnimType = nullptr;

			switch (animCount)
			{
			case 1:
				pAnimType = anims[0];
				break;

			case 8:
				pAnimType = anims[facing];
				break;

			default:
				pAnimType = anims[ScenarioClass::Instance->Random.RandomRanged(0, animCount - 1)];
				break;
			}

			if (pAnimType)
			{
				const auto pAnim = GameCreate<AnimClass>(pAnimType, pos);
				AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, pThis, true, false);

				if (pEaterType->AnimMove)
					pAnim->SetOwnerObject(pThis);
			}
		}
	}

	if (active && transDelay)
		this->TiberiumEaterTimer.Start(pEaterType->TransDelay);
}

void TechnoExtData::UpdateSpawnLimitRange()
{
	auto const pThis = This();
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	auto const pManager = pThis->SpawnManager;

	if (!pExt->Spawn_LimitedRange || !pManager)
		return;

	int weaponRange = pThis->Veterancy.IsElite() ? pExt->EliteSpawnerRange : pExt->SpawnerRange;
	if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
		pManager->ResetTarget();
}

bool TechnoExtData::IsHarvesting(TechnoClass* pThis)
{
	const auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	const auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExtData::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExtData::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const& pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld) > 0)
			return true;
	}

	return false;
}

void TechnoExtData::InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pType = pThis->GetTechnoType();

	if (pThis->WhatAmI() == AbstractType::Building || pType->Invisible)
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (bIsconverted)
		pExt->LaserTrails.clear();

	auto const pOwner = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExtData::FindFirstCivilianHouse();

	if (pExt->LaserTrails.empty())
	{
		pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size());
		for (auto const& entry : pTypeExt->LaserTrailData) {
			pExt->LaserTrails.emplace_back(std::move(std::make_unique<LaserTrailClass>(
					LaserTrailTypeClass::Array[entry.idxType].get(), pOwner->LaserColor, entry.FLH, entry.IsOnTurret)));
		}
	}

}

void TechnoExtData::UpdateLaserTrails(TechnoClass* pThis) {

	HelperedVector<std::unique_ptr<LaserTrailClass>> dummy;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	dummy.reserve(pExt->LaserTrails.size());

	for (auto& entry : pExt->LaserTrails) {
		if (entry->Permanent)
			dummy.emplace_back(std::move(entry));
	}

	pExt->LaserTrails.clear();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	pExt->LaserTrails.reserve(pTypeExt->LaserTrailData.size() + dummy.size());

	for (auto const& entry : pTypeExt->LaserTrailData) {
		pExt->LaserTrails.emplace_back(std::move(std::make_unique<LaserTrailClass>(
			LaserTrailTypeClass::Array[entry.idxType].get(), pThis->Owner->LaserColor, entry.FLH, entry.IsOnTurret)));
	}

	for (auto& entry_d : dummy) {
		pExt->LaserTrails.emplace_back(std::move(entry_d));
	}
}

void TechnoExtData::InitializeAttachEffects(TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->PhobosAttachEffects.AttachTypes.size() < 1)
		return;

	PhobosAttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, &pTypeExt->PhobosAttachEffects);
}

bool TechnoExtData::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	if (!pWeaponType)
		return false;

	WeaponTypeExtData::DetonateAt(pWeaponType, pThis, pThis, true, nullptr);
	return true;
}

Matrix3D TechnoExtData::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey)
{
	Matrix3D Mtx {};
	// Step 1: get body transform matrix
	if (pThis && (pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor) {
		((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Draw_Matrix(&Mtx, pKey);
		return Mtx;
	}

	// no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
	return Matrix3D::GetIdentity();
}

// reversed from 6F3D60
CoordStruct TechnoExtData::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& pCoord, bool isOnTurret, const CoordStruct& Overrider)
{
	auto const pType = pThis->GetTechnoType();
	Matrix3D mtx = TechnoExtData::GetTransform(pThis);
	auto pFoot = flag_cast_to<FootClass*>(pThis);

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && ( pThis->HasTurret() || !pFoot))
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, 1.0);
		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		// For BuildingClass turret facing is equal to primary facing

		float angle = pFoot ? (float)(turretRad - bodyRad) : (float)(turretRad);
		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(pCoord.X), static_cast<float>(pCoord.Y), static_cast<float>(pCoord.Z));

	Vector3D<float> result {};
	Matrix3D::MatrixMultiply(&result, &mtx, &Vector3D<float>::Empty);
	 //Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	//auto result = mtx.GetTranslation();

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetRenderCoords();
	location += CoordStruct { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };
	// += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

double TechnoExtData::GetROFMult(TechnoClass const* pTech)
{
	const bool rofAbility = pTech->HasAbility(AbilityType::ROF);

	return !rofAbility ? 1.0 :
		RulesClass::Instance->VeteranROF * ((!pTech->Owner || !pTech->Owner->Type) ?
			1.0 : pTech->Owner->Type->ROFMult);
}

int TechnoExtData::GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass const* pPassenger)
{
	auto const pData = TechnoTypeExtContainer::Instance.Find(pTransporterData);
	auto const pThis = This();
	auto const pDelType = &pData->PassengerDeletionType;

	int nRate = 0;

	if (pDelType->UseCostAsRate.Get())
	{
		// Use passenger cost as countdown.
		auto timerLength = static_cast<int>(pPassenger->GetTechnoType()->Cost * pDelType->CostMultiplier);
		const auto nCostRateCap = pDelType->CostRateCap.Get(-1);
		if (nCostRateCap > 0)
			timerLength = MinImpl(timerLength, nCostRateCap);

		nRate = timerLength;
	}
	else
	{
		// Use explicit rate optionally multiplied by unit size as countdown.
		auto timerLength = pDelType->Rate.Get();
		if (pDelType->Rate_SizeMultiply.Get() && pPassenger->GetTechnoType()->Size > 1.0)
			timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);

		nRate = timerLength;
	}

	if (pDelType->Rate_AffectedByVeterancy)
	{
		auto const nRof = TechnoExtData::GetROFMult(pThis);
		if (nRof != 1.0)
		{
			nRate = static_cast<int>(nRate * nRof);
		}
	}

	return nRate;
}

static int GetTotalSoylentOfPassengers(TechnoClass* pThis, PassengerDeletionTypeClass* pDelType, FootClass* pPassenger)
{
	FootClass* pPassengerL2;
	int nMoneyToGive = 0;
	while (pPassenger->Passengers.NumPassengers > 0)
	{
		pPassengerL2 = pPassenger->Passengers.RemoveFirstPassenger();

		if (pPassengerL2) {
			auto pSource = pDelType->DontScore ? nullptr : pThis;
			nMoneyToGive += (int)(pPassengerL2->GetTechnoType()->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);
			if (pPassengerL2->Passengers.NumPassengers > 0) {
				nMoneyToGive += GetTotalSoylentOfPassengers(pThis, pDelType, pPassengerL2);
			}

			pPassengerL2->KillPassengers(pSource);
			pPassengerL2->RegisterDestruction(pSource);
			pPassengerL2->UnInit();
		}
	}
	return nMoneyToGive;
}

void TechnoExtData::UpdateEatPassengers()
{
	auto const pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	auto const pDelType = &pTypeExt->PassengerDeletionType;

	if (!pDelType->Enabled || !TechnoExtData::IsActive(pThis, false))
		return;

	if (!pDelType->UnderEMP && (pThis->Deactivated || pThis->IsUnderEMP()))
		return;

		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(pDelType->AllowedHouses, pThis->Owner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = flag_cast_to <FootClass*>(pLastPassenger);
					pPassenger = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = flag_cast_to<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPassenger)
			{
				this->PassengerDeletionTimer.Stop();
				return;
			}

			if (!this->PassengerDeletionTimer.HasStarted()) // Execute only if timer has been stopped or not started
			{
				// Setting & start countdown. Bigger units needs more time
				int timerLength = this->GetEatPassangersTotalTime(pThis->GetTechnoType(), pPassenger);

				if (timerLength <= 0)
					return;

				this->PassengerDeletionTimer.Start(timerLength);
			}

			if (this->PassengerDeletionTimer.Completed()) // Execute only if timer has ran out after being started
			{
				--pThis->Passengers.NumPassengers;

				if (pLastPassenger)
					pLastPassenger->NextObject = nullptr;

				if (pPreviousPassenger)
					pPreviousPassenger->NextObject = pPassenger->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				if (auto const pPassengerType = pPassenger->GetTechnoType())
				{
					pPassenger->LiberateMember();

					auto const& nReportSound = pDelType->ReportSound;
					if(nReportSound.isset())
						VocClass::SafeImmedietelyPlayAt(nReportSound, &pThis->Location);

					auto const pThisOwner = pThis->GetOwningHouse();

					if (const auto pAnimType = pDelType->Anim.Get(nullptr))
					{
						auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
						pAnim->SetOwnerObject(pThis);
						AnimExtData::SetAnimOwnerHouseKind(pAnim, pThisOwner, pPassenger->GetOwningHouse(), pThis, false, false);
					}

					// Check if there is money refund
					if (pDelType->Soylent &&
						EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
					{
						int nMoneyToGive = (int)(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) *
							pDelType->SoylentMultiplier);

						if (pPassenger->Passengers.NumPassengers > 0) {
							nMoneyToGive += GetTotalSoylentOfPassengers(pThis, pDelType, pPassenger);
						}

						if (nMoneyToGive)
						{
							pThis->Owner->TransactMoney(nMoneyToGive);

							if (pDelType->DisplaySoylent)
							{
								FlyingStrings::AddMoneyString(true, nMoneyToGive, pThis,
									pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
							}
						}
					}

					// Handle gunner change.
					if (pThis->GetTechnoType()->Gunner)
					{
						if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
						{
							pFoot->RemoveGunner(pPassenger);
							FootClass* pGunner = nullptr;

							for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = flag_cast_to<FootClass*>(pNext->NextObject))
								pGunner = pNext;

							pFoot->ReceiveGunner(pGunner);
						}
					}

					if (pThis->GetTechnoType()->OpenTopped)
					{
						pThis->ExitedOpenTopped(pPassenger);
					}

					if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
					{
						if (pBld->Absorber())
						{
							pPassenger->Absorbed = false;

							if (pBld->Type->ExtraPowerBonus || pBld->Type->ExtraPowerDrain)
							{
								pBld->Owner->RecheckPower = true;
							}
						}
					}

					pPassenger->Transporter = nullptr;
					pPassenger->BunkerLinkedItem = nullptr;
					//auto const pPassengerOwner = pPassenger->Owner;

					//if (!pPassengerOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant)
					//{
					//	pPassengerOwner->RegisterLoss(pPassenger, false);
					//	pPassengerOwner->RemoveTracking(pPassenger);

					//	if (!pPassengerOwner->RecheckTechTree)
					//		pPassengerOwner->RecheckTechTree = true;
					//}


					pPassenger->RegisterDestruction(pDelType->DontScore ? nullptr : pThis);
					//Debug::LogInfo(__FUNCTION__" Called ");
					TechnoExtData::HandleRemove(pPassenger, pDelType->DontScore ? nullptr : pThis, false, false);
				}

				this->PassengerDeletionTimer.Stop();
			}
		}
}

bool NOINLINE TechnoExtData::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
			return true;
	}

	return false;
}

#include <AircraftTrackerClass.h>

void TechnoExtData::HandleRemove(TechnoClass* pThis, TechnoClass* pSource, bool SkipTrackingRemove, bool Delete)
{
	// kill passenger cargo to prevent memleak
	const auto pThisType = pThis->GetTechnoType();

	if (pThisType->OpenTopped) {
		pThis->MarkPassengersAsExited();
	}

	pThis->KillPassengers(pSource);

	const auto nWhat = pThis->WhatAmI();
	Debug::LogInfo(__FUNCTION__" Called For[({}){} - {}][{} - {}](Method :{})",
		AbstractClass::GetAbstractClassName(nWhat),
		pThisType->ID,
		(void*)pThis,
		pThis->Owner ? pThis->Owner->get_ID() : GameStrings::NoneStr(),
		(void*)pThis->Owner,
		Delete ? "Delete" :  "UnInit"
	);

	if (nWhat == BuildingClass::AbsID)
	{
		static_cast<BuildingClass*>(pThis)->KillOccupants(nullptr);
	} else {

		if (pThis->InLimbo && ((FootClass*)pThis)->ParasiteImUsing && ((FootClass*)pThis)->ParasiteImUsing->Victim)
			((FootClass*)pThis)->ParasiteImUsing->ExitUnit();

		const auto flight = pThis->GetLastFlightMapCoords();
		if(flight.IsValid())
			AircraftTrackerClass::Instance->Remove((FootClass*)pThis);

		if(auto& pTeam = pThis->OldTeam){
			pTeam->RemoveMember((FootClass*)pThis);
			pTeam->Reacalculate();
			pTeam = nullptr;
		}
	}

	//if (!Delete && !SkipTrackingRemove)
	//{
	//	if (const auto pOwner = pThis->GetOwningHouse())
	//	{
	//		if(!pThis->InLimbo)
	//			pOwner->RegisterLoss(pThis, false);
	//
	//		pOwner->RemoveTracking(pThis);
	//
	//		if (!pOwner->RecheckTechTree)
	//			pOwner->RecheckTechTree = true;
	//	}
	//}


	if (Delete)
		GameDelete<true, false>(pThis);
	else
	pThis->UnInit();

	// Handle extra power
	if (pThis->Absorbed && pThis->Transporter)
		pThis->Transporter->Owner->RecheckPower = true;
}

void TechnoExtData::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		// this shit is not really good idea to pull of
		// some stuffs doesnt really handled properly , wtf
		bool SkipRemoveTracking = false;
		if (!pThis->InLimbo)
		{
			SkipRemoveTracking = true;
			pThis->Limbo();
		}

		//Debug::LogInfo(__FUNCTION__" (2args) Called ");
		TechnoExtData::HandleRemove(pThis, nullptr, SkipRemoveTracking, false);
	}else{
		TechnoExtData::Kill(pThis, nullptr);
	}
}

static KillMethod NOINLINE GetKillMethod(KillMethod deathOption)
{
	if (deathOption == KillMethod::Random)
	{ //ensure no death loop , only random when needed to
		return ScenarioClass::Instance->Random.RandomRangedSpecific<KillMethod>(KillMethod::Explode, KillMethod::Sell);
	}

	return deathOption;
}

void TechnoExtData::Kill(TechnoClass* pThis, TechnoClass* pKiller) {
	if (pThis->IsAlive) {
		auto nHealth = pThis->GetType()->Strength;
		pThis->ReceiveDamage(&nHealth, 0, RulesClass::Instance()->C4Warhead, pKiller, true, false, pKiller ? pKiller->Owner : nullptr);
	}
}

void TechnoExtData::KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill, AnimTypeClass* pVanishAnim)
{
	if (!pThis || deathOption == KillMethod::None || !pThis->IsAlive)
		return;

	if (auto pBld = cast_to<BuildingClass*, false>(pThis)) {
		auto pBldExt = BuildingExtContainer::Instance.Find(pBld);

		if (pBldExt->LimboID != -1) {
			BuildingExtData::LimboKill(pBld);
			return;
		}
	}

	auto const pWhat = VTable::Get(pThis);

	switch (GetKillMethod(deathOption))
	{
	case KillMethod::Explode:
	{
		TechnoExtData::Kill(pThis, nullptr);
	}break;
	case KillMethod::Vanish:
	{
		// this shit is not really good idea to pull off
		// some stuffs doesnt really handled properly , wtf

		if (pVanishAnim && !pThis->InLimbo && (pThis->Location.IsValid() && pThis->InlineMapCoords().IsValid()))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pVanishAnim, pThis->GetCoords()),
				pThis->GetOwningHouse(),
				nullptr,
				 true
			);
		}

		pThis->Stun();
		bool skiptrackingremove = false;

		if (!pThis->InLimbo){
			skiptrackingremove = true;
			pThis->Limbo();
		}

		if (RegisterKill)
			pThis->RegisterKill(pThis->Owner);

		//Debug::LogInfo(__FUNCTION__" Called ");
		TechnoExtData::HandleRemove(pThis, nullptr, skiptrackingremove, false);

	}break;
	case KillMethod::Sell:
	{
		if (pWhat == BuildingClass::vtable)
		{
			const auto pBld = static_cast<BuildingClass*>(pThis);

			if (pBld->HasBuildup && (pBld->CurrentMission != Mission::Selling || pBld->CurrentMission != Mission::Unload))
			{
				BuildingExtContainer::Instance.Find(pBld)->Silent = true;
				pBld->Sell(true);
				return;
			}
		}
		else if (pThis->AbstractFlags & AbstractFlags::Foot)
		{
			const auto pFoot = static_cast<FootClass*>(pThis);

			if (pWhat != InfantryClass::vtable && pFoot->CurrentMission != Mission::Unload)
			{
				if (auto const pCell = pFoot->GetCell())
				{
					if (auto const pBuilding = pCell->GetBuilding())
					{
						if (pBuilding->Type->UnitRepair)
						{
							pFoot->Sell(true);
							return;
						}
					}
				}
			}
		}

		if (pThis) {
			TechnoExtData::Kill(pThis, nullptr);
		}

	}break;
	case KillMethod::Convert:
	{
		if(pThis && pThis->IsAlive && pThis->AbstractFlags & AbstractFlags::Foot) {
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

			if (auto pTo = pTypeExt->Convert_AutoDeath) {
				TechnoExt_ExtData::ConvertToType(pThis, pTo);
			}
		}
	}break;
	default:
	break;
	}
}

// Otamaa : preparation
// not sure when it gonna finish
enum class DeathConditions : char {
	CountDown = 0 ,
	NoAmmo,
	ListOfIfExist,
	ListOfIfNoExist,
	SlaveOwnerDie,
	NeutralTechno,
	CivilianTechno,
	AIControlled, //https://github.com/Phobos-developers/Phobos/discussions/1198
	PreWeaponFiring,
	AfterWeaponFiring,
	ChangeOwner
};

bool NOINLINE ImmeditelyReturn(TechnoClass* pTech, bool any, bool& result)
{
	//only immedieltely return if the techno dies
	if (!any && !pTech->IsAlive) {
		result = true;
		return true;
	} else if(any) { // immedietely retyrn and check the result
		result = !pTech->IsAlive;
		return true;
	}

	return false; //continue func
}

// Feature: Kill Object Automatically
// https://github.com/Phobos-developers/Phobos/pull/1346
// TODO : update
bool TechnoExtData::CheckDeathConditions()
{
	auto const pThis = This();
	const auto pTypeThis = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTypeThis);

	const KillMethod nMethod = pTypeExt->Death_Method.Get();
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation.Get();
	bool result = false;

	if (nMethod == KillMethod::None)
		return result;

	const bool Any = pTypeExt->AutoDeath_ContentIfAnyMatch;

	// Death by owning house
	if (pTypeExt->AutoDeath_OwnedByPlayer)
	{
		if (pThis->Owner && pThis->Owner->IsControlledByHuman()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}
		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_OwnedByAI)
	{
		if (pThis->Owner && !pThis->Owner->IsControlledByHuman()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by money
	if (pTypeExt->AutoDeath_MoneyExceed >= 0) {
		if (pThis->Owner && pThis->Owner->Available_Money() >= pTypeExt->AutoDeath_MoneyExceed) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_MoneyBelow >= 0) {
		if (pThis->Owner && pThis->Owner->Available_Money() <= pTypeExt->AutoDeath_MoneyBelow) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by power
	if (pTypeExt->AutoDeath_LowPower) {
		if (pThis->Owner && pThis->Owner->HasLowPower()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_FullPower) {
		if (pThis->Owner && pThis->Owner->HasFullPower()) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo) {
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death by passengers
	if (pTypeExt->AutoDeath_PassengerExceed >= 0)
	{
		if (pTypeThis->Passengers > 0 && pThis->Passengers.NumPassengers >= pTypeExt->AutoDeath_PassengerExceed) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	if (pTypeExt->AutoDeath_PassengerBelow >= 0) {
		if (pTypeThis->Passengers > 0 && pThis->Passengers.NumPassengers <= pTypeExt->AutoDeath_PassengerBelow) {
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	const auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
	{
		const auto existSingleType = [pThis, affectedHouse, allowLimbo](TechnoTypeClass* pType)
		{
			if(affectedHouse == AffectedHouse::Owner)
			{
				if(allowLimbo) {
					for (const auto& limbo : HouseExtData::LimboTechno) {
						if (!limbo->IsAlive || limbo->Owner != pThis->Owner)
							continue;

						const auto limboType = limbo->GetTechnoType();
						if (!limboType->Insignificant && !limboType->DontScore && limboType== pType)
							return true;
					}
				}

				return pThis->Owner->CountOwnedAndPresent(pType) > 0;

			} else if(affectedHouse != AffectedHouse::None){

				for (HouseClass* pHouse : *HouseClass::Array)
				{
					if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse))
					{
						if(allowLimbo) {
							for (const auto& limbo : HouseExtData::LimboTechno) {
								if (!limbo->IsAlive || limbo->Owner != pHouse)
									continue;

								const auto limboType = limbo->GetTechnoType();
								if (!limboType->Insignificant && !limboType->DontScore && limboType== pType)
									return true;
							}
						}

						return  pHouse->CountOwnedAndPresent(pType) > 0;
					}
				}
			}

			return false;
		};

		return any
			? vTypes.Any_Of(existSingleType)
			: vTypes.All_Of(existSingleType);
	};

	// Death if nonexist
	if (!pTypeExt->AutoDeath_Nonexist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_Nonexist,
			pTypeExt->AutoDeath_Nonexist_House,
			!pTypeExt->AutoDeath_Nonexist_Any, pTypeExt->AutoDeath_Nonexist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if exist
	if (!pTypeExt->AutoDeath_Exist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_Exist,
			pTypeExt->AutoDeath_Exist_House,
			pTypeExt->AutoDeath_Exist_Any,
			pTypeExt->AutoDeath_Exist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (!Death_Countdown.HasStarted())
		{
			Death_Countdown.Start(pTypeExt->Death_Countdown);
			HouseExtData::AutoDeathObjects.insert(pThis, nMethod);
		}
		else if (Death_Countdown.Completed())
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
		}

		if (ImmeditelyReturn(pThis, Any, result))
			return result;
	}

	return result;
}

//TODO : finish this if merged i suppose
constexpr void CountSelfHeal(HouseClass* pOwner, int& count, Nullable<int>& cap, bool allowPlayerControl, bool allowAllies, SelfHealGainType type)
{
	if (pOwner->Defeated ||
		(pOwner->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive))
		return;

		switch (type)
		{
		case SelfHealGainType::Infantry:
		{
			if (pOwner->InfantrySelfHeal <= 0)
				return;

			count = pOwner->InfantrySelfHeal;
			break;
		}
		case SelfHealGainType::Units:
		{
			if (pOwner->UnitsSelfHeal <= 0)
				return;

				count = pOwner->UnitsSelfHeal;
			break;
		}
		default:
			break;
		}

		if (cap.isset() && count >= cap) {
			count = cap;
		}
/*
	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->Defeated ||
			(pHouse->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive))
			continue;

		//TODO : causing desync , disable it
		//if (allowPlayerControl && !pHouse->ControlledByCurrentPlayer())
		//	continue;

		if (pHouse != pOwner && (allowAllies && !pHouse->IsAlliedWith(pOwner)))
			continue;

		switch (type)
		{
		case SelfHealGainType::Infantry:
		{
			if (!pOwner->InfantrySelfHeal)
				continue;

			count += pHouse->InfantrySelfHeal;
			break;
		}
		case SelfHealGainType::Units:
		{
			if (!pOwner->UnitsSelfHeal)
				continue;

			count += pHouse->UnitsSelfHeal;
			break;
		}
		default:
			break;
		}

		if (cap.isset() && count >= cap)
		{
			count = cap;
			break;//dont need to loop further , end it there
		}
	}
*/
}

constexpr int countSelfHealing(TechnoClass* pThis, const bool infantryHeal)
{
	auto const pOwner = pThis->Owner;

	int count = infantryHeal ? pOwner->InfantrySelfHeal : pOwner->UnitsSelfHeal;

	const bool hasCap = infantryHeal ? RulesExtData::Instance()->InfantryGainSelfHealCap.isset() : RulesExtData::Instance()->UnitsGainSelfHealCap.isset();
	const int cap = infantryHeal ? RulesExtData::Instance()->InfantryGainSelfHealCap.Get() : RulesExtData::Instance()->UnitsGainSelfHealCap.Get();

	if (hasCap && count >= cap)
	{
		count = cap;
		return count;
	}

	const bool allowPlayerControl = RulesExtData::Instance()->GainSelfHealFromPlayerControl && SessionClass::IsCampaign()&& (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
	const bool allowAlliesInCampaign = RulesExtData::Instance()->GainSelfHealFromAllies && SessionClass::IsCampaign();
	const bool allowAlliesDefault = RulesExtData::Instance()->GainSelfHealFromAllies && !SessionClass::IsCampaign();

	if (allowPlayerControl || allowAlliesInCampaign || allowAlliesDefault)
	{
		for (auto pHouse : *HouseClass::Array)
		{
			if (pHouse != pOwner && !pHouse->Type->MultiplayPassive)
			{
				const bool isHuman = pHouse->IsControlledByHuman();

				if ((allowPlayerControl && isHuman)
					|| (allowAlliesInCampaign && !isHuman && pHouse->IsAlliedWith(pOwner))
					|| (allowAlliesDefault && pHouse->IsAlliedWith(pOwner)))
				{
					count += infantryHeal ? pHouse->InfantrySelfHeal : pHouse->UnitsSelfHeal;

					if (hasCap && count >= cap)
					{
						count = cap;
						return count;
					}
				}
			}
		}
	}

	return count;
}

constexpr bool CanDoSelfHeal(TechnoClass* pThis , SelfHealGainType type , int& amount)
{
	switch (type)
	{
	case SelfHealGainType::Infantry:
	{
		if (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames)
			return false;

		amount = RulesClass::Instance->SelfHealInfantryAmount * countSelfHealing(pThis , true);

		if (amount <= 0)
			return false;

		return true;
	}
	case SelfHealGainType::Units:
	{
		if (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames)
			return false;

		amount = RulesClass::Instance->SelfHealUnitAmount * countSelfHealing(pThis, false);

		if (amount <= 0)
			return false;

		return true;
	}

	default:
		break;
	}

	return false;
}

constexpr SelfHealGainType GetSelfHealGainType(AbstractType what, bool organic , Nullable<SelfHealGainType>& type)
{
	if(!type.isset()){
		const bool isBuilding = what == AbstractType::Building;
		const bool isOrganic = what == AbstractType::Infantry || (what == AbstractType::Unit && organic);
		return isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
	}

	return type.Get();
}


void TechnoExtData::ApplyGainedSelfHeal(TechnoClass* pThis , bool wasDamaged)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (pThis->Health)
	{
		const auto pWhat = pThis->WhatAmI();
		const bool isBuilding = pWhat == AbstractType::Building;
		const int healthDeficit = pType->Strength - pThis->Health;

		if(healthDeficit > 0) {

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
			const SelfHealGainType selfHealType = GetSelfHealGainType(pWhat, pType->Organic, pTypeExt->SelfHealGainType);
				int amount = 0;

			if (CanDoSelfHeal(pThis , selfHealType, amount)) {

				if (amount >= healthDeficit)
					amount = healthDeficit;

				if (bool(Phobos::Debug_DisplayDamageNumbers > DrawDamageMode::disabled) && Phobos::Debug_DisplayDamageNumbers < DrawDamageMode::count )
					FlyingStrings::AddNumberString(amount, pThis->Owner, AffectedHouse::All, Drawing::DefaultColors[(int)DefaultColorList::White], pThis->Location, Point2D::Empty, false, L"");

				pThis->Health += amount;
			}
		}

		if ((wasDamaged || pThis->DamageParticleSystem) && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
			|| pThis->GetHeight() < -10))
		{
			bool Rubbled = false;

			if (isBuilding) {

				const auto pBuilding = static_cast<BuildingClass*>(pThis);
				//const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

				// restore rubble after reaching green health ?
				// not sure if this good implementation
				// maybe better to put it somewhere on update function ,..
				//if(pThis->GetHealthPercentage() >= RulesClass::Instance->ConditionGreen
				//	&& (pExt->RubbleIntact || pExt->RubbleIntactRemove))
				//{
				//	auto pRubble = TechnoExt_ExtData::CreateBuilding(pBuilding,
				//				pExt->RubbleIntactRemove,
				//				pExt->RubbleIntact,
				//				pExt->RubbleIntactOwner,
				//				pExt->RubbleIntactStrength,
				//				pExt->RubbleIntactAnim
				//				);
				//
				//	if (pRubble)
				//	{
				//		const bool wasSelected = pBuilding->IsSelected;
				//		Rubbled = true;
				//
				//		if(pExt->TunnelType != -1 || pBuilding->Absorber() || pBuilding->Type->CanBeOccupied)
				//		{
				//			CellStruct Cell = pThis->GetMapCoords();
				//			if(auto pData = HouseExtData::GetTunnelVector(pBuilding->Type , pBuilding->Owner)) {
				//				if (!pData->Vector.empty() && !TunnelFuncs::FindSameTunnel(pBuilding))
				//				{
				//					int nOffset = 0;
				//					auto nPos = pData->Vector.end();
				//
				//					while (std::begin(pData->Vector) != std::end(pData->Vector))
				//					{
				//						nOffset++;
				//						const auto Offs = CellSpread::CellOfssets[nOffset % CellSpread::CellOfssets.size()];
				//						auto const& [status, pPtr] = TunnelFuncs::UnlimboOne(&pData->Vector, pBuilding, (Cell.X + Offs.X) | ((Cell.Y + Offs.Y) << 16));
				//						if (!status)
				//						{
				//							TunnelFuncs::KillFootClass(pPtr, nullptr);
				//						}
				//					}
				//
				//					pData->Vector.clear();
				//				}
				//
				//			}else
				//			if(pBuilding->Absorber())
				//			{
				//				for(auto pFirst = pBuilding->Passengers.GetFirstPassenger();
				//						 pFirst;
				//						pFirst = generic_cast<FootClass*>(pFirst->NextObject)
				//				){
				//					pRubble->KickOutUnit(pFirst, Cell);
				//				}
				//			}
				//			else
				//			if(pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count != 0) {
				//				pBuilding->KickAllOccupants(false , false);
				//				if ( pBuilding->Owner->IsControlledByCurrentPlayer() )
				//				{
				//					auto idx = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
				//					if(RadarEventClass::Create(RadarEventType::GarrisonAbandoned , Cell))
				//						VoxClass::Play(GameStrings::EVA_StructureAbandoned());
				//				}
				//			}
				//		}
				//
				//		if (wasSelected) {
				//			pRubble->Select();
				//		}
				//
				//		TechnoExtData::HandleRemove(pBuilding, nullptr, false, false);
				//	}
				//}
				//else
				{
					pBuilding->Mark(MarkType::Change);
					pBuilding->ToggleDamagedAnims(false);
				}

			}

			if(!Rubbled) {
				if (auto& dmgParticle = pThis->DamageParticleSystem) {
					dmgParticle->UnInit();
				}
			}
		}
	}

	return;
}

void TechnoExtData::ApplyDrainMoney(TechnoClass* pThis)
{
	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSource->GetTechnoType());
	const auto pRules = RulesClass::Instance();
	const auto nDrainDelay = pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay);

	if ((Unsorted::CurrentFrame % nDrainDelay) == 0)
	{
		auto nDrainAmount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount);
		if (nDrainAmount != 0)
		{
			if (!pThis->Owner->CanTransactMoney(nDrainAmount)
				|| !pSource->Owner->CanTransactMoney(nDrainAmount))
				return;

			pThis->Owner->TransactMoney(-nDrainAmount);
			pSource->Owner->TransactMoney(nDrainAmount);

			if (pTypeExt->DrainMoney_Display)
			{
				auto const pDest = pTypeExt->DrainMoney_Display_AtFirer.Get() ? pSource : pThis;
				FlyingStrings::AddMoneyString(true, nDrainAmount, pDest,
					pTypeExt->DrainMoney_Display_Houses, pDest->Location,
					pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
			}
		}
	}
}

constexpr int GetFrames(SelfHealGainType type , HouseClass* Owner){
	switch (type)
	{
	case SelfHealGainType::Infantry:
		if(Owner->InfantrySelfHeal > 0){
			return RulesClass::Instance->SelfHealInfantryFrames;
		}
		break;
	case SelfHealGainType::Units:
		if(Owner->UnitsSelfHeal > 0){
			return RulesClass::Instance->SelfHealUnitFrames;
		}
		break;
	default:
		break;
	}

	return -1;
}

bool hasSelfHeal(TechnoClass* pThis , const bool infantryHeal)
{
	auto const pOwner = pThis->Owner;

	if (infantryHeal ? pOwner->InfantrySelfHeal > 0 : pOwner->UnitsSelfHeal > 0)
		return true;

	const bool allowPlayerControl = RulesExtData::Instance()->GainSelfHealFromPlayerControl && SessionClass::IsCampaign() && (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
	const bool allowAlliesInCampaign = RulesExtData::Instance()->GainSelfHealFromAllies && SessionClass::IsCampaign();
		const bool allowAlliesDefault = RulesExtData::Instance()->GainSelfHealFromAllies && !SessionClass::IsCampaign();

	if (allowPlayerControl || allowAlliesInCampaign || allowAlliesDefault) {
		for (auto pHouse : *HouseClass::Array) {
			if (pHouse != pOwner && !pHouse->Type->MultiplayPassive) {
				const bool isHuman = pHouse->IsControlledByHuman();

				if ((allowPlayerControl && isHuman)
					|| (allowAlliesInCampaign && !isHuman && pHouse->IsAlliedWith(pOwner))
					|| (allowAlliesDefault && pHouse->IsAlliedWith(pOwner)))
				{
					if (infantryHeal ? pHouse->InfantrySelfHeal > 0 : pHouse->UnitsSelfHeal > 0)
						return true;
				}
			}
		}
	}

	return false;
}

void TechnoExtData::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->Owner->Type->MultiplayPassive && !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive)
		return;

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pWhat = pThis->WhatAmI();
	const bool isOrganic = pWhat == InfantryClass::AbsID
		|| (pType->Organic && (pWhat == UnitClass::AbsID));

	auto const selfHealType = GetSelfHealGainType(pWhat , isOrganic, pExt->SelfHealGainType) ;

	if (selfHealType == SelfHealGainType::None || !hasSelfHeal(pThis, isOrganic))
		return;

	int selfHealFrames = GetFrames(selfHealType, pThis->Owner);

	if(selfHealFrames <= 0 )
		return;

	{
		Point2D pipFrames { 0,0 };
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		int nBracket = TechnoExtData::GetDisguiseType(pThis, false, true).first->PixelSelectionBracketDelta;

		switch (pWhat)
		{
		case UnitClass::AbsID:
		case AircraftClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Units.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Infantry.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case BuildingClass::AbsID:
		{
			const auto pBldType = static_cast<BuildingTypeClass*>(pType);
			int fHeight = pBldType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Buildings.Get();
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pBldType->Height * yAdjust;
		}
		break;
		default:
			break;
		}

		int pipFrame = selfHealType == SelfHealGainType::Infantry ? pipFrames.X : pipFrames.Y;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExtData::TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();

	// turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, TechnoTypeExtData::TurretMultiOffsetDefaultMult);
		double turretRad = (pThis->TurretFacing().GetFacing<32>() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.Current().GetFacing<32>() - 8) * -(Math::Pi / 16);
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}
}

Matrix3D TechnoExtData::GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Matrix3D nMTX = TechnoExtData::GetTransform(flag_cast_to<FootClass*, false>(pThis));
	TechnoExtData::TransformFLHForTurret(pThis, nMTX, isOnTurret);

	// apply FLH offset
	nMTX.Translate((float)nCoord.X, (float)nCoord.Y, (float)nCoord.Z);

	return nMTX;
}

CoordStruct TechnoExtData::GetFLHAbsoluteCoordsB(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Vector3D<float> result {};
	Matrix3D mtx = TechnoExtData::GetFLHMatrix(pThis, nCoord, isOnTurret);
	Matrix3D::MatrixMultiply(&result , &mtx , &Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

void TechnoExtData::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	const auto pType = pThis->GetTechnoType();

	if (!pType->OpenTopped || !pThis->Passengers.NumPassengers)
		return;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pExt->Ammo_Shared || !pType->Ammo)
		return;

	auto passenger = pThis->Passengers.FirstPassenger;

	do
	{
		TechnoTypeClass* passengerType = passenger->GetTechnoType();

		if (TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared.Get())
		{
			if (pExt->Ammo_Shared_Group.Get() < 0 ||
				pExt->Ammo_Shared_Group.Get() == TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared_Group.Get())
			{
				if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
				{
					pThis->Ammo--;
					passenger->Ammo++;
				}
			}
		}

		passenger = static_cast<FootClass*>(passenger->NextObject);

	}
	while (passenger);
}

double TechnoExtData::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	const double houseMultiplier = pThis->Owner->GetSpeedMult(pThis->GetTechnoType());
	double speed = pThis->SpeedMultiplier * houseMultiplier;

	if(pThis->HasAbility(AbilityType::Faster))
		speed *=  RulesClass::Instance->VeteranSpeed;

	return speed;
}

void TechnoExtData::UpdateMindControlAnim()
{
	const auto pThis = This();

	if (pThis->IsMindControlled())
	{
		if (pThis->MindControlRingAnim && !MindControlRingAnimType)
		{
			MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && MindControlRingAnimType &&
			pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;
			const auto pBuilding = cast_to<BuildingClass*, false>(pThis);

			if (pBuilding)
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;

			{
				auto anim = GameCreate<AnimClass>(MindControlRingAnimType, coords, 0, 1);
				pThis->MindControlRingAnim = anim;
				pThis->MindControlRingAnim->SetOwnerObject(pThis);

				if (pBuilding)
					pThis->MindControlRingAnim->ZAdjust = -1024;
			}
		}
	}
	else if (MindControlRingAnimType)
	{
		MindControlRingAnimType = nullptr;
	}
}

std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> TechnoExtData::GetFireSelfData()
{
	const auto pThis = This();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty() && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_RedHeath , &pTypeExt->FireSelf_ROF_RedHeath };
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_YellowHeath , &pTypeExt->FireSelf_ROF_YellowHeath };
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_GreenHeath , &pTypeExt->FireSelf_ROF_GreenHeath };
	}

	return  { &pTypeExt->FireSelf_Weapon , &pTypeExt->FireSelf_ROF };

}

void TechnoExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (auto& pShieldData = this->Shield)
			pShieldData->SetAnimationVisibility(false);

		for (auto& pos : this->LaserTrails)
		{
			pos->Visible = false;
			pos->LastLocation.clear();
		}

		TrailsManager::Hide(This());

		this->IsInTunnel = true;
	}

	const auto pType = This()->GetTechnoType();
	const auto pImage = pType->AlphaImage;

	if (pImage)
	{
		auto& alphaExt = StaticVars::ObjectLinkedAlphas;

		if (const auto pAlpha = alphaExt.get_or_default(This()))
		{
			GameDelete<true,false>(pAlpha);

			const auto tacticalPos = TacticalClass::Instance->TacticalPos;
			Point2D off = { tacticalPos.X - (pImage->Width / 2), tacticalPos.Y - (pImage->Height / 2) };
			const auto point = TacticalClass::Instance->CoordsToClient(This()->GetCoords()) + off;
			RectangleStruct dirty = { point.X - tacticalPos.X, point.Y - tacticalPos.Y, pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(dirty, true);
		}
	}
}

std::pair<WeaponTypeClass*, int> TechnoExtData::GetDeployFireWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pType = pThis->GetTechnoType();
	int weaponIndex = pType->DeployFireWeapon;

	if (pThis->WhatAmI() == UnitClass::AbsID)
	{
		if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				if (pThis->GetFireError(pThis->GetCell(), 0, true) != FireError::OK)
					weaponIndex = 1;
			}
		}
	}

	if (weaponIndex == -1)
	{
		weaponIndex = pThis->SelectWeapon(pTarget);
	}
	else if (weaponIndex < -1)
		return { nullptr   , -1 };

	if (auto const pWs = pThis->GetWeapon(weaponIndex))
	{
		return { pWs->WeaponType  , weaponIndex };
	}

	return { nullptr , -1 };
}

void TechnoExtData::UpdateType(TechnoTypeClass* currentType)
{
	static_assert(true, "Donot Use!");
	//auto const pThis = This();
	//const auto pOldType = this->Type;
	//const auto pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);
	//this->Type = currentType;
	//auto const pTypeExtData = TechnoTypeExtContainer::Instance.Find(currentType);

	//TechnoExtData::InitializeLaserTrail(pThis, true);

	// Reset Shield
	// This part should have been done by UpdateShield

	// Reset AutoDeath Timer
	//if (this->Death_Countdown.HasStarted())
	//{
	//	this->Death_Countdown.Stop();

	//	{
	//		HouseExtData::AutoDeathObjects.erase(pThis);
	//	}
	//}

	// Reset PassengerDeletion Timer - TODO : unchecked
	//if (this->PassengerDeletionTimer.IsTicking()
	//	&& !pTypeExtData->PassengerDeletionType.Enabled)
	//	this->PassengerDeletionTimer.Stop();

	//TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);

	/*if (!pTypeExtData->MyFighterData.Enable && this->MyFighterData)
		this->MyFighterData.reset(nullptr);

	else if (pTypeExtData->MyFighterData.Enable && !this->MyFighterData)
	{
		this->MyFighterData = std::make_unique<FighterAreaGuard>();
		this->MyFighterData->OwnerObject = (AircraftClass*)pThis;
	}

	if(!pTypeExtData->DamageSelfData.Enable && this->DamageSelfState)
		this->DamageSelfState.reset(nullptr);
	else if (pTypeExtData->DamageSelfData.Enable && !this->DamageSelfState)
		DamageSelfState::OnPut(this->DamageSelfState, pTypeExtData->DamageSelfData);

	if (!pTypeExtData->MyGiftBoxData.Enable && this->MyGiftBox)
		this->MyGiftBox.reset(nullptr);
	else if (pTypeExtData->MyGiftBoxData.Enable && !this->MyGiftBox)
		GiftBoxFunctional::Init(this, pTypeExtData);*/

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	//bool toOpenTopped = currentType->OpenTopped && !pOldType->OpenTopped;

	//if ((toOpenTopped || (!currentType->OpenTopped && pOldType->OpenTopped)) && pThis->Passengers.NumPassengers > 0)
	//{
	//	auto pPassenger = pThis->Passengers.FirstPassenger;

	//	while (pPassenger)
	//	{
	//		if (toOpenTopped)
	//		{
	//			pThis->EnteredOpenTopped(pPassenger);
	//		}
	//		else
	//		{
	//			pThis->ExitedOpenTopped(pPassenger);

	//			// Lose target & destination
	//			pPassenger->Guard();

	//			// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
	//			// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
	//			MapClass::Logics.get().RemoveObject(pPassenger);
	//		}

	//		pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
	//	}
	//}
}

void TechnoExtData::UpdateBuildingLightning()
{
	auto const pThis = This();

	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto pBldExt = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis));
	if (pBldExt->LighningNeedUpdate)
	{
		pThis->Mark(MarkType::Redraw);
		pBldExt->LighningNeedUpdate = false;

	}
}

void TechnoExtData::UpdateShield()
{
	auto const pThis = This();

	if (!this->CurrentShieldType)
		Debug::FatalErrorAndExit("Techno[%s] Missing CurrentShieldType ! ", pThis->get_ID());

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	// Set current shield type if it is not set.
	if (!this->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		this->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && !this->Shield)
	{
		this->Shield = std::make_unique<ShieldClass>(pThis);
		this->Shield->UpdateTint();
	}

	if (const  auto pShieldData = this->GetShield())
		pShieldData->OnUpdate();
}

#include <Ext/Cell/Body.h>

void TechnoExtData::UpdateRevengeWeapons()
{
	this->RevengeWeapons.remove_all_if([](TimedWarheadValue<WeaponTypeClass*>& item) {
	   return item.Timer.Expired();
	});
}

void TechnoExtData::UpdateAircraftOpentopped()
{
	auto const pThis = This();

	if (!TechnoExtData::IsAlive(pThis, true, true, true))
		return;

	const auto pType = pThis->GetTechnoType();

	if (pType->Passengers > 0 && !AircraftOpentoppedInitEd)
	{

		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (auto const pInf = flag_cast_to<FootClass*, false>(*object))
			{
				if (!pInf->Transporter || !pInf->InOpenToppedTransport)
				{
					if (pType->OpenTopped)
						pThis->EnteredOpenTopped(pInf);

					if (pType->Gunner)
						flag_cast_to<FootClass*, false>(pThis)->ReceiveGunner(pInf);

					pInf->Transporter = pThis;
					pInf->Undiscover();
				}
			}
		}

		AircraftOpentoppedInitEd = true;
	}
}

bool TechnoExtData::HasAmmoToDeploy(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	const int min = pTypeExt->Ammo_DeployUnlockMinimumAmount;
	const int max = pTypeExt->Ammo_DeployUnlockMaximumAmount;

	if (min < 0 && max < 0)
		return true;

	const int ammo = pThis->Ammo;

	if ((min < 0 || ammo >= min) && (max < 0 || ammo <= max))
		return true;

	return false;
}

void TechnoExtData::HandleOnDeployAmmoChange(TechnoClass* pThis, int maxAmmoOverride)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	int add = pTypeExt->Ammo_AddOnDeploy;

	if (add != 0)
	{
		int maxAmmo = pType->Ammo;

		if (maxAmmoOverride >= 0)
			maxAmmo = maxAmmoOverride;

		int originalAmmo = pThis->Ammo;
		pThis->Ammo = std::clamp(pThis->Ammo + add, 0, maxAmmo);

		if (originalAmmo != pThis->Ammo)
		{
			pThis->StartReloading();
			pThis->Mark(MarkType::Change);
		}
	}
}

bool TechnoExtData::SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes) {
	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return defaultValue;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pTypeConvert = pTypeExt->Convert_Deploy;

	if (alwaysCheckLandTypes || pTypeExt->IsSimpleDeployer_ConsiderPathfinding) {
		bool isHover = pType->Locomotor == HoverLocomotionClass::ClassGUID();
		bool isJumpjet = pType->Locomotor == JumpjetLocomotionClass::ClassGUID();
		bool isLander = pType->DeployToLand && (isJumpjet || isHover);
		auto const defaultLandTypes = isLander ? (LandTypeFlags)(LandTypeFlags::Water | LandTypeFlags::Beach) : LandTypeFlags::None;
		auto const disallowedLandTypes = pTypeExt->IsSimpleDeployer_DisallowedLandTypes.Get(defaultLandTypes);

		if (IsLandTypeInFlags(disallowedLandTypes, pThis->GetCell()->LandType))
			return false;

		if (alwaysCheckLandTypes && !pTypeExt->IsSimpleDeployer_ConsiderPathfinding)
			return true;
	} else {
		return defaultValue;
	}

	const SpeedType speed = pTypeConvert ? pTypeConvert->SpeedType : pType->SpeedType;
	const MovementZone mZone = pTypeConvert ? pTypeConvert->MovementZone : pType->MovementZone;

	if (speed != SpeedType::None && mZone != MovementZone::None) {
		auto const pCell = pThis->GetCell();
		return pCell->IsClearToMove(speed, true, true, ZoneType::None, mZone, -1, pCell->ContainsBridge());
	}

	return true;
}

void TechnoExtData::DepletedAmmoActions()
{
	auto const pThis = this->This();
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->Ammo <= 0)
		return;

	auto const rtti = pThis->WhatAmI();
	UnitClass* pUnit = nullptr;

	if (rtti == AbstractType::Unit)
	{
		pUnit = static_cast<UnitClass*>(pThis);
		auto const pUnitType = pUnit->Type;

		if (!pUnitType->IsSimpleDeployer && !pUnitType->DeploysInto && !pUnitType->DeployFire
			&& pUnitType->Passengers < 1 && pUnit->Passengers.NumPassengers < 1)
		{
			return;
		}
	}

	int const min = pTypeExt->Ammo_AutoDeployMinimumAmount;
	int const max = pTypeExt->Ammo_AutoDeployMaximumAmount;

	if (min < 0 && max < 0)
		return;

	int const ammo = pThis->Ammo;
	bool canDeploy = TechnoExtData::HasAmmoToDeploy(pThis) && (min < 0 || ammo >= min) && (max < 0 || ammo <= max);
	bool isDeploying = pThis->CurrentMission == Mission::Unload || pThis->QueuedMission == Mission::Unload;

	if (canDeploy && !isDeploying)
	{
		pThis->QueueMission(Mission::Unload, true);
	}
	else if (!canDeploy && isDeploying)
	{
		pThis->QueueMission(Mission::Guard, true);

		if (pUnit && pUnit->Type->IsSimpleDeployer && pThis->InAir)
		{
			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
				pJJLoco->NextState = JumpjetLocomotionClass::State::Ascending;
		}
	}
}

void TechnoExtData::UpdateLaserTrails()
{
	auto const pThis = (FootClass*)This();

	if (LaserTrails.empty())
		return;

	const bool IsDroppod = VTable::Get(pThis->Locomotor.GetInterfacePtr()) == DropPodLocomotionClass::vtable;

	for (auto& trail : LaserTrails)
	{
		if (trail->Type->DroppodOnly && !IsDroppod)
			continue;

		if (pThis->CloakState == CloakState::Cloaked)
		{
			if (!trail->Type->CloakVisible)
			{
				trail->Cloaked = true;
			} else if (trail->Type->CloakVisible_Houses && !HouseClass::IsCurrentPlayerObserver() && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer)) {
				auto const pCell = pThis->GetCell();
				trail->Cloaked = !pCell || !pCell->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
			}
		}

		if (!IsInTunnel)
			trail->Visible = true;

		if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir() && trail->LastLocation.isset())
			trail->LastLocation.clear();

		CoordStruct trailLoc = TechnoExtData::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret);
		if (pThis->CloakState == CloakState::Uncloaking && !trail->Type->CloakVisible)
		trail->LastLocation = trailLoc;
		else
		trail->Update(trailLoc);
	}
}

void TechnoExtData::UpdateGattlingOverloadDamage()
{
	auto const pThis = This();

	if (!pThis->IsAlive)
		return;

	const auto pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);


	if (!pType->IsGattling || !pTypeExt->Gattling_Overload.Get())
		return;

	auto const nDelay = Math::abs(pTypeExt->Gattling_Overload_Frames.Get(0));

	if (!nDelay)
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

		GattlingDmageDelay = nDelay;
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			GattlingDmageSound = false;
		}
		else
		{
			auto const pWarhead = pTypeExt->Gattling_Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pThis->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.isset())
					VocClass::SafeImmedietelyPlayAt(pTypeExt->Gattling_Overload_DeathSound, &pThis->Location, 0);

				GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get(nullptr))
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(1); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random(-200, 200);
					auto nLoc = pThis->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->Owner);
				}
			}

			if (pThis->WhatAmI() == UnitClass::AbsID && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioClass::Instance->Random.RandomBool() ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--GattlingDmageDelay;
	}
}

bool TechnoExtData::UpdateKillSelf_Slave()
{
	auto const pThis = This();

	if (VTable::Get(pThis) != InfantryClass::vtable)
		return false;

	const auto pInf = static_cast<InfantryClass*>(pThis);

	if (!pInf->Type->Slaved)
		return false;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pInf->Type);

	if (!pInf->SlaveOwner && (pTypeExt->Death_WithMaster.Get()
		|| pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide))
	{

		const KillMethod nMethod = pTypeExt->Death_Method.Get();

		if (nMethod != KillMethod::None)
			TechnoExtData::KillSelf(pInf, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	}

	return !pThis->IsAlive;
}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExtData::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno,
 AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (auto const pCell = cast_to<CellClass*>(pTarget))
			pTargetCell = pCell;
		else if (auto const pObject = cast_to<ObjectClass*>(pTarget)) {
			if (!pObject->IsAlive)
				pTargetCell = MapClass::Instance->TryGetCellAt(pObject->Location);
			else
				pTargetCell = pObject->GetCell();
		}
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	auto const pWeaponOne = pWeaponStructOne->WeaponType;
	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (auto const pSecondExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponTwo))
	{
		if(!pSecondExt->SkipWeaponPicking){
			if(pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true , true))
				return weaponIndexOne;

			if (
				(pTargetTechno &&
					(
						!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
						!TechnoExtData::ObjectHealthAllowFiring(pTargetTechno, pWeaponTwo) ||
						!pSecondExt->HasRequiredAttachedEffects(pTargetTechno, pThis)
					)))
			{
				return weaponIndexOne;
			}
		}

		if (auto const pFirstExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponOne))
		{
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
			const bool firstAllowedAE = pFirstExt->HasRequiredAttachedEffects(pTargetTechno, pThis);

			if (!allowFallback && (!allowAAFallback || !secondaryIsAA)
					&& !TechnoExtData::CanFireNoAmmoWeapon(pThis, 1) && firstAllowedAE)
				return weaponIndexOne;

			if(!pFirstExt->SkipWeaponPicking){
				if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true , true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner)
						|| !firstAllowedAE
						|| !TechnoExtData::ObjectHealthAllowFiring(pTargetTechno, pWeaponOne)
						)))
					{
					return weaponIndexTwo;
				}
			}
		}
	}

	auto const pType = pThis->GetTechnoType();

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && pTargetCell && (pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary)
		&& pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
	{
		return weaponIndexTwo;
	}

	return -1;
}

bool TechnoExtData::IsInWarfactory(TechnoClass* pThis, bool bCheckNaval)
{
	if (pThis->WhatAmI() != UnitClass::AbsID || pThis->IsInAir() || !pThis->IsTethered)
		return false;

	auto const pContact = pThis->GetNthLink();

	if (!pContact)
		return false;

	auto const pCell = pThis->GetCell();

	if (!pCell)
		return false;

	auto const pBld = pCell->GetBuilding();

	if (!pBld || pBld != pContact)
		return false;

	if (pBld->Type->WeaponsFactory || (bCheckNaval && pBld->Type->Naval))
	{
		return true;
	}

	return false;
}

CoordStruct TechnoExtData::GetPutLocation(CoordStruct current, int distance)
{
	// this whole thing does not at all account for cells which are completely occupied.
	const CellStruct tmpCoords = CellSpread::AdjacentCell[ScenarioClass::Instance->Random.RandomFromMax(7)];

	current.X += tmpCoords.X * distance;
	current.Y += tmpCoords.Y * distance;

	const auto tmpCell = MapClass::Instance->TryGetCellAt(current);

	if (!tmpCell)
	{
		return CoordStruct::Empty;
	}

	return tmpCell->FindInfantrySubposition(current, false, false, false, current.Z);
}

bool TechnoExtData::EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select, std::optional<bool> InAir)
{
	const CellClass* pCell = MapClass::Instance->TryGetCellAt(loc);
	//auto pType = Survivor->GetTechnoType();

	if (const auto pBld = pCell->GetBuilding())
	{
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

		if (!pTypeExt->IsPassable)
			return false;

		if (pTypeExt->Firestorm_Wall &&
			pBld->Owner &&
			pBld->Owner->FirestormActive)
			return false;
	}

	Survivor->OnBridge = pCell->ContainsBridge();
	const int floorZ = pCell->GetCoordsWithBridge().Z;

	bool Chuted = false;
	if(!InAir.has_value()){
		Chuted = (loc.Z - floorZ > 2 * Unsorted::LevelHeight);
	}
	else { Chuted = InAir.value(); }

	if (Chuted)
	{
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Limbo();

		if (!Survivor->SpawnParachuted(loc) || pCell->GetBuilding())
		{
			return false;
		}
	}
	else
	{
		loc.Z = floorZ;
		//if (!MapClass::Instance->GetCellAt(loc)->IsClearToMove(pType->SpeedType, pType->MovementZone))
		//	return false;

		if (!Survivor->Unlimbo(loc, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::North, DirType::NorthWest)))
		{
			return false;
		}
	}

	Survivor->Transporter = nullptr;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if (Chuted)
	{
		const bool scat = Survivor->OnBridge;
		const auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;

		if ((occupation & 0x1C) == 0x1C)
		{
			pCell->ScatterContent(CoordStruct::Empty, true, true, scat);
		}
	}
	else
	{
		Survivor->Scatter(CoordStruct::Empty, true, false);
		Survivor->QueueMission(!Survivor->Owner->IsControlledByHuman() ? Mission::Hunt : Mission::Guard, 0);
	}

	Survivor->ShouldEnterOccupiable = false;
	Survivor->ShouldGarrisonStructure = false;

	if (Select)
	{
		Survivor->Select();
	}

	return true;
}

void TechnoExtData::EjectPassengers(FootClass* pThis, int howMany)
{
	if (howMany < 0)
	{
		howMany = pThis->Passengers.NumPassengers;
	}

	auto const openTopped = pThis->GetTechnoType()->OpenTopped;

	for (int i = 0; i < howMany && pThis->Passengers.FirstPassenger; ++i)
	{
		FootClass* passenger = pThis->RemoveFirstPassenger();
		if (!EjectRandomly(passenger, pThis->Location, 128, false))
		{
			passenger->RegisterDestruction(nullptr);
			passenger->UnInit();
		}
		else if (openTopped)
		{
			pThis->ExitedOpenTopped(passenger);
		}
	}
}

bool TechnoExtData::EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select, std::optional<bool> InAir)
{
	CoordStruct destLoc = GetPutLocation(location, distance);

	if (destLoc == CoordStruct::Empty || !MapClass::Instance->IsWithinUsableArea(destLoc))
		return false;

	return EjectSurvivor(pEjectee, destLoc, select , InAir);
}

void TechnoExtData::ReplaceArmor(Armor& armor, ObjectClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoExtData::ReplaceArmor(armor, pTarget, pWeapon->Warhead);
}

void TechnoExtData::ReplaceArmor(Armor& armor, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoExtData::ReplaceArmor(armor, pTarget, pWeapon->Warhead);
}

void TechnoExtData::ReplaceArmor(Armor& armor, ObjectClass* pTarget, WarheadTypeClass* pWH)
{
	if(pTarget->AbstractFlags & AbstractFlags::Techno) {
		TechnoExtData::ReplaceArmor(armor , (TechnoClass*)pTarget, pWH);
	}
}

void TechnoExtData::ReplaceArmor(Armor& armor, TechnoClass* pTarget, WarheadTypeClass* pWH)
{
	if(const auto& pShieldData = TechnoExtContainer::Instance.Find(pTarget)->Shield){
		if(pShieldData->IsActive() && !pShieldData->CanBePenetrated(pWH)){
			armor = pShieldData->GetArmor(armor);
		}
	}
}

int TechnoExtData::GetInitialStrength(TechnoTypeClass* pType, int nHP)
{
	return TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(nHP);
}

bool TechnoExtData::IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger)
{
	auto pThisType = pThis->GetTechnoType();
	auto const pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);
	auto pThatType = pPassanger->GetTechnoType();

	if (pThatType->Size > pThisType->SizeLimit)
		return false;

	if (pThisTypeExt->Passengers_BySize.Get())
	{
		if (pThatType->Size > (pThisType->Passengers - pThis->Passengers.GetTotalSize()))
			return false;
	}
	else if (pThis->Passengers.NumPassengers >= pThisType->Passengers)
	{
		return false;
	}

	return true;
}

bool TechnoExtData::IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource)
{
	if (!pThis || !pSource)
		return false;

	auto const pType = pThis->GetTechnoType();

	if (!pType->TypeImmune)
		return false;

	return pType == pSource->GetTechnoType() && pThis->Owner == pSource->Owner;
}

void TechnoExtData::InitializeUnitIdleAction(TechnoClass* pThis, TechnoTypeClass* pType)
{
	if (pThis->WhatAmI() != AbstractType::Unit || !pThis->HasTurret())
		return;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pTypeExt->AutoFire || pType->TurretSpins)
		return;

	if (pTypeExt->UnitIdleRotateTurret.Get(RulesExtData::Instance()->UnitIdleRotateTurret))
		pExt->UnitIdleAction = true;

	if (!SessionClass::IsSingleplayer())
		return;

	if (pTypeExt->UnitIdlePointToMouse.Get(RulesExtData::Instance()->UnitIdlePointToMouse))
		pExt->UnitIdleActionSelected = true;
}

void TechnoExtData::StopIdleAction()
{
	if (!this->UnitIdleAction)
		return;

	if (this->UnitIdleActionTimer.IsTicking())
		this->UnitIdleActionTimer.Stop();

	if (this->UnitIdleActionGapTimer.IsTicking())
	{
		this->UnitIdleActionGapTimer.Stop();
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(This()->GetTechnoType());
		this->StopRotateWithNewROT(pTypeExt->TurretRot.Get(pTypeExt->This()->ROT));
	}
}

void TechnoExtData::ApplyIdleAction()
{
	TechnoClass* const pThis = This();
	FacingClass* const turret = &pThis->SecondaryFacing;

	if (this->UnitIdleActionTimer.Completed()) // Set first direction
	{
		this->UnitIdleActionTimer.Stop();
		this->UnitIdleActionGapTimer.Start(ScenarioClass::Instance->Random.
			RandomRanged(RulesExtData::Instance()->UnitIdleActionIntervalMin, RulesExtData::Instance()->UnitIdleActionIntervalMax));
		bool noNeedTurnForward = false;

		if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
			noNeedTurnForward = pUnit->BunkerLinkedItem ||!pUnit->Type->Speed || (pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
		else if (pThis->WhatAmI() == AbstractType::Building)
			noNeedTurnForward = true;

		const double extraRadian = ScenarioClass::Instance->Random.RandomDouble() - 0.5;

		DirStruct unitIdleFacingDirection;
		unitIdleFacingDirection.SetRadian<32>(pThis->PrimaryFacing.Current().GetRadian<32>() + (noNeedTurnForward ? extraRadian * Math::TwoPi : extraRadian));

		this->StopRotateWithNewROT(ScenarioClass::Instance->Random.RandomRanged(2, 4) >> 1);
		turret->Set_Desired(unitIdleFacingDirection);
	}
	else if (this->UnitIdleActionGapTimer.IsTicking()) // Check change direction
	{
		if (!this->UnitIdleActionGapTimer.HasTimeLeft()) // Set next direction
		{
			this->UnitIdleActionGapTimer.Start(ScenarioClass::Instance->Random.
				RandomRanged(RulesExtData::Instance()->UnitIdleActionIntervalMin, RulesExtData::Instance()->UnitIdleActionIntervalMax));
			bool noNeedTurnForward = false;

			if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
				noNeedTurnForward = pUnit->BunkerLinkedItem ||!pUnit->Type->Speed || (pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
			else if (pThis->WhatAmI() == AbstractType::Building)
				noNeedTurnForward = true;

			const double extraRadian = ScenarioClass::Instance->Random.RandomDouble() - 0.5;

			DirStruct unitIdleFacingDirection;
			unitIdleFacingDirection.SetRadian<32>(pThis->PrimaryFacing.Current().GetRadian<32>() + (noNeedTurnForward ? extraRadian * Math::TwoPi : extraRadian));

			this->StopRotateWithNewROT(ScenarioClass::Instance->Random.RandomRanged(2, 4) >> 1);
			turret->Set_Desired(unitIdleFacingDirection);
		}
	}
	else if (!this->UnitIdleActionTimer.IsTicking()) // In idle now
	{
		this->UnitIdleActionTimer.Start(ScenarioClass::Instance->Random.
			RandomRanged(RulesExtData::Instance()->UnitIdleActionRestartMin, RulesExtData::Instance()->UnitIdleActionRestartMax));
		bool noNeedTurnForward = false;

		if (UnitClass* const pUnit = cast_to<UnitClass*, false>(pThis))
			noNeedTurnForward = pUnit->BunkerLinkedItem || !pUnit->Type->Speed ||(pUnit->Type->IsSimpleDeployer && pUnit->Deployed);
		else if (pThis->WhatAmI() == AbstractType::Building)
			noNeedTurnForward = true;

		if (!noNeedTurnForward)
			turret->Set_Desired(pThis->PrimaryFacing.Current());
	}
}

void TechnoExtData::ManualIdleAction()
{
	TechnoClass* const pThis = This();
	FacingClass* const turret = &pThis->SecondaryFacing;

	if (pThis->IsSelected)
	{
		this->StopIdleAction();
		this->UnitIdleIsSelected = true;
		const CoordStruct mouseCoords = TacticalClass::Instance->ClientToCoords(WWMouseClass::Instance->XY1);

		if (mouseCoords != CoordStruct::Empty) // Mouse in tactical
		{
			CoordStruct technoCoords = This()->GetCoords();
			const int offset = -static_cast<int>(technoCoords.Z * ((Unsorted::LeptonsPerCell / 2.0) / Unsorted::LevelHeight));
			const double nowRadian = Math::atan2(double(technoCoords.Y + offset - mouseCoords.Y), double(mouseCoords.X - technoCoords.X - offset)) - 0.125;
			DirStruct unitIdleFacingDirection;
			unitIdleFacingDirection.SetRadian<32>(nowRadian);
			turret->Set_Desired(unitIdleFacingDirection);
		}
	}
	else if (this->UnitIdleIsSelected) // Immediately stop when is not selected
	{
		this->UnitIdleIsSelected = false;
		this->StopRotateWithNewROT();
	}
}

void TechnoExtData::StopRotateWithNewROT(int ROT)
{
	FacingClass* const turret = &This()->SecondaryFacing;

	const DirStruct currentFacingDirection = turret->Current();
	turret->DesiredFacing = currentFacingDirection;
	turret->StartFacing = currentFacingDirection;
	turret->RotationTimer.Start(0);

	if (ROT >= 0)
		turret->Set_ROT(ROT);
}

// =============================
// load / save

void TechnoExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{

	this->RadioExtData::InvalidatePointer(ptr, bRemoved);

	if (auto pSpawn = (FakeSpawnManagerClass*)This()->SpawnManager)
		pSpawn->_DetachB(ptr, bRemoved);

	MyWeaponManager.InvalidatePointer(ptr, bRemoved);

	AnnounceInvalidPointer(LinkedSW, ptr);
	AnnounceInvalidPointer(OriginalPassengerOwner, ptr);
	AnnounceInvalidPointer(GarrisonedIn, ptr , bRemoved);
	AnnounceInvalidPointer(WebbyLastTarget, ptr);
	AnnounceInvalidPointer(BuildingLight, ptr);
	AnnounceInvalidPointer(AirstrikeTargetingMe, ptr);

	for (auto& _phobos_AE : PhobosAE) {
		if (_phobos_AE) {
			_phobos_AE->InvalidatePointer(ptr, bRemoved);
		}
	}

	if (ptr && bRemoved)
	{
		auto& AttackerDatas = this->OnlyAttackData;
		if (!AttackerDatas.empty())
		{
			for (int index = int(AttackerDatas.size()) - 1; index >= 0; --index)
			{
				if (AttackerDatas[index].Attacker != ptr)
					continue;

				AttackerDatas.erase(AttackerDatas.begin() + index);
			}
		}
	}
}

TechnoExtContainer TechnoExtContainer::Instance;

void AEProperties::Recalculate(TechnoClass* pTechno) {

	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	auto _AresAE = &pExt->AeData;
	auto _AEProp = &pExt->AE;

	double ROF_Mult = 1.0;
	double ReceiveRelativeDamageMult = 1.0;
	double FP_Mult = _AEProp->FirepowerMultiplier;
	double Armor_Mult = _AEProp->ArmorMultiplier;
	double Speed_Mult = _AEProp->SpeedMultiplier;

	bool Cloak = pTechno->GetTechnoType()->Cloakable || pTechno->HasAbility(AbilityType::Cloak);

	bool forceDecloak = false;
	bool disableWeapons = false;
	bool disableSelfHeal = false;
	bool untrackable = false;
	bool disableRadar = false;
	bool disableSpySat = false;
	bool unkillable = false;
	auto extraRangeData = &_AEProp->ExtraRange;
	auto extraCritData = &_AEProp->ExtraCrit;
	auto armormultData = &_AEProp->ArmorMultData;
	bool hasExtraWH = false;
	bool hasFeedbackWeapon = false;

	extraRangeData->Clear();
	extraCritData->Clear();
	armormultData->Clear();

	bool hasTint = false;
	bool reflectsDamage = false;
	bool hasOnFireDiscardables = false;

	std::optional<double> cur_timerAE {};

	for (const auto& aeData : _AresAE->Data)
	{
		if (aeData.Type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!cur_timerAE.has_value())
				cur_timerAE = aeData.Type->ROFMultiplier;
			else
				cur_timerAE.value() *= aeData.Type->ROFMultiplier;
		}

		ROF_Mult *= aeData.Type->ROFMultiplier;
		ReceiveRelativeDamageMult += aeData.Type->ReceiveRelativeDamageMult;
		FP_Mult *= aeData.Type->FirepowerMultiplier;
		Speed_Mult *= aeData.Type->SpeedMultiplier;
		Armor_Mult *= aeData.Type->ArmorMultiplier;
		Cloak |= aeData.Type->Cloakable;
		forceDecloak |= aeData.Type->ForceDecloak;
		disableWeapons |= aeData.Type->DisableWeapons;
		disableSelfHeal |= aeData.Type->DisableSelfHeal;
		untrackable |= aeData.Type->Untrackable;
		disableRadar |= aeData.Type->DisableRadar;
		disableSpySat |= aeData.Type->DisableSpySat;
		unkillable |= aeData.Type->Unkillable;
		hasExtraWH |= aeData.Type->ExtraWarheads.size() > 0;

		if (!(aeData.Type->WeaponRange_Multiplier == 1.0 && aeData.Type->WeaponRange_ExtraRange == 0.0)) {

			auto& ranges_ = extraRangeData->ranges.emplace_back();
			ranges_.rangeMult = aeData.Type->WeaponRange_Multiplier;
			ranges_.extraRange = aeData.Type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;
			for (auto& allow : aeData.Type->WeaponRange_AllowWeapons)
				ranges_.allow.insert(allow);

			for (auto& disallow : aeData.Type->WeaponRange_DisallowWeapons)
				ranges_.disallow.insert(disallow);
		}
	}

	VectorSet<PhobosAttachEffectTypeClass*> cumulativeTypes {};

	for (const auto& attachEffect : pExt->PhobosAE) {

		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();
		FP_Mult *= type->FirepowerMultiplier;
		Speed_Mult *= type->SpeedMultiplier;
		ROF_Mult *= type->ROFMultiplier;
		Cloak |= type->Cloakable;
		forceDecloak |= type->ForceDecloak;
		disableWeapons |= type->DisableWeapons;
		disableSelfHeal |= type->DisableSelfHeal;
		untrackable |= type->Untrackable;
		ReceiveRelativeDamageMult += type->ReceiveRelativeDamageMult;
		hasTint |= type->HasTint();
		unkillable |= type->Unkillable;
		disableRadar |= type->DisableRadar;
		disableSpySat |= type->DisableSpySat;
		hasExtraWH |= type->ExtraWarheads.size() > 0;
		hasFeedbackWeapon |= type->FeedbackWeapon != nullptr;

		if (type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!cur_timerAE.has_value())
				cur_timerAE = type->ROFMultiplier;
			else
				cur_timerAE.value() *= type->ROFMultiplier;
		}

		if (!(type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0))
		{
			auto& ranges_ = extraRangeData->ranges.emplace_back();
			ranges_.rangeMult = type->WeaponRange_Multiplier;
			ranges_.extraRange = type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;

			for (auto& allow : type->WeaponRange_AllowWeapons)
				ranges_.allow.insert(allow);

			for (auto& disallow : type->WeaponRange_DisallowWeapons)
				ranges_.disallow.insert(disallow);
		}

		if (!(type->Crit_Multiplier == 1.0 && type->Crit_ExtraChance == 0.0))
		{
			auto& ranges_ = extraCritData->ranges.emplace_back();
			ranges_.Mult = type->Crit_Multiplier;
			ranges_.extra = type->Crit_ExtraChance;

			for (auto& allow : type->Crit_AllowWarheads)
				ranges_.allow.insert(allow);

			for (auto& disallow : type->Crit_DisallowWarheads)
				ranges_.disallow.insert(disallow);
		}

		if (type->ArmorMultiplier != 1.0)
		{
			auto& mults_ = armormultData->mults.emplace_back();
			mults_.Mult = type->ArmorMultiplier;


			for (auto& allow : type->ArmorMultiplier_AllowWarheads)
				mults_.allow.insert(allow);

			for (auto& disallow : type->ArmorMultiplier_DisallowWarheads)
				mults_.disallow.insert(disallow);
		}

		reflectsDamage |= type->ReflectDamage;
		hasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;

	}

	if (cur_timerAE.has_value() && cur_timerAE > 0.0)
	{
		const int timeleft = pTechno->RearmTimer.GetTimeLeft();

		if (timeleft > 0)
		{
			pTechno->RearmTimer.Start(int(timeleft * cur_timerAE.value()));
		}
		else
		{
			pTechno->RearmTimer.Stop();
		}

		pTechno->ROF = static_cast<int>(pTechno->ROF * cur_timerAE.value());
	}

	pTechno->FirepowerMultiplier = FP_Mult;
	pTechno->ArmorMultiplier = Armor_Mult;
	_AEProp->ROFMultiplier = ROF_Mult;
	_AEProp->ReceiveRelativeDamageMult = ReceiveRelativeDamageMult;
	pTechno->Cloakable = Cloak;
	_AEProp->ForceDecloak = forceDecloak;
	_AEProp->DisableWeapons = disableWeapons;
	_AEProp->DisableSelfHeal = disableSelfHeal;
	_AEProp->Untrackable = untrackable;
	_AEProp->HasTint = hasTint;
	_AEProp->ReflectDamage = reflectsDamage;
	_AEProp->HasOnFireDiscardables = hasOnFireDiscardables;
	_AEProp->Unkillable = unkillable;
	_AEProp->HasExtraWarheads = hasExtraWH;
	_AEProp->HasFeedbackWeapon = hasFeedbackWeapon;

	if ((_AEProp->DisableRadar != disableRadar) || (_AEProp->DisableSpySat != disableSpySat))
		pTechno->Owner->RecheckRadar = true;

	_AEProp->DisableRadar = disableRadar;
	_AEProp->DisableSpySat = disableSpySat;

	if (pTechno->AbstractFlags & AbstractFlags::Foot)
	{
		((FootClass*)pTechno)->SpeedMultiplier = Speed_Mult;
	}
}

void AEProperties::UpdateAEAnimLogic(TechnoClass* pTechno)
{
	for (auto const& attachEffect : TechnoExtContainer::Instance.Find(pTechno)->PhobosAE) {
		attachEffect->UpdateAnimLogic();
	}
}

TechnoExtData::~TechnoExtData()
{
	auto pThis = This();
	FakeHouseClass* pOwner = (FakeHouseClass*)pThis->Owner;
	auto pOwnerExt = pOwner->_GetExtData();

	ScenarioExtData::Instance()->LimboLaunchers.erase(pThis);

	if (this->UndergroundTracked)
		ScenarioExtData::Instance()->UndergroundTracker.erase(pThis);

	if (!Phobos::Otamaa::ExeTerminated)
	{
		if (auto pTemp = std::exchange(this->MyOriginalTemporal, nullptr))
		{
			GameDelete<true, false>(pTemp);
		}
	}

	this->WebbedAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	this->EMPSparkleAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	this->ClearElectricBolts();

	HouseExtData::AutoDeathObjects.erase_all_if([pThis](std::pair<TechnoClass*, KillMethod>& item) {
		return item.first == pThis;
	});

	HouseExtData::LimboTechno.remove(pThis);

	pOwnerExt->OwnedCountedHarvesters.erase(pThis);

	if (this->AbsType != AbstractType::Building) {
		for (auto& tun : pOwnerExt->Tunnels) {
			tun.Vector.remove((FootClass*)pThis);
		}

		if (RulesExtData::Instance()->ExtendedBuildingPlacing && this->AbsType == AbstractType::Unit && ((UnitClass*)pThis)->Type->DeploysInto)
		{
			pOwnerExt->OwnedDeployingUnits.remove((UnitClass*)pThis);
		}
	}
}

// =============================
// container hooks

//ASMJIT_PATCH(0x6F3183, TechnoClass_CTOR, 0x5)
//{
//	GET(TechnoClass*, pItem, ESI);
//	HouseExtData::LimboTechno.push_back_unique(pItem);
//	TechnoExtContainer::Instance.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6F4500, TechnoClass_DTOR, 0x5)
//{
//	GET(TechnoClass*, pItem, ECX);
//
//
//	//TechnoExtContainer::Instance.RemoveExtOf(pItem , pExt);
//	return 0;
//}

//ASMJIT_PATCH(0x7077C0, TechnoClass_Detach, 0x7)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, target, 0x4);
//	GET_STACK(bool, all, 0x8);
//
//
//	TechnoExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
//
//	return 0x0;
//}

ASMJIT_PATCH(0x710415, TechnoClass_AnimPointerExpired_add, 6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(TechnoClass*, pThis, ECX);

	if (auto pExt = TechnoExtContainer::Instance.Find(pThis))
	{
		if (pExt->EMPSparkleAnim.get() == pAnim)
			pExt->EMPSparkleAnim.release();

		if (auto& pShield = pExt->Shield)
			pShield->InvalidateAnimPointer(pAnim);

		if (pExt->WebbedAnim.get() == pAnim)
			pExt->WebbedAnim.release();

		if (pExt->CurrentDelayedFireAnim.get() == pAnim)
			pExt->CurrentDelayedFireAnim.release();

		pExt->AeData.InvalidateAnimPointer(pAnim);

		for (auto& _phobos_AE : pExt->PhobosAE) {

			if(!_phobos_AE)
				continue;

			_phobos_AE->InvalidateAnimPointer(pAnim);
		}
	}

	return 0x0;
}