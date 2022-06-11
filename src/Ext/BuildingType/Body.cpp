#include "Body.h"

#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;
void BuildingTypeExt::ExtData::InitializeConstants() {

	AIBuildInsteadPerDiff.reserve(3);
}

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	if (auto const pHouseExt = HouseExt::ExtMap.Find(pHouse))
	{
		if (pBuilding)
		{
			for (const auto& pair : pHouseExt->BuildingCounter)
			{
				const auto& pExt = pair.first;
				const auto& nCount = pair.second;
				if (pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
				{
					fFactor *= std::powf(pExt->PowerPlantEnhancer_Factor.Get(1.0f), static_cast<float>(nCount));
					nAmount += pExt->PowerPlantEnhancer_Amount.Get(0) * nCount;
				}
			}

			return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
		}
	}

	return 0;
}

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoClass* pWhat)
{
	double fFactor = 1.0;
	if (!pWhat || !pWhat->GetOwningHouse() || !pWhat->GetTechnoType() || pWhat->GetOwningHouse()->Defeated)
		return fFactor;

	if (auto pHouseExt = HouseExt::ExtMap.Find(pWhat->GetOwningHouse()))
	{
		if (!pHouseExt->Building_BuildSpeedBonusCounter.empty())
		{
			for (const auto pair : pHouseExt->Building_BuildSpeedBonusCounter)
			{
				if (const auto& pExt = pair.first)
				{
					if (!pExt->SpeedBonusTo.Contains(pWhat->GetTechnoType()))
						continue;

					const auto& nCount = pair.second;
					fFactor *= std::pow(pExt->SpeedBonus.Get(), nCount);
				}
			}
		}
	}

	return fFactor;
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBuilding)
						++result;
				}
			}
		}
	};

	if (pPowersUp[0])
	{
		if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			checkUpgrade(pTPowersUp);
	}

	if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
	{
		for (auto pTPowersUp : pBuildingExt->PowersUp_Buildings)
			checkUpgrade(pTPowersUp);
	}

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");
	this->PowerPlantEnhancer_Buildings.Read(exINI, pSection, "PowerPlantEnhancer.PowerPlants");
	this->PowerPlantEnhancer_Amount.Read(exINI, pSection, "PowerPlantEnhancer.Amount");
	this->PowerPlantEnhancer_Factor.Read(exINI, pSection, "PowerPlantEnhancer.Factor");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
		strcpy_s(pThis->PowersUpBuilding, this->PowersUp_Buildings[0]->ID);

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_Weapon.Read(exINI, pSection, "Grinding.Weapon", true);
	this->Grinding_DisplayRefund.Read(exINI, pSection, "Grinding.DisplayRefund");
	this->Grinding_DisplayRefund_Houses.Read(exINI, pSection, "Grinding.DisplayRefund.Houses");
	this->Grinding_DisplayRefund_Offset.Read(exINI, pSection, "Grinding.DisplayRefund.Offset");

	// Ares SuperWeapons tag
	pINI->ReadString(pSection, "SuperWeapons", "", Phobos::readBuffer);
	//char* super_weapons_list = Phobos::readBuffer;
	if (strlen(Phobos::readBuffer) > 0 && SuperWeaponTypeClass::Array->Count > 0)
	{
		//DynamicVectorClass<SuperWeaponTypeClass*> objectsList;
		char* context = nullptr;

		//pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			SuperWeaponTypeClass* buffer;
			if (Parser<SuperWeaponTypeClass*>::TryParse(cur, &buffer))
			{
				//Debug::Log("DEBUG: [%s]: Parsed SW [%s]\n", pSection, cur);
				this->SuperWeapons.AddItem(buffer);
			}
			else
			{
				Debug::Log("DEBUG: [%s]: Error parsing SuperWeapons= [%s]\n", pSection, cur);
			}
		}
	}

	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");

	this->PlacementPreview_Show.Read(exINI, pSection, "PlacementPreview.Show");

	if (pINI->GetString(pSection, "PlacementPreview.Shape", Phobos::readBuffer))
	{
		if (GeneralUtils::IsValidString(Phobos::readBuffer))
		{
			// we cannot load same SHP file twice it may produce artifact , prevent it !
			if (CRT::strcmpi(Phobos::readBuffer, pSection) || CRT::strcmpi(Phobos::readBuffer, pArtSection))
				this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
			else
				Debug::Log("Cannot Load PlacementPreview.Shape for [%s]Art[%s] ! \n", pSection, pArtSection);
		}
	}

	this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
	this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
	this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
	this->PlacementPreview_Palette.Read(pINI, pSection, "PlacementPreview.Palette");
	this->PlacementPreview_TranslucentLevel.Read(exINI, pSection, "PlacementPreview.Translucent");

