#include "Body.h"

#include <OverlayClass.h>
#include <TerrainClass.h>
#include <TacticalClass.h>
#include <EventClass.h>

#include <Utilities/GeneralUtils.h>

#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <Helpers/Macro.h>

#define IS_CELL_OCCUPIED(pCell)\
pCell->OccupationFlags & 0x20 || pCell->OccupationFlags & 0x40 || pCell->OccupationFlags & 0x80 || pCell->GetInfantry(false) \

static inline BuildingTypeClass* GetAnotherPlacingType(BuildingTypeClass* pType, BuildingTypeExtData* pTypeExt, CellStruct checkCell, bool opposite)
{
	if (!pType->PlaceAnywhere && !pTypeExt->LimboBuild)
	{
		const auto onWater = MapClass::Instance->GetCellAt(checkCell)->LandType == LandType::Water;
		const auto waterBound = pType->SpeedType == SpeedType::Float;

		if (const auto pAnotherType = (opposite ^ onWater) ? (waterBound ? nullptr : pTypeExt->PlaceBuilding_OnWater.Get()) : (waterBound ? pTypeExt->PlaceBuilding_OnLand.Get() : nullptr))
		{
			if (pAnotherType->BuildCat == pType->BuildCat && !pAnotherType->PlaceAnywhere && !BuildingTypeExtContainer::Instance.Find(pAnotherType)->LimboBuild)
				return pAnotherType;
		}
	}

	return nullptr;
}

static inline BuildingTypeClass* GetAnotherPlacingType(DisplayClass* pDisplay)
{
	if (const auto pCurrentBuilding = cast_to<BuildingClass*>(pDisplay->CurrentBuilding))
	{
		const auto pType = pCurrentBuilding->Type;
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

		if (pTypeExt->PlaceBuilding_OnLand || pTypeExt->PlaceBuilding_OnWater)
			return GetAnotherPlacingType(pType, pTypeExt, pDisplay->CurrentFoundation_CenterCell, false);
	}

	return nullptr;
}

static inline bool IsSameFenceType(BuildingTypeClass* pPostType, BuildingTypeClass* pFenceType)
{
	if (const auto pSpecificType = BuildingTypeExtContainer::Instance.Find(pPostType)->LaserFencePost_Fence.Get())
	{
		if (pSpecificType != pFenceType)
			return false;
	}
	else
	{
		const auto count = BuildingTypeClass::Array->Count;

		for (int i = 0; i < count; ++i)
		{
			const auto pSearchType = BuildingTypeClass::Array->Items[i];

			if (pSearchType->LaserFence)
			{
				if (pSearchType != pFenceType)
					return false;

				break;
			}
		}
	}

	return true;
}

static inline bool CheckCanNotExistHere(FootClass* pTechno, HouseClass* const pOwner, bool expand, bool& skipFlag, bool& builtOnCanBeBuiltOn, bool& landFootOnly)
{
	if (pTechno == TechnoExtData::Deployer)
	{
		skipFlag = true;
		return false;
	}

	const auto pTechnoType = pTechno->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	if (pTypeExt->CanBeBuiltOn)
	{
		if (pTechno->GetMapCoords() == pTechno->CurrentMapCoords)
			builtOnCanBeBuiltOn = true;
		else if (expand)
			landFootOnly = true;
		else
			return true;
	}
	else if (!expand || pTechnoType->Speed <= 0 || !BuildingTypeExtData::CheckOccupierCanLeave(pOwner, pTechno->Owner))
	{
		return true;
	}
	else
	{
		landFootOnly = true;
	}

	return false;
}

// Passable TerrainTypes Hook #1 - Do not set occupy bits.
ASMJIT_PATCH(0x71C110, TerrainClass_SetOccupyBit_PassableTerrain, 0x5)
{
	enum { Skip = 0x71C1A0 };

	GET(TerrainClass*, pThis, ECX);

	return (TerrainTypeExtContainer::Instance.Find(pThis->Type)->IsPassable) ? Skip : 0;
}

