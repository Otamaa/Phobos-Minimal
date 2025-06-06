#include "AdvancedWalkLocomotionClass.h"

#include <InfantryClass.h>
#include <FootClass.h>
#include <MissionClass.h>
#include <CellClass.h>
#include <MapClass.h>

#include <Locomotor/Cast.h>

// Minimal Walk locomotor implementation focusing on facing direction fix

bool __stdcall AdvancedWalkLocomotionClass::Process()
{
	if (!this->LinkedTo || !this->LinkedTo->IsAlive)
		return false;

	if (this->IsMoving && this->MovingDestination != CoordStruct::Empty)
	{
		return this->WalkingProcess();
	}

	return false;
}

void __stdcall AdvancedWalkLocomotionClass::Move_To(CoordStruct to)
{
	// Basic validation
	const auto pTargetCell = MapClass::Instance->TryGetCellAt(to);
	if (!pTargetCell)
		return;

	// INFANTRY SUBPOSITION SYSTEM: Find proper subposition for infantry placement
	// This ensures 3 infantry can occupy the same cell at different positions instead of stacking
	CoordStruct finalDestination = to;
	
	if (this->LinkedTo && this->LinkedTo->WhatAmI() == AbstractType::Infantry)
	{
		// Use CellClass::FindInfantrySubposition to get proper subposition within the cell
		finalDestination = pTargetCell->FindInfantrySubposition(to, false, false, false, to.Z);
		
		// Alternative method if the above doesn't work well
		if (finalDestination == to || finalDestination == CoordStruct::Empty)
		{
			// Use MapClass::PickInfantrySublocation as fallback
			finalDestination = MapClass::Instance->PickInfantrySublocation(to, false);
		}
	}

	this->MovingDestination = finalDestination;
	this->CoordHeadTo = finalDestination;
	this->IsMoving = true;
	this->IsReallyMoving = true;
}

void __stdcall AdvancedWalkLocomotionClass::Stop_Moving()
{
	this->MovingDestination = CoordStruct::Empty;
	this->CoordHeadTo = CoordStruct::Empty;
	this->IsMoving = false;
	this->IsReallyMoving = false;
	
	// Simple mission transition
	if (this->LinkedTo && this->LinkedTo->GetCurrentMission() == Mission::Move)
	{
		this->LinkedTo->QueueMission(Mission::Guard, false);
		this->LinkedTo->NextMission();
	}
}

// Cell occupancy check - max 3 infantry per cell with subposition system
Move __stdcall AdvancedWalkLocomotionClass::Can_Enter_Cell(CellStruct cell)
{
	if (!this->LinkedTo)
		return Move::No;

	const auto pCell = MapClass::Instance->TryGetCellAt(cell);
	if (!pCell)
		return Move::No;

	// Use FootClass::IsCellOccupied for primary check
	const auto move = this->LinkedTo->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false);
	
	// For infantry, additional subposition check
	if (this->LinkedTo->WhatAmI() == AbstractType::Infantry)
	{
		if (move == Move::OK || move == Move::Temp)
		{
			// Check if there's available subposition for infantry (max 3 per cell)
			if (!this->HasAvailableSubposition(pCell))
				return Move::Temp; // Cell full of infantry
		}
	}

	return move;
}

// Occupation marking for cell management
void __stdcall AdvancedWalkLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (!this->LinkedTo)
		return;

	const auto currentCoord = this->LinkedTo->Location;
	
	if (mark == 1) // MarkType::Down
	{
		this->LinkedTo->MarkAllOccupationBits(currentCoord);
	}
	else if (mark == 0) // MarkType::Up  
	{
		this->LinkedTo->UnmarkAllOccupationBits(currentCoord);
	}
}

// Stop movement animation properly
void __stdcall AdvancedWalkLocomotionClass::Stop_Movement_Animation()
{
	this->IsReallyMoving = false;
}

