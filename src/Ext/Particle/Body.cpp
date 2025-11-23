#include "Body.h"

#include "Ext/Bullet/Body.h"
#include "Ext/ParticleType/Body.h"
#include <Utilities/Macro.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

void FakeParticleClass::__AI(){
	bool Update = true;

	switch (this->Type->BehavesLike)
	{
	case ParticleTypeBehavesLike::Smoke:
		this->__Smoke_AI();
		break;
	case ParticleTypeBehavesLike::Gas:
		this->__Gas_AI();
		break;
	case ParticleTypeBehavesLike::Fire:
		this->__Fire_AI();
		break;
	case ParticleTypeBehavesLike::Spark:
		this->__Spark_AI();
		break;
	case ParticleTypeBehavesLike::Railgun:
		this->__Railgun_AI();
		break;
	case ParticleTypeBehavesLike(5):
		this->__Web_AI();
		break;
	default:
		Update = false;
		break;
	}

	if (Update) {
		const auto pParticleExt = this->_GetExtData();

		if (!pParticleExt->LaserTrails.empty())
		{
			const CoordStruct location = this->GetCoords();
			const CoordStruct drawnCoords = location;
			for (auto& trail : pParticleExt->LaserTrails)
			{
				if (!trail->LastLocation.isset())
					trail->LastLocation = location;

				//trail->Update(GetFLHAbsoluteCoords(trail->FLH, drawnCoords));
				trail->Update(drawnCoords);
				trail->Visible = this->IsOnMyView();
			}
		}

		TrailsManager::AI(this->_AsParticle());
	}

	if (!--this->RemainingEC) {
		this->hasremaining = 1;
	}
}

bool IsUnderwaterAtBridge(CoordStruct& pos, int newZ)
{
	CellClass* cell = MapClass::Instance->GetCellAt(pos);

	// Check if cell has bridge
	if (!cell->ContainsBridgeHead()) {
		return false;
	}

	// Check if particle is below bridge
	const int terrainHeight = MapClass::Instance->GetZPos(&pos);
	const int bridgeHeight = terrainHeight + (Unsorted::CellHeight * 2);

	if (pos.Z >= bridgeHeight) {
		return false; // Above bridge
	}

	// Check if particle would surface above water kill zone
	const int waterKillZone = bridgeHeight - static_cast<int>(Unsorted::LevelHeight * 2.5f);
	return newZ <= waterKillZone;
}

void FakeParticleClass::__Gas_Wind()
{
	CoordStruct currentPos = this->Location;

	// Apply wind effect
	const int windEffect = this->Type->WindEffect;
	const int windDir = RulesClass::Instance->WindDirection;
	const auto& mult = this->_GetTypeExtData()->WindMult[windDir];
	const int windX = windEffect * mult.X;
	const int windY = windEffect * mult.Y;

	// Calculate new position with drift and velocity
	CoordStruct newPos {
		.X = currentPos.X + this->GasVelocity.X + windX,
		.Y = currentPos.Y + this->GasVelocity.Y + windY,
		.Z = currentPos.Z + this->GasVelocity.Z + static_cast<int>(this->Velocity)
	};

	// Check if gas particle is underwater at a bridge
	if (IsUnderwaterAtBridge(currentPos, newPos.Z)) {
		this->hasremaining = 1; // Remove particle
		return;
	}

	// Update position
	this->SetLocation(newPos);
}

void FakeParticleClass::__Fire_Coord_AI()
{
	CoordStruct currentPos = this->Location;
	// Apply movement vector if particle has velocity
	CoordStruct newPos = this->Velocity > 0.0f ? currentPos + this->Fire100 : currentPos;

	// Check if particle hit ground
	if (this->Velocity > 0.0f) {
		const int currentTerrainHeight = MapClass::Instance->GetZPos(&currentPos);
		const int newTerrainHeight = MapClass::Instance->GetZPos(&newPos);

		if (newTerrainHeight > currentTerrainHeight) {
			// Hit ground - mark for removal
			this->CoordChange = 1;
			this->hasremaining = 1;
		}
	}

	// Update position
	this->SetLocation(newPos);
}

void FakeParticleClass::ApplyWindEffect(CoordStruct& pos)
{
	const int windEffect = this->Type->WindEffect;

	if (windEffect <= 0) {
		return;
	}

	// Apply wind every N frames based on wind effect strength
	const int windFrequency = 10 / windEffect;
	if ((Unsorted::CurrentFrame % windFrequency) != 0) {
		return;
	}

	const int windDir = RulesClass::Instance->WindDirection;
	const auto& mult = this->_GetTypeExtData()->WindMult[windDir];
	pos.X += mult.X;
	pos.Y += mult.Y;
}

