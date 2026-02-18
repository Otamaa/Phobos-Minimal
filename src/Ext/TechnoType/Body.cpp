#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>

#include <New/Type/TheaterTypeClass.h>
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/EnumFunctions.h>

bool TechnoTypeExtData::SelectWeaponMutex = false;

void TechnoTypeExtData::UpdateAdditionalAttributes(CCINIClass* const pINI)
{
	int Num = 0;
	int EliteNum = 0;

	this->ThreatTypes = { ThreatType::Normal,ThreatType::Normal };
	this->CombatDamages = { 0,0 };

	const auto pThis = this->This();
	int Count = 2;
	const bool attackFriendlies = pThis->AttackFriendlies;
	this->AttackFriendlies = { attackFriendlies ,attackFriendlies };

	if (this->MultiWeapon
		&& (!pThis->IsGattling && (!pThis->HasMultipleTurrets() || !pThis->Gunner)))
	{
		Count = pThis->WeaponCount;
	}

	auto WeaponCheck = [&](WeaponTypeClass* const pWeapon, const bool isElite) {
		if (!pWeapon)
			return;

		if (isElite) {
			if (pWeapon->Projectile)
				this->ThreatTypes.Y |= pWeapon->AllowedThreats();

			this->CombatDamages.Y += (pWeapon->Damage + pWeapon->AmbientDamage);
			EliteNum++;

			if (!this->AttackFriendlies.Y
				&& WeaponTypeExtContainer::Instance.Find(pWeapon)->AttackFriendlies.Get(false)) {
				this->AttackFriendlies.Y = true;
			}
		} else {
			if (pWeapon->Projectile)
				this->ThreatTypes.X |= pWeapon->AllowedThreats();

			this->CombatDamages.X += (pWeapon->Damage + pWeapon->AmbientDamage);
			Num++;

			if (!this->AttackFriendlies.X
				&& WeaponTypeExtContainer::Instance.Find(pWeapon)->AttackFriendlies.Get(false)) {
				this->AttackFriendlies.X = true;
			}
		}

	};

	for (int index = 0; index < Count; index++)
	{
		const auto pWeapon = pThis->GetWeapon(index)->WeaponType;
		auto pEliteWeapon = pThis->GetEliteWeapon(index)->WeaponType;

		if (!pEliteWeapon)
			pEliteWeapon = pWeapon;

		WeaponCheck(pWeapon, false);
		WeaponCheck(pEliteWeapon, true);
	}

	if (Num > 0)
		this->CombatDamages.X /= Num;

	if (EliteNum > 0)
		this->CombatDamages.Y /= EliteNum;
}

bool TechnoTypeExtData::IsSecondary(int nWeaponIndex)
{
	const auto pThis = This();

	if (pThis->IsGattling)
		return nWeaponIndex != 0 && nWeaponIndex % 2 != 0;

	if (this->MultiWeapon.Get() && nWeaponIndex >= 0
	&& !this->MultiWeapon_IsSecondary.empty())
	{
		return this->MultiWeapon_IsSecondary[nWeaponIndex];
	}


	return nWeaponIndex != 0;
}

#include <Ext/WeaponType/Body.h>

int ApplyForceWeaponInRange(TechnoClass* pThis, AbstractClass* pTarget)
{
	int forceWeaponIndex = -1;
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

	const bool useAASetting = !pTypeExt->ForceAAWeapon_InRange.empty() && pTarget->IsInAir();
	auto const& weaponIndices = useAASetting ? pTypeExt->ForceAAWeapon_InRange : pTypeExt->ForceWeapon_InRange;
	auto const& rangeOverrides = useAASetting ? pTypeExt->ForceAAWeapon_InRange_Overrides : pTypeExt->ForceWeapon_InRange_Overrides;
	const bool applyRangeModifiers = useAASetting ? pTypeExt->ForceAAWeapon_InRange_ApplyRangeModifiers : pTypeExt->ForceWeapon_InRange_ApplyRangeModifiers;

	const int defaultWeaponIndex = pThis->SelectWeapon(pTarget);
	const int currentDistance = pThis->DistanceFrom(pTarget);
	auto const pDefaultWeapon = pThis->GetWeapon(defaultWeaponIndex)->WeaponType;

	for (size_t i = 0; i < weaponIndices.size(); i++)
	{
		int range = 0;

		// Value below 0 means Range won't be overriden
		if (i < rangeOverrides.size() && rangeOverrides[i] > 0)
			range = static_cast<int>(rangeOverrides[i] * Unsorted::LeptonsPerCell);

		if (weaponIndices[i] >= 0)
		{
			if (range > 0 || applyRangeModifiers)
			{
				auto const pWeapon = weaponIndices[i] == defaultWeaponIndex ? pDefaultWeapon : pThis->GetWeapon(weaponIndices[i])->WeaponType;
				range = range > 0 ? range : pWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis, range);
			}

			if (currentDistance <= range)
			{
				forceWeaponIndex = weaponIndices[i];
				break;
			}
		}
		else
		{
			if (range > 0 || applyRangeModifiers)
			{
				range = range > 0 ? range : pDefaultWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExtData::GetRangeWithModifiers(pDefaultWeapon, pThis, range);
			}

			// Don't force weapon if range satisfied
			if (currentDistance <= range)
				break;
		}
	}

	return forceWeaponIndex;
}

int TechnoTypeExtData::SelectPhobosWeapon(TechnoClass* pThis, AbstractClass* pTarget){
	const int forceWeaponIndex = this->SelectForceWeapon(pThis, pTarget);

	if (forceWeaponIndex >= 0) {
		return forceWeaponIndex;
	}

	 const int multiWeaponIndex = this->SelectMultiWeapon(pThis, pTarget);

	if (multiWeaponIndex >= 0)  {
 		return multiWeaponIndex;
	}

	return -1;
}

int TechnoTypeExtData::SelectForceWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	if (TechnoTypeExtData::SelectWeaponMutex || !this->ForceWeapon_Check || !pTarget) // In theory, pTarget must exist
		return -1;

	int forceWeaponIndex = -1;
	const auto pTargetTechno = flag_cast_to<TechnoClass*,false>(pTarget);
	TechnoTypeClass* pTargetType = nullptr;

	if (pTargetTechno)
	{
		pTargetType = GET_TECHNOTYPE(pTargetTechno);

		if (this->ForceWeapon_Naval_Decloaked >= 0
			&& pTargetType->Cloakable
			&& pTargetType->Naval
			&& pTargetTechno->CloakState == CloakState::Uncloaked)
		{
			forceWeaponIndex = this->ForceWeapon_Naval_Decloaked;
		}
		else if (this->ForceWeapon_Cloaked >= 0
			&& pTargetTechno->CloakState == CloakState::Cloaked)
		{
			forceWeaponIndex = this->ForceWeapon_Cloaked;
		}
		else if (this->ForceWeapon_Disguised >= 0
			&& pTargetTechno->IsDisguised())
		{
			forceWeaponIndex = this->ForceWeapon_Disguised;
		}
		else if (this->ForceWeapon_UnderEMP >= 0
			&& pTargetTechno->IsUnderEMP())
		{
			forceWeaponIndex = this->ForceWeapon_UnderEMP;
		}
		else if (this->ForceWeapon_Capture >= 0) {
			if (const auto pBuilding = cast_to<BuildingClass*, false>(pTarget)) {
				if ((pBuilding->Type->Capturable || pBuilding->Type->NeedsEngineer)
					&& !pThis->Owner->IsAlliedWith(pBuilding->Owner))
					forceWeaponIndex = this->ForceWeapon_Capture;
			}
		}
	}

	if (forceWeaponIndex == -1
		&& (pTargetTechno || !this->ForceWeapon_InRange_TechnoOnly)
		&& (!this->ForceWeapon_InRange.empty() || !this->ForceAAWeapon_InRange.empty()))
	{
		TechnoTypeExtData::SelectWeaponMutex = true;
		forceWeaponIndex = ApplyForceWeaponInRange(pThis , pTarget);
		TechnoTypeExtData::SelectWeaponMutex = false;
	}

	if (forceWeaponIndex == -1 && pTargetType)
	{
		switch (pTarget->WhatAmI())
		{
		case AbstractType::Building:
		{
			forceWeaponIndex = this->ForceWeapon_Buildings;

			if (this->ForceWeapon_Defenses >= 0)
			{
				auto const pBuildingType = static_cast<BuildingTypeClass*>(pTargetType);

				if (pBuildingType->BuildCat == BuildCat::Combat)
					forceWeaponIndex = this->ForceWeapon_Defenses;
			}

			break;
		}
		case AbstractType::Infantry:
		{
			forceWeaponIndex = (this->ForceAAWeapon_Infantry >= 0 && pTarget->IsInAir())
				? this->ForceAAWeapon_Infantry : this->ForceWeapon_Infantry;

			break;
		}
		case AbstractType::Unit:
		{
			forceWeaponIndex = (this->ForceAAWeapon_Units >= 0 && pTarget->IsInAir())
				? this->ForceAAWeapon_Units : ((this->ForceWeapon_Naval_Units >= 0 && pTargetType->Naval)
				? this->ForceWeapon_Naval_Units : this->ForceWeapon_Units);

			break;
		}
		case AbstractType::Aircraft:
		{
			forceWeaponIndex = (this->ForceAAWeapon_Aircraft >= 0 && pTarget->IsInAir())
				? this->ForceAAWeapon_Aircraft : this->ForceWeapon_Aircraft;

			break;
		}
		}
	}

	return forceWeaponIndex;
}

int TechnoTypeExtData::SelectMultiWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	if (!pTarget || !this->MultiWeapon.Get())
		return -1;

	const auto pType = This();

	if (pType->IsGattling || (pType->HasMultipleTurrets() && pType->Gunner))
		return -1;

	const int weaponCount =  MinImpl(pType->WeaponCount, this->MultiWeapon_SelectCount.Get());
	const bool noSecondary = this->NoSecondaryWeaponFallback;

	if (weaponCount < 2)
		return 0;
	else if (weaponCount == 2 && !noSecondary)
		return -1;

	std::vector<bool> secondaryCanTargets {};
	secondaryCanTargets.resize(weaponCount, false);

	const bool isElite = pThis->Veterancy.IsElite();

	if (const auto pTargetTechno = flag_cast_to<TechnoClass*, true>(pTarget))
	{
		if (pTargetTechno->Health <= 0 || !pTargetTechno->IsAlive)
			return 0;

		bool getNavalTargeting = false;

		auto checkSecondary = [&](int weaponIndex) -> bool
			{
				const auto pWeapon = TechnoTypeExtData::GetWeaponStruct(pType, weaponIndex, isElite)->WeaponType;

				if (!pWeapon || pWeapon->NeverUse)
				{
					secondaryCanTargets[weaponIndex] = true;
					return false;
				}

				bool secondaryPriority = getNavalTargeting;

				if (!getNavalTargeting)
				{
					const auto pWH = pWeapon->Warhead;
					const bool isAllies = pThis->Owner->IsAlliedWith(pTargetTechno->Owner);
					const bool isInAir = pTargetTechno->IsInAir();

					if (pWH->Airstrike)
					{
						// Can it attack the air force now?
						secondaryPriority = isInAir ? !this->NoSecondaryWeaponFallback_AllowAA.Get() : !noSecondary;
					}
					else if (isInAir)
					{
						secondaryPriority = !this->NoSecondaryWeaponFallback_AllowAA.Get();
					}
					else if (pWeapon->DrainWeapon
						&& GET_TECHNOTYPE(pTargetTechno)->Drainable
						&& !pThis->DrainTarget && !isAllies)
					{
						secondaryPriority = !noSecondary;
					}
					else if (pWH->ElectricAssault && isAllies
						&& pTargetTechno->WhatAmI() == AbstractType::Building
						&& static_cast<BuildingClass*>(pTargetTechno)->Type->Overpowerable)
					{
						secondaryPriority = !noSecondary;
					}
				}

				if (secondaryPriority)
				{
					if (TechnoExtData::MultiWeaponCanFire(pThis, pTargetTechno, pWeapon))
						return true;

					secondaryCanTargets[weaponIndex] = true;
					return false;
				}

				return false;
			};

		const LandType landType = pTargetTechno->GetCell()->LandType;
		const bool targetOnWater = landType == LandType::Water || landType == LandType::Beach;

		if (!pTargetTechno->OnBridge && targetOnWater)
		{
			const auto result = pThis->SelectNavalTargeting(pTargetTechno);

			if (result != NavalTargetingType::NotAvaible)
				getNavalTargeting = (result == NavalTargetingType::Underwater_secondary);
		}

		const bool getSecondaryList = !this->MultiWeapon_IsSecondary.empty();

		for (int index = getSecondaryList ? 0 : 1; index < weaponCount; index++)
		{
			if (getSecondaryList && !this->MultiWeapon_IsSecondary[index])
				continue;

			if (checkSecondary(index))
				return index;
		}
	}
	else if (noSecondary)
	{
		return 0;
	}

	for (int index = 0; index < weaponCount; index++)
	{
		if (secondaryCanTargets[index])
			continue;

		if (TechnoExtData::MultiWeaponCanFire(pThis, pTarget, TechnoTypeExtData::GetWeaponStruct(pType, index, isElite)->WeaponType))
			return index;
	}

	return 0;
}

void TechnoTypeExtData::Initialize()
{
	this->ShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);

	this->SellSound = RulesClass::Instance->SellSound;
	auto Eva_ready = GameStrings::EVA_ConstructionComplete();
	auto Eva_sold = GameStrings::EVA_StructureSold() ;

	if (this->AbsType != BuildingTypeClass::AbsID)
	{
		Eva_ready = GameStrings::EVA_UnitReady();
		Eva_sold = GameStrings::EVA_UnitSold();

		if (this->AbsType == AircraftTypeClass::AbsID)
		{
			this->CustomMissileTrailerAnim = AnimTypeClass::Find(GameStrings::V3TRAIL());
			this->CustomMissileTakeoffAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
		}

		this->EVA_UnitLost = VoxClass::FindIndexById(GameStrings::EVA_UnitLost());
		const auto nPromotedEva = VoxClass::FindIndexById(GameStrings::EVA_UnitPromoted());
		this->Promote_Elite_Eva = nPromotedEva;
		this->Promote_Vet_Eva = nPromotedEva;
	}

	this->Eva_Complete = VoxClass::FindIndexById(Eva_ready);
	this->EVA_Sold = VoxClass::FindIndexById(Eva_sold);
	this->EVA_Combat = VoxClass::FindIndexById("EVA_UnitsInCombat");
}

void TechnoTypeExtData::CalculateSpawnerRange()
{
	auto pTechnoType = This();
	int weaponRangeExtra = this->Spawn_LimitedExtraRange * Unsorted::LeptonsPerCell;

	auto setWeaponRange = [](int& weaponRange, WeaponTypeClass* pWeaponType)
		{
			if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
				weaponRange = pWeaponType->Range;
		};

	const int wpCount = pTechnoType->IsGattling ? pTechnoType->WeaponCount : 2;
	for (int i = 0; i < wpCount ; i++) {
			setWeaponRange(this->SpawnerRange, FakeTechnoTypeClass::__GetWeapon(pTechnoType, discard_t(), i)->WeaponType);
			setWeaponRange(this->EliteSpawnerRange, FakeTechnoTypeClass::__GetEliteWeapon(pTechnoType, discard_t(),i)->WeaponType);
	}

	this->SpawnerRange += weaponRangeExtra;
	this->EliteSpawnerRange += weaponRangeExtra;
}

