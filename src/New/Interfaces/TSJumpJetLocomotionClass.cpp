#include "TSJumpJetLocomotionClass.h"

#include <Ext/Techno/Body.h>

#include <RulesClass.h>
#include <SuperClass.h>
#include <MapClass.h>
#include <HouseClass.h>
#include <AircraftTrackerClass.h>
#include <CellSpread.h>

TSJumpJetLocomotionClass::TSJumpJetLocomotionClass() :
	LocomotionClass {},
	JumpjetClimb {},
	JumpjetAcceleration {},
	JumpjetWobblesPerSecond {},
	JumpjetNoWobbles {},
	JumpjetWobbleDeviation {},
	JumpjetCloakDetectionRadius {},
	JumpjetCruiseHeight {},
	JumpjetTurnRate {},
	JumpjetSpeed {},

	HeadToCoord {},
	CurrentState {},
	Facing { RulesClass::Instance->TurnRate },
	CurrentSpeed {},
	TargetSpeed {},
	CurrentWobble {},
	FlightLevel {},
	IsLanding {},
	IsMoving {}
{

}

IFACEMETHODIMP TSJumpJetLocomotionClass::Link_To_Object(void* object)
{
	JumpjetTurnRate = RulesClass::Instance->TurnRate;
	JumpjetSpeed = RulesClass::Instance->Speed;
	JumpjetClimb = RulesClass::Instance->Climb;
	int height = RulesClass::Instance->CruiseHeight;
	height = std::max(height, 2 * 256);
	JumpjetCruiseHeight = height;
	JumpjetAcceleration = RulesClass::Instance->Acceleration;
	JumpjetWobblesPerSecond = RulesClass::Instance->WobblesPerSecond;
	JumpjetWobbleDeviation = RulesClass::Instance->WobbleDeviation;
	Facing.ROT = DirStruct((DirType)MinImpl(JumpjetTurnRate, 127));
	Facing.Set_Desired(DirStruct(0x4000));
	Facing.Set_Current(DirStruct(0x4000));
	JumpjetCloakDetectionRadius = 10;
	JumpjetNoWobbles = false;
	return LocomotionClass::Link_To_Object(object);
}

IFACEMETHODIMP_(bool) TSJumpJetLocomotionClass::Is_Moving()
{
	return IsMoving;
}

IFACEMETHODIMP_(Coordinate) TSJumpJetLocomotionClass::Destination()
{
	if (Is_Moving())
	{
		return HeadToCoord;
	}
	else
	{
		return CoordStruct::Empty;
	}
}

IFACEMETHODIMP_(bool) TSJumpJetLocomotionClass::Process()
{
	Layer layer = In_Which_Layer();

	if (Is_Moving() || Is_Moving_Now())
	{
		Movement_AI();
		if (!LinkedTo->IsAlive)
		{
			return false;
		}

		if (LightningStorm::IsActive())
		{
			if (CurrentState != TSJumpJetLocomotionClass::JumpjetState::GROUNDED)
			{
				auto result = LinkedTo->ReceiveDamage(&LinkedTo->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, true ,nullptr);
				if (result == DamageState::NowDead)
				{
					return false;
				}
			}
		}

		switch (CurrentState)
		{
		case TSJumpJetLocomotionClass::JumpjetState::GROUNDED:
			Process_Grounded();
			break;

		case TSJumpJetLocomotionClass::JumpjetState::ASCENDING:
			Process_Ascent();
			break;

		case TSJumpJetLocomotionClass::JumpjetState::HOVERING:
			Process_Hover();
			break;

		case TSJumpJetLocomotionClass::JumpjetState::CRUISING:
			Process_Cruise();
			break;

		case TSJumpJetLocomotionClass::JumpjetState::DESCENDING:
			Process_Descent();
			break;
		}

		if (LinkedTo->IsSelected && LinkedTo->Owner != HouseClass::CurrentPlayer())
		{
			if (MapClass::Instance->IsLocationShrouded(LinkedTo->Location) ||
					(ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar && MapClass::Instance->IsLocationFogged(LinkedTo->Location)))
			{
				LinkedTo->Deselect();
			}
		}

		if (!LinkedTo->DiscoveredByCurrentPlayer && LinkedTo->Owner != HouseClass::CurrentPlayer())
		{
			if (!MapClass::Instance->IsLocationShrouded(LinkedTo->Location))
			{
				LinkedTo->DiscoveredBy(HouseClass::CurrentPlayer());
			}
		}
	}

	if (In_Which_Layer() != layer)
	{
		MapClass::Logics->AddObject(LinkedTo , false);
	}

	return false;
}

