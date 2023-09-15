#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Misc/AresData.h>

std::vector<std::string> BuildingTypeExt::trenchKinds;
const DirStruct  BuildingTypeExt::DefaultJuggerFacing = DirStruct { 0x7FFF };
const CellStruct BuildingTypeExt::FoundationEndMarker = { 0x7FFF, 0x7FFF };

bool  BuildingTypeExt::ExtData::IsAcademy() const
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

void BuildingTypeExt::ExtData::UpdateFoundationRadarShape()
{
	this->FoundationRadarShape.Clear();

	if (this->IsCustom)
	{
		auto pType = this->OwnerObject();
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
				Point2D pixel = { j, i };
				this->FoundationRadarShape.AddItem(pixel);
			}
		}
	}
}

bool BuildingTypeExt::ExtData::IsFoundationEqual(BuildingTypeClass* pType1, BuildingTypeClass* pType2)
{
	// both types must be set and must have same foundation id
	if (!pType1 || !pType2 || pType1->Foundation != pType2->Foundation) {
		return false;
	}

	// non-custom foundations need no special handling
	if (pType1->Foundation != BuildingTypeExt::CustomFoundation)
	{
		return true;
	}

	// custom foundation
	auto const pExt1 = BuildingTypeExt::ExtMap.Find(pType1);
	auto const pExt2 = BuildingTypeExt::ExtMap.Find(pType2);
	const auto& data1 = pExt1->CustomData;
	const auto& data2 = pExt2->CustomData;

	// this works for any two foundations. it's linear with sorted ones
	return pExt1->CustomWidth == pExt2->CustomWidth
		&& pExt1->CustomHeight == pExt2->CustomHeight
		&& std::is_permutation(
			data1.begin(), data1.end(), data2.begin(), data2.end());
}

void BuildingTypeExt::ExtData::Initialize()
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
	this->Type = TechnoTypeExt::ExtMap.Find(this->Get());
	this->OccupierMuzzleFlashes.reserve(((BuildingTypeClass*)this->Type->Get())->MaxNumberOccupants);
	this->DockPoseDir.reserve(((BuildingTypeClass*)this->Type->Get())->NumberOfDocks);
	this->LostEvaEvent = VoxClass::FindIndexById(GameStrings::EVA_TechBuildingLost());
}

bool BuildingTypeExt::ExtData::CanBeOccupiedBy(InfantryClass* whom)
{
	// if CanBeOccupiedBy isn't empty, we have to check if this soldier is allowed in
	return this->AllowedOccupiers.empty() || this->AllowedOccupiers.Contains(whom->Type);
}

int BuildingTypeExt::BuildLimitRemaining(HouseClass* pHouse, BuildingTypeClass* pItem)
{
	const auto BuildLimit = pItem->BuildLimit;

	if (BuildLimit >= 0)
		return BuildLimit - BuildingTypeExt::GetUpgradesAmount(pItem, pHouse);
	else
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
}

int BuildingTypeExt::CheckBuildLimit(HouseClass* pHouse, BuildingTypeClass* pItem, bool includeQueued)
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
	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->PlacementPreview_TranslucentLevel.Get(RulesExt::Global()->BuildingPlacementPreview_TranslucentLevel));
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