bool TechnoTypeExtData::CanBeBuiltAt(TechnoTypeClass* pProduct, BuildingTypeClass* pFactoryType)
{
	const auto pProductTypeExt = TechnoTypeExtContainer::Instance.Find(pProduct);
	const auto pBExt = BuildingTypeExtContainer::Instance.Find(pFactoryType);
	return (pProductTypeExt->BuiltAt.empty() && !pBExt->Factory_ExplicitOnly)
		|| pProductTypeExt->BuiltAt.Contains(pFactoryType);
}

void  TechnoTypeExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	mtx->Translate((float)(this->TurretOffset->X * factor), (float)(this->TurretOffset->Y * factor), (float)(this->TurretOffset->Z * factor));
}

AnimTypeClass* TechnoTypeExtData::GetSinkAnim(TechnoClass* pThis)
{
	return GET_TECHNOTYPEEXT(pThis)->SinkAnim.Get(RulesClass::Instance->Wake);
}

double TechnoTypeExtData::GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules)
{
	return GET_TECHNOTYPEEXT(pThis)->Tunnel_Speed.Get(pRules->TunnelSpeed);
}

VoxelStruct* TechnoTypeExtData::GetBarrelsVoxel(TechnoTypeClass* const pThis, int const nIdx)
{
	if (nIdx == -1)/// ??
		return pThis->ChargerBarrels;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerBarrels + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);

	//if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.size()) {
	//	Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%s] Is Bigger than BarrelData ! ", pThis->ID, nAdditional);
	//}

	return TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.data() +
		nAdditional;
}

VoxelStruct* TechnoTypeExtData::GetTurretsVoxel(TechnoTypeClass* const pThis, int const nIdx)
{
	if (nIdx == -1)/// ??
		return pThis->ChargerTurrets;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerTurrets + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);
	//if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.size()) {
	//	Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%d]  Is Bigger than TurretData ! ", pThis->ID, nAdditional);
	//}

	return TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.data() + nAdditional;
}

VoxelStruct* TechnoTypeExtData::GetBarrelsVoxelFixedUp(TechnoTypeClass* const pThis, int const nIdx)
{
	if(!pThis->HasMultipleTurrets() || pThis->IsGattling || nIdx == -1)
		return &pThis->BarrelVoxel;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerBarrels + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);

	//if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.size()) {
	//	Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%s] Is Bigger than BarrelData ! ", pThis->ID, nAdditional);
	//}

	return TechnoTypeExtContainer::Instance.Find(pThis)->BarrelImageData.data() +
		nAdditional;
}

VoxelStruct* TechnoTypeExtData::GetTurretsVoxelFixedUp(TechnoTypeClass* const pThis, int const nIdx)
{
	if(!pThis->HasMultipleTurrets() || pThis->IsGattling || nIdx == -1)
		return &pThis->TurretVoxel;

	if (nIdx < TechnoTypeClass::MaxWeapons)
		return pThis->ChargerTurrets + nIdx;

	const auto nAdditional = (nIdx - TechnoTypeClass::MaxWeapons);
	//if ((size_t)nAdditional >= TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.size()) {
	//	Debug::FatalErrorAndExit(__FUNCTION__" [%s] Size[%d]  Is Bigger than TurretData ! ", pThis->ID, nAdditional);
	//}

	return TechnoTypeExtContainer::Instance.Find(pThis)->TurretImageData.data() + nAdditional;
}

// Ares 0.A source
const char* TechnoTypeExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : This()->ID;
}

bool TechnoTypeExtData::IsGenericPrerequisite() const
{
	if(this->GenericPrerequisite.empty()) {
		auto pThis = This();

		for(const auto& prerequisite : GenericPrerequisite::Array){
			const auto& alternates = prerequisite->Alternates;

			if (alternates.empty()) {
				continue;
			}

			if (std::ranges::find(alternates, pThis) != alternates.end()) {
				this->GenericPrerequisite = true;
				return true;
			}
		}

		this->GenericPrerequisite = false;
		return false;
	}

	return this->GenericPrerequisite;
}

const char* TechnoTypeExtData::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pTType = type_cast<TechnoTypeClass*>(pType))
		return TechnoTypeExtContainer::Instance.Find(pTType)->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExtData::HasSelectionGroupID(ObjectTypeClass* pType, const std::string& pID)
{
	const auto id = TechnoTypeExtData::GetSelectionGroupID(pType);

	return (IS_SAME_STR_(id, pID.c_str()));
}

//DO NOT USE !
void TechnoTypeExtData::GetBurstFLHs(TechnoTypeClass* pThis,
	INI_EX& exArtINI,
	const char* pArtSection,
	ColletiveCoordStructVectorData& nFLH,
	ColletiveCoordStructVectorData& nEFlh,
	const char** pPrefixTag)
{
	/*char tempBuffer[0x40];
	char tempBufferFLH[0x40];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;

	for (size_t g = 0; g < nFLH.size(); ++g) {
		nFLH[g]->resize(weaponCount);
		nEFlh[g]->resize(weaponCount);
	}

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			for(size_t k = 0; k < nFLH.size(); ++k){

				const auto pPrefixTagRes = *(pPrefixTag + k);

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTagRes, i + 1);
				auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTagRes);

				IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);

				CoordStruct flhFirst {};
				if(!detail::read(flhFirst, exArtINI , pArtSection, tempBufferFLH)){
					break;
				}

				(*nFLH[k])[i].emplace_back(flhFirst);
				IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
				if(!detail::read((*nEFlh[k])[i].emplace_back(), exArtINI , pArtSection, tempBufferFLH)){
					(*nEFlh[k])[i].back() = (*nFLH[k])[i].back();
				}
			}
		}
	}*/
}

void TechnoTypeExtData::GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection,
	std::vector<BurstFLHBundle>& nFLH, const char* pPrefixTag)
{
	char tempBuffer[0x40];
	char tempBufferFLH[0x40];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	nFLH.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);

			CoordStruct coord {};
			if (!detail::read(coord, exArtINI, pArtSection, tempBufferFLH))
				break;

			nFLH[i].Flh.emplace_back(coord);

			IMPL_SNPRNINTF(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			if (!detail::read(nFLH[i].EFlh.emplace_back(), exArtINI, pArtSection, tempBufferFLH))
				nFLH[i].EFlh.back() = coord;
		}
	}
};

void TechnoTypeExtData::GetFLH(INI_EX& exArtINI, const char* pArtSection, Nullable<CoordStruct>& nFlh, Nullable<CoordStruct>& nEFlh, const char* pFlag)
{
	std::string _key = "Elite";
	_key += pFlag;
	_key += "FLH";
	nFlh.Read(exArtINI, pArtSection, _key.data() + 5);
	nEFlh.Read(exArtINI, pArtSection, _key.c_str());

	if (!nEFlh.isset() && nFlh.isset())
		nEFlh = nFlh;
}

void TechnoTypeExtData::ParseVoiceWeaponAttacks(INI_EX& exINI, const char* pSection, ValueableVector<int>& n, ValueableVector<int>& nE)
{
	if (!this->ReadMultiWeapon)
	{
		n.clear();
		nE.clear();
		return;
	}

	const auto pThis = This();
	const auto WeaponCount = MaxImpl(pThis->WeaponCount, 0);

	while (int(n.size()) > WeaponCount)
		n.erase(n.begin() + int(n.size()) - 1);

	while (int(nE.size()) > WeaponCount)
		nE.erase(nE.begin() + int(nE.size()) - 1);

	char tempBuff[64];
	for (int index = 0; index < WeaponCount; index++)
	{
		NullableIdx<VocClass> VoiceAttack;
		IMPL_SNPRNINTF(tempBuff, sizeof(tempBuff), "VoiceWeapon%dAttack", index + 1);
		VoiceAttack.Read(exINI, pSection, tempBuff);

		NullableIdx<VocClass> VoiceEliteAttack;
		IMPL_SNPRNINTF(tempBuff, sizeof(tempBuff), "VoiceEliteWeapon%dAttack", index + 1);
		VoiceEliteAttack.Read(exINI, pSection, tempBuff);

		if (int(n.size()) > index) {
			n[index] = VoiceAttack.Get(n[index]);
			nE[index] = VoiceEliteAttack.Get(nE[index]);
		} else {
			int voiceattack = VoiceAttack.Get(-1);

			n.push_back(voiceattack);
			nE.push_back(VoiceEliteAttack.Get(voiceattack));
		}
	}
}