IFACEMETHODIMP_(void) TSJumpJetLocomotionClass::Move_To(Coordinate to)
{
	if (HeadToCoord != CoordStruct::Empty && CurrentState != TSJumpJetLocomotionClass::JumpjetState::GROUNDED && IsLanding)
	{
		LinkedTo->UnmarkAllOccupationBits(HeadToCoord);
		IsLanding = false;
	}

	HeadToCoord = to;

	if (to != CoordStruct::Empty)
	{
		CellStruct cell = MapClass::Instance->NearByLocation(
							CellClass::Coord2Cell(to),
							GET_TECHNOTYPE(LinkedTo)->SpeedType,
							ZoneType::None,
							MovementZone::Fly,
							MapClass::Instance->GetCellAt(to)->ContainsBridge(),0,
							0, true,false ,false , false ,CellStruct::Empty, false , true
		);

		Coordinate free = Closest_Free_Spot(CellClass::Cell2Coord(cell));

		if (free != CoordStruct::Empty)
		{
			HeadToCoord = free;
			LinkedTo->Destination = MapClass::Instance->GetCellAt(free);
			IsMoving = true;
			if (CurrentState == TSJumpJetLocomotionClass::JumpjetState::DESCENDING)
			{
				CurrentState = TSJumpJetLocomotionClass::JumpjetState::ASCENDING;
				FlightLevel = JumpjetCruiseHeight;
			}
		}
	}
	else
	{
		IsMoving = false;
	}
}

IFACEMETHODIMP_(void) TSJumpJetLocomotionClass::Stop_Moving()
{
	if (IsMoving)
	{
		if (HeadToCoord != CoordStruct::Empty && CurrentState != TSJumpJetLocomotionClass::JumpjetState::GROUNDED && IsLanding)
		{
			LinkedTo->UnmarkAllOccupationBits(HeadToCoord);
			IsLanding = false;
		}
		CellStruct cell = CellClass::Coord2Cell(LinkedTo->Location);
		CellStruct nearby = MapClass::Instance->NearByLocation(
							cell,
							SpeedType::Track,
							ZoneType::None,
							MovementZone::Fly,
							false, 0,
							0, true, false, false, false, CellStruct::Empty, false, true
		);

		if (nearby != CellStruct::Empty)
		{
			Coordinate nearby_coord = CellClass::Cell2Coord(nearby);
			nearby_coord.Z = MapClass::Instance->GetCellFloorHeight(nearby_coord);
			if (MapClass::Instance->GetCellAt(nearby)->ContainsBridge())
			{
				nearby_coord.Z += CellClass::BridgeHeight;
			}
			Move_To(nearby_coord);
		}
		else
		{
			LinkedTo->ReceiveDamage(&LinkedTo->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
			HeadToCoord = CoordStruct::Empty;
		}
	}
}

IFACEMETHODIMP_(void) TSJumpJetLocomotionClass::Do_Turn(DirStruct Coordinate)
{
	LinkedTo->PrimaryFacing.Set_Current(Coordinate);
}

IFACEMETHODIMP_(HRESULT) TSJumpJetLocomotionClass::GetClassID(CLSID* pClassID)
{
	if (pClassID == nullptr)
	{
		return E_POINTER;
	}

	*pClassID = __uuidof(this);

	return S_OK;
}

IFACEMETHODIMP_(HRESULT) TSJumpJetLocomotionClass::Load(IStream* stream)
{
	HRESULT result = LocomotionClass::Internal_Load(this, stream);
	if (SUCCEEDED(result)) {
		new (this) TSJumpJetLocomotionClass(noinit_t());
	}

	return result;
}

IFACEMETHODIMP_(HRESULT) TSJumpJetLocomotionClass::Save(IStream* pStm, BOOL fClearDirty)
{
	return LocomotionClass::Internal_Save(this, pStm, fClearDirty);
}

IFACEMETHODIMP_(Layer) TSJumpJetLocomotionClass::In_Which_Layer()
{
	int height = LinkedTo->HeightAGL;
	if (!LinkedTo->OnBridge)
	{
		if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge() && height >= CellClass::BridgeHeight && !LinkedTo->IsFallingDown)
		{
			height -= CellClass::BridgeHeight;
		}
	}

	if (height == 0)
	{
		return Layer::Ground;
	}
	else if (height < JumpjetCruiseHeight)
	{
		return Layer::Air;
	}
	else
	{
		return Layer::Top;
	}
}