void FakeParticleClass::ApplySmokeDrift(CoordStruct& pos)
{
	// Apply accumulated drift
	pos.X += static_cast<int>(this->SmokeVelocity.X);
	pos.Y += static_cast<int>(this->SmokeVelocity.Y);

	// Descend toward ground if above it
	const int terrainHeight = MapClass::Instance->GetZPos(&pos);
	const int minHeight = terrainHeight + 5;

	if (pos.Z > minHeight)
	{
		int descentRate = pos.Z - minHeight;
		descentRate = std::min(descentRate, 2); // Max 2 leptons per frame
		pos.Z -= descentRate;
	}

	// Apply gas drift
	pos += this->GasVelocity;

	// Ensure smoke stays above ground
	const int newTerrainHeight = MapClass::Instance->GetZPos(&pos);
	if (pos.Z < newTerrainHeight + 5) {
		pos.Z = newTerrainHeight + 5;
	}
}

void FakeParticleClass::__Smoke_Coord_AI()
{
	CoordStruct currentPos = this->Location;

	// Apply wind effect periodically
	this->ApplyWindEffect(currentPos);

	// Update position on odd frames
	if ((Unsorted::CurrentFrame & 1) != 0) {
		this->ApplySmokeDrift(currentPos);
	}

	// Update coordinate
	this->SetLocation(currentPos);
}

void FakeParticleClass::__Coord_AI()
{
	// Route to appropriate behavior based on particle type
	switch (this->Type->BehavesLike)
	{
	case ParticleTypeBehavesLike::Smoke:
		this->__Smoke_Coord_AI();
		break;

	case ParticleTypeBehavesLike::Gas:
		this->__Gas_Wind();
		break;

	case ParticleTypeBehavesLike::Fire:
		this->__Fire_Coord_AI();
		break;

	default:
		break;
	}
}

#include <YRMathVector.h>
#include <TacticalClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/Helpers.h>
#include <Misc/Ares/Hooks/Header.h>

constexpr CoordStruct VectorToCoord(const Vector3D<float> &vec) {
	return { (int)vec.X , (int)vec.Y , (int)vec.Z};
}

constexpr Vector3D<float> CoordToVector(const CoordStruct& coord) {
	return { (float)coord.X , (float)coord.Y , (float)coord.Z };
}

constexpr float GetCollisionHeight(const Collision& collision, int terrainHeight)
{
	switch (collision.type)
	{
	case CollisionType::BridgeTop:
		return static_cast<float>(terrainHeight + (Unsorted::CellHeight * 2));
	case CollisionType::BridgeBottom:
		return static_cast<float>(terrainHeight + (Unsorted::CellHeight * 2) - 20);
	case CollisionType::Building:
		return static_cast<float>(terrainHeight - 100);
	default:
		return static_cast<float>(terrainHeight);
	}
}

float GetRandomFloat(float min, float max)
{
	const float normalized = (float)ScenarioClass::Instance->Random.RandomDouble(); // Convert to [0,1]
	return min + normalized * (max - min);
}

constexpr bool ShouldAdjustForTerrain(float velocityZ, int terrainHeight,
								const CollisionState& collision) {
	return velocityZ < terrainHeight || collision.hitbridgeTop ||
		collision.hitbridge || collision.hitBuilding;
}

constexpr float AdjustForTerrainHeight(float velocityZ, int terrainHeight,
							  const CollisionState& collision)
{
	if (collision.hitbridgeTop) {
		return static_cast<float>(terrainHeight + (Unsorted::CellHeight * 2));
	}

	if (collision.hitbridge) {
		return static_cast<float>(terrainHeight + (Unsorted::CellHeight * 2) - 20);
	}

	// Gradual descent to terrain
	const int adjustedHeight = terrainHeight - 100;

	if (adjustedHeight < velocityZ) {
		return static_cast<float>(terrainHeight);
	}

	return velocityZ;
}

constexpr bool CheckCellBuildingCollision(CellClass* cell, float velocityZ, int terrainHeight)
{
	constexpr float COLLISION_RANGE = 150.0f;

	const float terrainZ = static_cast<float>(terrainHeight);
	if (velocityZ < terrainZ ||
		velocityZ - COLLISION_RANGE >= terrainZ) {
		return false;
	}

	BuildingClass* building = cell->GetBuilding();
	const bool hasWall = cell->ConnectsToOverlay();

	if (!building && !hasWall) {
		return false;
	}

	// Check exclusions
	if (building)
	{
		if (building->Type->LaserFence && building->LaserFenceFrame >= 8)
		{
			return false;
		}

		if (building->IsStrange())
		{
			return false;
		}
	}

	return true;
}