// Passable TerrainTypes Hook #2 - Do not display attack cursor unless force-firing.
ASMJIT_PATCH(0x7002E9, TechnoClass_WhatAction_PassableTerrain, 0x5)
{
	enum { ReturnAction = 0x70020E };

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(bool, isForceFire, STACK_OFFS(0x1C, -0x8));

	if (!pThis->Owner->IsControlledByHuman() || !pThis->IsControllable())
		return 0;

	if (auto const pTerrain = cast_to<TerrainClass*, false>(pTarget))
	{
		if (TerrainExtData::CanMoveHere(pThis, pTerrain) && !isForceFire)
		{
			R->EBP(Action::Move);
			return ReturnAction;
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #3 - Count passable TerrainTypes as completely passable.
ASMJIT_PATCH(0x483DDF, CellClass_CheckPassability_PassableTerrain, 0x6)
{
	enum { SkipToNextObject = 0x483DCD, ReturnFromFunction = 0x483E25, BreakFromLoop = 0x483DDF };

	GET(CellClass*, pThis, EDI);
	GET(TerrainClass*, pTerrain, ESI);

	if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->IsPassable)
	{
		pThis->Passability = PassabilityType::Passable;
		return ReturnFromFunction;
	}

	return 0x0;
}

// Passable TerrainTypes Hook #4 - Make passable for vehicles.
ASMJIT_PATCH(0x73FBA7, UnitClass_CanEnterCell_PassableTerrain, 0x5)
{
	enum { ReturnPassable = 0x73FD37, SkipTerrainChecks = 0x73FA7C };

	GET(UnitClass*, pThis, EBX);
	GET(TerrainClass*, pTerrain, ESI);

	if (TerrainExtData::CanMoveHere(pThis, pTerrain))
	{
		if (IS_CELL_OCCUPIED(pTerrain->GetCell()))
			return SkipTerrainChecks;

		R->EBP(0);
		return ReturnPassable;
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #1 - Allow placing buildings on top of them.
ASMJIT_PATCH(0x47C745, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)
{
	enum { Skip = 0x47C85F, SkipFlags = 0x47C6A0 };

	GET(CellClass*, pThis, EDI);

	if (auto const pTerrain = pThis->GetTerrain(false))
	{
		if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn)
		{
			if (IS_CELL_OCCUPIED(pThis))
				return Skip;
			else
				return SkipFlags;
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x47C80E, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)


// Buildable-upon TerrainTypes Hook #2 - Allow placing laser fences on top of them.
ASMJIT_PATCH(0x47C657, CellClass_IsClearTo_Build_BuildableTerrain_LF, 0x6)
{
	enum { Skip = 0x47C6A0, Return = 0x47C6D1 };

	GET(CellClass*, pThis, EDI);

	if (auto pObj = pThis->FirstObject)
	{
		bool isEligible = true;

		while (true)
		{
			const auto what = pObj->WhatAmI();

			isEligible = what != BuildingClass::AbsID;

			if (what == TerrainClass::AbsID)
			{
				if (auto const pTerrain = static_cast<TerrainClass*>(pObj))
				{
					isEligible = TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn;
				}
			}

			if (!isEligible)
				break;

			pObj = pObj->NextObject;

			if (!pObj)
				return Skip;
		}

		return Return;
	}

	return Skip;
}

// Buildable-upon TerrainTypes Hook #3 - Draw laser fence placement even if they are on the way.
ASMJIT_PATCH(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		return (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn) ? ContinueChecks : DontDraw;
	}

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #4 - Remove them when buildings are placed on them.
ASMJIT_PATCH(0x5684B1, MapClass_PlaceDown_BuildableTerrain, 0x6)
{
	GET(ObjectClass*, pPlaceObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pPlaceObject->WhatAmI() == AbstractType::Building) {
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
			const auto absType = pObject->WhatAmI();

			if (const auto pTechno = flag_cast_to<TechnoClass* , false>(pObject)) {
				const auto pType = pTechno->GetTechnoType();

				//TODO: this function can cause bug , since not all stuffs were handled properly
				if (TechnoTypeExtContainer::Instance.Find(pType)->CanBeBuiltOn) {
					//int damage = pTechno->Health;
					//pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
					pTechno->KillPassengers(nullptr);
					pTechno->Stun();
					pTechno->Limbo();
					pTechno->UnInit();
				}
			} else if (absType == AbstractType::Terrain) {
				auto pTerrain = static_cast<TerrainClass*>(pObject);

				if (pTerrain->Type && TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn)
				{
					pCell->RemoveContent(pTerrain, false);
					TerrainTypeExtData::Remove(pTerrain);
				}
			}
		}
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #4 -> sub_5FD270 - Allow placing buildings on top of them
ASMJIT_PATCH(0x5FD2B6, OverlayClass_Unlimbo_SkipTerrainCheck, 0x9)
{
	enum { Unlimbo = 0x5FD2CA, NoUnlimbo = 0x5FD2C3 };

	GET(CellClass* const, pCell, EAX);

	if (!Game::IsActive)
		return Unlimbo;

	auto pCellObject = pCell->FirstObject;

	for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
	{
		if (const auto pTerrain = cast_to<TerrainClass*, false>(pCellObject))
		{
			if (!TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn)
				return NoUnlimbo;

			pCell->RemoveContent(pTerrain, false);
			TerrainTypeExtData::Remove(pTerrain);
		}
	}

	return Unlimbo;
}

#undef IS_CELL_OCCUPIED

// Buildable Proximity Helper
namespace ProximityTemp
{
	bool Exist = false;
	bool Mouse = false;
	CellClass* CurrentCell = nullptr;
}

// Buildable-upon TerrainTypes Hook #1 -> sub_47C620 - Allow placing buildings on top of them
// Buildable-upon TechnoTypes Hook #1 -> sub_47C620 - Rewrite and check whether allow placing buildings on top of them
// Customized Laser Fence Hook #1 -> sub_47C620 - Forbid placing laser fence post on inappropriate laser fence
ASMJIT_PATCH(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(BuildingTypeClass*, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	ProximityTemp::Exist = false;

	if (!Game::IsActive)
		return CanExistHere;

	const auto expand = RulesExtData::Instance()->ExtendedBuildingPlacing.Get();
	bool landFootOnly = false;

	if (pBuildingType->LaserFence)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

				if (!pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
			else if (const auto pTerrain = cast_to<TerrainClass* , false>(pObject))
			{
				if(!pTerrain->Type)
					return CanNotExistHere;

				const auto pTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

				if (!pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool builtOnCanBeBuiltOn = false;
		bool skipFlag = TechnoExtData::Deployer ? TechnoExtData::Deployer->CurrentMapCoords == pCell->MapCoords : false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft)
			{
				const auto pAircraft = static_cast<AircraftClass*>(pObject);
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pAircraft->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnCanBeBuiltOn = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
				{
					builtOnCanBeBuiltOn = true;
				}
				else if (pOwner != pBuilding->Owner || !pType->LaserFence)
				{
					return CanNotExistHere;
				}
				else if (pBuildingType->LaserFencePost && !IsSameFenceType(pBuildingType, pType))
				{
					return CanNotExistHere;
				}
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, builtOnCanBeBuiltOn, landFootOnly))
					return CanNotExistHere;
			}
			else if (const auto pTerrain = cast_to<TerrainClass* , false>(pObject))
			{
				const auto pTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnCanBeBuiltOn = true;
				else
					return CanNotExistHere;
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}
	else if (pBuildingType->ToTile)
	{
		const auto isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array->Count && !IsometricTileTypeClass::Array->Items[isoTileTypeIndex]->Morphable)
			return CanNotExistHere;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Building)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

				if (!pTypeExt->CanBeBuiltOn)
					return CanNotExistHere;
			}
		}
	}
	else
	{
		bool skipFlag = TechnoExtData::Deployer ? TechnoExtData::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto absType = pObject->WhatAmI();

			if (absType == AbstractType::Aircraft || absType == AbstractType::Building)
			{
				const auto pTechno = static_cast<TechnoClass*>(pObject);
				const auto pTechnoType = pTechno->GetTechnoType();
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnCanBeBuiltOn = true;
				else
					return CanNotExistHere;
			}
			else if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
			{
				if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, builtOnCanBeBuiltOn, landFootOnly))
					return CanNotExistHere;
			}
			else if (const auto pTerrain = cast_to<TerrainClass* , false>(pObject))
			{
				const auto pTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

				if (pTypeExt && pTypeExt->CanBeBuiltOn)
					builtOnCanBeBuiltOn = true;
				else
					return CanNotExistHere;
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}

	if (landFootOnly)
		ProximityTemp::Exist = true;

	return CanExistHere; // Continue check the overlays .etc
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it then skip vanilla AltFlags check
ASMJIT_PATCH(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	enum { DontDrawAlt = 0x47EF1A, DrawVanillaAlt = 0x47EED6 };

	GET(CellClass* const, pCell, ESI);
	GET(const bool, zero, EDX);

	const BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;
	ProximityTemp::CurrentCell = pCell;

	if (!(pCell->AltFlags & AltCellFlags::ContainsBuilding))
	{
		if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		{
			R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
			return DrawVanillaAlt;
		}
		else if (BuildingTypeClass* const pType = cast_to<BuildingTypeClass*>(DisplayClass::Instance->CurrentBuildingTypeCopy))
		{
			R->Stack<bool>(STACK_OFFSET(0x30, -0x1D), pCell->CanThisExistHere(pType->SpeedType, pType, HouseClass::CurrentPlayer));
			R->EDX<BlitterFlags>(flags | BlitterFlags::TransLucent75);
			return DontDrawAlt;
		}
	}

	R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
	return DontDrawAlt;
}

static inline void ClearPlacingBuildingData(PlacingBuildingStruct* const pPlace)
{
	pPlace->Type = nullptr;
	pPlace->DrawType = nullptr;
	pPlace->Times = 0;
	pPlace->TopLeft = CellStruct::Empty;
	pPlace->Timer.Stop();
}

static inline void ClearCurrentBuildingData(DisplayClass* const pDisplay)
{
	pDisplay->SetActiveFoundation(nullptr);
	pDisplay->CurrentBuilding = nullptr;
	pDisplay->CurrentBuildingType = nullptr;
	pDisplay->CurrentBuildingOwnerArrayIndexCopy = -1;

	if (!Unsorted::ArmageddonMode)
	{
		pDisplay->SetCursorShape2(nullptr);
		pDisplay->CurrentBuildingCopy = nullptr;
		pDisplay->CurrentBuildingTypeCopy = nullptr;
	}
}

template <bool slam = false>
static inline void PlayConstructionYardAnim(BuildingClass* const pFactory)
{
	const auto pFactoryType = pFactory->Type;

	if (pFactoryType->ConstructionYard)
	{
		if constexpr (slam)
			VocClass::PlayGlobal(BuildingTypeExtContainer::Instance.Find(pFactoryType)->SlamSound.
				Get(RulesClass::Instance->BuildingSlam), Panning::Center, 1.0);

		pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
		pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

		const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

		if (pAnimName && *pAnimName)
			pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
	}
}

static inline bool CheckBuildingFoundation(BuildingTypeClass* const pBuildingType, const CellStruct topLeftCell, HouseClass* const pHouse, bool& noOccupy)
{
	for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		if (const auto pCell = MapClass::Instance->TryGetCellAt(topLeftCell + *pFoundation))
		{
			if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
				return false;
			else if (ProximityTemp::Exist)
				noOccupy = false;
		}
	}

	ProximityTemp::Exist = false;
	return true;
}

// Buildable-upon TechnoTypes Hook #3 -> sub_4FB0E0 - Hang up place event if there is only infantries and units on the cell
ASMJIT_PATCH(0x4FB1EA, HouseClass_UnitFromFactory_HangUpPlaceEvent, 0x5)
{
	enum { CanBuild = 0x4FB23C, TemporarilyCanNotBuild = 0x4FB5BA, CanNotBuild = 0x4FB35F, BuildSucceeded = 0x4FB649 };

	GET(HouseClass*, pHouse, EBP);
	GET(TechnoClass*, pTechno, ESI);
	GET(FactoryClass*, pPrimary, EBX);
	GET(BuildingClass*, pFactory, EDI);
	GET_STACK(CellStruct, topLeftCell, STACK_OFFSET(0x3C, 0x10));

	if (pTechno->WhatAmI() != AbstractType::Building)
	{
		pFactory->SendCommand(RadioCommand::RequestLink, pTechno);

		if (pTechno->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
			return CanBuild;

		ProximityTemp::Mouse = true;
		return CanNotBuild;
	}

	const auto pDisplay = DisplayClass::Instance();
	auto pBuilding = static_cast<BuildingClass*>(pTechno);
	auto pBuildingType = pBuilding->Type;
	const auto pBufferBuilding = pBuilding;
	const auto pBufferType = pBuildingType;
	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExtData::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);

		if (pDisplay->CurrentBuilding == pBuilding && HouseClass::CurrentPlayer == pHouse)
			ClearCurrentBuildingData(pDisplay);

		PlayConstructionYardAnim<true>(pFactory);
		return BuildSucceeded;
	}

	const bool expand = RulesExtData::Instance()->ExtendedBuildingPlacing && !pBuildingType->PlaceAnywhere && !pBuildingType->PowersUpBuilding[0];
	if (pTypeExt->PlaceBuilding_OnWater || pTypeExt->PlaceBuilding_OnLand)
	{
		if (!SessionClass::IsMultiplayer())
		{
			if (expand)
			{
				const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
				auto& place = pBufferType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

				if (place.DrawType && (place.DrawType == pTypeExt->PlaceBuilding_OnLand || place.DrawType == pTypeExt->PlaceBuilding_OnWater))
					pBuildingType = place.DrawType;
			}

			const auto& pTypeCopy = pDisplay->CurrentBuildingTypeCopy;

			if (pTypeCopy && (pTypeCopy == pTypeExt->PlaceBuilding_OnLand || pTypeCopy == pTypeExt->PlaceBuilding_OnWater))
				pBuildingType = static_cast<BuildingTypeClass*>(pTypeCopy);
		}
		else // When playing online, this can not rely on locally stored replicas, and must made speculation based on the event
		{
			auto checkCell = topLeftCell;

			if (pBuildingType->Gate)
			{
				if (pBuildingType->GetFoundationWidth() > 2)
					checkCell.X += 1;
				else if (pBuildingType->GetFoundationHeight(false) > 2)
					checkCell.Y += 1;
			}
			else if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
			{
				checkCell += CellStruct { 1, 1 };
			}

			if (const auto pOtherType = GetAnotherPlacingType(pBuildingType, pTypeExt, checkCell, false))
			{
				pBuildingType = pOtherType;
			}
			else if (const auto pAnotherType = GetAnotherPlacingType(pBuildingType , pTypeExt, topLeftCell, true)) // Center cell may be different, so make assumptions
			{
				checkCell = topLeftCell;

				if (pAnotherType->Gate)
				{
					if (pAnotherType->GetFoundationWidth() > 2)
						checkCell.X += 1;
					else if (pAnotherType->GetFoundationHeight(false) > 2)
						checkCell.Y += 1;
				}
				else if (pAnotherType->GetFoundationWidth() > 2 || pAnotherType->GetFoundationHeight(false) > 2)
				{
					checkCell += CellStruct { 1, 1 };
				}

				// If the land occupation of the two buildings is different, the larger one will prevail, And the smaller one may not be placed on the shore.
				if ((MapClass::Instance->GetCellAt(checkCell)->LandType == LandType::Water) ^ (pBuildingType->SpeedType == SpeedType::Float))
					pBuildingType = pAnotherType;
			}
		}
	}

	bool revert = false;

	if (expand)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
		const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
		auto& place = pBufferType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can Build

				do
				{
					if (topLeftCell != place.TopLeft || pBufferType != place.Type) // New command
					{
						place.Type = pBufferType;
						place.DrawType = pBuildingType;
						place.Times = 30;
						place.TopLeft = topLeftCell;
					}
					else if (place.Times <= 0)
					{
						break; // Time out
					}

					if (!(place.Times % 5) && BuildingTypeExtData::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
						break; // No place for cleaning

					if (pHouse == HouseClass::CurrentPlayer && place.Times == 30)
						ClearCurrentBuildingData(pDisplay);

					--place.Times;
					place.Timer.Start(8);

					return TemporarilyCanNotBuild;
				}
				while (false);
			}

			revert = place.Times == 30 || !place.Type;
			ClearPlacingBuildingData(&place);

			if (revert)
				ProximityTemp::Mouse = true;

			return CanNotBuild;
		}
		while (false);

		revert = !place.Type;
		ClearPlacingBuildingData(&place);
	}

	if (pBufferType != pBuildingType)
		pBuilding = static_cast<BuildingClass*>(pBuildingType->CreateObject(pHouse));

	pFactory->SendCommand(RadioCommand::RequestLink, pBuilding);

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		if (pBufferBuilding != pBuilding)
		{
			if (HouseClass::CurrentPlayer == pHouse)
			{
				if (pDisplay->CurrentBuilding == pBufferBuilding)
					pDisplay->CurrentBuilding = pBuilding;

				if (pDisplay->CurrentBuildingType == pBufferType)
					pDisplay->CurrentBuildingType = pBuildingType;

				if (pDisplay->CurrentBuildingCopy == pBufferBuilding)
					pDisplay->CurrentBuildingCopy = pBuilding;

				if (pDisplay->CurrentBuildingTypeCopy == pBufferType)
					pDisplay->CurrentBuildingTypeCopy = pBuildingType;

				if (Make_Global<BuildingClass*>(0xB0FE5C) == pBufferBuilding) // Q
					Make_Global<BuildingClass*>(0xB0FE5C) = pBuilding;

				if (Make_Global<BuildingClass*>(0xB0FE60) == pBufferBuilding) // W
					Make_Global<BuildingClass*>(0xB0FE60) = pBuilding;
			}

			pBufferBuilding->UnInit();
			pPrimary->Object = pBuilding;
			R->ESI(pBuilding);
		}

		return CanBuild;
	}

	if (pBufferBuilding != pBuilding)
		pBuilding->UnInit();

	if (revert)
		ProximityTemp::Mouse = true;

	return CanNotBuild;
}