void TSJumpJetLocomotionClass::Process_Grounded()
{
	if (Is_Moving())
	{
		LinkedTo->SetSpeedPercentage(1.0);
		Facing.Set_Current(LinkedTo->PrimaryFacing.Current());
		CurrentSpeed = 0;
		TargetSpeed = 0;
		FlightLevel = JumpjetCruiseHeight;
		if (!LightningStorm::IsActive())
		{
			if (LinkedTo->LastFlightMapCoords == CellStruct::Empty)
			{
				AircraftTrackerClass::Instance->Add(LinkedTo);
			}

			CurrentState = TSJumpJetLocomotionClass::JumpjetState::ASCENDING;
		}
	}
}

void TSJumpJetLocomotionClass::Process_Ascent()
{
	int height = LinkedTo->HeightAGL;
	if (!LinkedTo->OnBridge)
	{
		if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge() && height >= CellClass::BridgeHeight)
		{
			height -= CellClass::BridgeHeight;
		}
	}

	CellStruct oldcell = LinkedTo->LastFlightMapCoords;
	CellStruct newcell = LinkedTo->GetMapCoords();

	if (newcell != oldcell)
	{
		AircraftTrackerClass::Instance->Update(LinkedTo, oldcell, newcell);
	}

	if (height >= FlightLevel)
	{
		CurrentState = TSJumpJetLocomotionClass::JumpjetState::HOVERING;
	}
	else if (height > FlightLevel / 4)
	{
		TargetSpeed = JumpjetSpeed;
		DirStruct _desired {};
		_desired.GetDirOver(&LinkedTo->Location, &HeadToCoord);
		Facing.Set_Desired(_desired);
	}
}

void TSJumpJetLocomotionClass::Process_Hover()
{
	if (Is_Moving())
	{
		Coordinate headto = HeadToCoord;
		Coordinate position = LinkedTo->Location;
		if (Point2D(headto.X, headto.Y) == Point2D(position.X, position.Y))
		{
			if (LinkedTo->Target == nullptr)
			{
				CurrentState = TSJumpJetLocomotionClass::JumpjetState::DESCENDING;
			}
		}
		else
		{
			DirStruct _desired {};
			_desired.GetDirOver(&LinkedTo->Location, &HeadToCoord);
			Facing.Set_Desired(_desired);
			CurrentState = TSJumpJetLocomotionClass::JumpjetState::CRUISING;
		}
	}
}