void ReflectOffSurface(CoordStruct& coord, const Vector3D<float>& velocity , Vector3D<float>& result)
{
	const int rampType = TacticalClass::Instance->GetRamp(&coord);

	if(rampType >= 0 && rampType < (int)Game::VoxelRampMatrix.size()) {
		Matrix3D rampMatrix = Game::VoxelRampMatrix[rampType];
		Matrix3D rotMatrix = Matrix3D::TransposeMatrix(rampMatrix);

		Vector3D<float> reflected { velocity.X, -velocity.Y, velocity.Z };
		reflected = rotMatrix.RotateVector(reflected);
		reflected.Z = -reflected.Z;
		reflected = rotMatrix.RotateVector(reflected);
		result = Vector3D<float> { reflected.X, -reflected.Y, reflected.Z };
	}
}

#pragma region Fire

ObjectClass* FakeParticleClass::GetCellOccupiers(CellClass* cell) const
{
	// Check if cell has water

	if (!cell->ContainsBridgeHead()) {
		return cell->FirstObject;
	}

	// Determine if particle is above or below water
	const int waterLevel = (Unsorted::CellHeight * 2) * (cell->Level + 4);
	const bool isAboveWater = this->Location.Z >= waterLevel;

	return isAboveWater ? cell->AltObject : cell->FirstObject;
}

void FakeParticleClass::ApplyFireDamage()
{
	// Check if it's time to apply damage
	if (--this->RemainingDC > 0) {
		return;
	}

	const auto* type = this->Type;

	// Check if this particle can deal damage
	if (!type->Damage || !type->Warhead || this->StartStateAI > type->FinalDamageState) {
		return;
	}

	// Reset damage timer
	this->RemainingDC = (short)type->MaxDC;

	auto const& [pAttacker, pOwner] = ParticleExtData::GetOwnership(this);
	const auto pCell = MapClass::Instance->GetCellAt(this->Location);
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(this->Type);

	for (auto pOccupy = pCell->GetContent(this->Location.Z); pOccupy; pOccupy = pOccupy->NextObject)
	{

		if (pOccupy && pOccupy->IsAlive && pOccupy->Health > 0 && !pOccupy->InLimbo)
		{
			if (this->ParticleSystem && pOccupy == this->ParticleSystem->Owner)
				continue;

			if (auto pTechno = flag_cast_to<TechnoClass*, false>(pOccupy))
			{
				if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
					continue;

				if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
					continue;
			}

			int damage = this->Type->Damage;
			int length = (int)(this->Location.DistanceFrom(pOccupy->GetCoords()) / 10.0);

			pOccupy->ReceiveDamage(&damage, length, this->Type->Warhead, pAttacker, false, false, pOwner);
			if (pTypeExt->Fire_DamagingAnim)
			{
				if (auto pAnimType = MapClass::SelectDamageAnimation(this->Type->Damage, this->Type->Warhead, pCell->LandType, this->Location))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, this->Location),
						pOwner, pOccupy->GetOwningHouse(), pAttacker, false, false);
				}
			}
		}
	}
}

bool FakeParticleClass::ShouldAdvanceAnimationFrame() const
{
	const int particleID = this->Fetch_ID();
	const int elapsed = this->Type->MaxEC - this->RemainingEC + particleID;
	const int frameDelay = this->StateAIAdvance + (particleID & 1);

	return (elapsed % frameDelay) == 0;
}

void FakeParticleClass::UpdateTranslucency()
{
	const auto* type = this->Type;
	const char currentState = this->StartStateAI;

	// Check for translucency state transitions
	if (currentState == type->Translucent25State) {
		this->Translucency = 25;
	} else if (currentState == type->Translucent50State) {
		this->Translucency = 50;
	}
}

void FakeParticleClass::AdvanceAnimationState()
{
	const auto* type = this->Type;

	// Check if animation is complete
	if (this->StartStateAI >= type->EndStateAI) {
		return;
	}

	// Check if it's time to advance frame
	if (!this->ShouldAdvanceAnimationFrame()) {
		return;
	}

	// Advance to next frame
	++this->StartStateAI;

	// Handle translucency state transitions
	this->UpdateTranslucency();

	// Check if we've reached the end
	if (this->StartStateAI == type->EndStateAI && type->DeleteOnStateLimit) {
		this->hasremaining = 1;
	}
}

