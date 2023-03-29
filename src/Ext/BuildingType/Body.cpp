#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;
const DirClass BuildingTypeExt::DefaultJuggerFacing = DirClass { 0x7FFF };

int BuildingTypeExt::BuildLimitRemaining(HouseClass const* pHouse, BuildingTypeClass const* pItem)
{
	const auto BuildLimit = pItem->BuildLimit;

	if (BuildLimit >= 0)
		return BuildLimit - BuildingTypeExt::GetUpgradesAmount(const_cast<BuildingTypeClass*>(pItem), const_cast<HouseClass*>(pHouse));
	else
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
}

int  BuildingTypeExt::CheckBuildLimit(HouseClass const* pHouse, BuildingTypeClass const* pItem, bool includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	const int BuildLimit = pItem->BuildLimit;
	const int Remaining = BuildingTypeExt::BuildLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem)) ? NotReached : ReachedPermanently;

	return Remaining > 0 ? NotReached : ReachedTemporarily;
}

Point2D* BuildingTypeExt::GetOccupyMuzzleFlash(BuildingClass* pThis, int nOccupyIdx)
{
	return &BuildingTypeExt::ExtMap.Find(pThis->Type)->OccupierMuzzleFlashes[nOccupyIdx];
}

void BuildingTypeExt::DisplayPlacementPreview()
{
	const auto pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding);

	if (!pBuilding)
		return;

	const auto pType = pBuilding->Type;
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
	const bool bShow = pTypeExt->PlacementPreview_Show.Get(RulesExt::Global()->Building_PlacementPreview.Get(Phobos::Config::EnableBuildingPlacementPreview));

	if (!bShow)
		return;

	const auto pCell = MapClass::Instance->TryGetCellAt(Unsorted::Display_ZoneCell() + Unsorted::Display_ZoneOffset());
	if (!pCell)
		return;

	if (!MapClass::Instance->IsWithinUsableArea(pCell->GetCoords()))
		return;

	SHPStruct* Selected = nullptr;
	int nDecidedFrame = 0;

	if (!pTypeExt->PlacementPreview_Shape.isset())
	{
		if (const auto pBuildup = pType->LoadBuildup())
		{
			nDecidedFrame = ((pBuildup->Frames / 2) - 1);
			Selected = pBuildup;
		}
		else
		{
			Selected = pType->GetImage();
		}
	}
	else
	{
		Selected = pTypeExt->PlacementPreview_Shape.Get(nullptr);
	}

	if (!Selected)
		return;

	const auto& [nOffsetX, nOffsetY, nOffsetZ] = pTypeExt->PlacementPreview_Offset.Get();
	const auto nHeight = pCell->GetFloorHeight({ 0,0 });
	Point2D nPoint { 0,0 };

	if (!TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, nHeight + nOffsetZ), &nPoint))
		return;

	const auto nFrame = std::clamp(pTypeExt->PlacementPreview_ShapeFrame.Get(nDecidedFrame), 0, static_cast<int>(Selected->Frames));
	nPoint.X += nOffsetX;
	nPoint.Y += nOffsetY;
	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->PlacementPreview_TranslucentLevel.Get(RulesExt::Global()->BuildingPlacementPreview_TranslucentLevel.Get()));
	auto nREct = DSurface::Temp()->Get_Rect_WithoutBottomBar();
	const auto pPalette = pTypeExt->PlacementPreview_Remap.Get() ? pBuilding->GetRemapColour() : pTypeExt->PlacementPreview_Palette.GetOrDefaultConvert(FileSystem::UNITx_PAL());

	DSurface::Temp()->DrawSHP(pPalette, Selected, nFrame, &nPoint, &nREct, nFlag, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
}