void TSJumpJetLocomotionClass::Process_Cruise()
{
	Coordinate position = LinkedTo->PositionCoord;

	CellStruct oldcell = LinkedTo->LastFlightMapCoords;
	CellStruct newcell = LinkedTo->GetMapCoords();

	if (newcell != oldcell)
	{
		AircraftTrackerClass::Instance->Update(LinkedTo, oldcell, newcell);
	}

	DirStruct _desired {};
	_desired.GetDirOver(&position, &HeadToCoord);
	Facing.Set_Desired(_desired);

	int distance = (int)Point2D(position.X, position.Y).DistanceFrom(Point2D(HeadToCoord.X, HeadToCoord.Y));
	if (distance < 20)
	{
		CurrentSpeed = 0;
		TargetSpeed = 0;
		position.X = HeadToCoord.X;
		position.Y = HeadToCoord.Y;
		bool down = LinkedTo->IsOnMap;
		LinkedTo->IsOnMap = false;
		LinkedTo->SetLocation(position);
		LinkedTo->IsOnMap = down;
		if (LinkedTo->Target == nullptr)
		{
			FlightLevel = 0;
			CurrentState = TSJumpJetLocomotionClass::JumpjetState::DESCENDING;
		}
		else
		{
			CurrentState = TSJumpJetLocomotionClass::JumpjetState::HOVERING;
		}
	}
	else if (distance < Unsorted::LeptonsPerCell)
	{
		TargetSpeed = JumpjetSpeed * 0.3;
		if (LinkedTo->Target == nullptr)
		{
			FlightLevel = static_cast<int>(JumpjetCruiseHeight * 0.75);
		}
	}
	else if (distance < Unsorted::LeptonsPerCell * 2)
	{
		TargetSpeed = JumpjetSpeed * 0.5;
	}
	else
	{
		TargetSpeed = JumpjetSpeed;
		FlightLevel = JumpjetCruiseHeight;
	}
}

void TSJumpJetLocomotionClass::Process_Descent()
{
	CellClass* cellptr = MapClass::Instance->GetCellAt(HeadToCoord);
	Move move = LinkedTo->IsCellOccupied(cellptr ,FacingType::None ,-1 , nullptr, true);
	int spot = Game::Spot_Index(&HeadToCoord);
	bool stop = true;

	if ((cellptr->IsSpotFree(spot, cellptr->ContainsBridge()) || IsLanding) &&
		(MapClass::Instance->GetCellAt(HeadToCoord)->ContainsBridge() || move <= Move::MovingBlock && (move != Move::MovingBlock || IsLanding)))
	{

		if (!IsLanding)
		{
			IsLanding = true;
			LinkedTo->MarkAllOccupationBits(HeadToCoord);
		}

		FlightLevel = 0;

		int height = LinkedTo->HeightAGL;
		if (!LinkedTo->OnBridge)
		{
			if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge() && height >= CellClass::BridgeHeight)
			{
				height -= CellClass::BridgeHeight;
			}
		}

		if (height == 0)
		{
			LinkedTo->SetSpeedPercentage(0.0);
			LinkedTo->Mark(MarkType::Up);
			LinkedTo->SetLocation(HeadToCoord);

			if (LinkedTo->Location.Z > MapClass::Instance->GetCellFloorHeight(LinkedTo->Location))
			{
				if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge())
				{
					LinkedTo->OnBridge = true;
				}
			}

			LinkedTo->Mark(MarkType::Down);
			HeadToCoord = CoordStruct::Empty;
			IsMoving = false;
			LinkedTo->SetDestination(nullptr , false);
			LinkedTo->UpdatePosition(PCPType::End);

			if (LinkedTo != nullptr && LinkedTo->IsAlive && !LinkedTo->InLimbo && !LinkedTo->IsFallingDown)
			{
				LinkedTo->See(0,0);
				AircraftTrackerClass::Instance->Remove(LinkedTo);
				CurrentState = TSJumpJetLocomotionClass::JumpjetState::GROUNDED;
				IsLanding = false;
			}
		}
		stop = false;
	}

	if (stop) Stop_Moving();
}

IFACEMETHODIMP_(bool) TSJumpJetLocomotionClass::Is_Moving_Now()
{
	if (CurrentState != TSJumpJetLocomotionClass::JumpjetState::GROUNDED && CurrentState != TSJumpJetLocomotionClass::JumpjetState::HOVERING)
	{
		return true;
	}
	return false;
}