bool TechnoTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	auto pThis = This();
	const auto pArtIni = &CCINIClass::INI_Art();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (!parseFailAddr)
	{
		INI_EX exINI(pINI);
		// survivors
		this->Survivors_Pilots.resize(SideClass::Array->Count , nullptr);
		this->Survivors_PassengerChance.Read(exINI, pSection, "Survivor.%sPassengerChance");
		this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
		this->HealthBar_HidePips.Read(exINI, pSection, "HealthBar.HidePips");
		this->HealthBar_Permanent.Read(exINI, pSection, "HealthBar.Permanent");
		this->HealthBar_Permanent_PipScale.Read(exINI, pSection, "HealthBar.Permanent.PipScale");
		this->UIDescription.Read(exINI, pSection, "UIDescription");
		this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
		this->LowDeployPriority.Read(exINI, pSection, "LowDeployPriority");
		this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
		this->MindControl_IgnoreSize.Read(exINI, pSection, "MindControl.IgnoreSize");
		this->MindControlSize.Read(exINI, pSection, "MindControlSize");

		this->Phobos_EliteAbilities.Read(exINI, pSection, GameStrings::EliteAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);
		this->Phobos_VeteranAbilities.Read(exINI, pSection, GameStrings::VeteranAbilities(), EnumFunctions::PhobosAbilityType_ToStrings);

		this->E_ImmuneToType.Read(exINI, pSection, "EliteImmuneTo");
		this->V_ImmuneToType.Read(exINI, pSection, "VeteranImmuneTo");
		this->R_ImmuneToType.Read(exINI, pSection, "RookieImmuneTo");

		this->ImmuneToEMP.Read(exINI, pSection, "ImmuneToEMP");

		detail::read<bool>(pThis->Unbuildable, exINI, pSection, "Unbuildable", false);
		this->HumanUnbuildable.Read(exINI, pSection, "HumanUnbuildable");
		this->NoIdleSound.Read(exINI, pSection, "NoIdleSound");
		this->Soylent_Zero.Read(exINI, pSection, "Soylent.Zero");

		this->Interceptor.Read(exINI, pSection, "Interceptor");
		this->Interceptor_CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
		this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
		this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
		this->Interceptor_TargetingDelay.Read(exINI, pSection, "Interceptor.%sTargetingDelay");
		this->Interceptor_Weapon.Read(exINI, pSection, "Interceptor.Weapon");
		this->Interceptor_DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
		this->Interceptor_WeaponOverride.Read(exINI, pSection, "Interceptor.WeaponOverride");
		this->Interceptor_WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
		this->Interceptor_WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
		this->Interceptor_KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
		this->Interceptor_ConsiderWeaponRange.Read(exINI, pSection, "Interceptor.ConsiderWaponRange");
		this->Interceptor_OnlyTargetBullet.Read(exINI, pSection, "Interceptor.OnlyTargetBullet");
		this->Interceptor_ApplyFirepowerMult.Read(exINI, pSection, "Interceptor.ApplyFirepowerMult");

		this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
		this->Spawn_LimitedRange.Read(exINI, pSection, "Spawner.LimitRange");
		this->Spawn_LimitedExtraRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
		this->Spawner_DelayFrames.Read(exINI, pSection, "Spawner.DelayFrames");
		this->Spawner_AttackImmediately.Read(exINI, pSection, "Spawner.AttackImmediately");
		this->Spawner_UseTurretFacing.Read(exINI, pSection, "Spawner.UseTurretFacing");
		this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");

		this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
		this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
		this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
		this->NoManualMove.Read(exINI, pSection, "NoManualMove");
		this->InitialStrength.Read(exINI, pSection, "InitialStrength");

		//TODO : Tag name Change
		this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
		this->Death_NoAmmo.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
		this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");
		this->Death_Countdown.Read(exINI, pSection, "AutoDeath.AfterDelay");

		this->Death_Method.Read(exINI, pSection, "Death.Method");
		this->Death_Method.Read(exINI, pSection, "AutoDeath.Behavior");

		bool Death_Peaceful;
		if(detail::read(Death_Peaceful , exINI, pSection, "Death.Peaceful"))
			this->Death_Method = Death_Peaceful ? KillMethod::Vanish : this->Death_Method;

		this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.Nonexist");
		this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
		this->AutoDeath_Nonexist_Any.Read(exINI, pSection, "AutoDeath.Nonexist.Any");
		this->AutoDeath_Nonexist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
		this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.Nonexist.House");
		this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.TechnosDontExist.House");
		this->AutoDeath_Nonexist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Nonexist.AllowLimboed");
		this->AutoDeath_Nonexist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosDontExist.AllowLimboed");

		this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.Exist");
		this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.TechnosExist");
		this->AutoDeath_Exist_Any.Read(exINI, pSection, "AutoDeath.Exist.Any");
		this->AutoDeath_Exist_Any.Read(exINI, pSection, "AutoDeath.TechnosExist.Any");
		this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.Exist.House");
		this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.TechnosExist.House");
		this->AutoDeath_Exist_AllowLimboed.Read(exINI, pSection, "AutoDeath.Exist.AllowLimboed");
		this->AutoDeath_Exist_AllowLimboed.Read(exINI, pSection, "AutoDeath.TechnosExist.AllowLimboed");
		this->AutoDeath_VanishAnimation.Read(exINI, pSection, "AutoDeath.VanishAnimation");

		this->Convert_AutoDeath.Read(exINI, pSection, "Convert.AutoDeath");

		this->Death_WithMaster.Read(exINI, pSection, "Death.WithSlaveOwner");
		this->Death_WithMaster.Read(exINI, pSection, "AutoDeath.WithSlaveOwner");

		this->AutoDeath_MoneyExceed.Read(exINI, pSection, "AutoDeath.MoneyExceed");
		this->AutoDeath_MoneyBelow.Read(exINI, pSection, "AutoDeath.MoneyBelow");
		this->AutoDeath_LowPower.Read(exINI, pSection, "AutoDeath.LowPower");
		this->AutoDeath_FullPower.Read(exINI, pSection, "AutoDeath.FullPower");
		this->AutoDeath_PassengerExceed.Read(exINI, pSection, "AutoDeath.PassengersExceed");
		this->AutoDeath_PassengerBelow.Read(exINI, pSection, "AutoDeath.PassengersBelow");
		this->AutoDeath_ContentIfAnyMatch.Read(exINI, pSection, "AutoDeath.OnAnyCondition");

		this->AutoDeath_OwnedByPlayer.Read(exINI, pSection, "AutoDeath.OwnedByPlayer");
		this->AutoDeath_OwnedByAI.Read(exINI, pSection, "AutoDeath.OwnedByAI");

		this->Slaved_ReturnTo.Read(exINI, pSection, "Slaved.ReturnTo");
		this->Death_IfChangeOwnership.Read(exINI, pSection, "Death.IfChangeOwnership");

		this->ShieldType.Read(exINI, pSection, "ShieldType");
		this->CameoPriority.Read(exINI, pSection, "CameoPriority");

		this->WarpOut.Read(exINI, pSection, "%s.WarpOut");
		this->WarpIn.Read(exINI, pSection, "%s.WarpIn");
		this->WarpAway.Read(exINI, pSection, "%s.WarpAway");
		this->ChronoTrigger.Read(exINI, pSection, "%s.ChronoTrigger");
		this->ChronoDistanceFactor.Read(exINI, pSection, "%s.ChronoDistanceFactor");
		this->ChronoMinimumDelay.Read(exINI, pSection, "%s.ChronoMinimumDelay");
		this->ChronoRangeMinimum.Read(exINI, pSection, "%s.ChronoRangeMinimum");
		this->ChronoDelay.Read(exINI, pSection, "%s.ChronoDelay");

		this->WarpInWeapon.Read(exINI, pSection, "%s.WarpInWeapon");
		this->WarpInMinRangeWeapon.Read(exINI, pSection, "%s.WarpInMinRangeWeapon");
		this->WarpOutWeapon.Read(exINI, pSection, "%s.WarpOutWeapon");
		this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "%s.WarpInWeapon.UseDistanceAsDamage");

		this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
		this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
		this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

		this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
		ValueableVector<WarheadTypeClass*> DestroyAnimSpecificList {};
		DestroyAnimSpecificList.Read(exINI, pSection, "DestroyAnims.LinkedWarhead");

		if(!DestroyAnimSpecificList.empty()) {
			this->DestroyAnimSpecific.reserve(DestroyAnimSpecificList.size());
			for (size_t i = 0; i < DestroyAnimSpecificList.size(); ++i) {
				std::string _key = "DestroyAnims";
				_key += std::to_string(i);
				_key += ".Types";

				auto& it = this->DestroyAnimSpecific[DestroyAnimSpecificList[i]];
				detail::ReadVectors(
					it,
					exINI,
					pSection,
					_key.c_str()
				);
			}
		}

		this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

		this->PassengerDeletionType.LoadFromINI(pINI, pSection);

		this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

		this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
		this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
		this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
		this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");
		this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, pSection, "OpenTopped.AllowFiringIfDeactivated");
		this->OpenTopped_ShareTransportTarget.Read(exINI, pSection, "OpenTopped.ShareTransportTarget");
		this->OpenTopped_UseTransportRangeModifiers.Read(exINI, pSection, "OpenTopped.UseTransportRangeModifiers");
		this->OpenTopped_CheckTransportDisableWeapons.Read(exINI, pSection, "OpenTopped.CheckTransportDisableWeapons");

		this->AutoFire.Read(exINI, pSection, "AutoFire");
		this->AutoFire.Read(exINI, pSection, "AutoTargetOwnPosition");
		this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");
		this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoTargetOwnPosition.Self");

		this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");
		this->NoSecondaryWeaponFallback_AllowAA.Read(exINI, pSection, "NoSecondaryWeaponFallback.AllowAA");

		this->JumpjetAllowLayerDeviation.Read(exINI, pSection, "JumpjetAllowLayerDeviation");
		this->JumpjetTurnToTarget.Read(exINI, pSection, "JumpjetTurnToTarget");
		this->JumpjetCrash_Rotate.Read(exINI, pSection, "JumpjetCrashRotate");

		this->DeployingAnims.Read(exINI, pSection, "DeployingAnims");
		this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
		this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
		this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

		this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");

		// Ares 0.2
		this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");
		this->RadarJamHouses.Read(exINI, pSection, "RadarJamHouses");
		this->RadarJamDelay.Read(exINI, pSection, "RadarJamDelay");
		this->RadarJamAffect.Read(exINI, pSection, "RadarJamAffect");
		this->RadarJamIgnore.Read(exINI, pSection, "RadarJamIgnore");

		// Ares 0.9
		this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");
		this->DesignatorRange.Read(exINI, pSection, "DesignatorRange");

		// Ares 0.A
		this->GroupAs.Read(pINI, pSection, "GroupAs");

		// Ares 0.C
		this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
		this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

		this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");

		this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");
		this->ForceWeapon_UnderEMP.Read(exINI, pSection, "ForceWeapon.UnderEMP");
		this->ForceWeapon_Cloaked.Read(exINI, pSection, "ForceWeapon.Cloaked");
		this->ForceWeapon_Disguised.Read(exINI, pSection, "ForceWeapon.Disguised");
		this->ForceWeapon_Buildings.Read(exINI, pSection, "ForceWeapon.Buildings");
		this->ForceWeapon_Defenses.Read(exINI, pSection, "ForceWeapon.Defenses");
		this->ForceWeapon_Infantry.Read(exINI, pSection, "ForceWeapon.Infantry");
		this->ForceWeapon_Naval_Units.Read(exINI, pSection, "ForceWeapon.Naval.Units");
		this->ForceWeapon_Units.Read(exINI, pSection, "ForceWeapon.Units");
		this->ForceWeapon_Aircraft.Read(exINI, pSection, "ForceWeapon.Aircraft");
		this->ForceAAWeapon_Infantry.Read(exINI, pSection, "ForceAAWeapon.Infantry");
		this->ForceAAWeapon_Units.Read(exINI, pSection, "ForceAAWeapon.Units");
		this->ForceAAWeapon_Aircraft.Read(exINI, pSection, "ForceAAWeapon.Aircraft");
		this->ForceWeapon_Capture.Read(exINI, pSection, "ForceWeapon.Capture");

		this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
		this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
		this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
		this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

		this->UseDisguiseMovementSpeed.Read(exINI, pSection, "UseDisguiseMovementSpeed");

		// insignia type
		Nullable<InsigniaTypeClass*> InsigniaType;
		InsigniaType.Read(exINI, pSection, "InsigniaType");

		if (InsigniaType.isset() && InsigniaType)
		{
			this->Insignia = InsigniaType.Get()->Insignia;
			this->InsigniaFrame = InsigniaType.Get()->InsigniaFrame;
			this->InsigniaFrames = Vector3D<int>(-1, -1, -1); // override it so only InsigniaFrame will be used
		}
		else
		{
			this->Insignia.Read(exINI, pSection, "Insignia.%s");
			this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
			this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
		}

		this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");

		this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

		this->SelectBox.Read(exINI, pSection, "SelectBox");
		this->HideSelectBox.Read(exINI, pSection, "HideSelectBox");

		this->Explodes_KillPassengers.Read(exINI, pSection, "Explodes.KillPassengers");

		this->DeployFireWeapon.Read(exINI, pSection, "DeployFireWeapon");
		this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon", true);
		this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
		this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouse");
		this->TargetZoneScanType.Read(exINI, pSection, "TargetZoneScanType");

		this->GrapplingAttack.Read(exINI, pSection, "Parasite.GrapplingAttack");

		this->Cameo_AlwaysExist.Read(exINI, pSection, "Cameo.AlwaysExist");
		this->Cameo_AuxTechnos.Read(exINI, pSection, "Cameo.AuxTechnos");
		this->Cameo_NegTechnos.Read(exINI, pSection, "Cameo.NegTechnos");
		this->UIDescription_Unbuildable.Read(exINI, pSection, "UIDescription.Unbuildable");

#pragma region Otamaa
		this->DontShake.Read(exINI, pSection, "DontShakeScreen");
		this->DiskLaserChargeUp.Read(exINI, pSection, GameStrings::DiskLaserChargeUp());
		this->DiskLaserDetonate.Read(exINI, pSection, "DiskLaser.Detonate");

		this->DrainMoneyFrameDelay.Read(exINI, pSection, GameStrings::DrainMoneyFrameDelay());
		this->DrainMoneyAmount.Read(exINI, pSection, GameStrings::DrainMoneyAmount());
		this->DrainMoney_Display.Read(exINI, pSection, "DrainMoney.Display");
		this->DrainMoney_Display.Read(exINI, pSection, "DrainMoneyDisplay");
		this->DrainMoney_Display_Houses.Read(exINI, pSection, "DrainMoney.Display.Houses");
		this->DrainMoney_Display_OnTarget.Read(exINI, pSection, "DrainMoney.Display.OnTarget");
		this->DrainMoney_Display_OnTarget_UseDisplayIncome.Read(exINI, pSection, "DrainMoney.Display.OnTarget.UseDisplayIncome");
		this->DrainMoney_Display_Houses.Read(exINI, pSection, "DrainMoneyDisplay.Houses");
		this->DrainMoney_Display_Offset.Read(exINI, pSection, "DrainMoneyDisplay.Offset");
		this->DrainMoney_Display_Offset.Read(exINI, pSection, "DrainMoney.Display.Offset");
		this->DrainAnimationType.Read(exINI, pSection, GameStrings::DrainAnimationType());
		this->TalkBubbleTime.Read(exINI, pSection, GameStrings::TalkBubbleTime());

		//pipshape
		this->HealthBarSHP.Read(exINI, pSection, "HealthBarSHP");

		//pipbar
		this->HealthBarSHP_Selected.Read(exINI, pSection, "HealthBarSHP.Selected");
		this->HealthBarSHPBracketOffset.Read(exINI, pSection, "HealthBarSHP.BracketOffset");
		this->HealthBarSHP_HealthFrame.Read(exINI, pSection, "HealthBarSHP.HealthFrame");
		this->HealthBarSHP_Palette.Read(exINI, pSection, "HealthBarSHP.Palette");
		this->HealthBarSHP_PointOffset.Read(exINI, pSection, "HealthBarSHP.Point2DOffset");
		this->HealthbarRemap.Read(exINI, pSection, "HealthBarSHP.Remap");

		this->PipShapes02.Read(exINI, pSection, "PipShapes.Foot");
		this->PipGarrison.Read(exINI, pSection, "PipShapes.Garrison");
		this->PipGarrison_FrameIndex.Read(exINI, pSection, "PipShapes.GarrisonFrameIndex");
		this->PipGarrison_Palette.Read(exINI, pSection, "PipShapes.GarrisonPalette");
		this->PipShapes01.Read(exINI, pSection, "PipShapes.Building");

		this->HealthNumber_SHP.Read(exINI, pSection, "HealthNumber.Shape");
		this->HealthNumber_Show.Read(exINI, pSection, "HealthNumber.Show");
		this->HealthNumber_Percent.Read(exINI, pSection, "HealthNumber.Percent");
		this->Healnumber_Offset.Read(exINI, pSection, "HealthNumber.Offset");
		this->Healnumber_Decrement.Read(exINI, pSection, "HealthNumber.Decrement");

		this->ParasiteExit_Sound.Read(exINI, pSection, "Parasite.ExitSound");

		this->Overload_Count.Read(exINI, pSection, "Overload.Count");
		this->Overload_Damage.Read(exINI, pSection, "Overload.Damage");
		this->Overload_Frames.Read(exINI, pSection, "Overload.Frames");
		this->Overload_DeathSound.Read(exINI, pSection, "Overload.DeathSound");
		this->Overload_ParticleSys.Read(exINI, pSection, "Overload.ParticleSys");
		this->Overload_ParticleSysCount.Read(exINI, pSection, "Overload.ParticleSysCount");
		this->Overload_Warhead.Read(exINI, pSection, "Overload.Warhead", true);

		this->Landing_Anim.Read(exINI, pSection, "Landing.Anim");
		this->Landing_AnimOnWater.Read(exINI, pSection, "Landing.AnimOnWater");

		this->FacingRotation_Disable.Read(exINI, pSection, "FacingRotation.Disabled");
		this->FacingRotation_DisalbeOnEMP.Read(exINI, pSection, "FacingRotation.DisabledOnEMP");
		this->FacingRotation_DisalbeOnDeactivated.Read(exINI, pSection, "FacingRotation.DisabledOnDeactivated");
		this->FacingRotation_DisableOnDriverKilled.Read(exINI, pSection, "FacingRotation.DisabledOnDriverKilled"); // condition disabled , require Ares 3.0 ++

		this->Draw_MindControlLink.Read(exINI, pSection, "MindControlLink.VisibleToHouse");

		this->DeathWeapon.Read(exINI, pSection, "%s.DeathWeapon");
		this->Disable_C4WarheadExp.Read(exINI, pSection, "Crash.DisableC4WarheadExplosion");
		this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
		this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
		this->GClock_Palette.Read(exINI, pSection, "GClock.Palette");

		this->ROF_Random.Read(exINI, pSection, "ROF.AddRandom");
		this->Rof_RandomMinMax.Read(exINI, pSection, "ROF.RandomMinMax");

		this->CreateSound_Enable.Read(exINI, pSection, "CreateSound.Enable");

		this->Eva_Complete.Read(exINI, pSection, "EVA.Complete");

		if (exINI.ReadString(pSection, "VoiceCreated") > 0) {
			this->VoiceCreate = VocClass::FindIndexById(exINI.c_str());
		} else {
			this->VoiceCreate.Read(exINI, pSection, "VoiceCreate");
		}

		this->VoiceCreate_Instant.Read(exINI, pSection, "VoiceCreate.Instant");

		this->SlaveFreeSound_Enable.Read(exINI, pSection, "SlaveFreeSound.Enable");
		this->SlaveFreeSound.Read(exINI, pSection, "SlaveFreeSound");

		if(Phobos::Otamaa::CompatibilityMode)
			this->SinkAnim.Read(exINI, pSection, "Wake.Sink");

		this->SinkAnim.Read(exINI, pSection, "Sink.Anim");
		this->Tunnel_Speed.Read(exINI, pSection, "TunnelSpeed");
		this->HoverType.Read(exINI, pSection, "HoverType");

		this->Gattling_Overload.Read(exINI, pSection, "Gattling.Overload");
		this->Gattling_Overload_Damage.Read(exINI, pSection, "Gattling.Overload.Damage");
		this->Gattling_Overload_Frames.Read(exINI, pSection, "Gattling.Overload.Frames");
		this->Gattling_Overload_DeathSound.Read(exINI, pSection, "Gattling.Overload.DeathSound");
		this->Gattling_Overload_ParticleSys.Read(exINI, pSection, "Gattling.Overload.ParticleSys");
		this->Gattling_Overload_ParticleSysCount.Read(exINI, pSection, "Gattling.Overload.ParticleSysCount");
		this->Gattling_Overload_Warhead.Read(exINI, pSection, "Gattling.Overload.Warhead", true);

		this->IsHero.Read(exINI, pSection, "Hero"); //TODO : Move to InfType Ext
		this->IsDummy.Read(exINI, pSection, "Dummy");

		{
			// UpdateCode Disabled
			// TODO : what will happen if the vectors for different state have different item count ?
			// that will trigger crash because of out of bound idx
			// so disable these untill i can figure out better codes

			this->FireSelf_Weapon.Read(exINI, pSection, "FireSelf.Weapon");
			this->FireSelf_ROF.Read(exINI, pSection, "FireSelf.ROF");
			this->FireSelf_Weapon_GreenHeath.Read(exINI, pSection, "FireSelf.Weapon.GreenHealth");
			this->FireSelf_ROF_GreenHeath.Read(exINI, pSection, "FireSelf.ROF.GreenHealth");
			this->FireSelf_Weapon_YellowHeath.Read(exINI, pSection, "FireSelf.Weapon.YellowHealth");
			this->FireSelf_ROF_YellowHeath.Read(exINI, pSection, "FireSelf.ROF.YellowHealth");
			this->FireSelf_Weapon_RedHeath.Read(exINI, pSection, "FireSelf.Weapon.RedHealth");
			this->FireSelf_ROF_RedHeath.Read(exINI, pSection, "FireSelf.ROF.RedHealth");

		}

		this->AllowFire_IroncurtainedTarget.Read(exINI, pSection, "Firing.AllowICedTargetForAI");

		this->VirtualUnit.Read(exINI, pSection, "VirtualUnit");
		this->MyExtraFireData.ReadRules(exINI, pSection);
		this->MyGiftBoxData.Read(exINI, pSection);
		//this->MyJJData.Read(exINI, pSection);
		this->MyPassangersData.Read(exINI, pSection);
		this->MySpawnSupportDatas.Read(exINI, pSection);
		this->DamageSelfData.Read(exINI, pSection);

		this->IronCurtain_KeptOnDeploy.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
		this->ForceShield_KeptOnDeploy.Read(exINI, pSection, "ForceShield.KeptOnDeploy");
		this->IronCurtain_Effect.Read(exINI, pSection, "IronCurtain.Flag");
		this->IronCurtain_KillWarhead.Read(exINI, pSection, "IronCurtain.KillWarhead", true);
		this->ForceShield_Effect.Read(exINI, pSection, "ForceShield.Effect");
		this->ForceShield_KillWarhead.Read(exINI, pSection, "ForceShield.KillWarhead", true);

		this->SellSound.Read(exINI, pSection, "SellSound");
		this->EVA_Sold.Read(exINI, pSection, "EVA.Sold");
		this->EngineerCaptureDelay.Read(exINI, pSection, "Engineer.CaptureDelay"); // code unfinished

		this->CommandLine_Move_Color.Read(exINI, pSection, "ActionLine.Move.Color");
		this->CommandLine_Attack_Color.Read(exINI, pSection, "ActionLine.Attack.Color");
		this->PassiveAcquire_AI.Read(exINI, pSection, "CanPassiveAquire.AI");
		this->CanPassiveAquire_Naval.Read(exINI, pSection, "CanPassiveAquire.Naval");

		this->TankDisguiseAsTank.Read(exINI, pSection, "Disguise.AsTank"); // code disabled , crash
		this->DisguiseDisAllowed.Read(exINI, pSection, "Disguise.Allowed");  // code disabled , crash
		this->ChronoDelay_Immune.Read(exINI, pSection, "ChronoDelay.Immune");
		this->CrushLevel.Read(exINI, pSection, "%sCrushLevel");
		this->CrushableLevel.Read(exINI, pSection, "%sCrushableLevel");
		this->DeployCrushableLevel.Read(exINI, pSection, "%sDeployCrushableLevel");
		this->Experience_KillerMultiple.Read(exINI, pSection, "Experience.KillerMultiple");
		this->Experience_VictimMultiple.Read(exINI, pSection, "Experience.VictimMultiple");
		this->NavalRangeBonus.Read(exINI, pSection, "NavalRangeBonus");
		this->AI_LegalTarget.Read(exINI, pSection, "AI.LegalTarget");
		this->DeployFire_UpdateFacing.Read(exINI, pSection, "DeployFire.CheckFacing");
		this->Fake_Of.Read(exINI, pSection, "FakeOf");
		this->CivilianEnemy.Read(exINI, pSection, "CivilianEnemy");
		this->ImmuneToBerserk.Read(exINI, pSection, "ImmuneToBerserk");
		this->Berzerk_Modifier.Read(exINI, pSection, "Berzerk.Modifier");
		//this->IgnoreToProtect.Read(exINI, pSection, "ToProtect.Ignore");
		this->TargetLaser_Time.Read(exINI, pSection, "TargetLaser.Time");
		this->TargetLaser_WeaponIdx.Read(exINI, pSection, "TargetLaser.WeaponIndexes");
		this->AdjustCrushProperties();

		this->ConsideredNaval.Read(exINI, pSection, "ConsideredNaval");
		this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");

		this->LaserTargetColor.Read(exINI, pSection, "LaserTargetColor");
		this->VoicePickup.Read(exINI, pSection, "VoicePickup");

		this->CrateGoodie_RerollChance.Read(exINI, pSection, "CrateGoodie.RerollChance");
		this->Destroyed_CrateType.Read(exINI, pSection, "CrateGoodie.WhenDestroyed");
		this->Infantry_DimWhenEMPEd.Read(exINI, pSection, "Infantry.DimUnderEMP");
		this->Infantry_DimWhenDisabled.Read(exINI, pSection, "Infantry.DimWhenDisabled");