void FakeParticleClass::UpdateFireMovement()
{
	// Add random directional variation (-5% to +5%)
	auto rand = Math::abs(ScenarioClass::Instance->Random.Random());
	const int randomOffset = ((rand) % 10) - 5;
	const float variationFactor = (randomOffset * 0.02f) + 1.0f;

	// Apply variation to direction vector and calculate velocity components
	const Vector3D<float> variedDirection {
		variationFactor * this->Spark10C.X,
		variationFactor * this->Spark10C.Y,
		variationFactor * this->Spark10C.Z
	};

	// Update movement vector
	this->Fire100.X = static_cast<int>(variedDirection.X * this->Velocity);
	this->Fire100.Y = static_cast<int>(variedDirection.Y * this->Velocity);
	this->Fire100.Z = static_cast<int>(variedDirection.Z * this->Velocity);
}


void FakeParticleClass::__Fire_AI() {
	// Check if fire has burned out
	if (this->Velocity <= 0.0f) {
		this->Velocity = 0.0f;
		this->hasremaining = 1; // Mark for deletion
		return;
	}

	// Apply random directional variation and update movement
	this->UpdateFireMovement();

	// Advance animation state and handle translucency transitions
	this->AdvanceAnimationState();

	// Apply deceleration (fire dies down over time)
	this->Velocity -= this->Type->Deacc;

	// Apply damage to nearby units
	this->ApplyFireDamage();
}

#pragma endregion

#pragma region Smoke

CollisionState  FakeParticleClass::CheckCollision(
	CellClass* cell,
	const CoordStruct& currentPos,
	const CoordStruct& nextPos,
	int terrainHeight,
	const Vector3D<float>& velocity
)
{
	CollisionState state;

	// Check for water collision
	const bool hasBridgeHead = (cell->Flags & CellFlags::BridgeHead) != CellFlags::Empty;
	const bool nextCellhasBridgeHead = (MapClass::Instance->GetCellAt(currentPos)->Flags & CellFlags::BridgeHead) != CellFlags::Empty;

	if (hasBridgeHead || nextCellhasBridgeHead)
	{
		const int waterHeight = (Unsorted::CellHeight * 2) + terrainHeight;

		if (currentPos.X < waterHeight)
		{
			state.hitbridge = true;
		}
		else if (nextPos.X >= waterHeight)
		{
			state.hitbridgeTop = true;
		}
	}

	// Check for building collision
	if (!state.hitbridge && !state.hitbridgeTop) {
		state.hitBuilding = CheckCellBuildingCollision(cell, velocity.Z, terrainHeight);
	}

	return state;
}

void FakeParticleClass::UpdateGasMovement()
{
	if ((Math::abs(ScenarioClass::Instance->Random.Random()) & 7) != 0) {
		return;
	}

	int deltaX = 0;
	int deltaY = 0;

	// Randomly choose X or Y direction
	if ((Math::abs(ScenarioClass::Instance->Random.Random()) & 1) != 0) {
		deltaY = (Math::abs(ScenarioClass::Instance->Random.Random()) % 3) - 1;
	} else {
		deltaX = (Math::abs(ScenarioClass::Instance->Random.Random()) % 3) - 1;
	}

	const auto pExt = this->_GetTypeExtData();

	const auto maxDriftSpeedX = &pExt->Gas_DriftSpeedX.Get();
	const auto minDriftSpeedY = &pExt->Gas_DriftSpeedY.Get();

	// Update and clamp gas movement vector
	this->GasVelocity.X = std::clamp(this->GasVelocity.X + deltaX, maxDriftSpeedX->Y, maxDriftSpeedX->X);
	this->GasVelocity.Y = std::clamp(this->GasVelocity.Y + deltaY, minDriftSpeedY->Y, minDriftSpeedY->X);
}

void FakeParticleClass::UpdateGasHeight()
{
	const int height = this->GetHeight();

	if (height <= 5 || (Unsorted::CurrentFrame % 2) == 0) {
		this->GasVelocity.Z = std::max(0, this->GasVelocity.Z);
	} else {
		this->GasVelocity.Z = std::max(-5, this->GasVelocity.Z - 1);
	}
}

char FakeParticleClass::ProcessEndState()
{
	this->ProcessDamage();
	return this->UpdateStateAndCheckEnd();
}