void TSJumpJetLocomotionClass::Movement_AI()
{
	bool need_to_mark = CurrentState != TSJumpJetLocomotionClass::JumpjetState::HOVERING && CurrentState != TSJumpJetLocomotionClass::JumpjetState::CRUISING;
	bool was_down = LinkedTo->IsOnMap;

	if (need_to_mark)
	{
		LinkedTo->Mark(MarkType::Up);
	}
	else
	{
		LinkedTo->IsOnMap = false;
	}

	if (TargetSpeed > CurrentSpeed)
	{
		CurrentSpeed += JumpjetAcceleration;
		CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(JumpjetSpeed));
	}
	if (TargetSpeed < CurrentSpeed)
	{
		CurrentSpeed -= JumpjetAcceleration * 1.5;
		CurrentSpeed = std::max(CurrentSpeed, 0.0);
	}

	LinkedTo->SetSpeedPercentage(CurrentSpeed / JumpjetSpeed);

	bool at_destination = LinkedTo->GetMapCoords() == CellClass::Coord2Cell(HeadToCoord);

	if ((CurrentState == TSJumpJetLocomotionClass::JumpjetState::HOVERING || CurrentState == TSJumpJetLocomotionClass::JumpjetState::CRUISING) && !JumpjetNoWobbles)
	{
		CurrentWobble += Math::deg2rad(360) / (15.0 / JumpjetWobblesPerSecond);
	}
	else
	{
		CurrentWobble = 0;
	}

	int desired_height = (int)Math::sin(CurrentWobble) * JumpjetWobbleDeviation + FlightLevel;
	int height = LinkedTo->Height;
	int ground_height = MapClass::Instance->GetCellFloorHeight(LinkedTo->Location);

	if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge() &&
			LinkedTo->Location.Z >= ground_height + 4 * Unsorted::PixelLeptonHeight)
	{

		ground_height += CellClass::BridgeHeight;
	}

	int height_diff = 0;
	if (CurrentState != TSJumpJetLocomotionClass::JumpjetState::DESCENDING && CurrentState != TSJumpJetLocomotionClass::JumpjetState::GROUNDED && !at_destination)
	{
		height_diff = height - Desired_Flight_Level();
	}
	else
	{
		height_diff = height - ground_height;
	}

	bool moved = false;
	if (height_diff < desired_height)
	{
		int height_agl = LinkedTo->HeightAGL;
		if (MapClass::Instance->GetCellAt(LinkedTo->GetCoords())->ContainsBridge() && !LinkedTo->OnBridge)
		{
			if (LinkedTo->Location.Z >= MapClass::Instance->GetCellFloorHeight(LinkedTo->Location) + CellClass::BridgeHeight)
			{
				height_agl -= CellClass::BridgeHeight;
			}
		}
		if (height_agl == 0)
		{
			LinkedTo->UnmarkAllOccupationBits(LinkedTo->Location);
			LinkedTo->OnBridge = false;
		}
		height += (int)JumpjetClimb;
		moved = true;
	}
	if (height_diff > desired_height)
	{
		height -= (int)JumpjetClimb;
		height = std::max(height, ground_height);
		moved = true;
		height_diff = std::max(height_diff, 0);
	}

	if (LinkedTo->GetMapCoords() != CellClass::Coord2Cell(HeadToCoord))
	{
		if (height_diff < desired_height / 2)
		{
			CurrentSpeed *= 0.9;
		}
		if (height_diff < desired_height / 4)
		{
			CurrentSpeed *= 0.9;
		}
	}

	if (moved)
	{
		LinkedTo->Height = (height);
		if (need_to_mark)
		{
			LinkedTo->Mark(MarkType::Up);
		}
	}

	Coordinate new_coord {};
	auto cur_facing = Facing.Current();
	Game::Coord_Move(&new_coord  , &LinkedTo->Location, &cur_facing, (int)CurrentSpeed);
	LinkedTo->SetLocation(new_coord);

	if (LinkedTo != nullptr)
	{
		const int& rad = JumpjetCloakDetectionRadius;
		for (int x = -rad; x <= rad; x++)
		{
			for (int y = -rad; y <= rad; y++)
			{
				CellClass* cellptr = MapClass::Instance->TryGetCellAt(CellStruct(short(x), short(y)) + CellClass::Coord2Cell(new_coord));
				if (cellptr != nullptr)
				{
					ObjectClass* occupier = cellptr->FirstObject;
					while (occupier != nullptr)
					{
						TechnoClass* tech = flag_cast_to<TechnoClass*>(occupier);
						if (tech != nullptr)
						{
							tech->Uncloak(true);
						}
						occupier = occupier->NextObject;
					}
					occupier = cellptr->AltObject;
					while (occupier != nullptr)
					{
						TechnoClass* tech = flag_cast_to<TechnoClass*>(occupier);
						if (tech != nullptr)
						{
							tech->Uncloak(true);
						}
						occupier = occupier->NextObject;
					}
				}
			}
		}
	}

	//CellClass* cellptr = MapClass::Instance->GetCellAt(new_coord);
	//BuildingClass* building = cellptr->GetBuilding();
	//if (building && building->Type->FirestormWall && building->Owner->FirestormActive)
	//{
	//	building->Crossing_Firestorm(LinkedTo, true);
	//}

	LinkedTo->PrimaryFacing.Set_Current(Facing.Current());

	if (need_to_mark)
	{
		LinkedTo->Mark(MarkType::Down);
	}
	else
	{
		LinkedTo->IsOnMap = was_down;
	}
}