#pragma region Prereq

	std::string _Prerequisite_key = "Prerequisite";
	std::string _Prerequisite_ReqTheater_key = (_Prerequisite_key + ".RequiredTheaters");

	if(pINI->ReadString(pSection, _Prerequisite_ReqTheater_key.c_str(), "", Phobos::readBuffer) > 0) {
		this->Prerequisite_RequiredTheaters = 0;

		char* context = nullptr;
		for(char *cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			signed int idx = TheaterTypeClass::FindIndexById(cur);
			if(idx != -1) {
				this->Prerequisite_RequiredTheaters |= (1 << idx);
			} else if (!GameStrings::IsNone(cur)) {
				Debug::INIParseFailed(pSection, _Prerequisite_ReqTheater_key.c_str(), cur);
			}
		}
	}

	// subtract the default list, get tag (not less than 0), add one back
	const auto nRead = pINI->ReadInteger(pSection, (_Prerequisite_key + ".Lists").c_str(), static_cast<int>(this->Prerequisites.size()) - 1);
	this->Prerequisites.resize(static_cast<size_t>(MaxImpl(nRead, 0) + 1));
	GenericPrerequisite::Parse(pINI, pSection, _Prerequisite_key.c_str(), this->Prerequisites[0]);

	for (size_t i = 0u; i < this->Prerequisites.size(); ++i) {
		GenericPrerequisite::Parse(pINI,
		pSection,
		(_Prerequisite_key + std::string(".List") + std::to_string(i)).c_str(),
		this->Prerequisites[i]
		);
	}

		// Prerequisite.Negative with Generic Prerequistes support
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + ".Negative").c_str(), this->Prerequisite_Negative);
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + ".Display").c_str(), this->Prerequisite_Display);
		GenericPrerequisite::Parse(pINI, pSection, (_Prerequisite_key + "Override").c_str(), pThis->PrerequisiteOverride);

		//TODO : properly Enable this
		GenericPrerequisite::Parse(pINI, pSection, "BuildLimit.Requres", this->BuildLimit_Requires);

		GenericPrerequisite::Parse(pINI, pSection, (std::string("Convert.Script.") + _Prerequisite_key).c_str(), this->Convert_Scipt_Prereq);

		this->Prerequisite_Power.Read(exINI, pSection, (_Prerequisite_key + ".Power").c_str());
		std::string _Prerequisite_StolenTechs_key = _Prerequisite_key+ ".StolenTechs";

		if (pINI->ReadString(pSection, _Prerequisite_StolenTechs_key.c_str(), Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			this->RequiredStolenTech.reset();

			char* context = nullptr;
			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				signed int idx = std::atoi(cur);
				if (idx > -1 && idx < MaxHouseCount)
				{
					this->RequiredStolenTech.set(idx);
				}
				else if (idx != -1)
				{
					Debug::INIParseFailed(pSection, _Prerequisite_StolenTechs_key.c_str(), cur, "Expected a number between 0 and 31 inclusive");
				}
			}
		}

#pragma endregion Prereq

#ifndef _Handle_Old_
		this->AttachedEffect.Read(exINI);
#else
		int _AE_Dur { 0 };
		this->AttachEffect_AttachTypes.clear();
		if (detail::read(_AE_Dur, exINI, pSection, "AttachEffect.Duration") && _AE_Dur != 0) {
			auto& back = this->AttachEffect_AttachTypes.emplace_back(PhobosAttachEffectTypeClass::FindOrAllocate(pSection));
			back->Duration = _AE_Dur;
			back->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
			back->Animation.Read(exINI, pSection, "AttachEffect.Animation", true);
			if (!back->Animation)
				Debug::LogInfo("Failed to find [{}] AE Anim[{}]", pSection, exINI.c_str());

			back->Animation_ResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");

			bool AE_TemporalHidesAnim {};
			if (detail::read(AE_TemporalHidesAnim, exINI, pSection, "AttachEffect.TemporalHidesAnim") && AE_TemporalHidesAnim)
				back->Animation_TemporalAction = AttachedAnimFlag::Hides;

			back->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");

			bool AE_DiscardOnEntry {};
			if(detail::read(AE_DiscardOnEntry, exINI, pSection, "AttachEffect.DiscardOnEntry") && AE_DiscardOnEntry)
				back->DiscardOn = DiscardCondition::Entry;

			back->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
			back->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
			back->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
			back->ROFMultiplier.Read(exINI, pSection, "AttachEffect.ROFMultiplier");
			back->ReceiveRelativeDamageMult.Read(exINI, pSection, "AttachEffect.ReceiveRelativeDamageMultiplier");
			back->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");
			int AE_Delay {};
			detail::read(AE_Delay , exINI, pSection, "AttachEffect.Delay");
			this->AttachEffect_Delays.emplace_back(AE_Delay);

			int AE_IinitialDelay {};
			detail::read(AE_IinitialDelay, exINI, pSection, "AttachEffect.InitialDelay");
			this->AttachEffect_InitialDelays.emplace_back(AE_IinitialDelay);

			back->PenetratesIronCurtain.Read(exINI, pSection, "AttachEffect.PenetratesIronCurtain");
			back->DisableSelfHeal.Read(exINI, pSection, "AttachEffect.DisableSelfHeal");
			back->DisableWeapons.Read(exINI, pSection, "AttachEffect.DisableWeapons");
			back->Untrackable.Read(exINI, pSection, "AttachEffect.Untrackable");

			back->WeaponRange_Multiplier.Read(exINI, pSection, "AttachEffect.WeaponRange.Multiplier");
			back->WeaponRange_ExtraRange.Read(exINI, pSection, "AttachEffect.WeaponRange.ExtraRange");
			back->WeaponRange_AllowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.AllowWeapons");
			back->WeaponRange_DisallowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.DisallowWeapons");

			back->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "AttachEffect.ROFMultiplier.ApplyOnCurrentTimer");
		}