char FakeParticleClass::UpdateStateAndCheckEnd()
{
	const auto* type = this->Type;
	const int id = this->Fetch_ID();
	const int cycleOffset = type->MaxEC - this->RemainingEC + id;
	const int advanceRate = type->StateAIAdvance + (id & 1);

	if ((cycleOffset % advanceRate) == 0) {
		++this->StartStateAI;
	}

	if (this->StartStateAI == type->EndStateAI) {
		if (type->DeleteOnStateLimit) {
			this->hasremaining = 1;
		} else {
			this->StartStateAI = 0;
		}
	}

	return this->StartStateAI;
}

static void ApplyDamageToObject(ObjectClass* pItem, TechnoClass* pAttacker, HouseClass* pOwner, int distance, const CoordStruct& loc, ParticleTypeExtData* pTypeExt, HouseClass* transmoOwner)
{
	int damage = pTypeExt->This()->Damage;
	if (pItem->ReceiveDamage(&damage, distance, pTypeExt->This()->Warhead, pAttacker, false, false, pOwner) == DamageState::NowDead)
	{
		if (pTypeExt->TransmogrifyChance >= 0)
		{

			if (pTypeExt->TransmogrifyOwner != OwnerHouseKind::Neutral)
				transmoOwner = HouseExtData::GetHouseKind(pTypeExt->TransmogrifyOwner, true, nullptr, pOwner, pItem->GetOwningHouse());

			CoordStruct loc_ = loc;
			TechnoExt_ExtData::SpawnVisceroid(loc_, pTypeExt->TransmogrifyType, pTypeExt->TransmogrifyChance, pTypeExt->Transmogrify, transmoOwner);
		}
	}
}

void FakeParticleClass::ProcessDamage()
{
	if (--this->RemainingDC > 0) {
		return;
	}

	const auto* type = this->Type;
	if (!type->Damage || !type->Warhead) {
		return;
	}

	this->RemainingDC = (short)type->MaxDC;

	auto pTypeExt = this->_GetTypeExtData();

	const auto& [pAttacker, pOwner] = ParticleExtData::GetOwnership(this);
	HouseClass* transmoOwner = HouseExtData::FindNeutral();

	if (pTypeExt->DamageRange.Get() <= 0.0)
	{
		if (auto pCell = MapClass::Instance->TryGetCellAt(this->Location))
		{
			for (auto pOccupy = pCell->FirstObject; pOccupy; pOccupy = pOccupy->NextObject)
			{
				if (pOccupy->IsAlive && pOccupy->Health > 0)
				{
					if (auto pTechno = flag_cast_to<TechnoClass*, false>(pOccupy))
					{
						if (pTechno->IsSinking || pTechno->IsCrashing || pTechno->TemporalTargetingMe)
							continue;

						if (pTechno->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTechno)))
							continue;
					}

					auto nX = Math::abs(this->Location.X - pOccupy->Location.X);
					auto nY = Math::abs(this->Location.Y - pOccupy->Location.Y);
					ApplyDamageToObject(pOccupy, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pOccupy->Location, pTypeExt, transmoOwner);
				}
			}
		}

	}
	else
	{

		const auto pVec = Helpers::Alex::getCellSpreadItems(this->Location, std::ceil(pTypeExt->DamageRange.Get()));

		for (const auto pItem : pVec)
		{
			if (!pItem->IsAlive || pItem->Health <= 0)
				continue;

			if (pItem->IsSinking || pItem->IsCrashing || pItem->TemporalTargetingMe)
				continue;

			if (pItem->WhatAmI() != BuildingClass::AbsID && TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pItem)))
				continue;

			auto nX = Math::abs(this->Location.X - pItem->Location.X);
			auto nY = Math::abs(this->Location.Y - pItem->Location.Y);
			ApplyDamageToObject(pItem, pAttacker, pOwner, Game::AdjustHeight(nX + nY), pItem->Location, pTypeExt, transmoOwner);
		}
	}
}