Coordinate TSJumpJetLocomotionClass::Closest_Free_Spot(Coordinate const& to) const
{
	Coordinate closest = MapClass::Instance->PickInfantrySublocation(to);
	if (closest != CoordStruct::Empty)
	{
		closest.Z = MapClass::Instance->GetCellFloorHeight(closest);
		if (MapClass::Instance->GetCellAt(closest)->ContainsBridge())
		{
			closest.Z += CellClass::BridgeHeight;
		}
	}
	return closest;
}

int TSJumpJetLocomotionClass::Desired_Flight_Level() const
{
	CoordStruct Coordinate = LinkedTo->PositionCoord;
	auto pCoordCell = MapClass::Instance->GetCellAt(Coordinate);

	int height = pCoordCell->GetZPosAdjent();
	if (pCoordCell->ContainsBridge())
	{
		height += CellClass::BridgeHeight;
	}

	if (CurrentSpeed > 0)
	{
		Point2D _offset = CellSpread::GetNeighbourPointOffset(Facing.Current().GetFacing<8>());
		Coordinate.X += _offset.X;
		Coordinate.Y += _offset.Y;

		pCoordCell = MapClass::Instance->GetCellAt(Coordinate);

		int adjancent_height = pCoordCell->GetZPosAdjent();
		if (pCoordCell->ContainsBridge())
		{
			height += CellClass::BridgeHeight;
		}
		if (adjancent_height > height)
		{
			return adjancent_height;
		}
		return(height + adjancent_height) / 2;
	}
	return height;
}

IFACEMETHODIMP_(void) TSJumpJetLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (mark == 0)
	{
		Coordinate headto = Head_To_Coord();
		if (headto != CoordStruct::Empty && (CurrentState == TSJumpJetLocomotionClass::JumpjetState::GROUNDED || IsLanding))
		{
			LinkedTo->UnmarkAllOccupationBits(headto);
			IsLanding = false;
		}
	}
}

IFACEMETHODIMP_(Coordinate) TSJumpJetLocomotionClass::Head_To_Coord()
{
	if (CurrentState == TSJumpJetLocomotionClass::JumpjetState::GROUNDED)
	{
		return LinkedTo->GetCoords();
	}
	else
	{
		return HeadToCoord;
	}
}