bool BuildingTypeExt::IsLinkable(BuildingTypeClass* pThis)
{
	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis);

	return pExt->Firestorm_Wall || pExt->IsTrench >= 0;
}

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);
	for (const auto& [pBldType, nCount] : pHouseExt->PowerPlantEnhancerBuildings)
	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBldType);
		if (pExt->PowerPlantEnhancer_Buildings.empty() || !pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
			continue;

		fFactor *= std::powf(pExt->PowerPlantEnhancer_Factor.Get(1.0f), static_cast<float>(nCount));
		nAmount += pExt->PowerPlantEnhancer_Amount.Get(0) * nCount;
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 1.0;

	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExt::IsObserverPlayer(pOwner))
		return fFactor;

	const auto pType = pWhat->GetTechnoType();
	if (!pType)
		return fFactor;

	auto pHouseExt = HouseExt::ExtMap.Find(pOwner);
	if (pHouseExt->Building_BuildSpeedBonusCounter.empty())
		return fFactor;

	for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter)
	{
		if (auto const pExt = BuildingTypeExt::ExtMap.TryFind(pBldType))
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

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoTypeClass* pWhat, HouseClass* pOwner)
{
	double fFactor = 1.0;
	if (!pWhat || !pOwner || pOwner->Defeated || pOwner->IsNeutral() || HouseExt::IsObserverPlayer(pOwner))
		return fFactor;

	auto pHouseExt = HouseExt::ExtMap.Find(pOwner);
	if (pHouseExt->Building_BuildSpeedBonusCounter.empty())
		return fFactor;

	const auto what = pWhat->WhatAmI();
	for (const auto& [pBldType, nCount] : pHouseExt->Building_BuildSpeedBonusCounter)
	{
		if (auto const pExt = BuildingTypeExt::ExtMap.TryFind(pBldType))
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

double BuildingTypeExt::GetExternalFactorySpeedBonus(TechnoClass* pWhat)
{
	return BuildingTypeExt::GetExternalFactorySpeedBonus(pWhat, pWhat->GetOwningHouse());
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
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


	for (auto pTPowersUp : BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings)
		checkUpgrade(pTPowersUp);

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (pINI->ReadString(pSection, "IsTrench", "", Phobos::readBuffer))
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
		this->Grinding_DisplayRefund.Read(exINI, pSection, "Grinding.DisplayRefund");
		this->Grinding_DisplayRefund_Houses.Read(exINI, pSection, "Grinding.DisplayRefund.Houses");
		this->Grinding_DisplayRefund_Offset.Read(exINI, pSection, "Grinding.DisplayRefund.Offset");
		this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");

		this->Refinery_DisplayDumpedMoneyAmount.Read(exINI, pSection, "Refinery.DisplayDumpedTiberiumCost");
		this->Refinery_DisplayRefund_Offset.Read(exINI, pSection, "Refinery.DisplayDumpedTiberiumCostOffset");

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

		this->EnterBioReactorSound.Read(exINI, pSection, GameStrings::EnterBioReactorSound());
		this->LeaveBioReactorSound.Read(exINI, pSection, GameStrings::LeaveBioReactorSound());

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
		this->SpyEffect_StolenTechIndex.Read(exINI, pSection, "SpyEffect.StolenTechIndex");
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

		if (this->SpyEffect_StolenTechIndex >= 32)
		{
			Debug::Log("BuildingType %s has a SpyEffect.StolenTechIndex of %d. The value has to be less than 32.\n", pSection, this->SpyEffect_StolenTechIndex.Get());
			this->SpyEffect_StolenTechIndex = -1;
		}

		this->CanC4_AllowZeroDamage.Read(exINI, pSection, "CanC4.AllowZeroDamage");
		this->C4_Modifier.Read(exINI, pSection, "C4Modifier");
		this->DockUnload_Cell.Read(exINI, pSection, "DockUnloadCell");
		this->DockUnload_Facing.Read(exINI, pSection, "DockUnloadFacing");

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
		this->ProduceCashDisplay.Read(exINI, pSection, "ProduceCashDisplay");

	}
#pragma endregion

	if (pArtINI->GetSection(pArtSection))
	{
		INI_EX exArtINI(pArtINI);

		char strbuff[0x80];

		if (this->IsCustom)
		{
			//Reset
			pThis->Foundation = BuildingTypeExt::CustomFoundation;
			pThis->FoundationData = this->CustomData.data();
		}
		else if (pArtINI->ReadString(pArtSection, "Foundation", "", strbuff) && IS_SAME_STR_(strbuff, "Custom"))
		{
			//Custom Foundation!
			this->IsCustom = true;
			pThis->Foundation = BuildingTypeExt::CustomFoundation;

			//Load Width and Height
			this->CustomWidth = pArtINI->ReadInteger(pArtSection, "Foundation.X", 0);
			this->CustomHeight = pArtINI->ReadInteger(pArtSection, "Foundation.Y", 0);
			auto outlineLength = pArtINI->ReadInteger(pArtSection, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if (outlineLength < 10) {
				outlineLength = 10;
			}

			//Allocate CellStruct array
			const size_t dimension = this->CustomWidth * this->CustomHeight;

			this->CustomData.assign(dimension + 1, CellStruct::Empty);
			this->OutlineData.assign(outlineLength + 1, CellStruct::Empty);

			pThis->FoundationData = this->CustomData.data();
			pThis->FoundationOutside = this->OutlineData.data();

			using Iter = std::vector<CellStruct>::iterator;

			auto ParsePoint = [](Iter& cell, const char* str) -> void
				{
					int x = 0, y = 0;
					switch (sscanf_s(str, "%d,%d", &x, &y))
					{
					case 0:
						x = 0;
						// fallthrough
					case 1:
						y = 0;
					}
					*cell++ = CellStruct { static_cast<short>(x), static_cast<short>(y) };
				};

			//Load FoundationData
			auto itData = this->CustomData.begin();
			char key[0x20];

			for (size_t i = 0; i < dimension; ++i)
			{
				_snprintf_s(key, _TRUNCATE, "Foundation.%d", i);
				if (pArtINI->ReadString(pArtSection, key, "", strbuff))
				{
					ParsePoint(itData, strbuff);
				}
				else
				{
					break;
				}
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
			for (int i = 0; i < outlineLength; ++i)
			{
				_snprintf_s(key, _TRUNCATE, "FoundationOutline.%d", i);
				if (pArtINI->ReadString(pArtSection, key, "", strbuff))
				{
					ParsePoint(itOutline, strbuff);
				}
				else
				{
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					*itOutline++ = FoundationEndMarker;
				}
			}

			//Set end vector
			*itOutline = FoundationEndMarker;

			//if (this->OutlineData.begin() == this->OutlineData.end()) {
			//	Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n",pArtSection);
			//}
			//else
			//{
			//	for (auto iter = this->OutlineData.begin(); iter->X || iter->Y; ++iter) {
			//		if (++iter == this->OutlineData.end())
			//			Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n", pArtSection);
			//
			//	}
			//}
		}

		if (pThis->MaxNumberOccupants > 10)
		{
			char tempMuzzleBuffer[32];
			this->OccupierMuzzleFlashes.clear();
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
			Nullable<Point2D> nFire_offs {};
			IMPL_SNPRNINTF(tempFire_OffsBuffer, sizeof(tempFire_OffsBuffer), "%s%d", GameStrings::DamageFireOffset(), i);
			nFire_offs.Read(exArtINI, pArtSection, tempFire_OffsBuffer);

			if (!nFire_offs.isset())
				break;

			this->DamageFire_Offs.push_back(nFire_offs.Get());
		}
#endif
		this->BuildUp_UseNormalLIght.Read(exArtINI, pArtSection, "Buildup.UseNormalLight");
		this->RubblePalette.Read(exArtINI, pArtSection, "Rubble.Palette");

		if (pThis->Helipad)
		{
			char keyDock[0x40] = { '\0' };
			BuildingTypeExt::ExtMap.Find(pThis)->DockPoseDir.resize(pThis->NumberOfDocks);

			for (int i = 0; i < pThis->NumberOfDocks; ++i)
			{
				sprintf_s(keyDock, "DockingPoseDir%d", i);
				Valueable<FacingType> dummyDock { FacingType::North };
				dummyDock.Read(exArtINI, pArtSection, keyDock);
				BuildingTypeExt::ExtMap.Find(pThis)->DockPoseDir[i] = dummyDock.Get();
			}
		}
#pragma endregion
	}
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Type)
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
		.Process(this->SpyEffect_VictimSW_RealLaunch)
		.Process(this->SpyEffect_RevealProduction)
		.Process(this->SpyEffect_ResetSW)
		.Process(this->SpyEffect_ResetRadar)
		.Process(this->SpyEffect_RevealRadar)
		.Process(this->SpyEffect_RevealRadarPersist)
		.Process(this->SpyEffect_GainVeterancy)
		.Process(this->SpyEffect_UnReverseEngineer)
		.Process(this->SpyEffect_StolenTechIndex)
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
		.Process(this->EnterBioReactorSound)
		.Process(this->LeaveBioReactorSound)
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
		;
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

// =============================
// container
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;
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

DEFINE_HOOK(0x465300, BuildingTypeClass_Save, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x465010, BuildingTypeClass_Load, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x465151, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46531C, BuildingTypeClass_Save_Suffix, 0x6)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

// fixed the last delay on game loading !
DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x464A56);
	return 0;
}