#endif

		this->NoAmmoEffectAnim.Read(exINI, pSection, "NoAmmoEffectAnim", true);
		this->AttackFriendlies_WeaponIdx.Read(exINI, pSection, "AttackFriendlies.WeaponIdx");
		this->AttackFriendlies_AutoAttack.Read(exINI, pSection, "AttackFriendlies.AutoAttack");

		this->PipScaleIndex.Read(exINI, pSection, "PipScaleIndex");

		this->ShowSpawnsPips.Read(exINI, pSection, "ShowSpawnsPips");
		this->SpawnsPip.Read(exINI, pSection, "SpawnsPipFrame");
		this->EmptySpawnsPip.Read(exINI, pSection, "EmptySpawnsPipFrame");
		this->SpawnsPipSize.Read(exINI, pSection, "SpawnsPipSize");
		this->SpawnsPipOffset.Read(exINI, pSection, "SpawnsPipOffset");
		// #346, #464, #970, #1014
		this->PassengersGainExperience.Read(exINI, pSection, "Experience.PromotePassengers");
		this->ExperienceFromPassengers.Read(exINI, pSection, "Experience.FromPassengers");
		this->PassengerExperienceModifier.Read(exINI, pSection, "Experience.PassengerModifier");
		this->MindControlExperienceSelfModifier.Read(exINI, pSection, "Experience.MindControlSelfModifier");
		this->MindControlExperienceVictimModifier.Read(exINI, pSection, "Experience.MindControlVictimModifier");
		this->SpawnExperienceOwnerModifier.Read(exINI, pSection, "Experience.SpawnOwnerModifier");
		this->SpawnExperienceSpawnModifier.Read(exINI, pSection, "Experience.SpawnModifier");
		this->ExperienceFromAirstrike.Read(exINI, pSection, "Experience.FromAirstrike");
		this->AirstrikeExperienceModifier.Read(exINI, pSection, "Experience.AirstrikeModifier");
		this->Promote_IncludePassengers.Read(exINI, pSection, "Promote.IncludePassengers");
		this->Promote_Elite_Eva.Read(exINI, pSection, "EVA.ElitePromoted");
		this->Promote_Vet_Eva.Read(exINI, pSection, "EVA.VeteranPromoted");


		this->Promote_Elite_Flash.Read(exINI, pSection, "Promote.EliteFlash");
		this->Promote_Vet_Flash.Read(exINI, pSection, "Promote.VeteranFlash");

		this->Promote_Elite_Sound.Read(exINI, pSection, "Promote.EliteSound");
		this->Promote_Vet_Sound.Read(exINI, pSection, "Promote.VeteranSound");

		this->Promote_Vet_Type.Read(exINI, pSection, "Promote.VeteranType");
		this->Promote_Elite_Type.Read(exINI, pSection, "Promote.EliteType");

		// these shit readed twices becesue the phobos develop tag is different
		this->Promote_Vet_Anim.Read(exINI, pSection, "Promote.VeteranAnim");
		this->Promote_Elite_Anim.Read(exINI, pSection, "Promote.EliteAnim");

		this->Promote_Vet_Anim.Read(exINI, pSection, "Promote.VeteranAnimation");
		this->Promote_Elite_Anim.Read(exINI, pSection, "Promote.EliteAnimation");

		this->Promote_Vet_PlaySpotlight.Read(exINI, pSection, "Promote.VeteranPlaySpotLight");
		this->Promote_Elite_PlaySpotlight .Read(exINI, pSection, "Promote.ElitePlaySpotLight");

		this->Promote_Vet_Exp.Read(exINI, pSection, "Promote.VeteranExperience");
		this->Promote_Elite_Exp.Read(exINI, pSection, "Promote.EliteExperience");

		this->DeployDir.Read(exINI, pSection, "DeployDir");

		this->PassengersWhitelist.Read(exINI, pSection, "Passengers.Allowed");
		this->PassengersBlacklist.Read(exINI, pSection, "Passengers.Disallowed");

		this->NoManualUnload.Read(exINI, pSection, "NoManualUnload");
		this->NoSelfGuardArea.Read(exINI, pSection, "NoSelfGuardArea");
		this->NoManualFire.Read(exINI, pSection, "NoManualFire");
		this->NoManualEnter.Read(exINI, pSection, "NoManualEnter");
		this->NoManualEject.Read(exINI, pSection, "NoManualEject");

		//this->Crashable.Read(exINI, pSection, "Crashable");

		this->Passengers_BySize.Read(exINI, pSection, "Passengers.BySize");
		this->Convert_Deploy.Read(exINI, pSection, "Convert.Deploy");
		this->Convert_Deploy_Delay.Read(exINI, pSection, "Convert.DeployDelay");
		this->Convert_Script.Read(exINI, pSection, "Convert.Script");
		this->Convert_Water.Read(exINI, pSection, "Convert.Water");
		this->Convert_Land.Read(exINI, pSection, "Convert.Land");

		this->Harvester_LongScan.Read(exINI, pSection, "Harvester.LongScan");
		this->Harvester_ShortScan.Read(exINI, pSection, "Harvester.ShortScan");
		this->Harvester_ScanCorrection.Read(exINI, pSection, "Harvester.ScanCorrection");

		this->Harvester_TooFarDistance.Read(exINI, pSection, "Harvester.TooFarDistance");
		this->Harvester_KickDelay.Read(exINI, pSection, "Harvester.KickDelay");

		this->TurretRot.Read(exINI, pSection, "TurretROT");

		this->FallRate_Parachute.Read(exINI, pSection, "FallRate.Parachute");
		this->FallRate_NoParachute.Read(exINI, pSection, "FallRate.NoParachute");
		this->FallRate_ParachuteMax.Read(exINI, pSection, "FallRate.ParachuteMax");
		this->FallRate_NoParachuteMax.Read(exINI, pSection, "FallRate.NoParachuteMax");

		this->NoShadowSpawnAlt.Read(exINI, pSection, "NoShadowSpawnAlt");

		this->OmniCrusher_Aggressive.Read(exINI, pSection, "OmniCrusher.Aggressive");
		this->CrusherDecloak.Read(exINI, pSection, "Crusher.Decloak");
		this->Crusher_SupressLostEva.Read(exINI, pSection, "Crusher.SuppressLostEVA");
		this->CrushRange.Read(exINI, pSection, "Crusher.Range.%s");

		this->CrushFireDeathWeapon.Read(exINI, pSection, "CrushFireDeathWeaponChance.%s");
		this->CrushDamage.Read(exINI, pSection, "CrushDamage.%s");
		this->CrushDamageWarhead.Read(exINI, pSection, "CrushDamage.Warhead");
		this->CrushDamagePlayWHAnim.Read(exINI, pSection, "CrushDamage.PlayWarheadAnim");

		this->DigInSound.Read(exINI, pSection, "DigInSound");
		this->DigOutSound.Read(exINI, pSection, "DigOutSound");
		this->DigInAnim.Read(exINI, pSection, "DigIn");
		this->DigOutAnim.Read(exINI, pSection, "DigOut");

		this->EVA_UnitLost.Read(exINI, pSection, "EVA.Lost");

		this->BuildTime_Speed.Read(exINI, pSection, "BuildTime.Speed");
		this->BuildTime_Cost.Read(exINI, pSection, "BuildTime.Cost");
		this->BuildTime_LowPowerPenalty.Read(exINI, pSection, "BuildTime.LowPowerPenalty");
		this->BuildTime_MinLowPower.Read(exINI, pSection, "BuildTime.MinLowPower");
		this->BuildTime_MaxLowPower.Read(exINI, pSection, "BuildTime.MaxLowPower");
		this->BuildTime_MultipleFactory.Read(exINI, pSection, "BuildTime.MultipleFactory");

		this->CloakStages.Read(exINI, pSection, "Cloakable.Stages");
		this->DamageSparks.Read(exINI, pSection, "DamageSparks");

		this->ParticleSystems_DamageSmoke.Read(exINI, pSection, "DamageSmokeParticleSystems");
		this->ParticleSystems_DamageSparks.Read(exINI, pSection, "DamageSparksParticleSystems");
		// issue #896235: cyclic gattling
		this->GattlingCyclic.Read(exINI, pSection, "Gattling.Cycle");
		this->CloakSound.Read(exINI, pSection, "CloakSound");
		this->DecloakSound.Read(exINI, pSection, "DecloakSound");
		this->VoiceRepair.Read(exINI, pSection, "VoiceIFVRepair");
		this->ParseVoiceWeaponAttacks(exINI, pSection, this->VoiceWeaponAttacks, this->VoiceEliteWeaponAttacks);
		this->ReloadAmount.Read(exINI, pSection, "ReloadAmount");
		this->EmptyReloadAmount.Read(exINI, pSection, "EmptyReloadAmount");

		this->TiberiumProof.Read(exINI, pSection, "TiberiumProof");
		this->TiberiumSpill.Read(exINI, pSection, "TiberiumSpill");
		this->TiberiumRemains.Read(exINI, pSection, "TiberiumRemains");
		this->TiberiumTransmogrify.Read(exINI, pSection, "TiberiumTransmogrify");

		// sensors
		this->SensorArray_Warn.Read(exINI, pSection, "SensorArray.Warn");

		this->IronCurtain_Modifier.Read(exINI, pSection, "IronCurtain.Modifier");
		this->ForceShield_Modifier.Read(exINI, pSection, "ForceShield.Modifier");
		this->Survivors_PilotCount.Read(exINI, pSection, "Survivor.Pilots");
		// berserking options
		this->BerserkROFMultiplier.Read(exINI, pSection, "Berserk.ROFMultiplier");

		// refinery and storage
		this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");
		this->VHPscan_Value.Read(exINI, pSection, "VHPScan.Value");

		this->SelfHealing_Rate.Read(exINI, pSection, "SelfHealing.Rate");
		this->SelfHealing_Amount.Read(exINI, pSection, "SelfHealing.%sAmount");
		this->SelfHealing_Max.Read(exINI, pSection, "SelfHealing.%sMax");
		this->SelfHealing_CombatDelay.Read(exINI, pSection, "SelfHealing.%sCombatDelay");

		this->CloakAllowed.Read(exINI, pSection, "Cloakable.Allowed");

		this->InitialPayload_Types.Read(exINI, pSection, "InitialPayload.Types");
		this->InitialPayload_Nums.Read(exINI, pSection, "InitialPayload.Nums");
		this->InitialPayload_Vet.Read(exINI, pSection, "InitialPayload.Ranks");
		this->InitialPayload_AddToTransportTeam.Read(exINI, pSection, "InitialPayload.AddToTransportTeam");

		this->HasSpotlight.Read(exINI, pSection, "HasSpotlight");
		this->Spot_Height.Read(exINI, pSection, "Spotlight.StartHeight");
		this->Spot_Distance.Read(exINI, pSection, "Spotlight.Distance");
		this->Spot_AttachedTo.Read(exINI, pSection, "Spotlight.AttachedTo");
		this->Spot_DisableR.Read(exINI, pSection, "Spotlight.DisableRed");
		this->Spot_DisableG.Read(exINI, pSection, "Spotlight.DisableGreen");
		this->Spot_DisableB.Read(exINI, pSection, "Spotlight.DisableBlue");
		this->Spot_DisableColor.Read(exINI, pSection, "Spotlight.Color");
		this->Spot_Reverse.Read(exINI, pSection, "Spotlight.IsInverted");

		this->Crew_TechnicianChance.Read(exINI, pSection, "Crew.TechnicianChance");
		this->Crew_EngineerChance.Read(exINI, pSection, "Crew.EngineerChance");
		this->Saboteur.Read(exINI, pSection, "Saboteur");

		this->RadialIndicatorRadius.Read(exINI, pSection, "RadialIndicatorRadius");
		this->RadialIndicatorColor.Read(exINI, pSection, "RadialIndicatorColor");

		this->GapRadiusInCells.Read(exINI, pSection, "GapRadiusInCells");
		this->SuperGapRadiusInCells.Read(exINI, pSection, "SuperGapRadiusInCells");

		this->Survivors_PilotChance.Read(exINI, pSection, "Survivor.%sPilotChance");
		this->HijackerOneTime.Read(exINI, pSection, "VehicleThief.OneTime");
		this->HijackerKillPilots.Read(exINI, pSection, "VehicleThief.KillPilots");
		this->HijackerEnterSound.Read(exINI, pSection, "VehicleThief.EnterSound");
		this->HijackerLeaveSound.Read(exINI, pSection, "VehicleThief.LeaveSound");
		this->HijackerBreakMindControl.Read(exINI, pSection, "VehicleThief.BreakMindControl");
		this->HijackerAllowed.Read(exINI, pSection, "VehicleThief.Allowed");
		this->Cursor_Deploy.Read(exINI, pSection, "Cursor.Deploy");
		this->Cursor_NoDeploy.Read(exINI, pSection, "Cursor.NoDeploy");
		this->Cursor_Enter.Read(exINI, pSection, "Cursor.Enter");
		this->Cursor_NoEnter.Read(exINI, pSection, "Cursor.NoEnter");
		this->Cursor_Move.Read(exINI, pSection, "Cursor.Move");
		this->Cursor_NoMove.Read(exINI, pSection, "Cursor.NoMove");

		// #680, 1362
		this->ImmuneToAbduction.Read(exINI, pSection, "ImmuneToAbduction");
		this->UseROFAsBurstDelays.Read(exINI, pSection, "UseROFAsBurstDelays");
		this->Chronoshift_Crushable.Read(exINI, pSection, "Chronoshift.Crushable");

		this->CanBeReversed.Read(exINI, pSection, "CanBeReversed");
		this->ReversedAs.Read(exINI, pSection, "ReversedAs");
		this->AssaulterLevel.Read(exINI, pSection, "Assaulter.Level");

		// smoke when damaged
		this->SmokeAnim.Read(exINI, pSection, "Smoke.Anim");
		this->SmokeChanceRed.Read(exINI, pSection, "Smoke.ChanceRed");
		this->SmokeChanceDead.Read(exINI, pSection, "Smoke.ChanceDead");

		this->CarryallAllowed.Read(exINI, pSection, "Carryall.Allowed");
		this->CarryallSizeLimit.Read(exINI, pSection, "Carryall.SizeLimit");

		this->VoiceAirstrikeAttack.Read(exINI, pSection, "VoiceAirstrikeAttack");
		this->VoiceAirstrikeAbort.Read(exINI, pSection, "VoiceAirstrikeAbort");

		// note the wrong spelling of the tag for consistency
		this->CanPassiveAcquire_Guard.Read(exINI, pSection, "CanPassiveAquire.Guard");
		this->CanPassiveAcquire_Cloak.Read(exINI, pSection, "CanPassiveAquire.Cloak");

		this->CrashSpin.Read(exINI, pSection, "CrashSpin");
		this->AirRate.Read(exINI, pSection, "AirRate");
		this->Unsellable.Read(exINI, pSection, "Unsellable");
		this->CreateSound_afect.Read(exINI, pSection, "CreateSound.AffectOwner");

		this->Chronoshift_Allow.Read(exINI, pSection, "Chronoshift.Allow");
		this->Chronoshift_IsVehicle.Read(exINI, pSection, "Chronoshift.IsVehicle");

		this->SuppressorRange.Read(exINI, pSection, "SuppressorRange");
		this->AttractorRange.Read(exINI, pSection, "AttractorRange");
		this->FactoryPlant_Multiplier.Read(exINI, pSection, "FactoryPlant.Multiplier");
		this->MassSelectable.Read(exINI, pSection, "MassSelectable");

		this->TiltsWhenCrushes_Vehicles.Read(exINI, pSection, "TiltsWhenCrushes.Vehicles");
		this->TiltsWhenCrushes_Overlays.Read(exINI, pSection, "TiltsWhenCrushes.Overlays");
		this->CrushForwardTiltPerFrame.Read(exINI, pSection, "CrushForwardTiltPerFrame");
		this->CrushOverlayExtraForwardTilt.Read(exINI, pSection, "CrushOverlayExtraForwardTilt");
		this->CrushSlowdownMultiplier.Read(exINI, pSection, "CrushSlowdownMultiplier");

		this->AIIonCannonValue.Read(exINI, pSection, "AIIonCannonValue");
		this->ExtraPower_Amount.Read(exINI, pSection, "ExtraPower.Amount");
		this->CanDrive.Read(exINI, pSection, "CanDrive");

		if (exINI.ReadString(pSection, "Operator") > 0)
		{ // try to read the flag
			this->Operators.clear();
			// set whether this type accepts all operators
			this->Operator_Any = (IS_SAME_STR_N(exINI.value(), "_ANY_"));
			if (!this->Operator_Any)
			{ // if not, find the specific operator it allows
				detail::parse_values<TechnoTypeClass*, false>(this->Operators, exINI, pSection, "Operator");
			}
		}

		this->AlwayDrawRadialIndicator.Read(exINI, pSection, "RadialIndicator.AlwaysDraw");
		this->ReloadRate.Read(exINI, pSection, "ReloadRate");
		this->CloakAnim.Read(exINI, pSection, "CloakAnim");
		this->DecloakAnim.Read(exINI, pSection, "DecloakAnim");
		this->Cloak_KickOutParasite.Read(exINI, pSection, "Cloak.KickOutParasite");

		// killer tags
		this->Bounty.Read(exINI, pSection, "Bounty");
		this->Bounty_Display.Read(exINI, pSection, "Bounty.Display");
		this->BountyAllow.Read(exINI, pSection, "Bounty.AffectTypes");
		this->BountyDissallow.Read(exINI, pSection, "Bounty.IgnoreTypes");
		this->BountyBonusmult.Read(exINI, pSection, "Bounty.%sBonusMult");
		this->Bounty_IgnoreEnablers.Read(exINI, pSection, "Bounty.IgnoreEnablers");

		// victim tags
		this->Bounty_Value_Option.Read(exINI, pSection, "Bounty.RewardOption");

		if(	this->Bounty_Value_Option ==  BountyValueOption::ValuePercentOfConst || this->Bounty_Value_Option == BountyValueOption::ValuePercentOfSoylent)
			this->Bounty_Value_PercentOf.Read(exINI, pSection, "Bounty.%sValue");
		else
			this->Bounty_Value.Read(exINI, pSection, "Bounty.%sValue");

		this->Bounty_Value_mult.Read(exINI, pSection, "Bounty.%sValueMult");

		this->Bounty_ReceiveSound.Read(exINI, pSection, "Bounty.ReceiveSound");

		this->DeathWeapon_CheckAmmo.Read(exINI, pSection, "DeathWeapon.CheckAmmo");
		this->Initial_DriverKilled.Read(exINI, pSection, "Initial.DriverKilled");

		this->VoiceCantDeploy.Read(exINI, pSection, "VoiceCantDeploy");
		this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");
		this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");

		//not fully working atm , disabled
		//this->DeployAnims.Read(exINI, pSection, "DeployingAnim");

		this->AmmoPip_Palette.Read(exINI, pSection, "AmmoPipPalette");
		this->AmmoPipOffset.Read(exINI, pSection, "AmmoPipOffset");

		this->AmmoPip.Read(exINI, pSection, "AmmoPipFrame");
		this->EmptyAmmoPip.Read(exINI, pSection, "EmptyAmmoPipFrame");
		this->PipWrapAmmoPip.Read(exINI, pSection, "AmmoPipWrapStartFrame");
		this->AmmoPipSize.Read(exINI, pSection, "AmmoPipSize");
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashDisplay");

		// drain settings
		this->Drain_Local.Read(exINI, pSection, "Drain.Local");
		this->Drain_Amount.Read(exINI, pSection, "Drain.Amount");

		this->FactoryOwners.Read(exINI, pSection, "FactoryOwners");
		this->FactoryOwners_Forbidden.Read(exINI, pSection, "FactoryOwners.Forbidden");
		this->Wake.Read(exINI, pSection, "Wake");

		this->UnitIdleRotateTurret.Read(exINI, pSection, "UnitIdleRotateTurret");
		this->UnitIdlePointToMouse.Read(exINI, pSection, "UnitIdlePointToMouse");

		this->FallingDownDamage.Read(exINI, pSection, "FallingDownDamage");
		this->FallingDownDamage_Water.Read(exINI, pSection, "FallingDownDamage.Water");

		this->DropCrate.Read(exINI, pSection, "DropCrate");

		this->WhenCrushed_Warhead.Read(exINI, pSection, "WhenCrushed.Warhead.%s", nullptr,  true);
		this->WhenCrushed_Weapon.Read(exINI, pSection, "WhenCrushed.Weapon.%s", nullptr, true);
		this->WhenCrushed_Damage.Read(exINI, pSection, "WhenCrushed.Damage.%s");
		this->WhenCrushed_Warhead_Full.Read(exINI, pSection, "WhenCrushed.Warhead.Full");

		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.HaveAllPlans");
		this->FactoryOwners_HaveAllPlans.Read(exINI, pSection, "FactoryOwners.Permanent");
		this->FactoryOwners_HasAllPlans.Read(exINI, pSection, "FactoryOwners.HasAllPlans");

		this->HealthBar_Sections.Read(exINI, pSection, "HealthBar.Sections");
		this->HealthBar_Border.Read(exINI, pSection, "HealthBar.Border");
		this->HealthBar_BorderFrame.Read(exINI, pSection, "HealthBar.BorderFrame");
		this->HealthBar_BorderAdjust.Read(exINI, pSection, "HealthBar.BorderAdjust");

		this->IsBomb.Read(exINI, pSection, "IsBomb");
		this->ParachuteAnim.Read(exINI, pSection, "Parachute.Anim", true);

		this->Cloneable.Read(exINI, pSection, "Cloneable");
		this->ClonedAt.Read(exINI, pSection, "ClonedAt", true);
		this->ClonedAs.Read(exINI, pSection, "ClonedAs", true);
		this->AI_ClonedAs.Read(exINI, pSection, "AI.ClonedAs", true);
		this->BuiltAt.Read(exINI, pSection, "BuiltAt");
		this->EMP_Sparkles.Read(exINI, pSection, "EMP.Sparkles");
		this->EMP_Modifier.Read(exINI, pSection, "EMP.Modifier");

		if (pINI->ReadString(pSection, "EMP.Threshold", Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			if (IS_SAME_STR_(Phobos::readBuffer, "inair")) {
				this->EMP_Threshold = -1;
			} else {

				bool ret;
				if (Parser<bool, 1>::Parse(Phobos::readBuffer, &ret)) {
					this->EMP_Threshold = (int)ret;
				} else if(!Parser<int, 1>::Parse(Phobos::readBuffer , &this->EMP_Threshold)) {
					Debug::INIParseFailed(pSection, "EMP.Treshold", Phobos::readBuffer, "[Phobos] Invalid value");
				}
			}
		}

		this->PoweredBy.Read(exINI, pSection, "PoweredBy");

		for (int i = 0; i < SideClass::Array->Count; ++i) {
			detail::read(this->Survivors_Pilots[i],
			exINI,
			pSection,
			(std::string("Survivor.Side") + std::to_string(i)).c_str()
			);
		}

		this->Ammo_AddOnDeploy.Read(exINI, pSection, "Ammo.AddOnDeploy");
		this->Ammo_AutoDeployMinimumAmount.Read(exINI, pSection, "Ammo.AutoDeployMinimumAmount");
		this->Ammo_AutoDeployMaximumAmount.Read(exINI, pSection, "Ammo.AutoDeployMaximumAmount");
		this->Ammo_DeployUnlockMinimumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMinimumAmount");
		this->Ammo_DeployUnlockMaximumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMaximumAmount");

		this->ImmuneToWeb.Read(exINI, pSection, "ImmuneToWeb");
		this->Webby_Anims.Read(exINI, pSection, "Webby.Anims");
		this->Webby_Modifier.Read(exINI, pSection, "Webby.Modifier");
		this->Webby_Duration_Variation.Read(exINI, pSection, "Webby.DurationVariation");

		// new secret lab
		this->Secret_RequiredHouses
			= pINI->ReadHouseTypesList(pSection, "SecretLab.RequiredHouses", this->Secret_RequiredHouses);

		this->Secret_ForbiddenHouses
			= pINI->ReadHouseTypesList(pSection, "SecretLab.ForbiddenHouses", this->Secret_ForbiddenHouses);

		this->ReloadInTransport.Read(exINI, pSection, "ReloadInTransport");
		this->Weeder_TriggerPreProductionBuildingAnim.Read(exINI, pSection, "Weeder.TriggerPreProductionBuildingAnim");
		this->Weeder_PipIndex.Read(exINI, pSection, "Weeder.PipIndex");
		this->Weeder_PipEmptyIndex.Read(exINI, pSection, "Weeder.PipEmptyIndex");
		this->CanBeDriven.Read(exINI, pSection, "CanBeDriven");

		this->CloakPowered.Read(exINI, pSection, "Cloakable.Powered");
		this->CloakDeployed.Read(exINI, pSection, "Cloakable.Deployed");
		this->ProtectedDriver.Read(exINI, pSection, "ProtectedDriver");
		this->ProtectedDriver_MinHealth.Read(exINI, pSection, "ProtectedDriver.MinHealth");
		this->KeepAlive.Read(exINI, pSection, "KeepAlive");
		this->DetectDisguise_Percent.Read(exINI, pSection, "DetectDisguise.Percent");
		this->PassengerTurret.Read(exINI, pSection, "PassengerTurret");

		this->Tint_Color.Read(exINI, pSection, "Tint.Color");
		this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
		this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

		this->PhobosAttachEffects.LoadFromINI(pINI, pSection);

		this->KeepTargetOnMove.Read(exINI, pSection, "KeepTargetOnMove");
		this->KeepTargetOnMove_Weapon.Read(exINI, pSection, "KeepTargetOnMove.Weapon");
		this->KeepTargetOnMove_ExtraDistance.Read(exINI, pSection, "KeepTargetOnMove.ExtraDistance");
		this->KeepTargetOnMove_NoMorePursuit.Read(exINI, pSection, "KeepTargetOnMove.NoMorePursuit");
		this->AllowAirstrike.Read(exINI, pSection, "AllowAirstrike");
		this->ForbidParallelAIQueues.Read(exINI, pSection, "ForbidParallelAIQueues");

		this->EVA_Combat.Read(exINI, pSection, "EVA.Combat");
		this->CombatAlert.Read(exINI, pSection, "CombatAlert");
		this->CombatAlert_UseFeedbackVoice.Read(exINI, pSection, "CombatAlert.UseFeedbackVoice");
		this->CombatAlert_UseAttackVoice.Read(exINI, pSection, "CombatAlert.UseAttackVoice");
		this->CombatAlert_UseEVA.Read(exINI, pSection, "CombatAlert.UseEVA");
		this->CombatAlert_NotBuilding.Read(exINI, pSection, "CombatAlert.NotBuilding");
		this->SubterraneanHeight.Read(exINI, pSection, "SubterraneanHeight");

		this->Spawner_RecycleRange.Read(exINI, pSection, "Spawner.RecycleRange");
		this->Spawner_RecycleFLH.Read(exINI, pSection, "Spawner.RecycleCoord");
		this->Spawner_RecycleOnTurret.Read(exINI, pSection, "Spawner.RecycleOnTurret");
		this->Spawner_RecycleAnim.Read(exINI, pSection, "Spawner.RecycleAnim");

		this->AINormalTargetingDelay.Read(exINI, pSection, "AINormalTargetingDelay");
		this->PlayerNormalTargetingDelay.Read(exINI, pSection, "PlayerNormalTargetingDelay");
		this->AIGuardAreaTargetingDelay.Read(exINI, pSection, "AIGuardAreaTargetingDelay");
		this->PlayerGuardAreaTargetingDelay.Read(exINI, pSection, "PlayerGuardAreaTargetingDelay");
		this->DistributeTargetingFrame.Read(exINI, pSection, "DistributeTargetingFrame");
		this->AIAttackMoveTargetingDelay.Read(exINI, pSection, "AIAttackMoveTargetingDelay");
		this->PlayerAttackMoveTargetingDelay.Read(exINI, pSection, "PlayerAttackMoveTargetingDelay");

		this->CanBeBuiltOn.Read(exINI, pSection, "CanBeBuiltOn");
		this->UnitBaseNormal.Read(exINI, pSection, "UnitBaseNormal");
		this->UnitBaseForAllyBuilding.Read(exINI, pSection, "UnitBaseForAllyBuilding");
		this->ChronoSpherePreDelay.Read(exINI, pSection, "ChronoSpherePreDelay");
		this->ChronoSphereDelay.Read(exINI, pSection, "ChronoSphereDelay");
		this->PassengerWeapon.Read(exINI, pSection, "PassengerWeapon");


		this->IsSimpleDeployer_ConsiderPathfinding.Read(exINI, pSection, "IsSimpleDeployer.ConsiderPathfinding");
		this->IsSimpleDeployer_DisallowedLandTypes.Read(exINI, pSection, "IsSimpleDeployer.DisallowedLandTypes");

		//this->ShadowIndices.Read(exINI, pSection, "ShadowIndices");

		this->EliteArmor.Read(exINI, pSection, "EliteArmor");
		this->VeteranArmor.Read(exINI, pSection, "VeteranArmor");
		this->DeployedArmor.Read(exINI, pSection, "DeployedArmor");

		this->Cloakable_IgnoreArmTimer.Read(exINI, pSection, "Cloakable.IgnoreROFTimer");
		this->Convert_HumanToComputer.Read(exINI, pSection, "Convert.HumanToComputer");
		this->Convert_ComputerToHuman.Read(exINI, pSection, "Convert.ComputerToHuman");

		this->AutoDeath_OnOwnerChange.Read(exINI, pSection, "AutoDeath.OnOwnerChange");
		this->AutoDeath_OnOwnerChange_HumanToComputer.Read(exINI, pSection, "AutoDeath.OnOwnerChange.HumanToComputer");
		this->AutoDeath_OnOwnerChange_ComputerToHuman.Read(exINI, pSection, "AutoDeath.OnOwnerChange.ComputerToHuman");

		this->ShadowSizeCharacteristicHeight.Read(exINI, pSection, "ShadowSizeCharacteristicHeight");

		if(auto pTalkBuble = FileSystem::TALKBUBL_SHP()){
			for (int i = 0; i < pTalkBuble->Frames; ++i) {
				std::string base = "TalkbubbleFrame";
				base += std::to_string(i);
				this->TalkbubbleVoices.emplace_back().Read(exINI, pSection, (base + ".Voices").c_str());
			}
		}

		this->NoExtraSelfHealOrRepair.Read(exINI, pSection, "NoExtraSelfHealOrRepair");

#pragma region BuildLimitGroup
		this->BuildLimitGroup_Types.Read(exINI, pSection, "BuildLimitGroup.Types");
		this->BuildLimitGroup_Nums.Read(exINI, pSection, "BuildLimitGroup.Nums");
		this->BuildLimitGroup_Factor.Read(exINI, pSection, "BuildLimitGroup.Factor");
		this->BuildLimitGroup_ContentIfAnyMatch.Read(exINI, pSection, "BuildLimitGroup.ContentIfAnyMatch");
		this->BuildLimitGroup_NotBuildableIfQueueMatch.Read(exINI, pSection, "BuildLimitGroup.NotBuildableIfQueueMatch");
		this->BuildLimitGroup_ExtraLimit_Types.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.Types");
		this->BuildLimitGroup_ExtraLimit_Nums.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.Nums");
		this->BuildLimitGroup_ExtraLimit_MaxCount.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.MaxCount");
		this->BuildLimitGroup_ExtraLimit_MaxNum.Read(exINI, pSection, "BuildLimitGroup.ExtraLimit.MaxNum");
#pragma endregion

		this->Tiberium_EmptyPipIdx.Read(exINI, pSection, "StorageEmptyPipIndex");
		this->Tiberium_PipIdx.Read(exINI, pSection, "StoragePipIndexes");
		this->Tiberium_PipShapes.Read(exINI, pSection, "StoragePipShapes");
		this->Tiberium_PipShapes_Palette.Read(exINI, pSection, "StoragePipShapesPalette");

		if (this->AbsType != AbstractType::BuildingType)
		{
			this->Untrackable.Read(exINI, pSection, "Untrackable");
			this->LargeVisceroid.Read(exINI, pSection, "Visceroid.Large");
			this->HarvesterDumpAmount.Read(exINI, pSection, "HarvesterDumpAmount");
			this->AttackMove_Aggressive.Read(exINI, pSection, "AttackMove.Aggressive");
			this->AttackMove_UpdateTarget.Read(exINI, pSection, "AttackMove.UpdateTarget");
			this->HarvesterScanAfterUnload.Read(exINI, pSection, "HarvesterScanAfterUnload");
			this->DropPodProp.Read(exINI, pSection);
		}

		this->RefinerySmokeParticleSystemOne.Read(exINI, pSection, "RefinerySmokeParticleSystemOne");
		this->RefinerySmokeParticleSystemTwo.Read(exINI, pSection, "RefinerySmokeParticleSystemTwo");
		this->RefinerySmokeParticleSystemThree.Read(exINI, pSection, "RefinerySmokeParticleSystemThree");
		this->RefinerySmokeParticleSystemFour.Read(exINI, pSection, "RefinerySmokeParticleSystemFour");

		exINI.ReadSpeed(pSection, "SubterraneanSpeed", &this->SubterraneanSpeed);

		this->ForceWeapon_InRange_Overrides.Read(exINI, pSection, "ForceWeapon.InRange.Overrides");
		this->ForceWeapon_InRange_ApplyRangeModifiers.Read(exINI, pSection, "ForceWeapon.InRange.ApplyRangeModifiers");
		this->ForceAAWeapon_InRange.Read(exINI, pSection, "ForceAAWeapon.InRange");
		this->ForceAAWeapon_InRange_Overrides.Read(exINI, pSection, "ForceAAWeapon.InRange.Overrides");
		this->ForceAAWeapon_InRange_ApplyRangeModifiers.Read(exINI, pSection, "ForceAAWeapon.InRange.ApplyRangeModifiers");
		this->ForceWeapon_InRange_TechnoOnly.Read(exINI, pSection, "ForceWeapon.InRange.TechnoOnly");

		if (!RefinerySmokeParticleSystemOne.isset()) {
			RefinerySmokeParticleSystemOne = This()->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemTwo.isset()) {
			RefinerySmokeParticleSystemTwo = This()->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemThree.isset()) {
			RefinerySmokeParticleSystemThree = This()->RefinerySmokeParticleSystem;
		}

		if (!RefinerySmokeParticleSystemFour.isset()) {
			RefinerySmokeParticleSystemFour = This()->RefinerySmokeParticleSystem;
		}

		char tempBuffer[256];

		this->Convert_ToHouseOrCountry.clear();
		Nullable<TechnoTypeClass*> technoType;
		// put all sides into the map
		this->Convert_ToHouseOrCountry.reserve(SideClass::Array->Count + HouseTypeClass::Array->Count);

		for (auto const& pSide : *SideClass::Array) {
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Convert.To%s", pSide->ID);
			technoType.Read(exINI, pSection, tempBuffer);
			if (technoType.isset()) {
				this->Convert_ToHouseOrCountry.insert(pSide, technoType.Get());
			}
		}

		// put all countries into the map
		for (auto const& pTHouse : *HouseTypeClass::Array) {
			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Convert.To%s", pTHouse->ID);
			technoType.Read(exINI, pSection, tempBuffer);
			if (technoType.isset()) {
				this->Convert_ToHouseOrCountry.insert(pTHouse, technoType.Get());
			}
		}

		this->SuppressKillWeapons.Read(exINI, pSection, "SuppressKillWeapons");
		this->SuppressKillWeapons_Types.Read(exINI, pSection, "SuppressKillWeapons.Types");

		this->NoQueueUpToEnter.Read(exINI, pSection, "NoQueueUpToEnter");
		this->NoQueueUpToUnload.Read(exINI, pSection, "NoQueueUpToUnload");

		this->NoRearm_UnderEMP.Read(exINI, pSection, "NoRearm.UnderEMP");
		this->NoRearm_Temporal.Read(exINI, pSection, "NoRearm.Temporal");
		this->NoReload_UnderEMP.Read(exINI, pSection, "NoReload.UnderEMP");
		this->NoReload_Temporal.Read(exINI, pSection, "NoReload.Temporal");

		this->RateDown_Ammo.Read(exINI, pSection, "RateDown.Ammo");
		this->RateDown_Delay.Read(exINI, pSection, "RateDown.Delay");
		this->RateDown_Cover.Read(exINI, pSection, "RateDown.Cover");
		this->RateDown_Reset.Read(exINI, pSection, "RateDown.Reset");

		this->CanManualReload.Read(exINI, pSection, "CanManualReload");
		this->CanManualReload_ResetROF.Read(exINI, pSection, "CanManualReload.ResetROF");
		this->CanManualReload_DetonateWarhead.Read(exINI, pSection, "CanManualReload.DetonateWarhead");
		this->CanManualReload_DetonateConsume.Read(exINI, pSection, "CanManualReload.DetonateConsume");
		this->Power.Read(exINI, pSection, "Power");

		// please dont @ me if you got some weird bug with this tag turn on
		// @ the original author
		// - Otamaa
		this->BunkerableAnyway.Read(exINI, pSection, "BunkerableAnyway");

		this->JumpjetTilt.Read(exINI, pSection, "JumpjetTilt");
		this->JumpjetTilt_ForwardAccelFactor.Read(exINI, pSection, "JumpjetTilt.ForwardAccelFactor");
		this->JumpjetTilt_ForwardSpeedFactor.Read(exINI, pSection, "JumpjetTilt.ForwardSpeedFactor");
		this->JumpjetTilt_SidewaysRotationFactor.Read(exINI, pSection, "JumpjetTilt.SidewaysRotationFactor");
		this->JumpjetTilt_SidewaysSpeedFactor.Read(exINI, pSection, "JumpjetTilt.SidewaysSpeedFactor");

		this->NoTurret_TrackTarget.Read(exINI, pSection, "NoTurret.TrackTarget");
		this->RecountBurst.Read(exINI, pSection, "RecountBurst");

		this->AirstrikeLineColor.Read(exINI, pSection, "AirstrikeLineColor");

		this->InitialSpawnsNumber.Read(exINI, pSection, "InitialSpawnsNumber");
		this->Spawns_Queue.Read(exINI, pSection, "Spawns.Queue");

		this->Sinkable.Read(exINI, pSection, "Sinkable");
		this->SinkSpeed.Read(exINI, pSection, "SinkSpeed");
		this->Sinkable_SquidGrab.Read(exINI, pSection, "Sinkable.SquidGrab");

		this->AmphibiousEnter.Read(exINI, pSection, "AmphibiousEnter");
		this->AmphibiousUnload.Read(exINI, pSection, "AmphibiousUnload");

		this->DamagedSpeed.Read(exINI, pSection, "DamagedSpeed");
		this->RadarInvisibleToHouse.Read(exINI, pSection, "RadarInvisibleToHouse");
		// Spawner range
		this->ResetSpawnerRange();

		this->AdvancedDrive_Reverse.Read(exINI, pSection, "AdvancedDrive.Reverse");
		this->AdvancedDrive_Reverse_FaceTarget.Read(exINI, pSection, "AdvancedDrive.Reverse.FaceTarget");
		this->AdvancedDrive_Reverse_FaceTargetRange.Read(exINI, pSection, "AdvancedDrive.Reverse.FaceTargetRange");
		this->AdvancedDrive_Reverse_MinimumDistance.Read(exINI, pSection, "AdvancedDrive.Reverse.MinimumDistance");
		this->AdvancedDrive_Reverse_RetreatDuration.Read(exINI, pSection, "AdvancedDrive.Reverse.RetreatDuration");
		this->AdvancedDrive_Reverse_Speed.Read(exINI, pSection, "AdvancedDrive.Reverse.Speed");
		this->AdvancedDrive_Hover.Read(exINI, pSection, "AdvancedDrive.Hover");
		this->AdvancedDrive_Hover_Sink.Read(exINI, pSection, "AdvancedDrive.Hover.Sink");
		this->AdvancedDrive_Hover_Spin.Read(exINI, pSection, "AdvancedDrive.Hover.Spin");
		this->AdvancedDrive_Hover_Tilt.Read(exINI, pSection, "AdvancedDrive.Hover.Tilt");
		this->AdvancedDrive_Hover_Height.Read(exINI, pSection, "AdvancedDrive.Hover.Height");
		this->AdvancedDrive_Hover_Dampen.Read(exINI, pSection, "AdvancedDrive.Hover.Dampen");
		this->AdvancedDrive_Hover_Bob.Read(exINI, pSection, "AdvancedDrive.Hover.Bob");
		this->Harvester_CanGuardArea.Read(exINI, pSection, "Harvester.CanGuardArea");
		this->Harvester_CanGuardArea_RequireTarget.Read(exINI, pSection, "Harvester.CanGuardArea.RequireTarget");
	
		Nullable<int> transDelay {};
		transDelay.Read(exINI, pSection, "TiberiumEater.TransDelay");

		if (transDelay.isset() && transDelay >= 0 && !this->TiberiumEaterType)
			this->TiberiumEaterType = std::make_unique<TiberiumEaterTypeClass>();


		if (this->TiberiumEaterType) {

			if (transDelay.isset() && transDelay.Get() < 0)
				this->TiberiumEaterType.reset();
			else
				this->TiberiumEaterType->LoadFromINI(pINI, pSection);
		}

		if (This()->Passengers > 0)
		{
			size_t passengers = This()->Passengers + 1;

			if (this->Insignia_Passengers.empty() || this->Insignia_Passengers.size() != passengers)
			{
				this->Insignia_Passengers.resize(passengers);
				this->InsigniaFrame_Passengers.resize(passengers, Promotable<int>(-1));
				Valueable<Vector3D<int>> frames;
				frames = Vector3D<int>(-1, -1, -1);
				this->InsigniaFrames_Passengers.resize(passengers, frames);
			}

			for (size_t i = 0; i < passengers; i++)
			{
				Nullable<InsigniaTypeClass*> InsigniaType_Passengers;
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "InsigniaType.Passengers%d", i);
				InsigniaType_Passengers.Read(exINI, pSection, tempBuffer);

				if (InsigniaType_Passengers.isset())
				{
					this->Insignia_Passengers[i] = InsigniaType_Passengers.Get()->Insignia;
					this->InsigniaFrame_Passengers[i] = InsigniaType_Passengers.Get()->InsigniaFrame;
					this->InsigniaFrames_Passengers[i] = Vector3D<int>(-1, -1, -1); // override it so only InsigniaFrame will be used
				}
				else
				{
					IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "Insignia.Passengers%d.%s", i, "%s");
					this->Insignia_Passengers[i].Read(exINI, pSection, tempBuffer);

					IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "InsigniaFrame.Passengers%d.%s", i, "%s");
					this->InsigniaFrame_Passengers[i].Read(exINI, pSection, tempBuffer);

					IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "InsigniaFrames.Passengers%d", i);
					this->InsigniaFrames_Passengers[i].Read(exINI, pSection, tempBuffer);
				}
			}
		}

		this->BattlePoints.Read(exINI, pSection, "BattlePoints");
		this->DefaultVehicleDisguise.Read(exINI, pSection, "DefaultVehicleDisguise");
		this->TurretResponse.Read(exINI, pSection, "TurretResponse");
		this->Unload_SkipPassengers.Read(exINI, pSection, "Deploy.SkipPassengerUnload");
		this->Unload_NoPassengers.Read(exINI, pSection, "Deploy.NoPassenger");
		this->Unload_SkipHarvester.Read(exINI, pSection, "Deploy.NoTiberium");
		this->Unload_NoTiberiums.Read(exINI, pSection, "Unload.NoTiberiums");
		this->PlayerGuardModePursuit.Read(exINI, pSection, "PlayerGuardModePursuit");
		this->PlayerGuardModeStray.Read(exINI, pSection, "PlayerGuardModeStray");
		this->PlayerGuardModeGuardRangeMultiplier.Read(exINI, pSection, "PlayerGuardModeGuardRangeMultiplier");
		this->PlayerGuardModeGuardRangeAddend.Read(exINI, pSection, "PlayerGuardModeGuardRangeAddend");
		this->PlayerGuardStationaryStray.Read(exINI, pSection, "PlayerGuardStationaryStray");
		this->AIGuardModePursuit.Read(exINI, pSection, "AIGuardModePursuit");
		this->AIGuardModeStray.Read(exINI, pSection, "AIGuardModeStray");
		this->AIGuardModeGuardRangeMultiplier.Read(exINI, pSection, "AIGuardModeGuardRangeMultiplier");
		this->AIGuardModeGuardRangeAddend.Read(exINI, pSection, "AIGuardModeGuardRangeAddend");
		this->AIGuardStationaryStray.Read(exINI, pSection, "AIGuardStationaryStray");

		this->ForceWeapon_Check = (
			this->ForceWeapon_Naval_Decloaked >= 0	||
			this->ForceWeapon_Cloaked >= 0			||
			this->ForceWeapon_Disguised >= 0		||
			this->ForceWeapon_UnderEMP >= 0			||
			!this->ForceWeapon_InRange.empty()		||
			!this->ForceAAWeapon_InRange.empty()	||
			this->ForceWeapon_Buildings >= 0		||
			this->ForceWeapon_Defenses >= 0			||
			this->ForceWeapon_Infantry >= 0			||
			this->ForceWeapon_Naval_Units >= 0		||
			this->ForceWeapon_Units >= 0			||
			this->ForceWeapon_Aircraft >= 0			||
			this->ForceAAWeapon_Infantry >= 0		||
			this->ForceAAWeapon_Units >= 0			||
			this->ForceAAWeapon_Aircraft >= 0		||
			this->ForceWeapon_Capture >= 0
		);

		this->FiringForceScatter.Read(exINI, pSection, "FiringForceScatter");
		this->Convert_ResetMindControl.Read(exINI, pSection, "Convert.ResetMindControl");

		this->DigitalDisplay_Health_FakeAtDisguise.Read(exINI, pSection, "DigitalDisplay.Health.FakeAtDisguise");
		this->EngineerRepairAmount.Read(exINI, pSection, "EngineerRepairAmount");

		this->DebrisTypes_Limit.Read(exINI, pSection, "DebrisTypes.Limit");
		this->DebrisMinimums.Read(exINI, pSection, "DebrisMinimums");

		this->AttackMove_Follow.Read(exINI, pSection, "AttackMove.Follow");
		this->AttackMove_Follow_IncludeAir.Read(exINI, pSection, "AttackMove.Follow.IncludeAir");
		this->AttackMove_StopWhenTargetAcquired.Read(exINI, pSection, "AttackMove.StopWhenTargetAcquired");
		this->AttackMove_PursuitTarget.Read(exINI, pSection, "AttackMove.PursuitTarget");
		this->SkipCrushSlowdown.Read(exINI, pSection, "SkipCrushSlowdown");
		this->RecuitedAs.Read(exINI, pSection, "Recruited.As");

		this->AttackMove_Follow_IfMindControlIsFull.Read(exINI, pSection, "AttackMove.Follow.IfMindControlIsFull");
		this->PenetratesTransport_Level.Read(exINI, pSection, "PenetratesTransport.Level");
		this->PenetratesTransport_PassThroughMultiplier.Read(exINI, pSection, "PenetratesTransport.PassThroughMultiplier");
		this->PenetratesTransport_FatalRateMultiplier.Read(exINI, pSection, "PenetratesTransport.FatalRateMultiplier");
		this->PenetratesTransport_DamageMultiplier.Read(exINI, pSection, "PenetratesTransport.DamageMultiplier");

		this->PassiveAcquireMode.Read(exINI, pSection, "PassiveAcquireMode");
		this->PassiveAcquireMode_Togglable.Read(exINI, pSection, "PassiveAcquireMode.Togglable");
		this->VoiceEnterAggressiveMode.Read(exINI, pSection, "VoiceEnterAggressiveMode");
		this->VoiceExitAggressiveMode.Read(exINI, pSection, "VoiceExitAggressiveMode");
		this->VoiceEnterCeasefireMode.Read(exINI, pSection, "VoiceEnterCeasefireMode");
		this->VoiceExitCeasefireMode.Read(exINI, pSection, "VoiceExitCeasefireMode");

		this->CanBlock.Read(exINI, pSection, "CanBlock");

		if (this->BlockType == nullptr)
			this->BlockType = std::make_unique<BlockTypeClass>();

		this->BlockType->LoadFromINI(pINI, pSection);
		this->TeamMember_ConsideredAs.Read(exINI, pSection, "TeamMember.ConsideredAs");
		this->WeaponGroupAs.resize(pThis->WeaponCount);
		this->CanGoAboveTarget.Read(exINI, pSection, "CanGoAboveTarget");
		this->OpenTransport_RangeBonus.Read(exINI, pSection, "OpenTransport.RangeBonus");
		this->OpenTransport_DamageMultiplier.Read(exINI, pSection, "OpenTransport.DamageMultiplier");

		this->ParadropMission.Read(exINI, pSection, "ParadropMission");
		this->AIParadropMission.Read(exINI, pSection, "AIParadropMission");
		this->AreaGuardRange.Read(exINI, pSection, "AreaGuardRange");
		this->MaxGuardRange.Read(exINI, pSection, "MaxGuardRange");
		this->JumpjetClimbIgnoreBuilding.Read(exINI, pSection, "JumpjetClimbIgnoreBuilding");

		for (int idx = 0; idx < pThis->WeaponCount; ++idx) {
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "WeaponGroupAs%d", idx + 1);
			this->WeaponGroupAs[idx].Read(pINI, pSection, tempBuffer);
		}
	}

	this->TintColorAirstrike = GeneralUtils::GetColorFromColorAdd(this->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor));

	// Art tags
	if (pArtIni && pArtIni->GetSection(pArtSection))
	{
		INI_EX exArtINI(pArtIni);

		this->FireUp.Read(exArtINI, pArtSection, "FireUp");
		this->FireUp_ResetInRetarget.Read(exArtINI, pArtSection, "FireUp.ResetInRetarget");

		this->GreyCameoPCX.Read(&CCINIClass::INI_Art, pArtSection, "GreyCameoPCX");
		this->AlternateFLH_OnTurret.Read(exArtINI, pArtSection, "AlternateFLH.OnTurret");
		this->TurretOffset.Read(exArtINI, pArtSection, GameStrings::TurretOffset());
		this->AlternateFLH_ApplyVehicle.Read(exArtINI, pArtSection, "AlternateFLH.ApplyVehicle");

		if (!this->TurretOffset.isset())
		{
			//put ddedfault single value inside
			this->TurretOffset = PartialVector3D<int>{ pThis->TurretOffset , 0 ,0 , 1 };
		}

		this->TurretShadow.Read(exArtINI, pArtSection, "TurretShadow");


		static COMPILETIMEEVAL std::array<CoordStruct, 8> defaultSprayOffsets = { {
			{  256,   0,   0 },
			{  180, 180,   0 },
			{    0, 256,   0 },
			{ -180, 180,   0 },
			{ -256,   0,   0 },
			{ -180,-180,   0 },
			{    0,-256,   0 },
			{  180,-180,   0 }
		} };

		this->SprayOffsets.resize(defaultSprayOffsets.size());

		for (size_t i = 0; i < defaultSprayOffsets.size(); ++i) {
			this->SprayOffsets[i] = defaultSprayOffsets[i];
		}

		for (size_t c = 0; ; ++c) {
			std::string __base_key = "SprayOffsets";
			__base_key += std::to_string(c);
			CoordStruct val {};

			if (!detail::read(val, exArtINI, pArtSection, __base_key.c_str()))
				break;
			else
			{
				if (c < this->SprayOffsets.size())
					this->SprayOffsets[c] = val;
				else
					this->SprayOffsets.emplace_back(std::move(val));
			}
		}

		std::vector<int> shadow_indices {};
		detail::ReadVectors(shadow_indices, exArtINI, pArtSection, "ShadowIndices");
		std::vector<int> shadow_indices_frame {};
		detail::ReadVectors(shadow_indices_frame, exArtINI, pArtSection, "ShadowIndices.Frame");

		if (shadow_indices_frame.size() != shadow_indices.size())
		{
			if (!shadow_indices_frame.empty())
				Debug::LogInfo("[Developer warning] {} ShadowIndices.Frame size ({}) does not match ShadowIndices size ({}) "
					, pSection, shadow_indices_frame.size(), shadow_indices.size());

			shadow_indices_frame.resize(shadow_indices.size(), -1);
		}

		for (size_t i = 0; i < shadow_indices.size(); i++)
			this->ShadowIndices[shadow_indices[i]] = shadow_indices_frame[i];

		this->ShadowIndex_Frame.Read(exArtINI, pArtSection, "ShadowIndex.Frame");

		this->LaserTrailData.clear();

		for (size_t i = 0; ; ++i)
		{
			std::string _base_key = "LaserTrail";
			_base_key += std::to_string(i);

			if (exArtINI->ReadString(pArtSection, (_base_key + ".Type").c_str(), Phobos::readDefval, Phobos::readBuffer) <= 0)
				break;

			int def;
			if (!Parser<LaserTrailTypeClass,1>::TryParseIndex(Phobos::readBuffer, &def))
				break;

			auto data = &this->LaserTrailData.emplace_back();
			data->idxType = def;

			detail::read(data->FLH , exArtINI, pArtSection,  (_base_key + ".FLH").c_str());
			detail::read(data->IsOnTurret , exArtINI, pArtSection,  (_base_key + ".IsOnTurret").c_str());
		}

		this->AlternateFLHs.clear();

		for (size_t i = 0; ; ++i)
		{
			Nullable<CoordStruct> alternateFLH;
			alternateFLH.Read(exArtINI, pArtSection, (std::string("AlternateFLH") + std::to_string(i)).c_str());

			if (!alternateFLH.isset()) {
				if( i < 5){
					this->AlternateFLHs.emplace_back();
					continue;
				}
				else
					break;
			}

			this->AlternateFLHs.push_back(alternateFLH.Get());
		}

		this->HitCoordOffset.clear();

		for (size_t i = 0; ; ++i)
		{
			Nullable<CoordStruct> nHitBuff;
			nHitBuff.Read(exArtINI, pArtSection, (std::string("HitCoordOffset") + std::to_string(i)).c_str());

			if (!nHitBuff.isset())
				break;

			this->HitCoordOffset.push_back(nHitBuff);
		}

		this->HitCoordOffset_Random.Read(exArtINI, pArtSection, "HitCoordOffset.Random");

		this->Spawner_SpawnOffsets.Read(exArtINI, pArtSection, "SpawnOffset");
		this->Spawner_SpawnOffsets_OverrideWeaponFLH.Read(exArtINI, pArtSection, "SpawnOffsetOverrideFLH");
		this->ShadowScale.Read(exArtINI, pArtSection, "ShadowScale");

		//LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);

		// change the size on techno type and add more entry
		//ColletiveCoordStructVectorData nFLH = { &WeaponBurstFLHs  , &DeployedWeaponBurstFLHs , &CrouchedWeaponBurstFLHs };
		//ColletiveCoordStructVectorData nEFLH = { &EliteWeaponBurstFLHs  , &EliteDeployedWeaponBurstFLHs , &EliteCrouchedWeaponBurstFLHs };
		//const char* tags[sizeof(ColletiveCoordStructVectorData) / sizeof(void*)] = { Phobos::readDefval  , "Deployed" , "Prone" };
		//TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, nFLH, nEFLH, tags);

		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, "");
		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, "Deployed");
		TechnoTypeExtData::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, "Prone");

		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, PronePrimaryFireFLH, E_PronePrimaryFireFLH, "PronePrimaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, ProneSecondaryFireFLH, E_ProneSecondaryFireFLH, "ProneSecondaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, DeployedPrimaryFireFLH, E_DeployedPrimaryFireFLH, "DeployedPrimaryFire");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, DeployedSecondaryFireFLH, E_DeployedSecondaryFireFLH, "DeployedSecondaryFire");

		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, PrimaryCrawlFLH, Elite_PrimaryCrawlFLH, "PrimaryCrawling");
		TechnoTypeExtData::GetFLH(exArtINI, pArtSection, SecondaryCrawlFLH, Elite_SecondaryCrawlFLH, "SecondaryCrawling");

		this->MyExtraFireData.ReadArt(exArtINI, pArtSection);
		this->MySpawnSupportFLH.Read(exArtINI, pArtSection);
		this->Trails.Read(exArtINI, pArtSection, true);

		this->CameoPCX.Read(exArtINI.GetINI(), pArtSection, "CameoPCX");
		this->AltCameoPCX.Read(exArtINI.GetINI(), pArtSection, "AltCameoPCX");
		this->CameoPal.Read(exArtINI, pArtSection, "CameoPalette");
	}

	if (GeneralUtils::IsValidString(pThis->PaletteFile) && !pThis->Palette)
		Debug::Log("[Developer warning] [%s] has Palette=%s set but no palette file was loaded (missing file or wrong filename). Missing palettes cause issues with lighting recalculations.\n", pArtSection, pThis->PaletteFile);

	return true;
}