// Walking process with proper facing direction
bool AdvancedWalkLocomotionClass::WalkingProcess()
{
	if (!this->IsMoving || this->MovingDestination == CoordStruct::Empty)
		return false;

	const auto pLinked = this->LinkedTo;
	if (!pLinked || !pLinked->IsAlive)
	{
		this->Stop_Moving();
		return false;
	}

	// Get current and target locations
	const auto currentLoc = pLinked->Location;
	const auto targetLoc = this->MovingDestination;

	// Calculate distance 
	const auto distance = currentLoc.DistanceFrom(targetLoc);
	
	// Check if reached destination
	if (distance <= 64) // Fine-tuned threshold
	{
		// INFANTRY SUBPOSITION: Final placement at exact subposition
		if (this->LinkedTo && this->LinkedTo->WhatAmI() == AbstractType::Infantry)
		{
			const auto pTargetCell = MapClass::Instance->TryGetCellAt(targetLoc);
			if (pTargetCell)
			{
				// Find the best subposition for final placement
				const CoordStruct finalPos = pTargetCell->FindInfantrySubposition(targetLoc, false, false, false, targetLoc.Z);
				
				if (finalPos != CoordStruct::Empty && finalPos != targetLoc)
				{
					// Place infantry at exact subposition
					pLinked->Mark(MarkType::Up);
					pLinked->SetLocation(finalPos);
					pLinked->Mark(MarkType::Down);
				}
			}
		}
		
		this->Stop_Moving();
		return true;
	}

	// Get movement speed
	const auto speed = pLinked->GetCurrentSpeed();
	if (speed <= 0)
		return false;

	// Calculate direction vector
	const auto direction = targetLoc - currentLoc;
	const auto dirLength = direction.Length();
	
	if (dirLength <= 0)
	{
		this->Stop_Moving();
		return true;
	}

	// Calculate step size - proper movement
	const auto stepSize = std::min(speed, static_cast<int>(dirLength));
	const auto ratio = static_cast<float>(stepSize) / static_cast<float>(dirLength);
	
	// Calculate new position
	const auto newPos = CoordStruct {
		currentLoc.X + static_cast<int>(direction.X * ratio),
		currentLoc.Y + static_cast<int>(direction.Y * ratio),
		currentLoc.Z + static_cast<int>(direction.Z * ratio)
	};

	// CRITICAL: Update facing direction - this is the main fix
	this->UpdateFacingDirection(direction);

	// Mark occupation properly and move
	pLinked->Mark(MarkType::Up);    // Remove from current cell
	pLinked->SetLocation(newPos);   // Move to new position
	pLinked->Mark(MarkType::Down);  // Add to new cell
	
	this->CoordHeadTo = newPos;

	return false; // Continue processing
}

// Check if cell can accommodate infantry
bool AdvancedWalkLocomotionClass::CanEnterCell(CellClass* pCell)
{
	if (!pCell)
		return false;

	// Check clear to move
	if (!pCell->IsClearToMove(SpeedType::Foot, MovementZone::Infantry, false, false, -1))
		return false;

	// Check infantry count
	return this->CountInfantryInCell(pCell) < 3;
}

// Count infantry in cell (max 3 per cell)
int AdvancedWalkLocomotionClass::CountInfantryInCell(CellClass* pCell)
{
	if (!pCell)
		return 0;

	int count = 0;
	auto pObject = pCell->FirstObject;
	
	while (pObject)
	{
		if (pObject->WhatAmI() == AbstractType::Infantry)
		{
			++count;
		}
		pObject = pObject->NextObject;
	}

	// Also check AltObject (bridge level)
	pObject = pCell->AltObject;
	while (pObject)
	{
		if (pObject->WhatAmI() == AbstractType::Infantry)
		{
			++count;
		}
		pObject = pObject->NextObject;
	}

	return count;
}

// Check if cell has available infantry subposition (YR allows 3 infantry per cell)
bool AdvancedWalkLocomotionClass::HasAvailableSubposition(CellClass* pCell)
{
	if (!pCell)
		return false;

	// Count current infantry in cell
	const int currentCount = this->CountInfantryInCell(pCell);
	
	// YR allows maximum 3 infantry per cell in different subpositions
	if (currentCount >= 3)
		return false;
	
	// Additional check: try to find an actual subposition
	// Using cell center as base coordinate
	const CoordStruct cellCenter = pCell->GetCoordsWithBridge();
	const CoordStruct testSubpos = pCell->FindInfantrySubposition(cellCenter, false, false, false, cellCenter.Z);
	
	// If FindInfantrySubposition returns empty or same as center, no space available
	return (testSubpos != CoordStruct::Empty && testSubpos != cellCenter);
}

// MAIN FIX: Update facing direction properly without forced animation
void AdvancedWalkLocomotionClass::UpdateFacingDirection(const CoordStruct& direction)
{
	if (!this->LinkedTo)
		return;

	// Only update facing if there's actual movement direction
	if (direction.X != 0 || direction.Y != 0)
	{
		// Use direct direction mapping instead of angle calculation
		// YR facing: North=0, NE=32, East=64, SE=96, South=128, SW=160, West=192, NW=224
		DirStruct dirYR;
		
		// Get normalized direction components
		const auto absX = Math::abs(direction.X);
		const auto absY = Math::abs(direction.Y);
		
		// Determine primary direction based on which component is larger
		if (absX > absY * 2) // Primarily horizontal
		{
			if (direction.X > 0)
				dirYR = DirStruct { 64 << 8 };  // East
			else
				dirYR = DirStruct { 192 << 8 }; // West
		}
		else if (absY > absX * 2) // Primarily vertical
		{
			if (direction.Y < 0) // Moving North (negative Y in YR coords)
				dirYR = DirStruct { 0 << 8 };   // North
			else
				dirYR = DirStruct { 128 << 8 }; // South
		}
		else // Diagonal movement
		{
			if (direction.X > 0 && direction.Y < 0)
				dirYR = DirStruct { 32 << 8 };  // NE
			else if (direction.X > 0 && direction.Y > 0)
				dirYR = DirStruct { 96 << 8 };  // SE
			else if (direction.X < 0 && direction.Y > 0)
				dirYR = DirStruct { 160 << 8 }; // SW
			else // direction.X < 0 && direction.Y < 0
				dirYR = DirStruct { 224 << 8 }; // NW
		}
		
		// Set facing direction using Set_Desired for smooth rotation
		this->LinkedTo->PrimaryFacing.Set_Desired(dirYR);
	}
} 