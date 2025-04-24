/*
*	NPExt TODO :
*	IsNormalPlane -> DescendProximity , AscentSpeed
*/

static KickOutResult SendParaProduction(BuildingClass* pBld, FootClass* pFoot, CoordStruct* pCoord)
{
	++Unsorted::ScenarioInit;
	auto const pPlane = static_cast<AircraftClass*>(HouseExtData::GetParadropPlane(pBld->Owner)->CreateObject(pBld->Owner));
	--Unsorted::ScenarioInit;

	auto pOwner = pBld->Owner;

	if (!pPlane)
	{
		return KickOutResult::Failed;
	}

	pPlane->Spawned = true;

	//Get edge (direction for plane to come from)
	auto edge = pOwner->GetHouseEdge();

	// seems to retrieve a random cell struct at a given edge
	auto const spawn_cell = MapClass::Instance->PickCellOnEdge(
		edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true,
		MovementZone::Normal);

	pPlane->QueueMission(Mission::ParadropApproach, false);

	auto const bSpawned = AircraftExt::PlaceReinforcementAircraft(pPlane, spawn_cell);

	if (!bSpawned)
	{
		GameDelete<true, false>(pPlane);
		return KickOutResult::Failed;
	}

	auto pTarget = MapClass::Instance->GetCellAt(pCoord);
	auto pType = pFoot->GetTechnoType();

	// find the nearest cell the paradrop troopers can land on
		// the movement zone etc is checked within first types of the passanger
	CellClass* pDest = pTarget;
	bool allowBridges = GroundType::GetCost(LandType::Clear, pType->SpeedType) > 0.0;
	bool isBridge = allowBridges && pDest->ContainsBridge();

	while (!pDest->IsClearToMove(pType->SpeedType, 0, 0, ZoneType::None, pType->MovementZone, -1, isBridge))
	{
		pDest = MapClass::Instance->GetCellAt(
			MapClass::Instance->NearByLocation(
				pDest->MapCoords,
				pType->SpeedType,
				ZoneType::None,
				pType->MovementZone, isBridge, 1, 1, true, false, false, isBridge, CellStruct::Empty, false, false));

		isBridge = allowBridges && pDest->ContainsBridge();
	}

	pTarget = pDest;

	pPlane->SetTarget(pTarget);

	//only allow infantry and vehicles
	auto const abs = pType->WhatAmI();
	if (abs == AbstractType::UnitType || abs == AbstractType::InfantryType)
	{
		auto const pNew = static_cast<FootClass*>(pType->CreateObject(pOwner));

		if (!pNew)
		{
			++Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}

		pNew->SetLocation(pPlane->Location);
		pNew->Limbo();

		if (pPlane->Type->OpenTopped)
		{
			pPlane->EnteredOpenTopped(pNew);
		}

		pNew->Transporter = pPlane;
		pPlane->AddPassenger(static_cast<FootClass*>(pNew));
	}

	pPlane->HasPassengers = true;
	pPlane->NextMission();

	return KickOutResult::Succeeded;
}

// always fail need to  retry it until not
// the coords seems not suitable
static KickOutResult SendDroppodProduction(BuildingClass* pBld, FootClass* pFoot, CoordStruct* pCoord)
{
	if (!TechnoExtData::CreateWithDroppod(pFoot, *pCoord))
		return KickOutResult::Failed;

	return KickOutResult::Succeeded;
}

//ASMJIT_PATCH(0x444565 , BuildingClass_ExitObject_NonNavalUnit_Test, 0x6)
//{
//	GET(BuildingClass* , pThis , ESI);
//	GET(FootClass* , pProduct , EDI);
//
//	CoordStruct coord {};
//	pThis->GetExitCoords(&coord ,0u);
//
//	if(SendDroppodProduction(pThis , pProduct , &coord ) == KickOutResult::Succeeded){
//		TechnoExt_ExtData::KickOutClones(pThis, pProduct);
//		++Unsorted::ScenarioInit;
//		return 0x444971;
//	}
//
//	++Unsorted::ScenarioInit;
//	return 0x444EDE;
//}

//ASMJIT_PATCH(0x6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
//{
//	GET(AircraftTypeClass* const, pType, ECX);
//	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
//
//	if (pExt->IsCustomMissile) {
//		R->EAX(pExt->CustomMissileData.operator->());
//		return 0x66351B;
//	}
//
//	return 0;
//}
// 