#pragma region Otamaa
	//   this->OwnerObject()->StartFacing = 32 * ((std::clamp(pINI->ReadInteger(pSection, "StartFacing", 0), 0, 255)) << 5);

	auto GetGarrisonAnim = [&exINI, pSection](
PhobosMap<int, AnimTypeClass*>& nVec, const char* pBaseFlag, bool bAllocate = true, bool bParseDebug = false)
	{
		auto nHouseCount = HouseTypeClass::Array()->Count;
		char tempBuffer[2048];
		nVec.clear();

		for (int i = 0; i < nHouseCount; ++i)
		{
			Nullable<AnimTypeClass*> nBuffer;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d", pBaseFlag, i);

			if (bParseDebug)
				Debug::Log("GetGarrisonAnim for [%s]=%s idx[%d] \n", pSection, tempBuffer, i);

			nBuffer.Read(exINI, pSection, tempBuffer, bAllocate);

			if (!nBuffer.isset())
				continue;

			nVec[i] = nBuffer.Get(nullptr);
		}

		if (!nVec.empty())
		{
			//remove invalid items to keep memory clean !
			for (auto const& nData : nVec)
			{
				if (!nData.second)
					nVec.erase(nData.first);
			}
		}
	};

	GetGarrisonAnim(this->GarrisonAnim_idle, "GarrisonAnim.IdleForCountry");
	GetGarrisonAnim(this->GarrisonAnim_ActiveOne, "GarrisonAnim.ActiveOneForCountry");
	GetGarrisonAnim(this->GarrisonAnim_ActiveTwo, "GarrisonAnim.ActiveTwoForCountry");
	GetGarrisonAnim(this->GarrisonAnim_ActiveThree, "GarrisonAnim.ActiveThreeForCountry");
	GetGarrisonAnim(this->GarrisonAnim_ActiveFour, "GarrisonAnim.ActiveFourForCountry");

	this->AIBuildInsteadPerDiff.Read(exINI, pSection, "AIBuildInstead");

	this->PackupSound_PlayGlobal.Read(exINI, pSection, "PackupSoundPlayGlobal");

	this->DamageFireTypes.Read(exINI, pSection, "DamageFireTypes");

	this->RepairRate.Read(exINI, pSection, "RepairRate");
	this->RepairStep.Read(exINI, pSection, "RepairStep");

	this->DisableDamageSound.Read(exINI, pSection, "DisableDamagedSound");
	this->PlayerReturnFire.Read(exINI, pSection, "PlayerReturnFire");

	this->BuildingOccupyDamageMult.Read(exINI, pSection, "OccupyDamageMultiplier");
	this->BuildingOccupyROFMult.Read(exINI, pSection, "OccupyROFMultiplier");

	this->BuildingBunkerDamageMult.Read(exINI, pSection, "BunkerDamageMultiplier");
	this->BuildingBunkerROFMult.Read(exINI, pSection, "BunkerROFMultMultiplier");

	this->BunkerWallsUpSound.Read(exINI, pSection, "BunkerWallsUpSound");
	this->BunkerWallsDownSound.Read(exINI, pSection, "BunkerWallsDownSound");

	this->PipShapes01Palette.Read(exINI.GetINI(), pSection, "PipShapes.Palette");
	this->PipShapes01Remap.Read(exINI, pSection, "PipShapes.Remap");

	this->IsJuggernaut.Read(exINI, pSection, "IsJuggernaut");

	this->TurretAnim_LowPower.Read(exINI, pSection, "TurretAnim.LowPower");
	this->TurretAnim_DamagedLowPower.Read(exINI, pSection, "TurretAnim.DamagedLowPower");

	this->Power_DegradeWithHealth.Read(exINI, pSection, "Power.DegradeWithHealth");
	this->AutoSellTime.Read(exINI, pSection, "AutoSell.Time");
	this->BuildingPlacementGrid_Shape.Read(exINI, pSection, "BuildingPlacementGrid.Shape");
	this->SpeedBonus.Read(exINI, pSection, "ExternalFactorySpeedBonus");
	this->SpeedBonusTo.Read(exINI, pSection, "ExternalFactorySpeedBonus.Type");