void TechnoTypeExtData::LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI)
{
	//auto pThis = Get();
	//const char* pSection = pThis->ID;
	//INI_EX exINI(pINI);

}

void TechnoTypeExtData::InitializeConstant()
{
	this->AttachedEffect.Owner = This();
	this->PassengerDeletionType.OwnerType = This();
}

ImageStatusses ImageStatusses::ReadVoxel(const char* const nKey)
{
	std::string _buffer = nKey;
	const size_t key_len = _buffer.size();
	_buffer += ".VXL";
	CCFileClass CCFileV { _buffer.c_str() };

	if (CCFileV.Exists())
	{
		MotLib* pLoadedHVA = nullptr;
		VoxLib* pLoadedVXL = GameCreate<VoxLib>(&CCFileV, false);
		_buffer[key_len + 1] = 'H';
		_buffer[key_len + 2] = 'V';
		_buffer[key_len + 3] = 'A';
		CCFileClass  CCFileH { _buffer.c_str() };

		if (CCFileH.Open(FileAccessMode::Read)) {
			pLoadedHVA = GameCreate<MotLib>(&CCFileH);
		}

		if (!pLoadedHVA || pLoadedVXL->LoadFailed || pLoadedHVA->LoadedFailed)
		{
			GameDelete<true, true>(pLoadedHVA);
			GameDelete<true, true>(pLoadedVXL);

			return { {nullptr , nullptr} , false };
		}
		else
		{
			pLoadedHVA->Scale(pLoadedVXL->TailerData[pLoadedVXL->HeaderData->limb_number].HVAMultiplier);
			return { {pLoadedVXL , pLoadedHVA }, true };
		}
	}

	return { {nullptr , nullptr} , false };
}

