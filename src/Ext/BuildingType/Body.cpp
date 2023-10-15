#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Misc/AresData.h>

std::vector<std::string> BuildingTypeExtData::trenchKinds;
const DirStruct  BuildingTypeExtData::DefaultJuggerFacing = DirStruct { 0x7FFF };
const CellStruct BuildingTypeExtData::FoundationEndMarker = { 0x7FFF, 0x7FFF };

bool  BuildingTypeExtData::IsAcademy() const
{
	if (this->Academy.empty())
	{
		this->Academy = this->AcademyInfantry > 0.0
			|| this->AcademyAircraft > 0.0
			|| this->AcademyVehicle > 0.0
			|| this->AcademyBuilding > 0.0;
	}

	return this->Academy;
}

void BuildingTypeExtData::UpdateFoundationRadarShape()
{
	this->FoundationRadarShape.Clear();

	if (this->IsCustom)
	{
		auto pType = this->AttachedToObject;
		auto pRadar = RadarClass::Global();

		int width = pType->GetFoundationWidth();
		int height = pType->GetFoundationHeight(false);

		// transform between cell length and pixels on radar
		auto Transform = [](int length, double factor) -> int
			{
				double dblLength = length * factor + 0.5;
				double minLength = (length == 1) ? 1.0 : 2.0;

				if (dblLength < minLength)
				{
					dblLength = minLength;
				}

				return Game::F2I(dblLength);
			};

		// the transformed lengths
		int pixelsX = Transform(width, pRadar->RadarSizeFactor);
		int pixelsY = Transform(height, pRadar->RadarSizeFactor);

		// heigth of the foundation tilted by 45�
		int rows = pixelsX + pixelsY - 1;

		// this draws a rectangle standing on an edge, getting
		// wider for each line drawn. the start and end values
		// are special-cased to not draw the pixels outside the
		// foundation.
		for (int i = 0; i < rows; ++i)
		{
			int start = -i;
			if (i >= pixelsY)
			{
				start = i - 2 * pixelsY + 2;
			}

			int end = i;
			if (i >= pixelsX)
			{
				end = 2 * pixelsX - i - 2;
			}

			// fill the line
			for (int j = start; j <= end; ++j)
			{
				this->FoundationRadarShape.EmpalaceItem(j, i);
			}
		}
	}
}

void BuildingTypeExtData::UpdateBuildupFrames(BuildingTypeClass* pThis)
{
	auto pExt = BuildingTypeExtContainer::Instance.Find(pThis);

	if (const auto pShp = pThis->Buildup)
	{
		const auto frames = pThis->Gate ?
			pThis->GateStages + 1 : pShp->Frames / 2;

		const auto duration_build = pExt->BuildupTime.Get(RulesClass::Instance->BuildupTime);
		const auto duration_sell = pExt->SellTime.Get(duration_build);

		pExt->SellFrames = frames > 0 ? (int)(duration_sell / (double)frames * 900.0) : 1;
		pThis->BuildingAnimFrame[0].dwUnknown = 0;
		pThis->BuildingAnimFrame[0].FrameCount = frames;
		pThis->BuildingAnimFrame[0].FrameDuration = frames > 0 ? (int)(duration_build / (double)frames * 900.0) : 1;
	}
}

void BuildingTypeExtData::CompleteInitialization()
{
	auto const pThis = this->AttachedToObject;

	// enforce same foundations for rubble/intact building pairs
	if (this->RubbleDestroyed &&
		!BuildingTypeExtData::IsFoundationEqual(pThis, this->RubbleDestroyed))
	{
		Debug::FatalErrorAndExit(
			"BuildingType %s and its %s %s don't have the same foundation.",
			pThis->ID, "Rubble.Destroyed", this->RubbleDestroyed->ID);
	}

	if (this->RubbleIntact &&
		!BuildingTypeExtData::IsFoundationEqual(pThis, this->RubbleIntact))
	{
		Debug::FatalErrorAndExit(
			"BuildingType %s and its %s %s don't have the same foundation.",
			pThis->ID, "Rubble.Intact", this->RubbleIntact->ID);
	}

	BuildingTypeExtData::UpdateBuildupFrames(pThis);
}

bool BuildingTypeExtData::IsFoundationEqual(BuildingTypeClass* pType1, BuildingTypeClass* pType2)
{
	// both types must be set and must have same foundation id
	if (!pType1 || !pType2 || pType1->Foundation != pType2->Foundation) {
		return false;
	}

	// non-custom foundations need no special handling
	if (pType1->Foundation != BuildingTypeExtData::CustomFoundation)
	{
		return true;
	}

	// custom foundation
	auto const pExt1 = BuildingTypeExtContainer::Instance.Find(pType1);
	auto const pExt2 = BuildingTypeExtContainer::Instance.Find(pType2);
	const auto& data1 = pExt1->CustomData;
	const auto& data2 = pExt2->CustomData;

	// this works for any two foundations. it's linear with sorted ones
	return pExt1->CustomWidth == pExt2->CustomWidth
		&& pExt1->CustomHeight == pExt2->CustomHeight
		&& std::is_permutation(
			data1.begin(), data1.end(), data2.begin(), data2.end());
}