void FakeParticleClass::__Smoke_AI() {
	// Skip processing on odd frames
	if ((Unsorted::CurrentFrame & 1) != 0) {
		this->ProcessEndState();
		return;
	}

	// Update gas movement direction randomly
	this->UpdateGasMovement();

	// Initialize velocity and apply gravity

	this->SmokeVelocity.X = 0.0f;
	this->SmokeVelocity.Y = 0.0f;
	this->SmokeVelocity.Z = -2.0f - RulesClass::Instance->Gravity;

	Vector3D<float> velocity = CoordToVector(this->Location);

	// Calculate current and next positions
	CoordStruct currentPos = VectorToCoord(velocity);
				velocity += this->SmokeVelocity;
	CoordStruct nextPos = VectorToCoord(velocity);

	// Get terrain height and cell info
	const int terrainHeight = MapClass::Instance->GetZPos(&nextPos);
	CellClass* cell = MapClass::Instance->GetCellAt(nextPos);

	// Check collision conditions
	const CollisionState collision = this->CheckCollision(cell, currentPos, nextPos, terrainHeight, velocity);

	// Handle collision and update velocity
	if (ShouldAdjustForTerrain(velocity.Z, terrainHeight, collision)) {
		velocity.Z = AdjustForTerrainHeight(velocity.Z, terrainHeight, collision);
	}

	// Apply surface normal rotation if needed
	if (collision.shouldApplyRotation) {
		ReflectOffSurface(nextPos, velocity, this->SmokeVelocity);
	}

	// Update gas height based on particle height
	this->UpdateGasHeight();
	this->ProcessEndState();
}

#pragma endregion


void FakeParticleClass::__Web_AI()
{
	auto pCell = MapClass::Instance->GetCellAt(this->Location);

	if (auto pWarhead = this->Type->Warhead)
	{
		for (auto pCur = pCell->FirstObject; pCur; pCur = pCur->NextObject)
		{
			if (pCur && pCur->IsAlive && pCur->Health > 0)
			{
				int damage = this->Type->Damage;
				pCur->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr);
			}
		}
	}

	const int Id = this->Fetch_ID();
	const int Ecs = LOWORD(this->Type->MaxEC) - this->RemainingEC + Id;
	const int Ecs_ = LOBYTE(this->Type->StateAIAdvance) + (this->Fetch_ID() & 1);

	if (!(Ecs % Ecs_))
		++this->StartStateAI;

	if (this->StartStateAI == this->Type->EndStateAI)
	{
		if (this->Type->DeleteOnStateLimit)
			this->hasremaining = false;
		else
			this->StartStateAI = 0;
	}
}

#pragma region Railgun

void FakeParticleClass::__Railgun_AI() {
	// Apply velocity with random jitter
	this->ApplyVelocityWithJitter();

	// Update color animation
	this->AdvanceColorCycle();
}

void  FakeParticleClass::ApplyVelocityWithJitter() {
	// Add small random perturbation to velocity (-0.05 to +0.05)
	this->Velocity += GetRandomFloat(-0.05f, 0.05f);

	// Calculate and apply velocity delta
	const Vector3D<float> delta {
		this->Spark10C.X * this->Velocity,
		this->Spark10C.Y * this->Velocity,
		this->Spark10C.Z * this->Velocity
	};

	// Update accumulated position
	this->vector3_118 += delta;

	// Apply new coordinates
	const CoordStruct newCoord {
		static_cast<int>(this->vector3_118.X),
		static_cast<int>(this->vector3_118.Y),
		static_cast<int>(this->vector3_118.Z)
	};

	this->SetLocation(newCoord);
}

void FakeParticleClass::AdvanceColorCycle()
{
	// Progress color animation with random variation

	this->ColorSpeedResult += this->Type->ColorSpeed + GetRandomFloat(0.0f, 0.05f);

	// Wrap color index when cycle completes
	if (this->ColorSpeedResult > 1.0) {
		const bool isLastColor = this->RefCount >= this->Type->ColorList.Count - 2;

		if (isLastColor) {
			// Loop back to start
			this->RefCount = 0;
			this->ColorSpeedResult = 1.0;
		} else {
			// Move to next color
			++this->RefCount;
			this->ColorSpeedResult = 0.0;
		}
	}
}

#pragma endregion

#pragma region Gas

void FakeParticleClass::ProcessGasMovement()
{
	// 25% chance to update drift direction
	if ((Math::abs(ScenarioClass::Instance->Random.Random()) & 3) == 0) {
		this->ApplyRandomDrift();
	}

	// Gas doesn't drift vertically
	this->GasVelocity.Z = 0;
}

void FakeParticleClass::ApplyRandomDrift()
{
	// Choose random axis (X or Y) and direction (-1, 0, +1)
	const bool driftInY = (Math::abs(ScenarioClass::Instance->Random.Random()) & 1) != 0;
	const int drift = (Math::abs(ScenarioClass::Instance->Random.Random()) % 3) - 1;

	if (driftInY) {
		this->GasVelocity.Y = std::clamp(this->GasVelocity.Y + drift, -5, 5);
	} else {
		this->GasVelocity.X = std::clamp(this->GasVelocity.X + drift, -5, 5);
	}
}