bool BuildingTypeExt::CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner)
{
	const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgradeType);
	if (EnumFunctions::CanTargetHouse(pUpgradeExt->PowersUp_Owner, pUpgradeOwner, pBuilding->Owner))
	{
		// PowersUpBuilding
		if (_stricmp(pBuilding->Type->ID, pUpgradeType->PowersUpBuilding) == 0)
			return true;

		// PowersUp.Buildings
		for (auto& pPowerUpBuilding : pUpgradeExt->PowersUp_Buildings)
		{
			if (_stricmp(pBuilding->Type->ID, pPowerUpBuilding->ID) == 0)
				return true;
		}
	}

	return false;
}

// Assuming SuperWeapon & SuperWeapon2 are used (for the moment)
int BuildingTypeExt::ExtData::GetSuperWeaponCount() const
{
	// The user should only use SuperWeapon and SuperWeapon2 if the attached sw count isn't bigger than 2
	return 2 + this->SuperWeapons.size();
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index, HouseClass* pHouse) const
{
	auto idxSW = this->GetSuperWeaponIndex(index);

	if (auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW))
	{
		if (!SWTypeExt::ExtMap.Find(pSuper->Type)->IsAvailable(pHouse))
		{
			return -1;
		}
	}

	return idxSW;
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index) const
{
	const auto pThis = this->Get();

	if (index < 2)
	{
		return !index ? pThis->SuperWeapon : pThis->SuperWeapon2;
	}
	else if (index - 2 < (int)this->SuperWeapons.size())
	{
		return this->SuperWeapons[index - 2];
	}

	return -1;
}

void BuildingTypeExt::ExtData::InitializeConstants()
{
	AIBuildInsteadPerDiff.reserve(3);
}

int BuildingTypeExt::GetBuildingAnimTypeIndex(BuildingClass* pThis, const BuildingAnimSlot& nSlot, const char* pDefault)
{
	//pthis check is just in  case
	if (pThis
		&& pThis->IsAlive
		&& (pThis->Occupants.Count > 0)
		)
	{
		const auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

		{
			const auto nIndex = HouseTypeClass::Array()->FindItemIndex(pThis->Occupants[0]->Owner->Type);
			if (nIndex != -1)
			{

				AnimTypeClass* pDecidedAnim = nullptr;

				switch (nSlot)
				{
				case BuildingAnimSlot::Active:
					if (!pBuildingExt->GarrisonAnim_ActiveOne.empty())
						pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveOne.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveTwo:
					if (!pBuildingExt->GarrisonAnim_ActiveTwo.empty())
						pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveTwo.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveThree:
					if (!pBuildingExt->GarrisonAnim_ActiveThree.empty())
						pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveThree.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveFour:
					if (!pBuildingExt->GarrisonAnim_ActiveFour.empty())
						pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveFour.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::Idle:
					if (!pBuildingExt->GarrisonAnim_idle.empty())
						pDecidedAnim = pBuildingExt->GarrisonAnim_idle.get_or_default(nIndex);
					break;
				}

				if (pDecidedAnim)
				{
					return pDecidedAnim->ArrayIndex;
				}
			}
		}
	}

	return AnimTypeClass::FindIndexById(pDefault);

}

bool __fastcall BuildingTypeExt::IsFactory(BuildingClass* pThis, void* _)
{
	if (!pThis || !pThis->Type)
		return false;

	return pThis->Type->Factory == AbstractType::AircraftType || pThis->IsFactory();
}