void TechnoTypeExtData::AdjustCrushProperties()
{
	auto const pThis = This();

	if (this->CrushLevel.Rookie <= 0)
	{
		if (pThis->OmniCrusher)
			this->CrushLevel.Rookie = 10;
		else if (pThis->Crusher)
			this->CrushLevel.Rookie = 5;
		else
			this->CrushLevel.Rookie = 0;
	}
	if (this->CrushLevel.Veteran <= 0)
	{
		if (!pThis->OmniCrusher && pThis->VeteranAbilities.CRUSHER)
			this->CrushLevel.Veteran = 5;
		else
			this->CrushLevel.Veteran = this->CrushLevel.Rookie;
	}
	if (this->CrushLevel.Elite <= 0)
	{
		if (!pThis->OmniCrusher && pThis->EliteAbilities.CRUSHER)
			this->CrushLevel.Elite = 5;
		else
			this->CrushLevel.Elite = this->CrushLevel.Veteran;
	}
	if (!pThis->Crusher && (this->CrushLevel.Rookie > 0 || this->CrushLevel.Veteran > 0 || this->CrushLevel.Elite > 0) &&
		this->AbsType == UnitTypeClass::AbsID)
		pThis->Crusher = true;

	if (this->CrushableLevel.Rookie <= 0)
	{
		if (pThis->OmniCrushResistant)
			this->CrushableLevel.Rookie = 10;
		else if (!pThis->Crushable)
			this->CrushableLevel.Rookie = 5;
		else
			this->CrushableLevel.Rookie = 0;
	}
	if (this->CrushableLevel.Veteran <= 0)
		this->CrushableLevel.Veteran = this->CrushableLevel.Rookie;
	if (this->CrushableLevel.Elite <= 0)
		this->CrushableLevel.Elite = this->CrushableLevel.Veteran;

	if (const auto pInfType = type_cast<InfantryTypeClass*>(pThis))
	{
		if (this->DeployCrushableLevel.Rookie <= 0)
		{
			if (!pInfType->DeployedCrushable)
				this->DeployCrushableLevel.Rookie = 5;
			else
				this->DeployCrushableLevel.Rookie = this->CrushableLevel.Rookie;
		}
		if (this->DeployCrushableLevel.Veteran <= 0)
			this->DeployCrushableLevel.Veteran = this->DeployCrushableLevel.Rookie;
		if (this->DeployCrushableLevel.Elite <= 0)
			this->DeployCrushableLevel.Elite = this->DeployCrushableLevel.Veteran;
	}
}