void FakeParticleClass::UpdateAnimationFrame()
{
	const auto* type = this->Type;

	if (this->StartStateAI >= type->EndStateAI) {
		return; // Animation complete
	}

	// Check if it's time to advance to next frame
	if (this->ShouldAdvanceFrame()) {
		++this->StartStateAI;

		// Mark for deletion if animation ended
		if (this->StartStateAI == type->EndStateAI && type->DeleteOnStateLimit) {
			this->hasremaining = 1;
		}
	}
}

bool FakeParticleClass::ShouldAdvanceFrame() const
{
	const int id = this->Fetch_ID();
	const int elapsed = this->Type->MaxEC - this->RemainingEC + id;
	const int frameDelay = this->Type->StateAIAdvance + (id & 1);

	return (elapsed % frameDelay) == 0;
}

void FakeParticleClass::DecelerateIfNeeded()
{
	constexpr float DECEL_THRESHOLD = 3.0f;

	if (this->Velocity > DECEL_THRESHOLD) {
		this->Velocity -= this->Type->Deacc;
	}
}

void FakeParticleClass::__Gas_AI() {

	// Process gas movement on odd frames
	if (Unsorted::CurrentFrame & 1) {
		this->ProcessGasMovement();
	}

	// Update animation and physics
	this->UpdateAnimationFrame();
	this->DecelerateIfNeeded();
}

#pragma endregion

#pragma region Spark

PhysicsState FakeParticleClass::UpdatePhysics()
{
	PhysicsState state {};

	// Get current position
	state.position = Vector3D<float> {
		static_cast<float>(this->Location.X),
		static_cast<float>(this->Location.Y),
		static_cast<float>(this->Location.Z)
	};

	// Apply gravity twice (current + next frame)
	this->Spark10C.Z -= RulesClass::Instance->Gravity;
	state.velocity = this->Spark10C;
	state.velocity.Z -= RulesClass::Instance->Gravity;

	// Calculate next position
	state.position += state.velocity;
	state.coord = VectorToCoord(state.position);
	state.terrainHeight = MapClass::Instance->GetZPos(&state.coord);
	state.cell = MapClass::Instance->GetCellAt(state.coord);

	return state;
}

Collision FakeParticleClass::CheckBridgeCollision(const PhysicsState& physics) const
{
	const bool hasBridgeHead = (physics.cell->Flags & CellFlags::BridgeHead) != CellFlags::Empty;

	if (!hasBridgeHead) {
		return {};
	}

	const int waterHeight = physics.terrainHeight + (Unsorted::CellHeight * 2);
	const int prevZ = this->Location.Z;

	Collision result { true , true , CollisionType::None };

	if (physics.coord.Z < waterHeight && prevZ >= waterHeight) {
		result.type = CollisionType::BridgeTop;
	}
	else if (prevZ < waterHeight && physics.coord.Z >= waterHeight) {
		result.type = CollisionType::BridgeBottom;
	} else {
		result.occurred = false;
	}

	return result;
}

Collision FakeParticleClass::CheckBuildingCollision(const PhysicsState& physics) const
{
	if(!CheckCellBuildingCollision(physics.cell, physics.position.Z, physics.terrainHeight))
		return {};

	return Collision { true, true, CollisionType::Building };
}

Collision FakeParticleClass::DetectCollision(const PhysicsState& physics) const
{
	Collision result;

	// Check water collision
	if (const auto waterCol = this->CheckBridgeCollision(physics); waterCol.occurred) {
		return waterCol;
	}

	// Check building/terrain collision
	return this->CheckBuildingCollision(physics);
}

void FakeParticleClass::HandleCollisions(PhysicsState& physics)
{
	const auto collision = this->DetectCollision(physics);

	if (!collision.occurred)
	{
		// No collision, just update position
		this->SetLocation(physics.coord);
		return;
	}

	// Adjust position for collision
	physics.position.Z = GetCollisionHeight(collision, physics.terrainHeight);
	physics.coord = VectorToCoord(physics.position);
	this->SetLocation(physics.coord);

	// Reflect velocity and mark for deletion
	if (collision.shouldBounce) {
		ReflectOffSurface(physics.coord, physics.velocity, this->Spark10C);
		this->hasremaining = 1;
	}
}