void __fastcall BuildingTypeExt::DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	const auto nFlag = Flags | EnumFunctions::GetTranslucentLevel(RulesExt::Global()->PlacementGrid_TranslucentLevel.Get());

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, nFlag, Remap, ZAdjust,
		ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);
	{
		if (pBuilding)
		{
			for (const auto& [pBldType, nCount] : pHouseExt->BuildingCounter)
			{
				auto pExt = BuildingTypeExt::ExtMap.Find(pBldType);
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

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 1.0;

	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExt::IsObserverPlayer(pOwner))
		return fFactor;

	auto const pType = pWhat->GetTechnoType();
	if (!pType)
		return fFactor;

	auto pHouseExt = HouseExt::ExtMap.Find(pOwner);
	if(pHouseExt->Building_BuildSpeedBonusCounter.empty())
		return fFactor;

	for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter) {

		if (auto const pExt = BuildingTypeExt::ExtMap.TryFind(pBldType)) {

			if (!pExt->SpeedBonus.AffectedType.empty()) {
				if (!pExt->SpeedBonus.AffectedType.Contains(pType)) {
						continue;
				}
			}

			auto nBonus = 0.000;
			switch ((((DWORD*)pWhat)[0]))
			{
			case AircraftTypeClass::vtable:
				nBonus = pExt->SpeedBonus.SpeedBonus_Aircraft;
				break;
			case BuildingTypeClass::vtable:
				nBonus = pExt->SpeedBonus.SpeedBonus_Building;
				break;
			case UnitTypeClass::vtable:
				nBonus = pExt->SpeedBonus.SpeedBonus_Unit;
				break;
			case InfantryTypeClass::vtable:
				nBonus = pExt->SpeedBonus.SpeedBonus_Infantry;
				break;
			default:
				continue;
				break;
			}

			if (nBonus == 0.000)
				continue;

			fFactor *= std::pow(nBonus, nCount);
			}
		}

	return fFactor;
}

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoTypeClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 1.0;
	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExt::IsObserverPlayer(pOwner))
		return fFactor;

		auto pHouseExt = HouseExt::ExtMap.Find(pOwner);
		if(pHouseExt->Building_BuildSpeedBonusCounter.empty())
			return fFactor;

		for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter) {

			if (auto const pExt = BuildingTypeExt::ExtMap.TryFind(pBldType)) {

				if (!pExt->SpeedBonus.AffectedType.empty()) {
					if (!pExt->SpeedBonus.AffectedType.Contains(pWhat)) {
							continue;
					}
				}

				auto nBonus = 0.000;
				switch ((((DWORD*)pWhat)[0]))
				{
				case AircraftTypeClass::vtable:
					nBonus = pExt->SpeedBonus.SpeedBonus_Aircraft;
					break;
				case BuildingTypeClass::vtable:
					nBonus = pExt->SpeedBonus.SpeedBonus_Building;
					break;
				case UnitTypeClass::vtable:
					nBonus = pExt->SpeedBonus.SpeedBonus_Unit;
					break;
				case InfantryTypeClass::vtable:
					nBonus = pExt->SpeedBonus.SpeedBonus_Infantry;
					break;
				default:
					continue;
					break;
				}

				if (nBonus == 0.000)
					continue;

				fFactor *= std::pow(nBonus, nCount);
			}
		}

	return fFactor;
}
double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoClass* pWhat)
{
	return BuildingTypeExt::GetExternalFactorySpeedBonus(pWhat, pWhat->GetOwningHouse());
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	if(!pHouse)
		return 0;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade && pUpgrade == pBuilding)
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


	for (auto pTPowersUp : BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings)
			checkUpgrade(pTPowersUp);

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
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

	this->AllowAirstrike.Read(exINI, pSection, "AllowAirstrike");

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_Weapon.Read(exINI, pSection, "Grinding.Weapon", true);
	this->Grinding_DisplayRefund.Read(exINI, pSection, "Grinding.DisplayRefund");
	this->Grinding_DisplayRefund_Houses.Read(exINI, pSection, "Grinding.DisplayRefund.Houses");
	this->Grinding_DisplayRefund_Offset.Read(exINI, pSection, "Grinding.DisplayRefund.Offset");
	this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");

	this->Refinery_DisplayDumpedMoneyAmount.Read(exINI, pSection, "Refinery.DisplayDumpedTiberiumCost");
	this->Refinery_DisplayRefund_Offset.Read(exINI, pSection, "Refinery.DisplayDumpedTiberiumCostOffset");

	// Ares SuperWeapons tag
	auto const& pArray =  SuperWeaponTypeClass::Array;
	if (pArray->IsAllocated && pArray->Count > 0)
		this->SuperWeapons.Read(exINI, pSection, GameStrings::SuperWeapons());

	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");

	this->PlacementPreview_Show.Read(exINI, pSection, "PlacementPreview.Show");

	if (pINI->GetString(pSection, "PlacementPreview.Shape", Phobos::readBuffer))
	{
		auto pValue = Phobos::readBuffer;
		if (GeneralUtils::IsValidString(pValue))
		{
			// we cannot load same SHP file twice it may produce artifact , prevent it !
			if (CRT::strcmpi(pValue, pSection) || CRT::strcmpi(pValue, pArtSection))
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
	//   this->Get()->StartFacing = 32 * ((std::clamp(pINI->ReadInteger(pSection, "StartFacing", 0), 0, 255)) << 5);

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

			nVec[i] = nBuffer.Get();
		}

		if (!nVec.empty())
		{
			//remove invalid items to keep memory clean !
			for (auto const& [nIdx, pAnimType] : nVec)
			{
				if (!pAnimType)
					nVec.erase(nIdx);
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

	this->RepairRate.Read(exINI, pSection, GameStrings::RepairRate());
	this->RepairStep.Read(exINI, pSection, GameStrings::RepairStep());

	this->DisableDamageSound.Read(exINI, pSection, "DisableDamagedSound");
	this->PlayerReturnFire.Read(exINI, pSection, "PlayerReturnFire");

	this->BuildingOccupyDamageMult.Read(exINI, pSection, GameStrings::OccupyDamageMultiplier());
	this->BuildingOccupyROFMult.Read(exINI, pSection, GameStrings::OccupyROFMultiplier());

	this->BuildingBunkerDamageMult.Read(exINI, pSection, GameStrings::BunkerDamageMultiplier());
	this->BuildingBunkerROFMult.Read(exINI, pSection, GameStrings::BunkerROFMultMultiplier());

	this->BunkerWallsUpSound.Read(exINI, pSection, GameStrings::BunkerWallsUpSound());
	this->BunkerWallsDownSound.Read(exINI, pSection, GameStrings::BunkerWallsDownSound());

	this->PipShapes01Palette.Read(exINI.GetINI(), pSection, "PipShapes.Palette");
	this->PipShapes01Remap.Read(exINI, pSection, "PipShapes.Remap");

	this->IsJuggernaut.Read(exINI, pSection, "IsJuggernaut");

	this->TurretAnim_LowPower.Read(exINI, pSection, "TurretAnim.LowPower");
	this->TurretAnim_DamagedLowPower.Read(exINI, pSection, "TurretAnim.DamagedLowPower");

	this->Power_DegradeWithHealth.Read(exINI, pSection, "Power.DegradeWithHealth");
	this->AutoSellTime.Read(exINI, pSection, "AutoSell.Time");
	this->BuildingPlacementGrid_Shape.Read(exINI, pSection, "BuildingPlacementGrid.Shape");
	this->SpeedBonus.Read(exINI, pSection);
	this->RadialIndicator_Visibility.Read(exINI, pSection, "RadialIndicatorVisibility");

	this->EnterBioReactorSound.Read(exINI, pSection, GameStrings::EnterBioReactorSound());
	this->LeaveBioReactorSound.Read(exINI, pSection, GameStrings::LeaveBioReactorSound());

	this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
	
	this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
	if (this->SpyEffect_VictimSuperWeapon.isset())
		this->SpyEffect_VictimSW_RealLaunch.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon.RealLaunch");

	this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");
	if (this->SpyEffect_InfiltratorSuperWeapon.isset())
		this->SpyEffect_InfiltratorSW_JustGrant.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon.JustGrant");

	this->CanC4_AllowZeroDamage.Read(exINI, pSection, "CanC4.AllowZeroDamage");

	// no code attached
	this->RubbleDestroyed.Read(exINI, pSection, "Rubble.Destroyed");
	this->RubbleIntact.Read(exINI, pSection, "Rubble.Intact");
	this->RubbleDestroyedAnim.Read(exINI, pSection, "Rubble.Destroyed.Anim");
	this->RubbleIntactAnim.Read(exINI, pSection, "Rubble.Intact.Anim");
	this->RubbleDestroyedOwner.Read(exINI, pSection, "Rubble.Destroyed.Owner");
	this->RubbleIntactOwner.Read(exINI, pSection, "Rubble.Intact.Owner");
	this->RubbleDestroyedStrength.Read(exINI, pSection, "Rubble.Destroyed.Strength");
	this->RubbleIntactStrength.Read(exINI, pSection, "Rubble.Intact.Strength");
	this->RubbleDestroyedRemove.Read(exINI, pSection, "Rubble.Destroyed.Remove");
	this->RubbleIntactRemove.Read(exINI, pSection, "Rubble.Intact.Remove");
	//
#pragma endregion

	if (!pArtINI->GetSection(pArtSection))
		return;

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempMuzzleBuffer[32];
		this->OccupierMuzzleFlashes.clear();
		this->OccupierMuzzleFlashes.resize(pThis->MaxNumberOccupants);

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


#ifndef REPLACE_BUILDING_ONFIRE
	this->DamageFire_Offs.clear();
	char tempFire_OffsBuffer[32];
	for (int i = 0;; ++i)
	{
		Nullable<Point2D> nFire_offs{};
		_snprintf_s(tempFire_OffsBuffer, sizeof(tempFire_OffsBuffer), "DamageFireOffset%d", i);
		nFire_offs.Read(exArtINI, pArtSection, tempFire_OffsBuffer);

		if (!nFire_offs.isset() || nFire_offs.Get() == Point2D::Empty)
			break;

		this->DamageFire_Offs.emplace_back(nFire_offs.Get());
	}
#endif
	this->BuildUp_UseNormalLIght.Read(exArtINI, pArtSection, "Buildup.UseNormalLight");
	this->RubblePalette.Read(exArtINI.GetINI(), pArtSection, "Rubble.Palette");
#pragma endregion
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	//auto const pThis = this->Get();
	//UNREFERENCED_PARAMETER(pThis);
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
		.Process(this->AllowAirstrike)

		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_DisplayRefund)
		.Process(this->Grinding_DisplayRefund_Houses)
		.Process(this->Grinding_DisplayRefund_Offset)
		.Process(this->Grinding_PlayDieSound)


		.Process(this->Refinery_DisplayDumpedMoneyAmount)
		.Process(this->Refinery_DisplayRefund_Offset)

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

		.Process(this->RubbleIntact)
		.Process(this->RubbleDestroyed)
		.Process(this->RubbleDestroyedAnim)
		.Process(this->RubbleIntactAnim)
		.Process(this->RubbleDestroyedOwner)
		.Process(this->RubbleIntactOwner)
		.Process(this->RubbleDestroyedStrength)
		.Process(this->RubbleIntactStrength)
		.Process(this->RubbleDestroyedRemove)
		.Process(this->RubbleIntactRemove)
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
		.Process(this->RadialIndicator_Visibility)
		.Process(this->SpyEffect_Custom)
		.Process(this->SpyEffect_VictimSuperWeapon)
		.Process(this->SpyEffect_InfiltratorSuperWeapon)
		.Process(this->SpyEffect_InfiltratorSW_JustGrant)
		.Process(this->CanC4_AllowZeroDamage)
		.Process(this->SpyEffect_VictimSW_RealLaunch)
		.Process(this->RubblePalette)
		.Process(this->EnterBioReactorSound)
		.Process(this->LeaveBioReactorSound)
		.Process(this->DockPoseDir)
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


// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }
BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);
	BuildingTypeExt::ExtMap.Allocate(pItem);
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