bool TechnoTypeExtData::PassangersAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pPassanger)
{
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis);

	if (!pExt->PassengersWhitelist.Eligible(pPassanger))
		return false;

	if (!pExt->PassengersBlacklist.empty() && pExt->PassengersBlacklist.Contains(pPassanger))
		return false;

	return true;
}

// used for more WeaponX added by Ares.
WeaponStruct* TechnoTypeExtData::GetWeaponStruct(TechnoTypeClass* pThis, int nWeaponIndex, bool isElite)
{
	return isElite ? pThis->GetEliteWeapon(nWeaponIndex) : pThis->GetWeapon(nWeaponIndex);
}


// =============================
// container
TechnoTypeExtContainer TechnoTypeExtContainer::Instance;
//std::vector<TechnoTypeExtData*> Container<TechnoTypeExtData>::Array;

// =============================
// container hooks

//ASMJIT_PATCH(0x711835, TechnoTypeClass_CTOR, 0x5)
//{
//	GET(TechnoTypeClass* , pItem, ESI);
//
//	TechnoTypeExtContainer::Instance.Allocate(pItem);
//	return 0;
//}

//ASMJIT_PATCH(0x711A67, TechnoTypeClass_CTOR_NoInt, 0x5)
//{
//	GET(TechnoTypeClass*, pItem, ESI);
//
//	TechnoTypeExtContainer::Instance.AllocateNoInit(pItem);
//	return 0;
//}


//ASMJIT_PATCH(0x711AE0, TechnoTypeClass_DTOR, 0x5)
//{
//	GET(TechnoTypeClass*, pItem, ECX);
//	TechnoTypeExtContainer::Instance.Remove(pItem);
//	return 0;
//}

//ASMJIT_PATCH(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
//{
//	GET(TechnoTypeClass*, pItem, EBP);
//	GET_STACK(CCINIClass*, pINI, 0x380);
//
//	if (R->Origin() == 0x716132) {
//		if(!pItem->Strength) {
//			pItem->Strength = 1;
//		}
//	}
//
//	TechnoTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x716132);
//
//	return 0;
//}ASMJIT_PATCH_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5) // this should make the techno unusable ? becase the game will return false when it
//

////hook before stuffs got pop-ed to remove crash possibility
//ASMJIT_PATCH(0x41CD74, AircraftTypeClass_LoadFromINI, 0x6)
//{
//	GET(AircraftTypeClass*, pItem, ESI);
//	GET(CCINIClass* const, pINI, EBX);
//
//	R->AL(pINI->ReadBool(pItem->ID, GameStrings::FlyBack(), R->CL()));
//
//	if (auto pExt = TechnoTypeExtContainer::Instance.Find(pItem))
//		pExt->LoadFromINIFile_Aircraft(pINI);
//
//	return 0x41CD82;
//}