void BuildingTypeExtData::Initialize()
{
	this->AIBuildInsteadPerDiff.reserve(3);
	this->PowersUp_Buildings.reserve(3);
	this->PowerPlantEnhancer_Buildings.reserve(8);
	this->Grinding_AllowTypes.reserve(8);
	this->Grinding_DisallowTypes.reserve(8);
	this->DamageFireTypes.reserve(8);
	this->OnFireTypes.reserve(8);
	this->OnFireIndex.reserve(8);
	this->DamageFire_Offs.reserve(8);
	this->GarrisonAnim_idle.reserve(HouseTypeClass::Array->Count);
	this->GarrisonAnim_ActiveOne.reserve(HouseTypeClass::Array->Count);
	this->GarrisonAnim_ActiveTwo.reserve(HouseTypeClass::Array->Count);
	this->GarrisonAnim_ActiveThree.reserve(HouseTypeClass::Array->Count);
	this->GarrisonAnim_ActiveFour.reserve(HouseTypeClass::Array->Count);
	this->Type = TechnoTypeExtContainer::Instance.Find(this->AttachedToObject);
	this->OccupierMuzzleFlashes.reserve(((BuildingTypeClass*)this->Type->AttachedToObject)->MaxNumberOccupants);
	this->DockPoseDir.reserve(((BuildingTypeClass*)this->Type->AttachedToObject)->NumberOfDocks);
	this->LostEvaEvent = VoxClass::FindIndexById(GameStrings::EVA_TechBuildingLost());
	this->PrismForwarding.Initialize(this->AttachedToObject);
}

bool BuildingTypeExtData::CanBeOccupiedBy(InfantryClass* whom)
{
	// if CanBeOccupiedBy isn't empty, we have to check if this soldier is allowed in
	return this->AllowedOccupiers.empty() || this->AllowedOccupiers.Contains(whom->Type);
}

int BuildingTypeExtData::BuildLimitRemaining(HouseClass* pHouse, BuildingTypeClass* pItem)
{
	const auto BuildLimit = pItem->BuildLimit;

	if (BuildLimit >= 0)
		return BuildLimit - BuildingTypeExtData::GetUpgradesAmount(pItem, pHouse);
	else
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
}

int BuildingTypeExtData::CheckBuildLimit(HouseClass* pHouse, BuildingTypeClass* pItem, bool includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	const int BuildLimit = pItem->BuildLimit;
	const int Remaining = BuildingTypeExtData::BuildLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && pHouse->GetFactoryProducing(pItem)) ? NotReached : ReachedPermanently;

	return Remaining > 0 ? NotReached : ReachedTemporarily;
}

Point2D* BuildingTypeExtData::GetOccupyMuzzleFlash(BuildingClass* pThis, int nOccupyIdx)
{
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)
		->OccupierMuzzleFlashes.data() + nOccupyIdx;
}

void BuildingTypeExtData::DisplayPlacementPreview()
{
	const auto pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding);

	if (!pBuilding)
		return;

	const auto pType = pBuilding->Type;
	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);
	const bool bShow = pTypeExt->PlacementPreview_Show.Get(RulesExtData::Instance()->Building_PlacementPreview.Get(Phobos::Config::EnableBuildingPlacementPreview));

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
	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->PlacementPreview_TranslucentLevel.Get(RulesExtData::Instance()->BuildingPlacementPreview_TranslucentLevel));
	auto nREct = DSurface::Temp()->Get_Rect_WithoutBottomBar();

	ConvertClass* pDecidedPal = FileSystem::UNITx_PAL();

	if (!pTypeExt->PlacementPreview_Remap.Get()) {
		if (const auto pCustom = pTypeExt->PlacementPreview_Palette) {
			pDecidedPal = pCustom->GetConvert<PaletteManager::Mode::Temperate>();
		}

	} else {
		if (pTypeExt->PlacementPreview_Palette && pTypeExt->PlacementPreview_Palette->ColorschemeDataVector) {
			pDecidedPal = pTypeExt->PlacementPreview_Palette->ColorschemeDataVector->GetItem(pBuilding->Owner->ColorSchemeIndex)->LightConvert;
		} else {
			pDecidedPal = pBuilding->GetRemapColour();
		}
	}

	DSurface::Temp()->DrawSHP(pDecidedPal, Selected, nFrame, &nPoint, &nREct, nFlag, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
}

bool BuildingTypeExtData::CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner)
{
	const auto pUpgradeExt = BuildingTypeExtContainer::Instance.Find(pUpgradeType);
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
int BuildingTypeExtData::GetSuperWeaponCount() const
{
	// The user should only use SuperWeapon and SuperWeapon2 if the attached sw count isn't bigger than 2
	return 2 + this->SuperWeapons.size();
}

int BuildingTypeExtData::GetSuperWeaponIndex(const int index, HouseClass* pHouse) const
{
	auto idxSW = this->GetSuperWeaponIndex(index);

	if (auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW))
	{
		if (!SWTypeExtContainer::Instance.Find(pSuper->Type)->IsAvailable(pHouse))
		{
			return -1;
		}
	}

	return idxSW;
}