#pragma endregion

	if (!pArtINI->GetSection(pArtSection))
		return;

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempMuzzleBuffer[32];
		this->OccupierMuzzleFlashes.Clear();
		this->OccupierMuzzleFlashes.Reserve(pThis->MaxNumberOccupants);

		for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
		{
			Nullable<Point2D> nMuzzleLocation;
			_snprintf_s(tempMuzzleBuffer, sizeof(tempMuzzleBuffer), "MuzzleFlash%d", i);
			nMuzzleLocation.Read(exArtINI, pArtSection, tempMuzzleBuffer);
			this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get(Point2D::Empty);
		}
	}

#pragma region Otamaa
	this->HealthOnfire.Read(exArtINI, pArtSection, "OnFire.Health");

#ifdef REPLACE_BUILDING_ONFIRE
	this->DamageFire_Offs.Clear();

	char tempFire_OffsBuffer[32];
	for (int i = 0;; ++i)
	{
		Nullable<Point2D> nFire_offs;
		_snprintf_s(tempFire_OffsBuffer, sizeof(tempFire_OffsBuffer), "DamageFireOffset%d", i);
		nFire_offs.Read(exArtINI, pArtSection, tempFire_OffsBuffer);

		if (!nFire_offs.isset() || nFire_offs.Get() == Point2D::Empty)
			break;

		this->DamageFire_Offs.AddItem(nFire_offs.Get());
	}
#endif
	this->BuildUp_UseNormalLIght.Read(exArtINI, pArtSection, "Buildup.UseNormalLight");
	this->RubblePalette.Read(exArtINI.GetINI(), pArtSection, "Rubble.Palette");
#pragma endregion
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		.Process(this->PowerPlantEnhancer_Buildings)
		.Process(this->PowerPlantEnhancer_Amount)
		.Process(this->PowerPlantEnhancer_Factor)
		.Process(this->SuperWeapons)
		.Process(this->OccupierMuzzleFlashes)
		.Process(this->Refinery_UseStorage)
		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_DisplayRefund)
		.Process(this->Grinding_DisplayRefund_Houses)
		.Process(this->Grinding_DisplayRefund_Offset)

		.Process(this->PlacementPreview_Remap)
		.Process(this->PlacementPreview_Palette)
		.Process(this->PlacementPreview_Offset)
		.Process(this->PlacementPreview_Show)
		.Process(this->PlacementPreview_Shape)
		.Process(this->PlacementPreview_ShapeFrame)
		.Process(this->PlacementPreview_TranslucentLevel)

		.Process(this->DamageFireTypes)
		.Process(this->OnFireTypes)
		.Process(this->OnFireIndex)
		.Process(this->HealthOnfire)
		.Process(this->DamageFire_Offs)

		.Process(this->RepairRate)
		.Process(this->RepairStep)

		.Process(this->PlayerReturnFire)

		.Process(this->PackupSound_PlayGlobal)
		.Process(this->DisableDamageSound)

		.Process(this->BuildingOccupyDamageMult)
		.Process(this->BuildingOccupyROFMult)

		.Process(this->BuildingBunkerDamageMult)
		.Process(this->BuildingBunkerROFMult)

		.Process(this->BunkerWallsUpSound)
		.Process(this->BunkerWallsDownSound)

		.Process(this->AIBuildInsteadPerDiff)

		.Process(this->GarrisonAnim_idle)
		.Process(this->GarrisonAnim_ActiveOne)
		.Process(this->GarrisonAnim_ActiveTwo)
		.Process(this->GarrisonAnim_ActiveThree)
		.Process(this->GarrisonAnim_ActiveFour)

		.Process(this->PipShapes01Remap)
		.Process(this->PipShapes01Palette)

		.Process(this->TurretAnim_LowPower)
		.Process(this->TurretAnim_DamagedLowPower)
		.Process(this->BuildUp_UseNormalLIght)
		.Process(this->Power_DegradeWithHealth)
		.Process(this->IsJuggernaut)
		.Process(this->BuildingPlacementGrid_Shape)
		.Process(this->SpeedBonus)
		.Process(this->SpeedBonusTo)
		.Process(this->RubblePalette)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{


	return Stm.Success();
}

void BuildingTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }
BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x465300, BuildingTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4652ED, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46536A, BuildingTypeClass_Save_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