// Buildable-upon TechnoTypes Hook #4-1 -> sub_4FB0E0 - Check whether need to skip the replace command
ASMJIT_PATCH(0x4FB395, HouseClass_UnitFromFactory_SkipMouseReturn, 0x6)
{
	enum { SkipGameCode = 0x4FB489, CheckMouseCoords = 0x4FB3E3 };

	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	if (ProximityTemp::Mouse)
	{
		ProximityTemp::Mouse = false;
		return 0;
	}

	R->EBX(0);
	if (!DisplayClass::Instance->CurrentBuildingTypeCopy)
		return SkipGameCode;

	R->ECX(DisplayClass::Instance->CurrentBuildingTypeCopy);
	return CheckMouseCoords;
}

// Buildable-upon TechnoTypes Hook #4-2 -> sub_4FB0E0 - Check whether need to skip the clear command
ASMJIT_PATCH(0x4FB339, HouseClass_UnitFromFactory_SkipMouseClear, 0x6)
{
	enum { SkipGameCode = 0x4FB4A0 };

	GET(TechnoClass* const, pTechno, ESI);

	if (RulesExtData::Instance()->ExtendedBuildingPlacing)
	{
		if (const auto pBuilding = cast_to<BuildingClass*>(pTechno))
		{
			if (const auto pCurrentType = cast_to<BuildingTypeClass*>(DisplayClass::Instance->CurrentBuildingType))
			{
				if (!BuildingTypeExtData::IsSameBuildingType(pBuilding->Type, pCurrentType))
					return SkipGameCode;
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #4-3 -> sub_4FB0E0 - Check whether need to skip the clear command
ASMJIT_PATCH(0x4FAB83, HouseClass_AbandonProductionOf_SkipMouseClear, 0x7)
{
	enum { SkipGameCode = 0x4FABA4 };

	GET(const int, index, EBX);

	if (RulesExtData::Instance()->ExtendedBuildingPlacing && index >= 0)
	{
		if (const auto pCurrentBuildingType = cast_to<BuildingTypeClass*>(DisplayClass::Instance->CurrentBuildingType))
		{
			if (!BuildingTypeExtData::IsSameBuildingType(BuildingTypeClass::Array->Items[index], pCurrentBuildingType))
				return SkipGameCode;
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
ASMJIT_PATCH(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExtData::Instance()->ExtendedBuildingPlacing)
	{
		const auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);
		const auto pBuilding = cast_to<BuildingClass*>(pFactory->Object);

		if (!pBuilding)
			return 0;

		auto place = &(pBuilding->Type->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat);
		ClearPlacingBuildingData(place);
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #6 -> sub_443C60 - Try to clean up the building space when AI is building
ASMJIT_PATCH(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum {
		CanBuild = 0x4452F0,
		TemporarilyCanNotBuild = 0x445237,
		CanNotBuild = 0x4454E6,
		BuildSucceeded = 0x4454D4,
		BuildFailed = 0x445696
	};

	GET(BaseNodeClass*, pBaseNode, EBX);
	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pFactory, ESI);
	GET(CellStruct, topLeftCell, EDX);

	const auto pBuildingType = pBuilding->Type;

	if (RulesExtData::Instance()->AIForbidConYard && pBuildingType->ConstructionYard)  {
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;
		}
		return BuildFailed;
	}

	const auto pHouse = pFactory->Owner;
	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExtData::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;

			if (pHouse->ProducingBuildingTypeIndex == pBuildingType->ArrayIndex)
				pHouse->ProducingBuildingTypeIndex = -1;
		}
		PlayConstructionYardAnim<true>(pFactory);
		return BuildSucceeded;
	}

	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
		if (!pBuildingType->PowersUpBuilding[0])
		{
			bool noOccupy = true;
			bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
			const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
			auto& place = pBuildingType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

			do
			{
				if (canBuild)
				{
					if (noOccupy)
						break; // Can Build

					do
					{
						if (topLeftCell != place.TopLeft || pBuildingType != place.Type) // New command
						{
							place.Type = pBuildingType;
							place.DrawType = pBuildingType;
							place.TopLeft = topLeftCell;
						}

						if (!place.Timer.HasTimeLeft())
						{
							place.Timer.Start(40);

							if (BuildingTypeExtData::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning
						}

						return TemporarilyCanNotBuild;
					}
					while (false);
				}

				ClearPlacingBuildingData(&place);
				return CanNotBuild;
			}
			while (false);

			ClearPlacingBuildingData(&place);
		}
		else
		{
			const auto pCell = MapClass::Instance->GetCellAt(topLeftCell);
			const auto pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !pCellBuilding->CanUpgrade(pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct{ (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		PlayConstructionYardAnim(pFactory);
		return CanBuild;
	}

	return CanNotBuild;
}

static inline bool CanDrawGrid(bool draw)
{
	if (ProximityTemp::Exist)
	{
		ProximityTemp::Exist = false;
		return false;
	}

	return draw;
}

// Laser fence use GetBuilding to check whether can build and draw, so no need to change
// Buildable-upon TechnoTypes Hook #8-1 -> sub_6D5C50 - Don't draw overlay wall grid when have occupiers
ASMJIT_PATCH(0x6D5D38, TacticalClass_DrawOverlayWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5D40, Invalid = 0x6D5F0F };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-2 -> sub_6D59D0 - Don't draw firestorm wall grid when have occupiers
ASMJIT_PATCH(0x6D5A9D, TacticalClass_DrawFirestormWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5AA5, Invalid = 0x6D5C2F };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-3 -> sub_588750 - Don't place overlay wall when have occupiers
ASMJIT_PATCH(0x588873, MapClass_BuildingToWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58887B, Invalid = 0x588935 };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #8-4 -> sub_588570 - Don't place firestorm wall when have occupiers
ASMJIT_PATCH(0x588664, MapClass_BuildingToFirestormWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58866C, Invalid = 0x588730 };

	GET(bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #9-1 -> sub_7393C0 - Try to clean up the building space when is deploying
ASMJIT_PATCH(0x73946C, UnitClass_TryToDeploy_CleanUpDeploySpace, 0x6)
{
	enum { CanDeploy = 0x73958A, TemporarilyCanNotDeploy = 0x73953B, CanNotDeploy = 0x7394E0 };

	GET(UnitClass* const, pUnit, EBP);
	GET(CellStruct, topLeftCell, ESI);

	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pUnit);
	const auto pBuildingType = pUnit->Type->DeploysInto;
	const auto pHouseExt = HouseExtContainer::Instance.Find(pUnit->Owner);
	auto& vec = pHouseExt->OwnedDeployingUnits;

	if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
		topLeftCell -= CellStruct { 1, 1 };

	R->Stack<CellStruct>(STACK_OFFSET(0x28, -0x14), topLeftCell);

	if (!pBuildingType->PlaceAnywhere)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pUnit->Owner, noOccupy);

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can build

				do
				{
					if (pTechnoExt && !pTechnoExt->UnitAutoDeployTimer.InProgress())
					{
						if (BuildingTypeExtData::CleanUpBuildingSpace(pBuildingType, topLeftCell, pUnit->Owner, pUnit))
							break; // No place for cleaning

						if (vec.empty() || !vec.contains(pUnit))
							vec.push_back(pUnit);

						pTechnoExt->UnitAutoDeployTimer.Start(40);
					}

					return TemporarilyCanNotDeploy;
				}
				while (false);
			}

			if (!vec.empty())
				vec.remove(pUnit);

			if (pTechnoExt)
				pTechnoExt->UnitAutoDeployTimer.Stop();

			return CanNotDeploy;
		}
		while (false);
	}

	if (!vec.empty())
		vec.remove(pUnit);

	if (pTechnoExt)
		pTechnoExt->UnitAutoDeployTimer.Stop();

	return CanDeploy;
}

// Buildable-upon TechnoTypes Hook #9-2 -> sub_73FD50 - Push the owner house into deploy check
ASMJIT_PATCH(0x73FF8F, UnitClass_MouseOverObject_ShowDeployCursor, 0x6)
{
	if (RulesExtData::Instance()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(const UnitClass* const, pUnit, ESI);
		LEA_STACK(HouseClass**, pHousePtr, STACK_OFFSET(0x20, -0x20));
		*pHousePtr = pUnit->Owner;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #10 -> sub_4C6CB0 - Stop deploy when get stop command
ASMJIT_PATCH(0x4C7665, EventClass_RespondToEvent_StopDeployInIdleEvent, 0x6)
{
	if (RulesExtData::Instance()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(UnitClass*, pUnit, ESI);

		if (pUnit->Type->DeploysInto)
		{
			const auto mission = pUnit->CurrentMission;

			if (mission == Mission::Guard || mission == Mission::Unload)
			{
				if (const auto pHouseExt = HouseExtContainer::Instance.Find(pUnit->Owner))
				{
					auto& vec = pHouseExt->OwnedDeployingUnits;

					if (!vec.empty())
						vec.remove(pUnit);
				}
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #11 -> sub_4F8440 - Check whether can place again in each house
ASMJIT_PATCH(0x4F8DB1, HouseClass_Update_CheckHangUpBuilding, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	if (!pHouse->IsControlledByHuman())
		return 0;

	if (pHouse->RecheckTechTree || !(Unsorted::CurrentFrame() % 15))
	{
		if (const auto pFactory = pHouse->Primary_ForBuildings)
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = cast_to<BuildingClass*>(pFactory->Object))
					BuildingTypeExtData::AutoPlaceBuilding(pBuilding);
			}
		}

		if (const auto pFactory = pHouse->Primary_ForDefenses)
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = cast_to<BuildingClass*>(pFactory->Object))
					BuildingTypeExtData::AutoPlaceBuilding(pBuilding);
			}
		}
	}

	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	auto buildCurrent = [&pHouse, &pHouseExt](BuildingTypeClass* pType, CellStruct cell)
	{		
		if (!pType)
		 return;

		auto currentCanBuild = [&pHouse, &pType]() -> const bool
		{
			auto const bitsOwners = pType->GetOwners();

			for(auto const& pConYard : pHouse->ConYards)
			{
				if (pConYard->InLimbo || !pConYard->HasPower)
					continue;

				if (pConYard->CurrentMission == Mission::Selling || pConYard->QueuedMission == Mission::Selling)
					continue;

				auto const pType = pConYard->Type;

				if (pType->Factory != AbstractType::Building || !pType->InOwners(bitsOwners))
					continue;

				return true;
			}

			return false;
		};

		if (currentCanBuild()) // ShouldDisableCameo
		{
			ClearPlacingBuildingData(pType->BuildCat != BuildCat::Combat ? &pHouseExt->Common : &pHouseExt->Combat);

			if (pHouse == HouseClass::CurrentPlayer)
				VoxClass::Play(GameStrings::EVA_CannotDeployHere);
		}
		else if (pHouse == HouseClass::CurrentPlayer) // Prevent unexpected wrong event
		{
			const EventClass event (pHouse->ArrayIndex, EventType::PLACE, AbstractType::Building, pType->GetArrayIndex(), pType->Naval, cell);
			EventClass::AddEvent(&event);
		}
	};

	if (pHouseExt->Common.Timer.Completed()) {
		pHouseExt->Common.Timer.Stop();
		buildCurrent(pHouseExt->Common.Type, pHouseExt->Common.TopLeft);
	}

	if (pHouseExt->Combat.Timer.Completed()) {
		pHouseExt->Combat.Timer.Stop();
		buildCurrent(pHouseExt->Common.Type, pHouseExt->Common.TopLeft);
	}

	pHouseExt->OwnedDeployingUnits.remove_all_if([pHouse](UnitClass* pUnit){
		if (!pUnit->InLimbo && pUnit->IsOnMap && !pUnit->IsSinking && pUnit->Owner == pHouse && !pUnit->Destination && pUnit->CurrentMission == Mission::Guard && !pUnit->ParasiteEatingMe && !pUnit->TemporalTargetingMe) {
			if (const auto pType = pUnit->Type) {
				if (pType->DeploysInto) {
					if (const auto pExt = TechnoExtContainer::Instance.Find(pUnit)) {
						if (!(pExt->UnitAutoDeployTimer.GetTimeLeft() % 8))
							pUnit->QueueMission(Mission::Unload, true);

						return false;
					}
				}
			}
		}

		return true;
	});

	return 0;
}

ASMJIT_PATCH(0x4A946E, DisplayClass_PreparePassesProximityCheck_ReplaceBuildingType, 0x6)
{
	const auto pDisplay = DisplayClass::Instance();

	if (const auto pAnotherType = GetAnotherPlacingType(pDisplay))
	{
		if (pDisplay->CurrentBuildingType && pDisplay->CurrentBuildingType != pAnotherType)
		{
			pDisplay->CurrentBuildingType = pAnotherType;
			pDisplay->SetActiveFoundation(pAnotherType->GetFoundationData(true));
		}
	}
	else if (const auto pCurrentBuilding = cast_to<BuildingClass*>(pDisplay->CurrentBuilding))
	{
		if (pDisplay->CurrentBuildingType && pDisplay->CurrentBuildingType != pCurrentBuilding->Type)
		{
			pDisplay->CurrentBuildingType = pCurrentBuilding->Type;
			pDisplay->SetActiveFoundation(pCurrentBuilding->Type->GetFoundationData(true));
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x4ABA47, DisplayClass_PreparePassesProximityCheck_ReplaceBuildingType, 0x6)

void drawImage(BuildingTypeClass* const pType, HouseClass* const pHouse, const CellStruct cell)
{
	const auto pDisplay = DisplayClass::Instance();
	const auto pCell = pDisplay->TryGetCellAt(cell);

	if (!pCell || cell == CellStruct::Empty)
		return;

	auto pImage = pType->LoadBuildup();
	int imageFrame = 0;

	if (pImage)
		imageFrame = ((pImage->Frames / 2) - 1);
	else
		pImage = pType->GetImage();

	if (!pImage)
		return;

	constexpr BlitterFlags blitFlags = BlitterFlags::TransLucent25 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
	auto rect = DSurface::Temp->Get_Rect();
	rect.Height -= 32;
	CoordStruct coord_ = CellClass::Cell2Coord(cell, (1 + pCell->GetFloorHeight(Point2D::Empty)));
	auto point = TacticalClass::Instance->CoordsToView(coord_);
	//point.Y -= 15;

	if (point.X < 0 || point.Y < 0 || point.X > rect.Width || point.Y > rect.Height) return;

	const auto ColorSchemeIndex = pHouse->ColorSchemeIndex;
	const auto Palettes = pType->Palette;
	const auto pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array->Items[ColorSchemeIndex];
	const COLORREF foreColor = pHouse->Color.ToInit();
	DSurface::Temp->DrawText_Old(pType->UIName, &point, foreColor);
	DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
}

// Buildable-upon TechnoTypes Hook #12 -> sub_6D5030 - Draw the placing building preview
ASMJIT_PATCH(0x6D504C, TacticalClass_DrawPlacement_DrawPlacingPreview, 0x6)
{
	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	const auto pPlayer = HouseClass::CurrentPlayer();
	const auto pDisplay = DisplayClass::Instance();

	for (const auto& pHouse : *HouseClass::Array)
	{
		if (pPlayer->IsObserver() || pHouse->IsAlliedWith(pPlayer))
		{
			const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

			if (const auto pType = cast_to<BuildingTypeClass*>(pDisplay->CurrentBuildingTypeCopy))
				drawImage(pType, pHouse, (pDisplay->CurrentFoundationCopy_TopLeftOffset + pDisplay->CurrentFoundationCopy_CenterCell));

			if (const auto pType = pHouseExt->Common.Type)
				drawImage(pType, pHouse, pHouseExt->Common.TopLeft);

			if (const auto pType = pHouseExt->Combat.Type)
				drawImage(pType, pHouse, pHouseExt->Combat.TopLeft);

			if (pHouseExt->OwnedDeployingUnits.size() <= 0)
				continue;

			for (const auto& pUnit : pHouseExt->OwnedDeployingUnits)
			{
				const auto pType = pUnit->Type->DeploysInto;

				if (!pType)
					continue;

				auto displayCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords();

				if (pType->GetFoundationWidth() > 2 || pType->GetFoundationHeight(false) > 2)
					displayCell -= CellStruct { 1, 1 };

				drawImage(pType, pHouse, displayCell);
			}
		}
	}


	return 0;
}

// Auto Build Hook -> sub_6A8B30 - Auto Build Buildings
ASMJIT_PATCH(0x6A8E34, StripClass_Update_AutoBuildBuildings, 0x7)
{
	enum { SkipSetStripShortCut = 0x6A8E4D };

	GET(BuildingClass* const, pBuilding, ESI);
	BuildingTypeExtData::BuildLimboBuilding(pBuilding);
	BuildingTypeExtData::AutoPlaceBuilding(pBuilding);

	return 0;
}

// Limbo Build Hook -> sub_42EB50 - Check Base Node
ASMJIT_PATCH(0x42EB8E, BaseClass_GetBaseNodeIndex_CheckValidBaseNode, 0x6)
{
	enum { Valid = 0x42EBC3, Invalid = 0x42EBAE };

	GET(BaseClass* const, pBase, ESI);
	GET(BaseNodeClass* const, pBaseNode, EAX);

	if (pBaseNode->Placed)
	{
		const auto index = pBaseNode->BuildingTypeIndex;

		if (index >= 0 && index < BuildingTypeClass::Array->Count)
		{
			const auto pType = BuildingTypeClass::Array->Items[index];

			if (RulesExtData::Instance()->AIForbidConYard || BuildingTypeExtContainer::Instance.Find(pType)->LimboBuild)
				return Invalid;
		}
	}

	return pBase->Owner->ai_replace_node_50CAD0(pBaseNode) ? Valid : Invalid;
}

// Customized Laser Fence Hook #2 -> sub_453060 - Select the specific laser fence type
ASMJIT_PATCH(0x452E2C, BuildingClass_CreateLaserFence_FindSpecificIndex, 0x5)
{
	enum { SkipGameCode = 0x452E50 };

	GET(BuildingClass* const, pThis, EDI);

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (const auto pFenceType = pExt->LaserFencePost_Fence.Get())
	{
		if (pFenceType->LaserFence)
		{
			R->EBP(pFenceType->ArrayIndex);
			R->EAX(BuildingTypeClass::Array->Count);
			return SkipGameCode;
		}
	}

	return 0;
}

// Customized Laser Fence Hook #3 -> sub_440580 - Skip uninit inappropriate laser fence
ASMJIT_PATCH(0x440AE9, BuildingClass_Unlimbo_SkipUninitFence, 0x7)
{
	enum { SkipGameCode = 0x440B07 };

	GET(BuildingClass* const, pFence, EDI);
	GET(BuildingClass* const, pThis, ESI);

	return IsSameFenceType(pThis->Type, pFence->Type) ? 0 : SkipGameCode;
}

static inline bool IsMatchedPostType(BuildingTypeClass* pThisType, BuildingTypeClass* pPostType)
{
	if (pThisType) {
		const auto pThisTypeExt = BuildingTypeExtContainer::Instance.Find(pThisType);
		if (pPostType) {
			const auto pPostTypeExt = BuildingTypeExtContainer::Instance.Find(pPostType);

			if (pThisTypeExt->LaserFencePost_Fence.Get() != pPostTypeExt->LaserFencePost_Fence.Get())
				return false;
		}
	}

	return true;
}

// Customized Laser Fence Hook #4 -> sub_452BB0 - Only accept specific fence post
ASMJIT_PATCH(0x452CB4, BuildingClass_FindLaserFencePost_CheckLaserFencePost, 0x7)
{
	enum { SkipGameCode = 0x452D2C };

	GET(BuildingClass* const, pPost, ESI);
	GET(BuildingClass* const, pThis, EDI);

	return IsMatchedPostType(pThis->Type, pPost->Type) ? 0 : SkipGameCode;
}

// Customized Laser Fence Hook #5 -> sub_6D5730 - Break draw inappropriate laser fence grids
ASMJIT_PATCH(0x6D5815, TacticalClass_DrawLaserFenceGrid_SkipDrawLaserFence, 0x6)
{
	enum { SkipGameCode = 0x6D59A6 };

	GET(BuildingTypeClass* const, pPostType, ECX);

	// Have used CurrentBuilding->Type yet, so simply use static_cast
	return IsMatchedPostType(static_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding)->Type, pPostType) ? 0 : SkipGameCode;
}