int BuildingTypeExtData::GetSuperWeaponIndex(const int index) const
{
	const auto pThis = this->AttachedToObject;

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

int BuildingTypeExtData::GetBuildingAnimTypeIndex(BuildingClass* pThis, const BuildingAnimSlot& nSlot, const char* pDefault)
{
	//pthis check is just in  case
	if (pThis
		&& pThis->IsAlive
		&& (pThis->Occupants.Count > 0)
		)
	{
		const auto pBuildingExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

		{
			const auto nIndex = HouseTypeClass::Array()->FindItemIndex(pThis->Occupants[0]->Owner->Type);
			if (nIndex != -1)
			{

				AnimTypeClass* pDecidedAnim = nullptr;

				switch (nSlot)
				{
				case BuildingAnimSlot::Active:
					pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveOne.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveTwo:
					pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveTwo.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveThree:
					pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveThree.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::ActiveFour:
					pDecidedAnim = pBuildingExt->GarrisonAnim_ActiveFour.get_or_default(nIndex);
					break;
				case BuildingAnimSlot::Idle:
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

bool __fastcall BuildingTypeExtData::IsFactory(BuildingClass* pThis, void* _)
{
	if (!pThis || !pThis->Type)
		return false;

	return pThis->Type->Factory == AbstractType::AircraftType || pThis->IsFactory();
}

void __fastcall BuildingTypeExtData::DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	const auto nFlag = Flags | EnumFunctions::GetTranslucentLevel(RulesExtData::Instance()->PlacementGrid_TranslucentLevel.Get());

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, nFlag, Remap, ZAdjust,
		ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

bool BuildingTypeExtData::IsLinkable(BuildingTypeClass* pThis)
{
	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis);

	return pExt->Firestorm_Wall || pExt->IsTrench >= 0;
}

int BuildingTypeExtData::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	for (const auto& [pBldType, nCount] : pHouseExt->PowerPlantEnhancerBuildings)
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		if (pExt->PowerPlantEnhancer_Buildings.empty() || !pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
			continue;

		fFactor *= std::powf(pExt->PowerPlantEnhancer_Factor.Get(1.0f), static_cast<float>(nCount));
		nAmount += pExt->PowerPlantEnhancer_Amount.Get(0) * nCount;
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

float BuildingTypeExtData::GetPurifierBonusses(HouseClass* pHouse)
{
	/*removing the counter reference
	 Unit unload , 73E437 done
	 inf storage Ai , 522DD9 done
	 limbo 1 , 44591F done
	 Grand open , 44637C done
	 Captured1 , 448AC2 done
	 Captured2 , 4491EB done*/

	float fFactor = 0.00f;

	if (!pHouse || pHouse->Defeated || pHouse->IsNeutral() || HouseExtData::IsObserverPlayer(pHouse))
		return 0.00f;

	auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (pHouseExt->Building_OrePurifiersCounter.empty())
		return 0.00f;

	// AI VirtualPurifiers only applicable outside campaign
	// the bonus is using default rules value
	const bool Eligible = SessionClass::Instance->GameMode != GameMode::Campaign;
	const int bonusCount = !Eligible ? 0 : RulesClass::Instance->AIVirtualPurifiers[pHouse->GetAIDifficultyIndex()];
	//virtual purifier using rules value
	const float bonusAI = RulesClass::Instance->PurifierBonus * bonusCount;

	for (const auto& [pBldType, nCount] : pHouseExt->Building_OrePurifiersCounter)
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		const auto bonusses = pExt->PurifierBonus.Get(RulesClass::Instance->PurifierBonus);

		if (bonusses > 0.00f) {
			fFactor += (bonusses * nCount);
		}
	}

	return (fFactor > 0.00f) ? fFactor + bonusAI : 0.00f ;
}

double BuildingTypeExtData::GetExternalFactorySpeedBonus(TechnoClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 0.0;

	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExtData::IsObserverPlayer(pOwner))
		return fFactor;

	const auto pType = pWhat->GetTechnoType();
	if (!pType)
		return fFactor;

	auto pHouseExt = HouseExtContainer::Instance.Find(pOwner);
	if (pHouseExt->Building_BuildSpeedBonusCounter.empty())
		return fFactor;

	for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter)
	{
		auto const pExt = BuildingTypeExtContainer::Instance.Find(pBldType);
		{
			if (!pExt->SpeedBonus.AffectedType.empty() && !pExt->SpeedBonus.AffectedType.Contains(pType))
				continue;

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

double BuildingTypeExtData::GetExternalFactorySpeedBonus(TechnoTypeClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 1.0;
	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExtData::IsObserverPlayer(pOwner))
		return fFactor;

	auto pHouseExt = HouseExtContainer::Instance.Find(pOwner);
	if (pHouseExt->Building_BuildSpeedBonusCounter.empty())
		return fFactor;

	const auto what = pWhat->WhatAmI();
	for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter)
	{
		//if (pBldType->PowerDrain && pOwner->HasLowPower())
		//	continue;

		if (auto const pExt = BuildingTypeExtContainer::Instance.TryFind(pBldType))
		{
			if (!pExt->SpeedBonus.AffectedType.empty() && !pExt->SpeedBonus.AffectedType.Contains(pWhat))
				continue;

			auto nBonus = 0.000;
			switch (what)
			{
			case AircraftTypeClass::AbsID:
				nBonus = pExt->SpeedBonus.SpeedBonus_Aircraft;
				break;
			case BuildingTypeClass::AbsID:
				nBonus = pExt->SpeedBonus.SpeedBonus_Building;
				break;
			case UnitTypeClass::AbsID:
				nBonus = pExt->SpeedBonus.SpeedBonus_Unit;
				break;
			case InfantryTypeClass::AbsID:
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

double BuildingTypeExtData::GetExternalFactorySpeedBonus(TechnoClass* pWhat)
{
	return BuildingTypeExtData::GetExternalFactorySpeedBonus(pWhat, pWhat->GetOwningHouse());
}

int BuildingTypeExtData::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	if (!pHouse)
		return 0;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings) {
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


	for (auto pTPowersUp : BuildingTypeExtContainer::Instance.Find(pBuilding)->PowersUp_Buildings)
		checkUpgrade(pTPowersUp);

	return isUpgrade ? result : -1;
}

void BuildingTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (pINI->ReadString(pSection, "IsTrench", "", Phobos::readBuffer) > 0)
	{
		/*  Find the name in the list of kinds; if the list is empty, distance is 0, if the item isn't in
			the list, the index is the current list's size(); if the returned iterator is beyond the list,
			add the name to the list, which makes the previously calculated index (th distance) valid.
			(changed by AlexB 2014-01-16)

			I originally thought of using a map here, but I figured the probability that the kinds list
			grows so long that the search through all kinds takes up significant time is very low, and
			vectors are far simpler to use in this situation.
		*/
		const auto it = std::find_if(trenchKinds.begin(), trenchKinds.end(), [](auto const& pItem)
						{ return pItem == Phobos::readBuffer; });
		this->IsTrench = std::distance(trenchKinds.begin(), it);
		if (it == trenchKinds.end())
		{
			trenchKinds.push_back(Phobos::readBuffer);
		}
	}

	if (pThis->UnitRepair && pThis->Factory == AbstractType::AircraftType) {
		Debug::FatalErrorAndExit(
			"BuildingType [%s] has both UnitRepair=yes and Factory=AircraftType.\n"
			"This combination causes Internal Errors and other unwanted behaviour.", pSection);
	}

	if (!parseFailAddr)
	{
		INI_EX exINI(pINI);

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
		this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");

		this->DisplayIncome.Read(exINI, pSection, "DisplayIncome");
		this->DisplayIncome_Houses.Read(exINI, pSection, "DisplayIncome.Houses");
		this->DisplayIncome_Offset.Read(exINI, pSection, "DisplayIncome.Offset");

		// Ares SuperWeapons tag
		auto const& pArray = SuperWeaponTypeClass::Array;
		if (pArray->IsAllocated && pArray->Count > 0)
			this->SuperWeapons.Read(exINI, pSection, GameStrings::SuperWeapons());

		this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");
		this->PlacementPreview_Show.Read(exINI, pSection, "PlacementPreview.Show");

		if (pINI->GetString(pSection, "PlacementPreview.Shape", Phobos::readBuffer))
		{
			if (GeneralUtils::IsValidString(Phobos::readBuffer))
			{
				// we cannot load same SHP file twice it may produce artifact , prevent it !
				if (IMPL_STRCMPI(Phobos::readBuffer, pSection) || IMPL_STRCMPI(Phobos::readBuffer, pArtSection))
					this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
				else
					Debug::Log("Cannot Load PlacementPreview.Shape for [%s]Art[%s] ! \n", pSection, pArtSection);
			}
		}

		this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
		this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
		this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
		this->PlacementPreview_Palette.Read(exINI, pSection, "PlacementPreview.Palette");
		this->PlacementPreview_TranslucentLevel.Read(exINI, pSection, "PlacementPreview.Translucent");

#pragma region Otamaa
		//   this->Get()->StartFacing = 32 * ((std::clamp(pINI->ReadInteger(pSection, "StartFacing", 0), 0, 255)) << 5);

		auto GetGarrisonAnim = [&exINI, pSection](
			PhobosMap<int, AnimTypeClass*>& nVec, const char* pBaseFlag, bool bAllocate = true, bool bParseDebug = false)
		{
			char tempBuffer[0x55];
			for (int i = 0; i < HouseTypeClass::Array()->Count; ++i)
			{
				Nullable<AnimTypeClass*> nBuffer;
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "%s%d", pBaseFlag, i);

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

		this->DamageFireTypes.Read(exINI, pSection, GameStrings::DamageFireTypes());

		this->RepairRate.Read(exINI, pSection, GameStrings::RepairRate());
		this->RepairStep.Read(exINI, pSection, GameStrings::RepairStep());

		this->DisableDamageSound.Read(exINI, pSection, "DisableFallbackDamagedSound");
		this->PlayerReturnFire.Read(exINI, pSection, "PlayerReturnFire");

		this->BuildingOccupyDamageMult.Read(exINI, pSection, GameStrings::OccupyDamageMultiplier());
		this->BuildingOccupyROFMult.Read(exINI, pSection, GameStrings::OccupyROFMultiplier());

		this->BuildingBunkerDamageMult.Read(exINI, pSection, GameStrings::BunkerDamageMultiplier());
		this->BuildingBunkerROFMult.Read(exINI, pSection, GameStrings::BunkerROFMultMultiplier());

		this->BunkerWallsUpSound.Read(exINI, pSection, GameStrings::BunkerWallsUpSound());
		this->BunkerWallsDownSound.Read(exINI, pSection, GameStrings::BunkerWallsDownSound());

		this->PipShapes01Palette.Read(exINI, pSection, "PipShapes.Palette");
		this->PipShapes01Remap.Read(exINI, pSection, "PipShapes.Remap");

		this->IsJuggernaut.Read(exINI, pSection, "IsJuggernaut");

		this->TurretAnim_LowPower.Read(exINI, pSection, "TurretAnim.LowPower");
		this->TurretAnim_DamagedLowPower.Read(exINI, pSection, "TurretAnim.DamagedLowPower");

		this->Power_DegradeWithHealth.Read(exINI, pSection, "Power.DegradeWithHealth");
		this->AutoSellTime.Read(exINI, pSection, "AutoSell.Time");
		this->BuildingPlacementGrid_Shape.Read(exINI, pSection, "BuildingPlacementGrid.Shape");
		this->SpeedBonus.Read(exINI, pSection);
		this->RadialIndicator_Visibility.Read(exINI, pSection, "RadialIndicatorVisibility");

		this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
		this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
		if (this->SpyEffect_VictimSuperWeapon.isset())
			this->SpyEffect_VictimSW_RealLaunch.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon.RealLaunch");

		this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");
		if (this->SpyEffect_InfiltratorSuperWeapon.isset())
			this->SpyEffect_InfiltratorSW_JustGrant.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon.JustGrant");

		this->SpyEffect_RevealProduction.Read(exINI, pSection, "SpyEffect.RevealProduction");
		this->SpyEffect_ResetSW.Read(exINI, pSection, "SpyEffect.ResetSuperweapons");
		this->SpyEffect_ResetRadar.Read(exINI, pSection, "SpyEffect.ResetRadar");
		this->SpyEffect_RevealRadar.Read(exINI, pSection, "SpyEffect.RevealRadar");
		this->SpyEffect_RevealRadarPersist.Read(exINI, pSection, "SpyEffect.KeepRadar");
		this->SpyEffect_GainVeterancy.Read(exINI, pSection, "SpyEffect.UnitVeterancy");
		std::vector<int> SpyEffect_StolenTechIndex {};
		detail::parse_values(SpyEffect_StolenTechIndex, exINI, pSection, "SpyEffect.StolenTechIndex");

		this->SpyEffect_PowerOutageDuration.Read(exINI, pSection, "SpyEffect.PowerOutageDuration");
		this->SpyEffect_StolenMoneyAmount.Read(exINI, pSection, "SpyEffect.StolenMoneyAmount");
		this->SpyEffect_StolenMoneyPercentage.Read(exINI, pSection, "SpyEffect.StolenMoneyPercentage");
		this->SpyEffect_SabotageDelay.Read(exINI, pSection, "SpyEffect.SabotageDelay");
		this->SpyEffect_SuperWeapon.Read(exINI, pSection, "SpyEffect.SuperWeapon");
		this->SpyEffect_SuperWeaponPermanent.Read(exINI, pSection, "SpyEffect.SuperWeaponPermanent");
		this->SpyEffect_UnReverseEngineer.Read(exINI, pSection, "SpyEffect.UndoReverseEngineer");

		this->SpyEffect_InfantryVeterancy.Read(exINI, pSection, "SpyEffect.InfantryVeterancy"); {}
		this->SpyEffect_VehicleVeterancy.Read(exINI, pSection, "SpyEffect.VehicleVeterancy");
		this->SpyEffect_NavalVeterancy.Read(exINI, pSection, "SpyEffect.NavalVeterancy");
		this->SpyEffect_AircraftVeterancy.Read(exINI, pSection, "SpyEffect.AircraftVeterancy");
		this->SpyEffect_BuildingVeterancy.Read(exINI, pSection, "SpyEffect.BuildingVeterancy");

		auto pos = SpyEffect_StolenTechIndex.begin();
		const auto end = SpyEffect_StolenTechIndex.end();

		if(pos != end) {
			this->SpyEffect_StolenTechIndex_result.reset();
			do{
				if ((*pos) > -1 && (*pos) < 32)
				{
					this->SpyEffect_StolenTechIndex_result.set((*pos));
				}
				else if ((*pos) != -1)
				{
					Debug::Log("BuildingType %s has a SpyEffect.StolenTechIndex of %d. The value has to be less than 32.\n", pSection, (*pos));
				}

			}while(++pos != end);
		}

		this->CanC4_AllowZeroDamage.Read(exINI, pSection, "CanC4.AllowZeroDamage");
		this->C4_Modifier.Read(exINI, pSection, "C4Modifier");

		// relocated the solid tag from artmd to rulesmd
		this->Solid_Height.Read(exINI, pSection, "SolidHeight");
		this->Solid_Level.Read(exINI, pSection, "SolidLevel");
		this->AIBaseNormal.Read(exINI, pSection, "AIBaseNormal");
		this->AIInnerBase.Read(exINI, pSection, "AIInnerBase");
		this->EngineerRepairable.Read(exINI, pSection, "EngineerRepairable");

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

		this->TunnelType.Read(exINI, pSection, "Tunnel");

		this->SellBuildupLength.Read(exINI, pSection, "SellBuildupLength");

		// added on 11.11.09 for #221 and children (Trenches)
		this->UCPassThrough.Read(exINI, pSection, "UC.PassThrough");
		this->UCFatalRate.Read(exINI, pSection, "UC.FatalRate");
		this->UCDamageMultiplier.Read(exINI, pSection, "UC.DamageMultiplier");

		this->Cursor_Spy.Read(exINI, pSection, "Cursor.Spy");
		this->ImmuneToSaboteurs.Read(exINI, pSection, "ImmuneToSaboteurs");
		this->ReverseEngineersVictims.Read(exINI, pSection, "ReverseEngineersVictims");
		this->Cursor_Sabotage.Read(exINI, pSection, "Cursor.Sabotage");

		this->GateDownSound.Read(exINI, pSection, "GateDownSound");
		this->GateUpSound.Read(exINI, pSection, "GateUpSound");
		this->UnitSell.Read(exINI, pSection, "UnitSell");

		this->LightningRod_Modifier.Read(exINI, pSection, "LightningRod.Modifier");
		this->Returnable.Read(exINI, pSection, "Returnable");
		this->BuildupTime.Read(exINI, pSection, "BuildupTime");
		this->SellTime.Read(exINI, pSection, "SellTime");
		this->SlamSound.Read(exINI, pSection, "SlamSound");
		this->Destroyed_CreateSmudge.Read(exINI, pSection, "Destroyed.CreateSmudge");

		//TODO , the hook is disabled
		// need better implementation
		this->LaserFenceType.Read(exINI, pSection, "LaserFence.Type");
		this->LaserFenceWEType.Read(exINI, pSection, "LaserFence.WEType");
		this->LaserFencePostLinks.Read(exINI, pSection, "LaserFence.PostLinks");
		this->LaserFenceDirection.Read(exINI, pSection, "LaserFence.Direction");

		// #218 Specific Occupiers
		this->AllowedOccupiers.Read(exINI, pSection, "CanBeOccupiedBy");
		if (!this->AllowedOccupiers.empty()) {
			// having a specific occupier list implies that this building is supposed to be occupiable
			pThis->CanBeOccupied = true;
		}

		this->BunkerRaidable.Read(exINI, pSection, "Bunker.Raidable");
		this->Firestorm_Wall.Read(exINI, pSection, "Firestorm.Wall");

		//if (!pThis->FirestormWall && this->Firestorm_Wall)
		//	pThis->FirestormWall = this->Firestorm_Wall;
		//else if (!this->Firestorm_Wall && pThis->FirestormWall)
		//	this->Firestorm_Wall = pThis->FirestormWall;

		this->AbandonedSound.Read(exINI, pSection, "AbandonedSound");
		this->CloningFacility.Read(exINI, pSection, "CloningFacility");
		this->Factory_ExplicitOnly.Read(exINI, pSection, "Factory.ExplicitOnly");

		this->LostEvaEvent.Read(exINI, pSection, "LostEvaEvent");
		this->MessageCapture.Read(exINI, pSection, "Message.Capture");
		this->MessageLost.Read(exINI, pSection, "Message.Lost");

		this->AIBuildCounts.Read(exINI, pSection, "AIBuildCounts");
		this->AIExtraCounts.Read(exINI, pSection, "AIExtraCounts");
		this->LandingDir.Read(exINI, pSection, "LandingDir");

		this->Secret_Boons.Read(exINI, pSection, "SecretLab.PossibleBoons");
		this->Secret_RecalcOnCapture.Read(exINI, pSection, "SecretLab.GenerateOnCapture");

		this->Academy.clear();
		this->AcademyWhitelist.Read(exINI, pSection, "Academy.Types");
		this->AcademyBlacklist.Read(exINI, pSection, "Academy.Ignore");
		this->AcademyInfantry.Read(exINI, pSection, "Academy.InfantryVeterancy");
		this->AcademyAircraft.Read(exINI, pSection, "Academy.AircraftVeterancy");
		this->AcademyVehicle.Read(exINI, pSection, "Academy.VehicleVeterancy");
		this->AcademyBuilding.Read(exINI, pSection, "Academy.BuildingVeterancy");
		this->IsAcademy();

		this->DegradeAmount.Read(exINI, pSection, "Degrade.Amount");
		this->DegradePercentage.Read(exINI, pSection, "Degrade.Percentage");
		this->IsPassable.Read(exINI, pSection, "IsPassable");
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashProclaim");
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashDisplay");

		this->Storage_ActiveAnimations.Read(exINI, pSection, "Storage.ActiveAnimations");

		this->PurifierBonus.Read(exINI, pSection, "PurifierBonus");
		this->PurifierBonus_RequirePower.Read(exINI, pSection, "PurifierBonus.RequirePower");
		this->FactoryPlant_RequirePower.Read(exINI, pSection, "FactoryPlant.RequirePower");
		this->SpySat_RequirePower.Read(exINI, pSection, "SpySat.RequirePower");
		this->Cloning_RequirePower.Read(exINI, pSection, "Cloning.RequirePower");
		this->PrismForwarding.LoadFromINIFile(pThis, pINI);
	}
#pragma endregion

	if (pArtINI->GetSection(pArtSection))
	{
		INI_EX exArtINI(pArtINI);

		char strbuff[0x80];

		if (this->IsCustom)
		{
			//Reset
			pThis->Foundation = BuildingTypeExtData::CustomFoundation;
			pThis->FoundationData = this->CustomData.data();
			pThis->FoundationOutside = this->OutlineData.data();
		}
		else if ((pArtINI->ReadString(pArtSection, "Foundation", "", strbuff) > 0 ) && IS_SAME_STR_(strbuff, "Custom"))
		{
			//Custom Foundation!
			this->IsCustom = true;
			pThis->Foundation = BuildingTypeExtData::CustomFoundation;

			//Load Width and Height
			detail::read(this->CustomWidth, exArtINI, pArtSection, "Foundation.X");
			detail::read(this->CustomHeight, exArtINI, pArtSection, "Foundation.Y");

			auto outlineLength = pArtINI->ReadInteger(pArtSection, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if (outlineLength < 10) {
				outlineLength = 10;
			}

			//Allocate CellStruct array
			const size_t dimension = this->CustomWidth * this->CustomHeight;

			this->CustomData.assign(dimension + 1, CellStruct::Empty);
			this->OutlineData.assign(outlineLength + 1, CellStruct::Empty);

			//using Iter = std::vector<CellStruct>::iterator;
			//
			//auto ParsePoint = [](Iter& cell, const char* str) -> void
			//	{
			//		int x = 0, y = 0;
			//		switch (sscanf_s(str, "%d,%d", &x, &y))
			//		{
			//		case 0:
			//			x = 0;
			//		[[fallthrough]];
			//		case 1:
			//			y = 0;
			//		}
			//		*cell++ = CellStruct { static_cast<short>(x), static_cast<short>(y) };
			//	};

			//Load FoundationData
			auto itData = this->CustomData.begin();
			char key[0x20];

			for (size_t i = 0; i < dimension; ++i) {
				IMPL_SNPRNINTF(key, sizeof(key), "Foundation.%d", i);
				if(!detail::read((*itData++), exArtINI, pArtSection, key))
					break;
			}

			//Sort, remove dupes, add end marker
			std::sort(this->CustomData.begin(), itData,
			[](const CellStruct& lhs, const CellStruct& rhs) {
				 if (lhs.Y != rhs.Y) {
					 return lhs.Y < rhs.Y;
				 }
				 return lhs.X < lhs.X;
			});

			itData = std::unique(this->CustomData.begin(), itData);
			*itData = FoundationEndMarker;
			this->CustomData.erase(itData + 1, this->CustomData.end());

			auto itOutline = this->OutlineData.begin();
			for (int i = 0; i < outlineLength; ++i) {
				IMPL_SNPRNINTF(key, sizeof(key), "FoundationOutline.%d", i);
				if (!detail::read((*itOutline++), exArtINI, pArtSection, key)) {
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					*itOutline++ = FoundationEndMarker;
				}
			}

			//Set end vector
			*itOutline = FoundationEndMarker;

			if (this->CustomData.begin() == this->CustomData.end()) {
				Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n",pArtSection);
			}
			else
			{
				auto iter = this->CustomData.begin();
				while (iter->X || iter->Y) {
					if (++iter == this->CustomData.end())
						Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n", pArtSection);

				}
			}

			pThis->FoundationData = this->CustomData.data();
			pThis->FoundationOutside = this->OutlineData.data();
		}

		if (pThis->MaxNumberOccupants > 10)
		{
			char tempMuzzleBuffer[32];
			this->OccupierMuzzleFlashes.resize(pThis->MaxNumberOccupants);

			for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
			{
				Nullable<Point2D> nMuzzleLocation;
				IMPL_SNPRNINTF(tempMuzzleBuffer, sizeof(tempMuzzleBuffer), "%s%d", GameStrings::MuzzleFlash(), i);
				nMuzzleLocation.Read(exArtINI, pArtSection, tempMuzzleBuffer);

				if (nMuzzleLocation.isset())
					this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get();
			}
		}

		this->ZShapePointMove_OnBuildup.Read(exArtINI, pSection, "ZShapePointMove.OnBuildup");
#pragma region Otamaa
		this->HealthOnfire.Read(exArtINI, pArtSection, "OnFire.Health");

#ifndef REPLACE_BUILDING_ONFIRE
		this->DamageFire_Offs.clear();
		this->DamageFire_Offs.reserve(8u);

		char tempFire_OffsBuffer[0x25];
		for (int i = 0;; ++i)
		{
			Nullable<Point2D> nFire_offs;
			IMPL_SNPRNINTF(tempFire_OffsBuffer, sizeof(tempFire_OffsBuffer), "%s%d", GameStrings::DamageFireOffset(), i);
			nFire_offs.Read(exArtINI, pArtSection, tempFire_OffsBuffer);

			if (!nFire_offs.isset())
				break;

			this->DamageFire_Offs.push_back(nFire_offs.Get());
		}
#endif
		this->BuildUp_UseNormalLIght.Read(exArtINI, pArtSection, "Buildup.UseNormalLight");
		this->RubblePalette.Read(exArtINI, pArtSection, "Rubble.Palette");

		this->DockPoseDir.clear();
		if (pThis->Helipad)
		{
			char keyDock[0x40];
			this->DockPoseDir.resize(pThis->NumberOfDocks);

			for (int i = 0; i < pThis->NumberOfDocks; ++i)
			{
				IMPL_SNPRNINTF(keyDock, sizeof(keyDock), "DockingPoseDir%d", i);
				detail::read(this->DockPoseDir[i], exArtINI, pArtSection, keyDock, false);
			}
		}
#pragma endregion

		this->DockUnload_Cell.Read(exArtINI, pArtSection, "DockUnloadCell");
		this->DockUnload_Facing.Read(exArtINI, pArtSection, "DockUnloadFacing");
	}
}

template <typename T>
void BuildingTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Type)
		.Process(this->PrismForwarding)
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
		.Process(this->Grinding_PlayDieSound)


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
		.Process(this->SpyEffect_VictimSW_RealLaunch)
		.Process(this->SpyEffect_RevealProduction)
		.Process(this->SpyEffect_ResetSW)
		.Process(this->SpyEffect_ResetRadar)
		.Process(this->SpyEffect_RevealRadar)
		.Process(this->SpyEffect_RevealRadarPersist)
		.Process(this->SpyEffect_GainVeterancy)
		.Process(this->SpyEffect_UnReverseEngineer)
		.Process(this->SpyEffect_StolenTechIndex_result)
		.Process(this->SpyEffect_StolenMoneyAmount)
		.Process(this->SpyEffect_StolenMoneyPercentage)
		.Process(this->SpyEffect_PowerOutageDuration)
		.Process(this->SpyEffect_SabotageDelay)
		.Process(this->SpyEffect_SuperWeapon)
		.Process(this->SpyEffect_SuperWeaponPermanent)
		.Process(this->SpyEffect_InfantryVeterancy)
		.Process(this->SpyEffect_VehicleVeterancy)
		.Process(this->SpyEffect_NavalVeterancy)
		.Process(this->SpyEffect_AircraftVeterancy)
		.Process(this->SpyEffect_BuildingVeterancy)
		.Process(this->ZShapePointMove_OnBuildup)
		.Process(this->SellBuildupLength)
		.Process(this->CanC4_AllowZeroDamage)
		.Process(this->C4_Modifier)
		.Process(this->DockUnload_Cell)
		.Process(this->DockUnload_Facing)
		.Process(this->Solid_Height)
		.Process(this->Solid_Level)
		.Process(this->AIBaseNormal)
		.Process(this->AIInnerBase)
		.Process(this->RubblePalette)
		.Process(this->DockPoseDir)
		.Process(this->EngineerRepairable)
		.Process(this->IsTrench)
		.Process(this->TunnelType)
		.Process(this->UCPassThrough)
		.Process(this->UCFatalRate)
		.Process(this->UCDamageMultiplier)
		.Process(this->Cursor_Spy)
		.Process(this->ImmuneToSaboteurs)
		.Process(this->ReverseEngineersVictims)
		.Process(this->Cursor_Sabotage)
		.Process(this->GateDownSound)
		.Process(this->GateUpSound)
		.Process(this->UnitSell)
		.Process(this->LightningRod_Modifier)
		.Process(this->Returnable)
		.Process(this->BuildupTime)
		.Process(this->SellTime)
		.Process(this->SlamSound)
		.Process(this->Destroyed_CreateSmudge)

		.Process(this->LaserFenceType)
		.Process(this->LaserFenceWEType)
		.Process(this->LaserFencePostLinks)
		.Process(this->LaserFenceDirection)
		.Process(this->AllowedOccupiers)
		.Process(this->BunkerRaidable)
		.Process(this->Firestorm_Wall)

		.Process(this->AbandonedSound)
		.Process(this->CloningFacility)
		.Process(this->Factory_ExplicitOnly)

		.Process(this->LostEvaEvent)
		.Process(this->MessageCapture)
		.Process(this->MessageLost)
		.Process(this->AIBuildCounts)
		.Process(this->AIExtraCounts)
		.Process(this->LandingDir)
		.Process(this->SellFrames)

		.Process(this->IsCustom)
		.Process(this->CustomWidth)
		.Process(this->CustomHeight)
		.Process(this->OutlineLength)
		.Process(this->CustomData)
		.Process(this->OutlineData)
		.Process(this->FoundationRadarShape)
		.Process(this->Secret_Boons)
		.Process(this->Secret_RecalcOnCapture)
		.Process(this->AcademyWhitelist)
		.Process(this->AcademyBlacklist)
		.Process(this->AcademyInfantry)
		.Process(this->AcademyAircraft)
		.Process(this->AcademyVehicle)
		.Process(this->AcademyBuilding)
		.Process(this->DegradeAmount)
		.Process(this->DegradePercentage)
		.Process(this->IsPassable)
		.Process(this->ProduceCashDisplay)
		.Process(this->PurifierBonus)
		.Process(this->PurifierBonus_RequirePower)
		.Process(this->FactoryPlant_RequirePower)
		.Process(this->SpySat_RequirePower)
		.Process(this->Cloning_RequirePower)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_Houses)
		.Process(this->DisplayIncome_Offset)
		;
}

bool BuildingTypeExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

// =============================
// container
BuildingTypeExtContainer BuildingTypeExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExtContainer::Instance.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK(0x465300, BuildingTypeClass_Save, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x465010, BuildingTypeClass_Load, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x465151, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46531C, BuildingTypeClass_Save_Suffix, 0x6)
{
	BuildingTypeExtContainer::Instance.SaveStatic();
	return 0;
}

// fixed the last delay on game loading !
DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x464A56);
	return 0;
}