/* TODO : Aircraft , Building , Changes
int ExitObject(BuildingClass* pThis, TechnoClass* pTechnoToKick, CellStruct overrider)
{
	if (!pTechnoToKick)
	{
		return 0;
	}

	pTechnoToKick->IsTethered = true;
	auto pBuildingType = pThis->Type;

	switch (pTechnoToKick->WhatAmI())
	{
	case AbstractType::Unit:
	{
		if (!pBuildingType->Hospital &&
			!pBuildingType->Armory &&
			!pBuildingType->WeaponsFactory &&
			!pThis->HasAnyLink()
			)
		{
			return 1;
		}

		if (!pBuildingType->Hospital && !pBuildingType->Armory)
		{
			pThis->Owner->WhimpOnMoney(AbstractType::Unit);
			pThis->Owner->ProducingUnitTypeIndex = -1;
		}

		if (pBuildingType->Refinery || pBuildingType->Weeder)
		{
			auto _coord = pThis->GetCoords();
			auto _coord_cell = CellClass::Coord2Cell(_coord);
			_coord_cell.X += CellSpread::AdjacentCell[5].X;
			_coord_cell.Y += CellSpread::AdjacentCell[5].Y;

			_coord_cell.X += CellSpread::AdjacentCell[4].X;
			_coord_cell.Y += CellSpread::AdjacentCell[4].Y;

			_coord = CellClass::Cell2Coord(_coord_cell);

			++Unsorted::ScenarioInit;

			if (pTechnoToKick->Unlimbo(_coord, DirType::SouthWest))
			{
				DirStruct _pri {};
				_pri.Raw = 0x8000;
				pTechnoToKick->PrimaryFacing.Set_Current(_pri);
				pTechnoToKick->QueueMission(Mission::Harvest, false);
			}

			--Unsorted::ScenarioInit;
			return 0;
		}

		if (!pBuildingType->WeaponsFactory)
		{
			if (pBuildingType->Hospital || pBuildingType->Armory || pBuildingType->Cloning)
			{
				pTechnoToKick->SetArchiveTarget(pThis->ArchiveTarget);
				auto docked_ = pThis->FindBuildingExitCell(pTechnoToKick, overrider);

				if (!docked_.IsValid())
					return 0;

				auto _coord = pThis->GetCoords();
				auto _cellcoord = CellClass::Cell2Coord(docked_);
				DirStruct _dir { double(_coord.Y - _cellcoord.Y) , double(_coord.X - _cellcoord.X) };
				CellStruct _copy = docked_;
				auto cell = pThis->GetMapCoords();
				if (_copy.X < (cell.X + pBuildingType->GetFoundationWidth()) && (_copy.X < cell.X))
				{
					_copy.X += 1;
				}
				else
				{
					_copy.X -= 1;
				}

				if (_copy.Y < (cell.Y + pBuildingType->GetFoundationHeight(false)) && (_copy.Y < cell.Y))
				{
					_copy.Y += 1;
				}
				else
				{
					_copy.Y -= 1;
				}

				_cellcoord = CellClass::Cell2Coord(_copy);

				++Unsorted::ScenarioInit;

				if (pTechnoToKick->Unlimbo(_cellcoord, _dir.GetDir()))
				{
					auto pToKickType = pTechnoToKick->GetTechnoType();

					if (pTechnoToKick->ArchiveTarget
					  && !pToKickType->JumpJet
					  && !pToKickType->Teleporter)
					{
						if (pTechnoToKick->ArchiveTarget)
						{
							pTechnoToKick->SetArchiveTarget(pTechnoToKick->ArchiveTarget);
						}

						pTechnoToKick->QueueMission(Mission::Move, false);
						pTechnoToKick->SetDestination(MapClass::Instance->GetCellAt(docked_), true);
					}

					if (!pThis->Owner->IsControlledByHuman() || pBuildingType->Hospital)
					{
						pTechnoToKick->QueueMission(Mission::Area_Guard, false);
						CellStruct Where {};
						pThis->Owner->WhereToGo(&Where, pTechnoToKick);

						if (!Where.IsValid())
						{
							pTechnoToKick->SetArchiveTarget(nullptr);
						}
						else
						{
							auto pDest = MapClass::Instance->GetCellAt(Where);
							pTechnoToKick->SetArchiveTarget(pDest);
							((FootClass*)pTechnoToKick)->QueueNavList(pDest);
						}
					}

					if (pThis->SendCommand(RadioCommand::RequestLink, pTechnoToKick) == RadioCommand::AnswerPositive)
					{
						pThis->SendCommand(RadioCommand::RequestUnload, pTechnoToKick);
					}

					--Unsorted::ScenarioInit;
					return 2;
				}

				--Unsorted::ScenarioInit;
				return 0;
			}
			else
			{
				auto docked_ = pThis->FindBuildingExitCell(pTechnoToKick, overrider);

				if (!docked_.IsValid())
					return 0;

				auto _coord = pThis->GetCoords();
				auto _cellcoord = CellClass::Cell2Coord(docked_);
				DirStruct _dir { double(_coord.Y - _cellcoord.Y) , double(_coord.X - _cellcoord.X) };
				CellStruct _copy = docked_;
				auto cell = pThis->GetMapCoords();
				if (_copy.X < (cell.X + pBuildingType->GetFoundationWidth()) && (_copy.X < cell.X))
				{
					_copy.X += 1;
				}
				else
				{
					_copy.X -= 1;
				}

				if (_copy.Y < (cell.Y + pBuildingType->GetFoundationHeight(false)) && (_copy.Y < cell.Y))
				{
					_copy.Y += 1;
				}
				else
				{
					_copy.Y -= 1;
				}

				auto _cellcoord_res = CellClass::Cell2Coord(_copy);

				++Unsorted::ScenarioInit;

				if (pTechnoToKick->Unlimbo(_cellcoord_res, _dir.GetDir()))
				{
					pTechnoToKick->QueueMission(Mission::Move, false);
					pTechnoToKick->SetDestination(MapClass::Instance->GetCellAt(docked_), true);

					if (!pThis->Owner->IsControlledByHuman())
					{
						pTechnoToKick->QueueMission(Mission::Area_Guard, false);
						CellStruct Where {};
						pThis->Owner->WhereToGo(&Where, pTechnoToKick);

						if (!Where.IsValid())
						{
							pTechnoToKick->SetArchiveTarget(nullptr);
						}
						else
						{
							auto pDest = MapClass::Instance->GetCellAt(Where);
							pTechnoToKick->SetArchiveTarget(pDest);
						}
					}

					--Unsorted::ScenarioInit;
					return 2;
				}

				--Unsorted::ScenarioInit;
				return 0;
			}
		}

		if (!pBuildingType->Naval)
		{
			pTechnoToKick->SetArchiveTarget(pThis->ArchiveTarget);

			if (pThis->GetMission() == Mission::Unload)
			{
				const int _bldCount = pThis->Owner->Buildings.Count;

				if (_bldCount <= 0)
					return 1;

				for (int i = 0; i < _bldCount; ++i)
				{
					if (pThis->Owner->Buildings[i]->Type == pBuildingType && pThis->Owner->Buildings[i] != pThis)
					{
						if (pThis->Owner->Buildings[i]->GetMission() == Mission::Guard && pThis->Owner->Buildings[i]->Factory)
						{
							pThis->Owner->Buildings[i]->Factory = std::exchange(pThis->Factory, nullptr);
							return (int)pThis->Owner->Buildings[i]->KickOutUnit(pTechnoToKick, overrider);
						}
					}
				}

				return 1;
			}

			++Unsorted::ScenarioInit;
			CoordStruct docked_ {};
			pThis->GetExitCoords(&docked_, 0);

			if (pTechnoToKick->Unlimbo(docked_, DirType::East))
			{
				pTechnoToKick->Mark(MarkType::Remove);
				pTechnoToKick->SetLocation(docked_);
				pTechnoToKick->Mark(MarkType::Put);
				pThis->SendCommand(RadioCommand::RequestLink, pTechnoToKick);
				pThis->SendCommand(RadioCommand::RequestTether, pTechnoToKick);
				pThis->QueueMission(Mission::Unload, false);
				--Unsorted::ScenarioInit;
				return 2;
			}

			--Unsorted::ScenarioInit;
			return 0;
		}

		if (!pThis->HasAnyLink())
			pThis->QueueMission(Mission::Unload, false);
		auto _this_Mapcoord = pThis->GetMapCoords();
		auto _this_cell = MapClass::Instance->GetCellAt(_this_Mapcoord);

		if (pThis->ArchiveTarget)
		{
			auto _focus_coord = pThis->ArchiveTarget->GetCoords();
			auto _focus_coord_cell = CellClass::Coord2Cell(_focus_coord);
			DirStruct __face { double(_this_Mapcoord.Y - _focus_coord_cell.Y) ,  double(_this_Mapcoord.X - _focus_coord_cell.X) };

			auto v41 = MapClass::Instance->GetCellAt(_this_Mapcoord);
			int v40 = 0;
			if (v41->GetBuilding() == pThis)
			{
				auto v42 = &CellSpread::AdjacentCell[(int)__face.GetDir() & 7];
				CellClass* v44 = nullptr;

				do
				{
					v44 = MapClass::Instance->GetCellAt(CellStruct { short(_this_Mapcoord.X + v42->X) , short(_this_Mapcoord.Y + v42->Y) });
				}
				while (v44->GetBuilding() == pThis);
			}
		}

		if (!pThis->ArchiveTarget
			 || _this_cell->LandType != LandType::Water
			 || _this_cell->FindTechnoNearestTo(Point2D::Empty, false, nullptr)
			 || !MapClass::Instance->IsWithinUsableArea(_this_Mapcoord, true))
		{

			auto _near = MapClass::Instance->NearByLocation(_this_Mapcoord, pTechnoToKick->GetTechnoType()->SpeedType, -1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
			auto _near_cell = MapClass::Instance->GetCellAt(_near);
			auto _near_coord = _near_cell->GetCoords();

			if (pTechnoToKick->Unlimbo(_near_coord, DirType::East))
			{
				if (pThis->ArchiveTarget)
				{
					pTechnoToKick->SetDestination(pThis->ArchiveTarget, true);
					pTechnoToKick->QueueMission(Mission::Move, 0);
				}

				pTechnoToKick->Mark(MarkType::Remove);
				pTechnoToKick->SetLocation(_near_coord);
				pTechnoToKick->Mark(MarkType::Put);
				return 2;
			}
		}

		return 0;
	}
	case AbstractType::Infantry:
	{
		if (!pBuildingType->Hospital &&
			!pBuildingType->Armory &&
			!pBuildingType->WeaponsFactory &&
			!pThis->HasAnyLink()
			)
		{
			return 1;
		}

		if (!pBuildingType->Hospital && !pBuildingType->Armory)
		{
			pThis->Owner->WhimpOnMoney(AbstractType::Infantry);
			pThis->Owner->ProducingInfantryTypeIndex = -1;
		}

		if (pBuildingType->Refinery || pBuildingType->Weeder)
		{
			pTechnoToKick->Scatter(CoordStruct::Empty, true, false);
			return 0;
		}

		if (!pBuildingType->WeaponsFactory)
		{
			if (pBuildingType->Factory == AbstractType::InfantryType || pBuildingType->Hospital || pBuildingType->Armory || pBuildingType->Cloning)
			{
				pTechnoToKick->SetArchiveTarget(pThis->ArchiveTarget);
				auto docked_ = pThis->FindBuildingExitCell(pTechnoToKick, overrider);

				if (!docked_.IsValid())
					return 0;

				if (pBuildingType->Factory == AbstractType::InfantryType && !pBuildingType->Cloning)
				{
					for (auto i = 0; i < pThis->Owner->CloningVats.Count; ++i)
					{
						auto pTech = (TechnoClass*)pTechnoToKick->GetTechnoType()->CreateObject(pThis->Owner->CloningVats[i]->Owner);
						pThis->Owner->CloningVats[i]->KickOutUnit(pTech, CellStruct::Empty);
					}
				}

				auto _coord = pThis->GetCoords();
				auto _cellcoord = CellClass::Cell2Coord(docked_);
				DirStruct _dir { double(_coord.Y - _cellcoord.Y) , double(_coord.X - _cellcoord.X) };
				CellStruct _copy = docked_;
				auto cell = pThis->GetMapCoords();
				if (_copy.X < (cell.X + pBuildingType->GetFoundationWidth()) && (_copy.X < cell.X))
				{
					_copy.X += 1;
				}
				else
				{
					_copy.X -= 1;
				}

				if (_copy.Y < (cell.Y + pBuildingType->GetFoundationHeight(false)) && (_copy.Y < cell.Y))
				{
					_copy.Y += 1;
				}
				else
				{
					_copy.Y -= 1;
				}

				_cellcoord = CellClass::Cell2Coord(_copy);
				if (pBuildingType->GDIBarracks && docked_.X == cell.X + 1 && docked_.Y == cell.Y + 2)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				if (pBuildingType->NODBarracks && docked_.X == cell.X + 2 && docked_.Y == cell.Y + 2)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				if (pBuildingType->YuriBarracks && docked_.X == cell.X + 2 && docked_.Y == cell.Y + 1)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				++Unsorted::ScenarioInit;

				if (pTechnoToKick->Unlimbo(_cellcoord, _dir.GetDir()))
				{
					auto pToKickType = pTechnoToKick->GetTechnoType();

					if (pTechnoToKick->ArchiveTarget
					  && !pToKickType->JumpJet
					  && !pToKickType->Teleporter)
					{
						if (pTechnoToKick->ArchiveTarget)
						{
							pTechnoToKick->SetArchiveTarget(pTechnoToKick->ArchiveTarget);
						}

						pTechnoToKick->QueueMission(Mission::Move, false);
						pTechnoToKick->SetDestination(MapClass::Instance->GetCellAt(docked_), true);
					}

					if (!pThis->Owner->IsControlledByHuman() || pBuildingType->Hospital)
					{
						pTechnoToKick->QueueMission(Mission::Area_Guard, false);
						CellStruct Where {};
						pThis->Owner->WhereToGo(&Where, pTechnoToKick);

						if (!Where.IsValid())
						{
							pTechnoToKick->SetArchiveTarget(nullptr);
						}
						else
						{
							auto pDest = MapClass::Instance->GetCellAt(Where);
							pTechnoToKick->SetArchiveTarget(pDest);
							((FootClass*)pTechnoToKick)->QueueNavList(pDest);
						}
					}

					if (pThis->SendCommand(RadioCommand::RequestLink, pTechnoToKick) == RadioCommand::AnswerPositive)
					{
						pThis->SendCommand(RadioCommand::RequestUnload, pTechnoToKick);
					}

					--Unsorted::ScenarioInit;
					return 2;
				}

				--Unsorted::ScenarioInit;
				return 0;
			}
			else
			{
				auto docked_ = pThis->FindBuildingExitCell(pTechnoToKick, overrider);

				if (!docked_.IsValid())
					return 0;

				auto _coord = pThis->GetCoords();
				auto _cellcoord = CellClass::Cell2Coord(docked_);
				DirStruct _dir { double(_coord.Y - _cellcoord.Y) , double(_coord.X - _cellcoord.X) };
				CellStruct _copy = docked_;
				auto cell = pThis->GetMapCoords();
				if (_copy.X < (cell.X + pBuildingType->GetFoundationWidth()) && (_copy.X < cell.X))
				{
					_copy.X += 1;
				}
				else
				{
					_copy.X -= 1;
				}

				if (_copy.Y < (cell.Y + pBuildingType->GetFoundationHeight(false)) && (_copy.Y < cell.Y))
				{
					_copy.Y += 1;
				}
				else
				{
					_copy.Y -= 1;
				}

				auto _cellcoord_res = CellClass::Cell2Coord(_copy);
				if (pBuildingType->GDIBarracks && docked_.X == cell.X + 1 && docked_.Y == cell.Y + 2)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				if (pBuildingType->NODBarracks && docked_.X == cell.X + 2 && docked_.Y == cell.Y + 2)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				if (pBuildingType->YuriBarracks && docked_.X == cell.X + 2 && docked_.Y == cell.Y + 1)
				{
					_cellcoord += pBuildingType->ExitCoord;
				}

				++Unsorted::ScenarioInit;

				if (pTechnoToKick->Unlimbo(_cellcoord_res, _dir.GetDir()))
				{
					pTechnoToKick->QueueMission(Mission::Move, false);
					pTechnoToKick->SetDestination(MapClass::Instance->GetCellAt(docked_), true);

					if (!pThis->Owner->IsControlledByHuman())
					{
						pTechnoToKick->QueueMission(Mission::Area_Guard, false);
						CellStruct Where {};
						pThis->Owner->WhereToGo(&Where, pTechnoToKick);

						if (!Where.IsValid())
						{
							pTechnoToKick->SetArchiveTarget(nullptr);
						}
						else
						{
							auto pDest = MapClass::Instance->GetCellAt(Where);
							pTechnoToKick->SetArchiveTarget(pDest);
						}
					}

					--Unsorted::ScenarioInit;
					return 2;
				}

				--Unsorted::ScenarioInit;
				return 0;
			}
		}

		if (!pBuildingType->Naval)
		{
			pTechnoToKick->SetArchiveTarget(pThis->ArchiveTarget);

			if (pThis->GetMission() == Mission::Unload)
			{
				const int _bldCount = pThis->Owner->Buildings.Count;

				if (_bldCount <= 0)
					return 1;

				for (int i = 0; i < _bldCount; ++i)
				{
					if (pThis->Owner->Buildings[i]->Type == pBuildingType && pThis->Owner->Buildings[i] != pThis)
					{
						if (pThis->Owner->Buildings[i]->GetMission() == Mission::Guard && pThis->Owner->Buildings[i]->Factory)
						{
							pThis->Owner->Buildings[i]->Factory = std::exchange(pThis->Factory, nullptr);
							return (int)pThis->Owner->Buildings[i]->KickOutUnit(pTechnoToKick, overrider);
						}
					}
				}

				return 1;
			}

			++Unsorted::ScenarioInit;
			CoordStruct docked_ {};
			pThis->GetExitCoords(&docked_, 0);

			if (pTechnoToKick->Unlimbo(docked_, DirType::East))
			{
				pTechnoToKick->Mark(MarkType::Remove);
				pTechnoToKick->SetLocation(docked_);
				pTechnoToKick->Mark(MarkType::Put);
				pThis->SendCommand(RadioCommand::RequestLink, pTechnoToKick);
				pThis->SendCommand(RadioCommand::RequestTether, pTechnoToKick);
				pThis->QueueMission(Mission::Unload, false);
				--Unsorted::ScenarioInit;
				return 2;
			}

			--Unsorted::ScenarioInit;
			return 0;
		}

		if (!pThis->HasAnyLink())
			pThis->QueueMission(Mission::Unload, false);
		auto _this_Mapcoord = pThis->GetMapCoords();
		auto _this_cell = MapClass::Instance->GetCellAt(_this_Mapcoord);

		if (pThis->ArchiveTarget)
		{
			auto _focus_coord = pThis->ArchiveTarget->GetCoords();
			auto _focus_coord_cell = CellClass::Coord2Cell(_focus_coord);
			DirStruct __face { double(_this_Mapcoord.Y - _focus_coord_cell.Y) ,  double(_this_Mapcoord.X - _focus_coord_cell.X) };

			auto v41 = MapClass::Instance->GetCellAt(_this_Mapcoord);
			int v40 = 0;
			if (v41->GetBuilding() == pThis)
			{
				auto v42 = &CellSpread::AdjacentCell[(int)__face.GetDir() & 7];
				CellClass* v44 = nullptr;

				do
				{
					v44 = MapClass::Instance->GetCellAt(CellStruct { short(_this_Mapcoord.X + v42->X) , short(_this_Mapcoord.Y + v42->Y) });
				}
				while (v44->GetBuilding() == pThis);
			}
		}

		if (!pThis->ArchiveTarget
			 || _this_cell->LandType != LandType::Water
			 || _this_cell->FindTechnoNearestTo(Point2D::Empty, false, nullptr)
			 || !MapClass::Instance->IsWithinUsableArea(_this_Mapcoord, true))
		{

			auto _near = MapClass::Instance->NearByLocation(_this_Mapcoord, pTechnoToKick->GetTechnoType()->SpeedType, -1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
			auto _near_cell = MapClass::Instance->GetCellAt(_near);
			auto _near_coord = _near_cell->GetCoords();

			if (pTechnoToKick->Unlimbo(_near_coord, DirType::East))
			{
				if (pThis->ArchiveTarget)
				{
					pTechnoToKick->SetDestination(pThis->ArchiveTarget, true);
					pTechnoToKick->QueueMission(Mission::Move, 0);
				}

				pTechnoToKick->Mark(MarkType::Remove);
				pTechnoToKick->SetLocation(_near_coord);
				pTechnoToKick->Mark(MarkType::Put);
				return 2;
			}
		}
		return 0;
	}
	case AbstractType::Aircraft:
	{
		pThis->Owner->WhimpOnMoney(AbstractType::Aircraft);
		pThis->Owner->ProducingAircraftTypeIndex = -1;
		auto air = static_cast<AircraftClass*>(pTechnoToKick);

		if (pThis->HasFreeLink(pTechnoToKick) || pThis->Owner->IonSensitivesShouldBeOffline() && !air->Type->AirportBound)
		{

			pTechnoToKick->MarkDownSetZ(0);

			++Unsorted::ScenarioInit;
			if (pThis->Owner->IonSensitivesShouldBeOffline())
			{
				CellStruct v14 {};
				pTechnoToKick->NearbyLocation(&v14, pThis);
				auto coord_v14 = CellClass::Cell2Coord(v14);
				auto pose_dir = RulesClass::Instance->PoseDir;

				if (pTechnoToKick->Unlimbo(coord_v14, (DirType)pose_dir))
				{
					--Unsorted::ScenarioInit;
					return 2;
				}
			}
			else
			{
				auto pose_dir = RulesClass::Instance->PoseDir;
				auto DockCoord = pThis->GetDockCoords(pTechnoToKick);

				if (pTechnoToKick->Unlimbo(DockCoord, (DirType)pose_dir))
				{
					pThis->SendCommand(RadioCommand::RequestLink, pTechnoToKick);
					pThis->SendCommand(RadioCommand::RequestTether, pTechnoToKick);
					pTechnoToKick->SetLocation(DockCoord);
					air->DockedTo = pThis;

					if (pThis->ArchiveTarget && !air->Type->AirportBound)
					{
						air->SetDestination(pThis->ArchiveTarget, true);
						air->QueueMission(Mission::Move, false);
					}

					--Unsorted::ScenarioInit;
					return 2;
				}
			}
		}
		else
		{
			if (air->Type->AirportBound)
			{
				return 0;
			}
			static COMPILETIMEEVAL reference<RectangleStruct, 0x87F8E4> MapClass_MapLocalSize {};

			CellStruct v211 {};
			CellStruct v206 {};
			CellStruct v215 {};
			CellStruct v216 {};
			CellStruct v205 {};

			v211.X = MapClass_MapLocalSize->Y;
			v211.Y = MapClass_MapLocalSize->Y;
			v206.Y = MapClass_MapLocalSize->Width;
			v215.X = MapClass_MapLocalSize->X;
			v215.Y = -MapClass_MapLocalSize->X;
			v216.X = MapClass_MapLocalSize->X + 1;
			v216.Y = MapClass_MapLocalSize->Width - MapClass_MapLocalSize->X;
			v206.X = MapClass_MapLocalSize->Y + MapClass_MapLocalSize->X + 1;
			v206.Y = MapClass_MapLocalSize->Width - MapClass_MapLocalSize->X + MapClass_MapLocalSize->Y;
			auto mapCoord_ = pThis->GetCoords();
			auto mapCoord_cell = CellClass::Coord2Cell(mapCoord_);
			++Unsorted::ScenarioInit;

			if ((((mapCoord_cell.X - v206.X) - (mapCoord_cell.Y) - v206.Y)) <= MapClass_MapLocalSize->Width)
			{
				--v206.X;
			}
			else
			{
				v216.X = MapClass_MapLocalSize->Width;
				v215.Y = v206.Y;
				v216.Y = -MapClass_MapLocalSize->Width;
				v215.X = v206.X - 1;
				v206.X = MapClass_MapLocalSize->Width + v206.X - 1;
				v206.Y = v206.Y - MapClass_MapLocalSize->Width;
			}

			auto v9 = ScenarioClass::Instance->Random.RandomFromMax(MapClass_MapLocalSize->Height);
			v205.X = v9 + v206.X;
			v205.Y = v9 + v206.Y;
			auto v205_coord = CellClass::Cell2Coord(v205);

			if (pTechnoToKick->Unlimbo(v205_coord, DirType::Min))
			{
				if (auto pFocus = pThis->ArchiveTarget)
				{
					pTechnoToKick->SetDestination(pFocus, true);
				}
				else
				{
					CellStruct v230 {};
					air->NearbyLocation(&v230, pThis);

					if (!v230.IsValid())
					{
						pTechnoToKick->SetDestination(nullptr, true);
					}
					else
					{
						pTechnoToKick->SetDestination(MapClass::Instance->GetCellAt(v230), true);
					}
				}

				pTechnoToKick->ForceMission(Mission::Move);
				--Unsorted::ScenarioInit;
				return 2;
			}
		}

		// ????????
		--Unsorted::ScenarioInit;
		return  0;
	}
	case AbstractType::Building:
	{

		if (pThis->Owner->IsControlledByHuman())
		{
			return 0;
		}

		auto bld = static_cast<BuildingClass*>(pTechnoToKick);
		pThis->Owner->WhimpOnMoney(AbstractType::Building);
		pThis->Owner->ProducingBuildingTypeIndex = -1;

		const auto node = pThis->Owner->Base.NextBuildable(bld->Type->ArrayIndex);
		CoordStruct build_Coord {};

		if (node && node->MapCoords.IsValid())
		{
			if (bld->Type->PowersUpBuilding[0] || pThis->Owner->HasSpaceFor(bld->Type, &node->MapCoords))
			{
				build_Coord = CellClass::Cell2Coord(node->MapCoords);
			}
			else
			{
				CellStruct _loc {};
				pThis->Owner->FindBuildLocation(&_loc, bld->Type, *reinterpret_cast<HouseClass::placement_callback*>(0x505F80), -1);
				build_Coord = CellClass::Cell2Coord(_loc);

				if (build_Coord.IsEmpty())
				{
					return 0;
				}

				node->MapCoords = _loc;
			}
		}
		else
		{
			if (bld->Type->PowersUpBuilding[0])
			{
				CellStruct v143 {};
				pThis->Owner->GetPoweups(&v143, bld->Type);

				if (v143.IsValid())
				{
					build_Coord = CellClass::Cell2Coord(v143);
				}
			}
			else
			{
				CellStruct _loc {};
				pThis->Owner->FindBuildLocation(&_loc, bld->Type, *reinterpret_cast<HouseClass::placement_callback*>(0x505F80), -1);
				build_Coord = CellClass::Cell2Coord(_loc);
			}

			if (node && build_Coord.IsValid())
			{
				node->MapCoords = CellClass::Coord2Cell(build_Coord);
			}
		}

		if (build_Coord.IsEmpty())
		{
			if (bld->Type->PowersUpBuilding[0])
			{
				if (pThis->Owner->Base.NextBuildable() == node)
				{
					int idx__ = pThis->Owner->Base.NextBuildableIdx(-1);

					if (idx__ < pThis->Owner->Base.BaseNodes.Count)
					{
						int last__ = pThis->Owner->Base.BaseNodes.Count - 1;
						pThis->Owner->Base.BaseNodes.Count = last__;
						if (idx__ < last__)
						{
							auto v194 = 4 * idx__;
							int v193 = 0;
							do
							{
								auto at_ = (BaseNodeClass*)&pThis->Owner->Base.BaseNodes[v194 + 4];
								auto v197 = (BaseNodeClass*)&pThis->Owner->Base.BaseNodes[v194];
								++v193;
								v194 += 4;
								*v197 = *at_;
							}
							while (v193 < pThis->Owner->Base.BaseNodes.Count);

						}
					}
				}
			}
			return 0;
		}

		auto built_cell = CellClass::Coord2Cell(build_Coord);
		if (int v147 = bld->Type->FlushPlacement(&built_cell, pThis->Owner))
		{
			auto v148 = v147 - 1;
			if (!v148)
			{
				auto v149 = pThis->Owner->Base.FailedToPlaceNode(node);

				if (SessionClass::Instance->GameMode != GameMode::Campaign)
				{
					if (v149 > RulesClass::Instance->MaximumBuildingPlacementFailures)
					{
						if (node)
						{
							int v150 = pThis->Owner->Base.BaseNodes.FindItemIndex(*node);

							auto v151 = &pThis->Owner->Base.BaseNodes;
							auto v152 = v151->Count;

							if (v150 < v152)
							{
								auto v153 = v152 - 1;
								auto v154 = v150;
								v151->Count = v153;

								if (v150 < v153)
								{
									auto v155 = 4 * v150;
									do
									{
										auto v157 = &v151->Items[v155 + 4];
										auto v158 = &v151->Items[v155];
										++v154;
										v155 += 4;
										*v158 = *v157;
										v158[1] = v157[1];
										v158[2] = v157[2];
										v158[3] = v157[3];
									}
									while (v154 < v151->Count);
								}
							}
						}
					}
				}
				return 1;
			}
			if (v148 != 1)
			{
				return 0;
			}
		}
		else if (pTechnoToKick->Unlimbo(build_Coord, DirType::Min))
		{
			auto SlaveManager = bld->SlaveManager;
			if (SlaveManager)
			{
				SlaveManager->Deploy2();
			}

			if (node)
			{
				if (bld->Type->ArrayIndex == pThis->Owner->ProducingBuildingTypeIndex)
				{
					pThis->Owner->ProducingBuildingTypeIndex = -1;
				}
			}

			if (bld->CurrentMission == Mission::None && bld->QueuedMission == Mission::Selling)
			{
				bld->NextMission();
			}


			if (bld->Type->FirestormWall) {
				MapClass::Instance->BuildingToFirestormWall(CellClass::Coord2Cell(build_Coord), pThis->Owner, bld->Type);
			}
			else
			{
				auto ToOverlay = bld->Type->ToOverlay;
				if (ToOverlay && ToOverlay->Wall) {
					MapClass::Instance->BuildingToWall(CellClass::Coord2Cell(build_Coord), pThis->Owner, bld->Type);
				}
			}

			if (bld->Type == RulesClass::Instance->WallTower)
			{
				auto v163 = pThis->Owner->Base.BaseNodes.FindItemIndex(*node);

				auto v165 = v163 + 1;
				auto v166 = pThis->Owner->Base.BaseNodes.Count;
				if (v163 + 1 < v166)
				{
					auto v167 = pThis->Owner->Base.BaseNodes.Items;
					for (auto i = (int*)&v167[4 * v165]; *i < 0 || !BuildingTypeClass::Array->Items[*i]->IsBaseDefense; i += 4)
					{
						if (++v165 >= v166)
						{
							return 2;
						}
					}
					 v167[4 * v165 + 1].MapCoords = CellClass::Coord2Cell(bld->Location);
				}
			}
			return 2;
		}

		if (!node) {
			return 0;
		}

		auto v171 = pThis->Owner->Base.BaseNodes.FindItemIndex(*node);
		auto v172 = BuildingTypeClass::Array->Items[node->BuildingTypeIndex];

		if (!v172->Wall && !v172->Gate)
		{
			v173 = this->t.House;
			v174 = 0;
			v175 = *(_DWORD*)&v211 / 256;
			v176 = v212 / 256;

			if (pThis->Owner->Base.BaseNodes.Count <= 0)
			{
				return 0;
			}

			int v177 = 0;
			do
			{
				v178 = v173->Base.Nodes.Vector_Item;
				v179 = LOWORD(v178[v177 + 1]) == (unsigned __int16)v175;
				v180 = (int)&v178[v177 + 1];
				if (v179 && *(_WORD*)(v180 + 2) == (_WORD)v176)
				{
					ActiveCount.X = 0;
					ActiveCount.Y = 0;
					*(_DWORD*)v180 = 0;
				}
				v173 = this->t.House;
				++v174;
				v177 += 4;
			}
			while (v174 < v173->Base.Nodes.ActiveCount);
			return 0;
		}
		v181 = &this->t.House->Base.Nodes;
		v182 = v181->ActiveCount;
		if (v171 < v182)
		{
			v183 = v182 - 1;
			v184 = v171;
			v181->ActiveCount = v183;
			if (v171 < v183)
			{
				v185 = 4 * v171;
				do
				{
					v186 = v181->Vector_Item;
					v187 = &v186[v185 + 4];
					v188 = &v186[v185];
					++v184;
					v185 += 4;
					*v188 = *v187;
					v188[1] = v187[1];
					v188[2] = v187[2];
					v188[3] = v187[3];
				}
				while (v184 < v181->ActiveCount);
				return 0;
			}
		}
		return 0;
	}
	default:
		return 0;
	}
}
*/