void FakeParticleClass::AdvanceColorAnimation()
{
	const float random = (float)ScenarioClass::Instance->Random.RandomDouble();
	this->ColorSpeedResult += this->Type->ColorSpeed + (random * 0.05f);

	if (this->ColorSpeedResult > 1.0)
	{
		const bool isLastColor = this->RefCount >= this->Type->ColorList.Count - 2;
		this->RefCount = isLastColor ? 0 : this->RefCount + 1;
		this->ColorSpeedResult = isLastColor ? 1.0 : 0.0;
	}
}

void FakeParticleClass::__Spark_AI() {
	// Physics update: gravity and movement
	PhysicsState physics = this->UpdatePhysics();

	// Collision detection and response
	this->HandleCollisions(physics);

	// Visual update: color cycling
	this->AdvanceColorAnimation();
}

#pragma endregion

DEFINE_FUNCTION_JUMP(LJMP, 0x62CE40, FakeParticleClass::__AI)
DEFINE_FUNCTION_JUMP(CALL, 0x62E6ED, FakeParticleClass::__AI)
DEFINE_FUNCTION_JUMP(CALL, 0x62ECFE, FakeParticleClass::__AI)
DEFINE_FUNCTION_JUMP(CALL, 0x62EDD0, FakeParticleClass::__AI)
DEFINE_FUNCTION_JUMP(CALL, 0x62F961, FakeParticleClass::__AI)
DEFINE_FUNCTION_JUMP(CALL, 0x62F9C2, FakeParticleClass::__AI)

std::pair<TechnoClass*, HouseClass*> ParticleExtData::GetOwnership(ParticleClass* pThis)
{
	TechnoClass* pAttacker = nullptr;
	HouseClass* pOwner = nullptr;
	BulletClass* pBullet = nullptr;

	if (auto const pSystem = pThis->ParticleSystem)
	{
		if (auto pSystemOwner = pSystem->Owner)
		{
			if (((pSystemOwner->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
				pAttacker = static_cast<TechnoClass*>(pSystemOwner);
			else if (pSystemOwner->WhatAmI() == BulletClass::AbsID)
			{
				pBullet = static_cast<BulletClass*>(pSystemOwner);
				pAttacker = static_cast<BulletClass*>(pSystemOwner)->Owner;
			}

		}

		if (pAttacker)
			pOwner = pAttacker->GetOwningHouse();
		else if (pBullet)
			pOwner = BulletExtContainer::Instance.Find(pBullet)->Owner;
	}

	return { pAttacker , pOwner };
}

// =============================
// load / save

template <typename T>
void ParticleExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LaserTrails)
		.Process(this->Trails)

		;
}

// =============================
// container
ParticleExtContainer ParticleExtContainer::Instance;
std::vector<ParticleExtData*> Container<ParticleExtData>::Array;

void Container<ParticleExtData>::Clear()
{
	Array.clear();
}

bool ParticleExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool ParticleExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks

ASMJIT_PATCH(0x62BB13, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);

	if (pItem->Type) {
		auto pExt = ParticleExtContainer::Instance.Allocate(pItem);
		const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pItem->Type);
		CoordStruct nFLH = CoordStruct::Empty;
		const ColorStruct nColor = pItem->GetOwningHouse() ? pItem->GetOwningHouse()->LaserColor : ColorStruct::Empty;

		if (pExt->LaserTrails.empty() && !LaserTrailTypeClass::Array.empty())
		{
			pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

			for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
			{
				pExt->LaserTrails.emplace_back(
					std::move(std::make_unique<LaserTrailClass>(
						LaserTrailTypeClass::Array[idxTrail].get(), nColor, nFLH)));
			}
		}

		TrailsManager::Construct(pItem);
	}

	return 0;
}

ASMJIT_PATCH(0x62D9CD, ParticleClass_DTOR, 0xA)
{
	GET(ParticleClass* const, pItem, ESI);
	ParticleExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x62BCED, ParticleClass_DTOR, 0xA)

void FakeParticleClass::_Detach(AbstractClass* pTarget, bool bRemove)
{
	this->ObjectClass::PointerExpired(pTarget ,bRemove);

	//ParticleExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF97C, FakeParticleClass::_Detach)

HRESULT __stdcall FakeParticleClass::_Load(IStream* pStm)
{
	HRESULT hr = this->ParticleClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = ParticleExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeParticleClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->ParticleClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = ParticleExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF968, FakeParticleClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF96C, FakeParticleClass::_Save)