// Precipitation Particle AI - Rain and Snow effects
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
// - Clear visual distinction between member and non-member calls
//
// Enum Value: ParticleTypeBehavesLike::Precipitation = 5
// 
// INI Properties Usage:
// - Velocity: Fall speed (higher = rain, lower = snow)
// - WindEffect: How much wind affects particle (snow = high, rain = low)
// - XVelocity/YVelocity: Wobble amplitude for snow
// - ZVelocityRange: Wobble speed variation
// - Radius: Splash/puff effect radius
// - NextParticle: Splash/puff particle to spawn on impact
// - Translucency: Visibility (rain can be more transparent)
//
// Extension Properties Needed:
// - bool IsSnow: Determines drift behavior (snow drifts more)
// - float WobblePhase: Current wobble phase for snow
// - int AccumulationValue: Visual-only accumulation tracking

class ParticleClass
{
public:
	void Precipitation_AI()
	{
		// Update particle falling motion
		this->UpdateFallingMotion();

		// Check ground collision
		if (this->CheckGroundImpact())
		{
			this->CreateImpactEffect();
			this->hasremaining = 1; // Remove particle
		}

		// Update color/translucency for fade effect
		this->UpdateVisualFade();
	}

	void Precipitation_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Apply wind drift
		this->ApplyWindDrift(newPos);

		// Apply falling motion with wobble
		this->ApplyFalling(newPos);

		// Update position
		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void UpdateFallingMotion()
	{
		// Snow wobbles as it falls, rain doesn't
		// Check if this is snow by checking velocity (snow is slower)
		const bool isSnow = this->Velocity < 5.0f;

		if (isSnow)
		{
			// Apply wobble to snow particles
			this->ApplySnowWobble();
		}

		// Extension: More explicit snow detection
		// if (this->Ext->IsSnow) {
		//     this->ApplySnowWobble();
		// }
	}

	void ApplySnowWobble()
	{
		// Update wobble phase
		// Extension: this->Ext->WobblePhase += 0.1f;
		const float wobblePhase = Frame * 0.1f;

		// Calculate wobble offset based on particle type settings
		const float wobbleAmplitudeX = this->Type->XVelocity * 0.01f;
		const float wobbleAmplitudeY = this->Type->YVelocity * 0.01f;

		// Apply sinusoidal wobble to direction
		// Store wobble in Gas_AI_C0 (reusing existing field)
		this->Gas_AI_C0.X = static_cast<int>(FastMath::Sin(wobblePhase) * wobbleAmplitudeX * 10.0f);
		this->Gas_AI_C0.Y = static_cast<int>(FastMath::Cos(wobblePhase * 1.3f) * wobbleAmplitudeY * 10.0f);
	}

	bool CheckGroundImpact() const
	{
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);

		// Check if particle reached ground level
		return this->o.Coord.Z <= terrainHeight + 5;
	}

	void CreateImpactEffect()
	{
		// Spawn splash (rain) or puff (snow) particle
		if (this->Type->NextParticle == -1)
		{
			return;
		}

		// Extension: Track accumulation
		// this->Ext->AccumulationValue could be added to cell data
		// CellClass->Ext->SnowAccumulation += 1;

		// Spawn impact particle at ground level
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);
		CoordStruct impactPos {
			this->o.Coord.X,
			this->o.Coord.Y,
			terrainHeight
		};

		// Create splash/puff particle
		ParticleClass* impact = operator new(sizeof(ParticleClass));
		if (impact)
		{
			ParticleTypeClass* impactType = ParticleTypes.Vector_Item[this->Type->NextParticle];
			ParticleClass::ParticleClass(impact, impactType, &impactPos, &impactPos, this->ParticleSystem);
		}
	}

	void UpdateVisualFade()
	{
		// Fade out as particle gets close to ground for smooth transition
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);
		const int heightAboveGround = this->o.Coord.Z - terrainHeight;

		if (heightAboveGround < 100)
		{
			// Gradually increase translucency as it gets closer
			const int fadeAmount = (100 - heightAboveGround) / 4;
			this->Translucency = std::min(100, this->Type->Translucency + fadeAmount);
		}
	}

	void ApplyWindDrift(CoordStruct& pos)
	{
		const int windEffect = this->Type->WindEffect;
		if (windEffect <= 0)
		{
			return;
		}

		// Get wind direction
		const int windDir = Rule->WindDirection;
		const Point2D windVector = Wind[windDir];

		// Snow drifts more than rain (based on velocity as proxy for type)
		const bool isSnow = this->Velocity < 5.0f;
		const float driftMultiplier = isSnow ? 1.5f : 1.0f;

		// Extension: More explicit control
		// float driftMultiplier = this->Ext->IsSnow ? this->Ext->DriftIntensity : 1.0f;

		// Apply wind effect
		pos.X += static_cast<int>(windVector.X * windEffect * driftMultiplier);
		pos.Y += static_cast<int>(windVector.Y * windEffect * driftMultiplier);
	}

	void ApplyFalling(CoordStruct& pos)
	{
		// Apply downward falling velocity
		const int fallSpeed = static_cast<int>(this->Velocity);
		pos.Z -= fallSpeed;

		// Apply wobble offset for snow
		const bool isSnow = this->Velocity < 5.0f;
		if (isSnow)
		{
			pos.X += this->Gas_AI_C0.X;
			pos.Y += this->Gas_AI_C0.Y;
		}

		// Clamp to ground
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &pos);
		if (pos.Z < terrainHeight)
		{
			pos.Z = terrainHeight;
		}
	}
};

// =============================================================================
// ParticleSystemClass - Weather system spawner
// =============================================================================

class ParticleSystemClass
{
public:
	char Precipitation_AI()
	{
		// Spawn new precipitation particles
		if (!this->TimeToDie && this->obj.IsActive)
		{
			this->SpawnPrecipitationParticles();
		}

		// Update all particles
		this->UpdateAllParticles();

		// Remove dead particles
		return this->RemoveDeadParticles();
	}

private:
	void SpawnPrecipitationParticles()
	{
		// Check spawn timing
		if ((Frame % this->Class->SpawnFrames) != 0)
		{
			return;
		}

		// Spawn multiple particles for area coverage
		const int particleCount = this->CalculatePrecipitationDensity();

		for (int i = 0; i < particleCount; ++i)
		{
			this->SpawnPrecipitationParticle();
		}
	}

	int CalculatePrecipitationDensity() const
	{
		// Extension: Could vary based on weather intensity
		// return this->Ext->WeatherIntensity * this->Class->ParticleCap;

		// Default: spawn multiple particles per frame for coverage
		return std::min(5, this->Class->ParticleCap / 10);
	}

	void SpawnPrecipitationParticle()
	{
		if (this->Class->HoldsWhat == -1)
		{
			return;
		}

		// Spawn at random position within radius above system
		const int spawnRadius = this->Class->SpawnRadius;
		const int randomX = Random2Class::operator()(&Scen->RandomNumber) % (spawnRadius * 2) - spawnRadius;
		const int randomY = Random2Class::operator()(&Scen->RandomNumber) % (spawnRadius * 2) - spawnRadius;

		// Spawn high above ground
		const CoordStruct spawnPos {
			this->obj.Coord.X + randomX,
			this->obj.Coord.Y + randomY,
			this->obj.Coord.Z + 500 // High starting altitude
		};

		// Create particle
		ParticleClass* particle = operator new(sizeof(ParticleClass));
		if (!particle)
		{
			return;
		}

		ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
		particle = ParticleClass::ParticleClass(particle, type, &spawnPos, &spawnPos, this);

		if (particle)
		{
			DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &particle);

			// Set initial velocity based on type
			particle->Velocity = type->Velocity;

			// Initialize wobble for snow
			if (type->Velocity < 5.0f)
			{ // Snow check
				particle->Gas_AI_C0.X = 0;
				particle->Gas_AI_C0.Y = 0;
				particle->Gas_AI_C0.Z = 0;
			}
		}
	}

	void UpdateAllParticles()
	{
		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			ParticleClass::AI(particle);
			ParticleClass::Coord_AI(particle);
		}
	}

	char RemoveDeadParticles()
	{
		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			if (particle->hasremaining)
			{
				particle->o.a.vftable->t.r.m.o.Remove_This_deletethis(&particle->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}
};

// =============================================================================
// INI Example Configuration
// =============================================================================
/*
[RainParticle]
BehavesLike=5          ; Precipitation
Velocity=12.0          ; Fast falling (rain)
WindEffect=2           ; Slight wind drift
NextParticle=RainSplash ; Splash on impact
Translucency=40        ; Semi-transparent
XVelocity=0            ; No wobble for rain
YVelocity=0
ZVelocityRange=0
Radius=10

[SnowParticle]
BehavesLike=5          ; Precipitation
Velocity=2.0           ; Slow falling (snow)
WindEffect=8           ; Heavy wind drift
NextParticle=SnowPuff  ; Puff on impact
Translucency=20        ; More visible
XVelocity=30           ; Wobble amplitude X
YVelocity=25           ; Wobble amplitude Y
ZVelocityRange=10      ; Wobble variation
Radius=15

[RainSplash]
BehavesLike=1          ; Gas (quick dissipate)
Velocity=0.5
MaxEC=15               ; Short lifetime
Translucency=60

[SnowPuff]
BehavesLike=1          ; Gas (settles on ground)
Velocity=0.2
MaxEC=30
Translucency=30

[RainSystem]
BehavesLike=5          ; Precipitation system
HoldsWhat=RainParticle
SpawnFrames=1          ; Continuous spawning
SpawnRadius=300        ; Wide area coverage
ParticleCap=100        ; Many particles for dense rain

[SnowSystem]
BehavesLike=5          ; Precipitation system
HoldsWhat=SnowParticle
SpawnFrames=2          ; Slower spawn rate
SpawnRadius=400        ; Even wider coverage
ParticleCap=80         ; Moderate density

; Usage Example:
; Attach to map trigger or weather controller
; Can also attach to invisible dummy unit that follows camera
*/

//========================================================================================================================================

// Electric Particle AI - Lightning and electrical arcs
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
// - Clear visual distinction between member and non-member calls
//
// Enum Value: ParticleTypeBehavesLike::Electric = 6
//
// INI Properties Usage:
// - Velocity: Arc travel speed
// - Damage/Warhead: Electrical damage on contact
// - NextParticle: Branch/fork particle type
// - XVelocity/YVelocity/ZVelocityRange: Jitter amount
// - Radius: Arc search radius for targets
// - ColorList: Lightning color cycling (white/blue)
// - MaxDC: Damage application frequency
//
// Extension Properties Needed:
// - int ChainCount: How many times it can chain to new targets
// - float BranchChance: Probability of creating branches (0.0-1.0)
// - int SearchRadius: Radius to search for conductive targets
// - bool SeeksTargets: Whether to arc toward units/buildings

class ParticleClass
{
public:
	void Electric_AI()
	{
		// Add erratic jitter to movement
		this->ApplyElectricJitter();

		// Try to arc to nearby targets
		if (this->TryArcToTarget())
		{
			return; // Arced to new target, particle transforms
		}

		// Random branching
		this->TryCreateBranch();

		// Apply damage to nearby units
		this->ApplyElectricDamage();

		// Update visual flickering
		this->UpdateElectricFlicker();
	}

	void Electric_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Move along velocity vector with jitter
		newPos.X += this->vector3_10C.X * this->Velocity;
		newPos.Y += this->vector3_10C.Y * this->Velocity;
		newPos.Z += this->vector3_10C.Z * this->Velocity;

		// Add random jitter for erratic movement
		const int jitterX = Random2Class::operator()(&Scen->RandomNumber) % 20 - 10;
		const int jitterY = Random2Class::operator()(&Scen->RandomNumber) % 20 - 10;
		const int jitterZ = Random2Class::operator()(&Scen->RandomNumber) % 20 - 10;

		newPos.X += jitterX;
		newPos.Y += jitterY;
		newPos.Z += jitterZ;

		// Update position
		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ApplyElectricJitter()
	{
		// Randomly change direction slightly for jagged appearance
		if ((Frame & 1) != 0)
		{
			const float jitterX = (Random2Class::operator()(&Scen->RandomNumber) % this->Type->XVelocity) * 0.01f;
			const float jitterY = (Random2Class::operator()(&Scen->RandomNumber) % this->Type->YVelocity) * 0.01f;
			const float jitterZ = (Random2Class::operator()(&Scen->RandomNumber) % this->Type->ZVelocityRange) * 0.01f;

			this->vector3_10C.X += jitterX;
			this->vector3_10C.Y += jitterY;
			this->vector3_10C.Z += jitterZ;

			// Normalize to maintain speed
			const float length = FastMath::Sqrt(
				this->vector3_10C.X * this->vector3_10C.X +
				this->vector3_10C.Y * this->vector3_10C.Y +
				this->vector3_10C.Z * this->vector3_10C.Z
			);

			if (length > 0.0f)
			{
				this->vector3_10C.X /= length;
				this->vector3_10C.Y /= length;
				this->vector3_10C.Z /= length;
			}
		}
	}

	bool TryArcToTarget()
	{
		// Extension: Check if this particle seeks targets
		// if (!this->Ext->SeeksTargets) return false;
		// const int searchRadius = this->Ext->SearchRadius;

		const int searchRadius = this->Type->Radius;
		if (searchRadius <= 0)
		{
			return false;
		}

		// Find nearest conductive target (unit or building)
		TechnoClass* target = this->FindNearestConductiveTarget(searchRadius);
		if (!target)
		{
			return false;
		}

		// Extension: Check chain count
		// if (this->Ext->ChainCount <= 0) return false;

		// Create arc to target
		this->CreateArcToTarget(target);
		return true;
	}

	TechnoClass* FindNearestConductiveTarget(int radius) const
	{
		// Search cells in radius for units/buildings
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* centerCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		TechnoClass* nearestTarget = nullptr;
		int nearestDistance = radius;

		// Simple radius search (in production, would use proper cell iteration)
		for (int dx = -3; dx <= 3; ++dx)
		{
			for (int dy = -3; dy <= 3; ++dy)
			{
				Point2DStruct searchCell { cellPos.X + dx * 256, cellPos.Y + dy * 256 };
				CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &searchCell);

				if (cell->OccupierPtr)
				{
					TechnoClass* unit = &cell->OccupierPtr->t;
					if (unit->r.m.o.IsActive && unit->r.m.o.Strength > 0)
					{
						const int distance = this->CalculateDistance(unit->r.m.o.Coord);
						if (distance < nearestDistance)
						{
							nearestTarget = unit;
							nearestDistance = distance;
						}
					}
				}
			}
		}

		return nearestTarget;
	}

	int CalculateDistance(const CoordStruct& target) const
	{
		const int dx = target.X - this->o.Coord.X;
		const int dy = target.Y - this->o.Coord.Y;
		const int dz = target.Z - this->o.Coord.Z;
		return FastMath::Sqrt(dx * dx + dy * dy + dz * dz);
	}

	void CreateArcToTarget(TechnoClass* target)
	{
		// Extension: Decrement chain count
		// this->Ext->ChainCount--;

		// Spawn new electric particle at target
		CoordStruct targetPos;
		target->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(&target->r.m.o.a, &targetPos);

		ParticleClass* arc = operator new(sizeof(ParticleClass));
		if (arc)
		{
			ParticleClass::ParticleClass(arc, this->Type, &targetPos, &targetPos, this->ParticleSystem);
			// Extension: Copy chain count to new particle
			// arc->Ext->ChainCount = this->Ext->ChainCount;
		}

		// Mark this particle for removal (it transformed into arc)
		this->hasremaining = 1;
	}

	void TryCreateBranch()
	{
		// Extension: Use branch chance
		// const float branchChance = this->Ext->BranchChance;
		const float branchChance = 0.15f; // 15% default

		if (this->Type->NextParticle == -1)
		{
			return;
		}

		const float random = Random2Class::operator()(&Scen->RandomNumber, 0, INT_MAX) * 4.656612877414201e-10f;
		if (random > branchChance)
		{
			return;
		}

		// Create branch particle
		CoordStruct branchPos = this->o.Coord;
		ParticleClass* branch = operator new(sizeof(ParticleClass));
		if (branch)
		{
			ParticleTypeClass* branchType = ParticleTypes.Vector_Item[this->Type->NextParticle];
			ParticleClass::ParticleClass(branch, branchType, &branchPos, &branchPos, this->ParticleSystem);

			// Give branch a random direction perpendicular to current
			branch->vector3_10C.X = this->vector3_10C.Y;
			branch->vector3_10C.Y = -this->vector3_10C.X;
			branch->vector3_10C.Z = this->vector3_10C.Z * 0.5f;
			branch->Velocity = this->Velocity * 0.8f;
		}
	}

	void ApplyElectricDamage()
	{
		if (--this->RemainingDC > 0)
		{
			return;
		}

		if (!this->Type->Damage)
		{
			return;
		}

		this->RemainingDC = this->Type->MaxDC;

		// Damage units in small radius
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		for (FootClass* unit = cell->OccupierPtr; unit; unit = unit->t.r.m.o.Next)
		{
			if (unit->t.r.m.o.IsActive && unit->t.r.m.o.Strength > 0)
			{
				const float damage = this->Type->Damage;
				unit->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
					&unit->t.r.m.o,
					&damage,
					0,
					this->Type->Warhead,
					nullptr,
					false,
					false,
					0
				);
			}
		}
	}

	void UpdateElectricFlicker()
	{
		// Rapidly cycle through colors for flickering effect
		if ((Frame & 1) != 0)
		{
			this->RefCount = (this->RefCount + 1) % this->Type->ColorList.ActiveCount;
		}
	}
};

// =============================================================================
// ParticleSystemClass - Lightning strike spawner
// =============================================================================

class ParticleSystemClass
{
public:
	char Electric_AI()
	{
		// Spawn initial lightning bolt
		if (!this->TimeToDie && this->particles.ActiveCount == 0)
		{
			this->SpawnLightningBolt();
			this->TimeToDie = 1; // One-shot effect
		}

		// Update all particles
		this->UpdateAllParticles();

		// Remove dead particles
		return this->RemoveDeadParticles();
	}

private:
	void SpawnLightningBolt()
	{
		if (this->Class->HoldsWhat == -1)
		{
			return;
		}

		// Spawn from source to target
		const int segments = 5; // Create multiple segments for jagged appearance

		for (int i = 0; i < segments; ++i)
		{
			const float progress = static_cast<float>(i) / static_cast<float>(segments);

			CoordStruct segmentPos;
			CoordStruct::Lerp(&segmentPos,
							 reinterpret_cast<Point3D*>(&this->obj.Coord),
							 reinterpret_cast<Point3D*>(&this->TargetCoords),
							 progress);

			// Add random offset for jagged appearance
			segmentPos.X += Random2Class::operator()(&Scen->RandomNumber) % 100 - 50;
			segmentPos.Y += Random2Class::operator()(&Scen->RandomNumber) % 100 - 50;

			// Create segment
			this->CreateLightningSegment(segmentPos);
		}
	}

	void CreateLightningSegment(const CoordStruct& position)
	{
		ParticleClass* segment = operator new(sizeof(ParticleClass));
		if (!segment)
		{
			return;
		}

		ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
		segment = ParticleClass::ParticleClass(segment, type, &position, &this->TargetCoords, this);

		if (segment)
		{
			DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &segment);
		}
	}

	void UpdateAllParticles()
	{
		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}
	}

	char RemoveDeadParticles()
	{
		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			if (particle->hasremaining)
			{
				particle->o.a.vftable->t.r.m.o.Remove_This_deletethis(&particle->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}
};

// =============================================================================
// INI Example Configuration
// =============================================================================
/*
[LightningParticle]
BehavesLike=Electric
Velocity=15.0
Damage=50
Warhead=Electric
XVelocity=30
YVelocity=30
ZVelocityRange=30
Radius=200              ; Search radius for arcing
NextParticle=LightningBranch
MaxDC=5

[LightningSystem]
BehavesLike=Electric
HoldsWhat=LightningParticle
Lifetime=10
*/

//========================================================================================================================================

// Liquid Particle AI - Water, acid, lava, blood effects
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
// - Clear visual distinction between member and non-member calls
//
// Enum Value: ParticleTypeBehavesLike::Liquid = 7
//
// INI Properties Usage:
// - Velocity: Initial spray velocity
// - Deacc: Gravity pull (how fast it falls)
// - NextParticle: Puddle/splash particle on impact
// - Damage/Warhead: Acid/lava damage
// - Translucency: Liquid transparency
// - Radius: Splash radius
// - WindEffect: How wind affects droplets
//
// Extension Properties Needed:
// - float Viscosity: How thick the liquid is (affects flow speed)
// - int PuddleLifetime: How long puddles last
// - bool FlowsDownSlopes: Whether liquid follows terrain slopes
// - ColorStruct LiquidColor: Color of the liquid

class ParticleClass
{
public:
	void Liquid_AI()
	{
		// Apply gravity
		this->ApplyLiquidGravity();

		// Check for ground impact
		if (this->CheckLiquidImpact())
		{
			this->CreateSplashEffect();
			this->hasremaining = 1;
		}

		// Apply damage if harmful liquid (acid/lava)
		if (this->Type->Damage > 0)
		{
			this->ApplyLiquidDamage();
		}
	}

	void Liquid_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Apply velocity with gravity
		newPos.X += static_cast<int>(this->vector3_10C.X * this->Velocity);
		newPos.Y += static_cast<int>(this->vector3_10C.Y * this->Velocity);
		newPos.Z += static_cast<int>(this->vector3_10C.Z * this->Velocity);

		// Apply wind effect (lighter for droplets)
		if (this->Type->WindEffect > 0 && (Frame & 1) != 0)
		{
			const int windDir = Rule->WindDirection;
			const Point2D windVector = Wind[windDir];
			newPos.X += windVector.X * this->Type->WindEffect;
			newPos.Y += windVector.Y * this->Type->WindEffect;
		}

		// Extension: Flow down slopes if on ground
		// if (this->Ext->FlowsDownSlopes) { this->ApplySlope Flow(newPos); }

		// Update position
		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ApplyLiquidGravity()
	{
		// Decelerate velocity (gravity pull)
		this->Velocity -= this->Type->Deacc;
		if (this->Velocity < 0.0f)
		{
			this->Velocity = 0.0f;
		}

		// Update Z velocity component (falling)
		this->vector3_10C.Z -= Rule->Gravity * 0.1f;
	}

	bool CheckLiquidImpact() const
	{
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);

		// Check ground impact
		if (this->o.Coord.Z <= terrainHeight + 10)
		{
			return true;
		}

		// Check building collision
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);
		BuildingClass* building = CellClass::Cell_Building(cell);

		if (building)
		{
			return true;
		}

		return false;
	}

	void CreateSplashEffect()
	{
		if (this->Type->NextParticle == -1)
		{
			return;
		}

		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);

		// Create splash at impact point
		CoordStruct splashPos {
			this->o.Coord.X,
			this->o.Coord.Y,
			terrainHeight
		};

		// Spawn splash/puddle particle
		ParticleClass* splash = operator new(sizeof(ParticleClass));
		if (splash)
		{
			ParticleTypeClass* splashType = ParticleTypes.Vector_Item[this->Type->NextParticle];
			ParticleClass::ParticleClass(splash, splashType, &splashPos, &splashPos, this->ParticleSystem);

			// Extension: Set puddle lifetime
			// splash->Ext->RemainingLifetime = this->Type->Ext->PuddleLifetime;
		}

		// Create smaller droplets in splash radius
		const int splashRadius = this->Type->Radius;
		if (splashRadius > 0)
		{
			this->CreateSplashDroplets(splashPos, splashRadius);
		}
	}

	void CreateSplashDroplets(const CoordStruct& center, int radius)
	{
		const int dropletCount = 3 + (Random2Class::operator()(&Scen->RandomNumber) % 5);

		for (int i = 0; i < dropletCount; ++i)
		{
			const int angle = (i * 360) / dropletCount;
			const int distance = Random2Class::operator()(&Scen->RandomNumber) % radius;

			CoordStruct dropletPos {
				center.X + static_cast<int>(distance * FastMath::Cos(angle * 0.01745f)),
				center.Y + static_cast<int>(distance * FastMath::Sin(angle * 0.01745f)),
				center.Z + 20
			};

			ParticleClass* droplet = operator new(sizeof(ParticleClass));
			if (droplet)
			{
				ParticleClass::ParticleClass(droplet, this->Type, &dropletPos, &dropletPos, this->ParticleSystem);
				droplet->Velocity = this->Type->Velocity * 0.3f;
				droplet->vector3_10C.Z = 2.0f; // Small upward velocity
			}
		}
	}

	void ApplyLiquidDamage()
	{
		if (--this->RemainingDC > 0)
		{
			return;
		}

		this->RemainingDC = this->Type->MaxDC;

		// Damage units in cell
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		for (FootClass* unit = cell->OccupierPtr; unit; unit = unit->t.r.m.o.Next)
		{
			if (unit->t.r.m.o.IsActive && unit->t.r.m.o.Strength > 0)
			{
				const float damage = this->Type->Damage;
				unit->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
					&unit->t.r.m.o,
					&damage,
					0,
					this->Type->Warhead,
					nullptr,
					false,
					false,
					0
				);
			}
		}
	}
};

// =============================================================================
// ParticleSystemClass - Fountain/stream spawner
// =============================================================================

class ParticleSystemClass
{
public:
	char Liquid_AI()
	{
		// Spawn liquid stream
		if (!this->TimeToDie && this->obj.IsActive)
		{
			this->SpawnLiquidStream();
		}

		// Update all particles
		this->UpdateAllParticles();

		// Remove dead particles
		return this->RemoveDeadParticles();
	}

private:
	void SpawnLiquidStream()
	{
		if ((Frame % this->Class->SpawnFrames) != 0)
		{
			return;
		}

		if (this->Class->HoldsWhat == -1)
		{
			return;
		}

		// Spawn liquid droplet with arc trajectory
		this->SpawnLiquidDroplet();
	}

	void SpawnLiquidDroplet()
	{
		// Calculate trajectory to target
		const Vector3 direction {
			static_cast<float>(this->TargetCoords.X - this->obj.Coord.X),
			static_cast<float>(this->TargetCoords.Y - this->obj.Coord.Y),
			static_cast<float>(this->TargetCoords.Z - this->obj.Coord.Z)
		};

		const float distance = FastMath::Sqrt(
			direction.X * direction.X +
			direction.Y * direction.Y +
			direction.Z * direction.Z
		);

		// Normalize direction
		Vector3 normalized {
			direction.X / distance,
			direction.Y / distance,
			direction.Z / distance
		};

		// Add upward arc
		normalized.Z += 0.5f;

		// Create droplet
		CoordStruct spawnPos = this->obj.Coord;
		ParticleClass* droplet = operator new(sizeof(ParticleClass));
		if (!droplet)
		{
			return;
		}

		ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
		droplet = ParticleClass::ParticleClass(droplet, type, &spawnPos, &this->TargetCoords, this);

		if (droplet)
		{
			DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &droplet);
			droplet->vector3_10C = normalized;
			droplet->Velocity = type->Velocity;
		}
	}

	void UpdateAllParticles()
	{
		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			ParticleClass::AI(particle);
			ParticleClass::Coord_AI(particle);
		}
	}

	char RemoveDeadParticles()
	{
		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			if (particle->hasremaining)
			{
				particle->o.a.vftable->t.r.m.o.Remove_This_deletethis(&particle->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}
};

// =============================================================================
// INI Example Configuration
// =============================================================================
/*
[WaterDroplet]
BehavesLike=Liquid
Velocity=8.0
Deacc=0.5              ; Gravity pull
NextParticle=WaterSplash
Translucency=30
Radius=40              ; Splash radius
WindEffect=3

[AcidDroplet]
BehavesLike=Liquid
Velocity=6.0
Deacc=0.4
Damage=25
Warhead=Acid
NextParticle=AcidPuddle
Translucency=20
Radius=30

[LavaDroplet]
BehavesLike=Liquid
Velocity=5.0
Deacc=0.3
Damage=50
Warhead=Fire
NextParticle=LavaSplash
Translucency=0
Radius=50

[WaterFountain]
BehavesLike=Liquid
HoldsWhat=WaterDroplet
SpawnFrames=2
ParticleCap=50

[AcidStream]
BehavesLike=Liquid
HoldsWhat=AcidDroplet
SpawnFrames=3
ParticleCap=30
*/

//========================================================================================================================================

// Debris Particle AI - Explosion fragments and shrapnel
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
// - Clear visual distinction between member and non-member calls
//
// Enum Value: ParticleTypeBehavesLike::Debris = 8
//
// INI Properties Usage:
// - Velocity: Initial ejection speed
// - Deacc: Air resistance / gravity
// - Damage/Warhead: Shrapnel damage on impact
// - XVelocity/YVelocity/ZVelocityRange: Tumble rotation speeds
// - Radius: Damage radius on impact
// - NextParticle: Smaller fragments to spawn on bounce
//
// Extension Properties Needed:
// - int BounceCount: Number of times debris can bounce
// - float Elasticity: How much velocity retained on bounce (0.0-1.0)
// - float RotationSpeed: How fast debris tumbles
// - bool SticksToGround: Whether it stops after final bounce

class ParticleClass
{
public:
	void Debris_AI()
	{
		// Apply physics
		this->ApplyDebrisPhysics();

		// Check for collisions
		if (this->CheckDebrisCollision())
		{
			this->HandleDebrisImpact();
		}

		// Apply tumbling rotation (visual only)
		this->UpdateDebrisTumble();

		// Decelerate over time
		this->Velocity = std::max(0.0f, this->Velocity - this->Type->Deacc);

		// Remove if stopped and on ground
		if (this->Velocity <= 0.1f && this->IsOnGround())
		{
			// Extension: Check if it sticks
			// if (this->Ext->SticksToGround) {
			this->hasremaining = 1;
			// }
		}
	}

	void Debris_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Apply velocity
		newPos.X += static_cast<int>(this->vector3_10C.X * this->Velocity);
		newPos.Y += static_cast<int>(this->vector3_10C.Y * this->Velocity);
		newPos.Z += static_cast<int>(this->vector3_10C.Z * this->Velocity);

		// Apply gravity
		newPos.Z -= Rule->Gravity;

		// Update position
		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ApplyDebrisPhysics()
	{
		// Update Z velocity for gravity
		this->vector3_10C.Z -= Rule->Gravity * 0.1f;

		// Apply air resistance
		const float drag = 0.98f;
		this->vector3_10C.X *= drag;
		this->vector3_10C.Y *= drag;
		this->vector3_10C.Z *= drag;
	}

	bool CheckDebrisCollision()
	{
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);

		// Check ground collision
		if (this->o.Coord.Z <= terrainHeight + 5)
		{
			return true;
		}

		// Check building collision
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);
		BuildingClass* building = CellClass::Cell_Building(cell);

		return building != nullptr;
	}

	void HandleDebrisImpact()
	{
		// Extension: Check bounce count
		// if (this->Ext->BounceCount <= 0) {
		//     this->hasremaining = 1;
		//     return;
		// }
		// this->Ext->BounceCount--;

		// Apply bounce
		this->BounceDebris();

		// Apply impact damage
		if (this->Type->Damage > 0)
		{
			this->ApplyImpactDamage();
		}

		// Spawn smaller fragments
		if (this->Type->NextParticle != -1)
		{
			this->SpawnDebrisFragments();
		}
	}

	void BounceDebris()
	{
		// Extension: Use elasticity factor
		// const float elasticity = this->Ext->Elasticity;
		const float elasticity = 0.6f; // Default 60% energy retained

		// Reverse and reduce Z velocity (bounce)
		this->vector3_10C.Z = -this->vector3_10C.Z * elasticity;

		// Reduce horizontal velocity
		this->vector3_10C.X *= elasticity;
		this->vector3_10C.Y *= elasticity;

		// Reduce overall velocity
		this->Velocity *= elasticity;

		// Add some randomness to bounce direction
		const float randomX = (Random2Class::operator()(&Scen->RandomNumber) % 20 - 10) * 0.01f;
		const float randomY = (Random2Class::operator()(&Scen->RandomNumber) % 20 - 10) * 0.01f;
		this->vector3_10C.X += randomX;
		this->vector3_10C.Y += randomY;
	}

	void ApplyImpactDamage()
	{
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		// Damage units in radius
		for (FootClass* unit = cell->OccupierPtr; unit; unit = unit->t.r.m.o.Next)
		{
			if (!unit->t.r.m.o.IsActive || unit->t.r.m.o.Strength <= 0)
			{
				continue;
			}

			const int distance = this->CalculateDistanceToUnit(&unit->t.r.m.o);
			if (distance > this->Type->Radius)
			{
				continue;
			}

			const float damage = this->Type->Damage;
			const int adjustedDistance = distance / 10;

			unit->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
				&unit->t.r.m.o,
				&damage,
				adjustedDistance,
				this->Type->Warhead,
				nullptr,
				false,
				false,
				0
			);
		}
	}

	int CalculateDistanceToUnit(TechnoClass* unit) const
	{
		const int dx = unit->r.m.o.Coord.X - this->o.Coord.X;
		const int dy = unit->r.m.o.Coord.Y - this->o.Coord.Y;
		const int dz = unit->r.m.o.Coord.Z - this->o.Coord.Z;
		return FastMath::Sqrt(dx * dx + dy * dy + dz * dz);
	}

	void SpawnDebrisFragments()
	{
		// Spawn 2-4 smaller fragments
		const int fragmentCount = 2 + (Random2Class::operator()(&Scen->RandomNumber) % 3);

		for (int i = 0; i < fragmentCount; ++i)
		{
			CoordStruct fragPos = this->o.Coord;

			ParticleClass* fragment = operator new(sizeof(ParticleClass));
			if (!fragment)
			{
				continue;
			}

			ParticleTypeClass* fragType = ParticleTypes.Vector_Item[this->Type->NextParticle];
			fragment = ParticleClass::ParticleClass(fragment, fragType, &fragPos, &fragPos, this->ParticleSystem);

			if (fragment)
			{
				// Give random direction
				const float angle = (i * 360.0f) / fragmentCount;
				fragment->vector3_10C.X = FastMath::Cos(angle * 0.01745f);
				fragment->vector3_10C.Y = FastMath::Sin(angle * 0.01745f);
				fragment->vector3_10C.Z = 0.5f;
				fragment->Velocity = this->Velocity * 0.5f;
			}
		}
	}

	void UpdateDebrisTumble()
	{
		// Extension: Use rotation speed
		// this->Ext->CurrentRotation += this->Ext->RotationSpeed;

		// Visual rotation would be applied in rendering code
		// This just updates the state for rendering
		if ((Frame & 1) != 0)
		{
			this->Gas_AI_C0.X += this->Type->XVelocity / 10;
			this->Gas_AI_C0.Y += this->Type->YVelocity / 10;
			this->Gas_AI_C0.Z += this->Type->ZVelocityRange / 10;
		}
	}

	bool IsOnGround() const
	{
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &this->o.Coord);
		return this->o.Coord.Z <= terrainHeight + 10;
	}
};

// =============================================================================
// ParticleSystemClass - Explosion debris cloud
// =============================================================================

class ParticleSystemClass
{
public:
	char Debris_AI()
	{
		// Spawn initial debris burst
		if (!this->TimeToDie && this->particles.ActiveCount == 0)
		{
			this->SpawnDebrisBurst();
			this->TimeToDie = 1; // One-shot effect
		}

		// Update all particles
		this->UpdateAllParticles();

		// Remove dead particles
		return this->RemoveDeadParticles();
	}

private:
	void SpawnDebrisBurst()
	{
		if (this->Class->HoldsWhat == -1)
		{
			return;
		}

		// Spawn debris in all directions
		const int debrisCount = std::min(this->Class->ParticleCap, 20);

		for (int i = 0; i < debrisCount; ++i)
		{
			this->SpawnDebrisParticle(i, debrisCount);
		}
	}

	void SpawnDebrisParticle(int index, int total)
	{
		// Calculate direction (spread evenly in sphere)
		const float theta = (index * 360.0f) / total;
		const float phi = (Random2Class::operator()(&Scen->RandomNumber) % 180) - 90;

		Vector3 direction {
			FastMath::Cos(theta * 0.01745f) * FastMath::Cos(phi * 0.01745f),
			FastMath::Sin(theta * 0.01745f) * FastMath::Cos(phi * 0.01745f),
			FastMath::Sin(phi * 0.01745f)
		};

		// Random velocity variation
		const float velocityVariation = 0.5f + (Random2Class::operator()(&Scen->RandomNumber) % 100) * 0.01f;

		// Create debris particle
		CoordStruct spawnPos = this->obj.Coord;
		ParticleClass* debris = operator new(sizeof(ParticleClass));
		if (!debris)
		{
			return;
		}

		ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
		debris = ParticleClass::ParticleClass(debris, type, &spawnPos, &spawnPos, this);

		if (debris)
		{
			DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &debris);
			debris->vector3_10C = direction;
			debris->Velocity = type->Velocity * velocityVariation;
		}
	}

	void UpdateAllParticles()
	{
		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			ParticleClass::AI(particle);
			ParticleClass::Coord_AI(particle);
		}
	}

	char RemoveDeadParticles()
	{
		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			ParticleClass* particle = this->particles.Vector_Item[i];
			if (particle->hasremaining)
			{
				particle->o.a.vftable->t.r.m.o.Remove_This_deletethis(&particle->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}
};

// =============================================================================
// INI Example Configuration
// =============================================================================
/*
[MetalDebris]
BehavesLike=Debris
Velocity=10.0
Deacc=0.3              ; Air resistance
Damage=15
Warhead=AP
XVelocity=50           ; Tumble speed X
YVelocity=50           ; Tumble speed Y
ZVelocityRange=50      ; Tumble speed Z
Radius=50              ; Impact damage radius
NextParticle=SmallDebris

[ConcreteDebris]
BehavesLike=Debris
Velocity=8.0
Deacc=0.4
Damage=20
Warhead=HE
Radius=60

[ExplosionDebris]
BehavesLike=Debris
HoldsWhat=MetalDebris
ParticleCap=20
Lifetime=1
*/

//========================================================================================================================================

// Plasma Particle AI - Energy orbs and plasma effects
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Plasma = 9
//
// INI Properties Usage:
// - Velocity: Orbit speed
// - Radius: Orbit radius from origin
// - ColorList/ColorSpeed: Pulsing colors
// - Damage/Warhead: Energy damage
// - MaxDC: Damage frequency
//
// Extension Properties Needed:
// - float OrbitAngle: Current angle in orbit
// - CoordStruct OrbitCenter: Point to orbit around
// - float PulsePhase: Oscillation phase for size changes

class ParticleClass
{
public:
	void Plasma_AI()
	{
		// Orbit around center point
		this->UpdateOrbit();

		// Oscillate size/brightness
		this->UpdatePulse();

		// Try to merge with nearby plasma
		this->TryMergeWithNearby();

		// Apply energy damage
		if (this->Type->Damage > 0)
		{
			this->ApplyEnergyDamage();
		}

		// Update color cycling
		this->UpdateColorCycle();
	}

	void Plasma_Coord_AI()
	{
		// Extension: Calculate orbital position
		// float angle = this->Ext->OrbitAngle;
		// CoordStruct center = this->Ext->OrbitCenter;

		// For now, use spiral movement
		const float time = Frame * 0.1f;
		const int orbitRadius = this->Type->Radius;

		CoordStruct newPos;
		newPos.X = this->Gas_AI_C0.X + static_cast<int>(orbitRadius * FastMath::Cos(time));
		newPos.Y = this->Gas_AI_C0.Y + static_cast<int>(orbitRadius * FastMath::Sin(time));
		newPos.Z = this->Gas_AI_C0.Z + static_cast<int>(10.0f * FastMath::Sin(time * 2.0f));

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void UpdateOrbit()
	{
		// Extension: this->Ext->OrbitAngle += this->Velocity * 0.01f;
		// Rotation handled in Coord_AI
	}

	void UpdatePulse()
	{
		// Extension: Update pulse phase
		// this->Ext->PulsePhase += 0.1f;

		// Oscillate translucency for pulse effect
		const float pulse = FastMath::Sin(Frame * 0.1f);
		this->Translucency = static_cast<int>(30.0f + pulse * 20.0f);
	}

	void TryMergeWithNearby()
	{
		// Extension: Search for nearby plasma particles and merge
		// Would increase size/power and remove the other particle
	}

	void ApplyEnergyDamage()
	{
		if (--this->RemainingDC > 0)
		{
			return;
		}

		this->RemainingDC = this->Type->MaxDC;

		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		for (FootClass* unit = cell->OccupierPtr; unit; unit = unit->t.r.m.o.Next)
		{
			if (unit->t.r.m.o.IsActive && unit->t.r.m.o.Strength > 0)
			{
				const float damage = this->Type->Damage;
				unit->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
					&unit->t.r.m.o, &damage, 0, this->Type->Warhead,
					nullptr, false, false, 0
				);
			}
		}
	}

	void UpdateColorCycle()
	{
		if ((Frame % 2) == 0 && this->Type->ColorSpeed > 0.0)
		{
			this->doubleB8 += this->Type->ColorSpeed;
			if (this->doubleB8 > 1.0)
			{
				this->RefCount = (this->RefCount + 1) % this->Type->ColorList.ActiveCount;
				this->doubleB8 = 0.0;
			}
		}
	}
};

class ParticleSystemClass
{
public:
	char Plasma_AI()
	{
		if (!this->TimeToDie && this->obj.IsActive)
		{
			if ((Frame % this->Class->SpawnFrames) == 0)
			{
				this->SpawnPlasmaOrb();
			}
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnPlasmaOrb()
	{
		if (this->Class->HoldsWhat == -1 || this->particles.ActiveCount >= this->Class->ParticleCap)
		{
			return;
		}

		CoordStruct spawnPos = this->obj.Coord;
		ParticleClass* orb = operator new(sizeof(ParticleClass));
		if (orb)
		{
			ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
			orb = ParticleClass::ParticleClass(orb, type, &spawnPos, &spawnPos, this);
			if (orb)
			{
				DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &orb);
				orb->Gas_AI_C0 = this->obj.Coord; // Store orbit center
			}
		}
	}
};

//========================================================================================================================================

// Ember Particle AI - Rising ash and glowing embers
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Ember = 10
//
// INI Properties Usage:
// - Velocity: Rise speed (thermal updraft)
// - WindEffect: Horizontal drift
// - ColorList: Glow fade (bright orange to dark)
// - Translucency: Fade out as it rises

class ParticleClass
{
public:
	void Ember_AI()
	{
		// Rise with thermal convection
		this->ApplyThermalRise();

		// Random flare-ups
		this->UpdateEmberGlow();

		// Fade out as it cools
		this->UpdateCooling();

		// Remove when fully cooled
		if (this->Translucency >= 100)
		{
			this->hasremaining = 1;
		}
	}

	void Ember_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Rise upward (heat convection)
		newPos.Z += static_cast<int>(this->Velocity);

		// Apply turbulent drift
		const int turbulence = Random2Class::operator()(&Scen->RandomNumber) % 10 - 5;
		newPos.X += turbulence;
		newPos.Y += turbulence;

		// Heavy wind influence
		if (this->Type->WindEffect > 0)
		{
			const int windDir = Rule->WindDirection;
			const Point2D wind = Wind[windDir];
			newPos.X += wind.X * this->Type->WindEffect;
			newPos.Y += wind.Y * this->Type->WindEffect;
		}

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ApplyThermalRise()
	{
		// Gradually slow down as it rises and cools
		this->Velocity = std::max(0.5f, this->Velocity - 0.05f);
	}

	void UpdateEmberGlow()
	{
		// Random flare-ups
		if ((Random2Class::operator()(&Scen->RandomNumber) % 100) < 5)
		{
			// Flare brighter temporarily
			this->RefCount = 0; // Reset to brightest color
		}
		else if ((Frame & 3) == 0)
		{
			// Gradual dimming
			if (this->RefCount < this->Type->ColorList.ActiveCount - 1)
			{
				++this->RefCount;
			}
		}
	}

	void UpdateCooling()
	{
		// Gradually fade out (cooling)
		if ((Frame & 3) == 0)
		{
			this->Translucency = std::min(100, this->Translucency + 2);
		}
	}
};

class ParticleSystemClass
{
public:
	char Ember_AI()
	{
		if (!this->TimeToDie && this->obj.IsActive)
		{
			if ((Frame % this->Class->SpawnFrames) == 0)
			{
				this->SpawnEmber();
			}
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnEmber()
	{
		if (this->Class->HoldsWhat == -1) return;

		const int radius = this->Class->SpawnRadius;
		CoordStruct spawnPos {
			this->obj.Coord.X + Random2Class::operator()(&Scen->RandomNumber) % radius - radius / 2,
			this->obj.Coord.Y + Random2Class::operator()(&Scen->RandomNumber) % radius - radius / 2,
			this->obj.Coord.Z
		};

		ParticleClass* ember = operator new(sizeof(ParticleClass));
		if (ember)
		{
			ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
			ember = ParticleClass::ParticleClass(ember, type, &spawnPos, &spawnPos, this);
			if (ember)
			{
				DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &ember);
			}
		}
	}
};

//========================================================================================================================================

// Bubble Particle AI - Underwater bubbles
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Bubble = 11
//
// INI Properties Usage:
// - Velocity: Rise speed (increases as bubble rises)
// - Radius: Wobble radius
// - XVelocity/YVelocity: Wobble speed

class ParticleClass
{
public:
	void Bubble_AI()
	{
		// Check if still underwater
		if (!this->IsUnderwater())
		{
			this->PopBubble();
			return;
		}

		// Rise with acceleration (buoyancy)
		this->ApplyBuoyancy();

		// Wobble side to side
		this->ApplyWobble();
	}

	void Bubble_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Rise upward with increasing speed
		newPos.Z += static_cast<int>(this->Velocity);

		// Apply wobble
		const float wobble = FastMath::Sin(Frame * 0.1f);
		newPos.X += static_cast<int>(wobble * this->Type->Radius);
		newPos.Y += static_cast<int>(FastMath::Cos(Frame * 0.15f) * this->Type->Radius * 0.5f);

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	bool IsUnderwater() const
	{
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		// Check for bridge (water)
		if ((*cell->Bitfield2 & static_cast<unsigned int>(CellFlags::BridgeHead)) == 0)
		{
			return false;
		}

		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &cellPos);
		const int waterSurface = terrainHeight + 416; // dword_AC4A0C

		return this->o.Coord.Z < waterSurface;
	}

	void PopBubble()
	{
		// Spawn pop effect if configured
		if (this->Type->NextParticle != -1)
		{
			CoordStruct popPos = this->o.Coord;
			ParticleClass* pop = operator new(sizeof(ParticleClass));
			if (pop)
			{
				ParticleTypeClass* popType = ParticleTypes.Vector_Item[this->Type->NextParticle];
				ParticleClass::ParticleClass(pop, popType, &popPos, &popPos, this->ParticleSystem);
			}
		}

		this->hasremaining = 1;
	}

	void ApplyBuoyancy()
	{
		// Accelerate upward (buoyancy increases with rise)
		this->Velocity += 0.1f;
		this->Velocity = std::min(this->Velocity, 5.0f);
	}

	void ApplyWobble()
	{
		// Wobble handled in Coord_AI
	}
};

class ParticleSystemClass
{
public:
	char Bubble_AI()
	{
		if (!this->TimeToDie && this->obj.IsActive)
		{
			if ((Frame % this->Class->SpawnFrames) == 0)
			{
				this->SpawnBubble();
			}
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnBubble()
	{
		if (this->Class->HoldsWhat == -1) return;

		CoordStruct spawnPos {
			this->obj.Coord.X + Random2Class::operator()(&Scen->RandomNumber) % 50 - 25,
			this->obj.Coord.Y + Random2Class::operator()(&Scen->RandomNumber) % 50 - 25,
			this->obj.Coord.Z
		};

		ParticleClass* bubble = operator new(sizeof(ParticleClass));
		if (bubble)
		{
			ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
			bubble = ParticleClass::ParticleClass(bubble, type, &spawnPos, &spawnPos, this);
			if (bubble)
			{
				DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &bubble);
				bubble->Velocity = 1.0f + (Random2Class::operator()(&Scen->RandomNumber) % 10) * 0.1f;
			}
		}
	}
};

//========================================================================================================================================

// Spore Particle AI - Biological spores and infection
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Spore = 12
//
// INI Properties Usage:
// - Velocity: Float speed
// - Radius: Infection/attraction radius
// - Damage/Warhead: Infection damage
// - WindEffect: Very high (spores drift a lot)

class ParticleClass
{
public:
	void Spore_AI()
	{
		// Pulsing movement
		this->ApplyPulsingMotion();

		// Seek nearby living targets
		this->SeekLivingTargets();

		// Apply infection damage
		if (this->Type->Damage > 0)
		{
			this->ApplyInfectionDamage();
		}

		// Grow and shrink during lifetime
		this->UpdateSporeSize();
	}

	void Spore_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Slow floating with pulsing
		const float pulse = FastMath::Sin(Frame * 0.05f);
		newPos.X += static_cast<int>(this->vector3_10C.X * this->Velocity);
		newPos.Y += static_cast<int>(this->vector3_10C.Y * this->Velocity);
		newPos.Z += static_cast<int>(pulse * 5.0f);

		// Heavy wind drift
		if (this->Type->WindEffect > 0)
		{
			const int windDir = Rule->WindDirection;
			const Point2D wind = Wind[windDir];
			newPos.X += wind.X * this->Type->WindEffect;
			newPos.Y += wind.Y * this->Type->WindEffect;
		}

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ApplyPulsingMotion()
	{
		// Random direction changes
		if ((Frame % 30) == 0)
		{
			this->vector3_10C.X += (Random2Class::operator()(&Scen->RandomNumber) % 20 - 10) * 0.01f;
			this->vector3_10C.Y += (Random2Class::operator()(&Scen->RandomNumber) % 20 - 10) * 0.01f;

			// Normalize
			const float length = FastMath::Sqrt(
				this->vector3_10C.X * this->vector3_10C.X +
				this->vector3_10C.Y * this->vector3_10C.Y
			);
			if (length > 0.0f)
			{
				this->vector3_10C.X /= length;
				this->vector3_10C.Y /= length;
			}
		}
	}

	void SeekLivingTargets()
	{
		// Find nearest living unit
		TechnoClass* target = this->FindNearestLivingTarget();
		if (!target)
		{
			return;
		}

		// Adjust direction toward target slightly
		CoordStruct targetPos;
		target->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(&target->r.m.o.a, &targetPos);

		const Vector3 direction {
			static_cast<float>(targetPos.X - this->o.Coord.X),
			static_cast<float>(targetPos.Y - this->o.Coord.Y),
			0.0f
		};

		const float length = FastMath::Sqrt(direction.X * direction.X + direction.Y * direction.Y);
		if (length > 0.0f)
		{
			// Gradually turn toward target
			const float attractionStrength = 0.1f;
			this->vector3_10C.X += (direction.X / length) * attractionStrength;
			this->vector3_10C.Y += (direction.Y / length) * attractionStrength;
		}
	}

	TechnoClass* FindNearestLivingTarget() const
	{
		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		TechnoClass* nearestTarget = nullptr;
		int nearestDistance = this->Type->Radius;

		for (int dx = -2; dx <= 2; ++dx)
		{
			for (int dy = -2; dy <= 2; ++dy)
			{
				Point2DStruct searchCell { cellPos.X + dx * 256, cellPos.Y + dy * 256 };
				CellClass* searchCellPtr = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &searchCell);

				if (searchCellPtr->OccupierPtr)
				{
					TechnoClass* unit = &searchCellPtr->OccupierPtr->t;
					if (unit->r.m.o.IsActive && unit->r.m.o.Strength > 0)
					{
						const int dx = unit->r.m.o.Coord.X - this->o.Coord.X;
						const int dy = unit->r.m.o.Coord.Y - this->o.Coord.Y;
						const int distance = FastMath::Sqrt(dx * dx + dy * dy);

						if (distance < nearestDistance)
						{
							nearestTarget = unit;
							nearestDistance = distance;
						}
					}
				}
			}
		}

		return nearestTarget;
	}

	void ApplyInfectionDamage()
	{
		if (--this->RemainingDC > 0)
		{
			return;
		}

		this->RemainingDC = this->Type->MaxDC;

		const Point2DStruct cellPos { this->o.Coord.X, this->o.Coord.Y };
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cellPos);

		for (FootClass* unit = cell->OccupierPtr; unit; unit = unit->t.r.m.o.Next)
		{
			if (unit->t.r.m.o.IsActive && unit->t.r.m.o.Strength > 0)
			{
				const float damage = this->Type->Damage;
				unit->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
					&unit->t.r.m.o, &damage, 0, this->Type->Warhead,
					nullptr, false, false, 0
				);
			}
		}
	}

	void UpdateSporeSize()
	{
		// Pulsing translucency for "breathing" effect
		const float pulse = FastMath::Sin(Frame * 0.1f);
		this->Translucency = static_cast<int>(40.0f + pulse * 15.0f);
	}
};

class ParticleSystemClass
{
public:
	char Spore_AI()
	{
		if (!this->TimeToDie && this->obj.IsActive)
		{
			if ((Frame % this->Class->SpawnFrames) == 0)
			{
				this->SpawnSpore();
			}
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnSpore()
	{
		if (this->Class->HoldsWhat == -1) return;

		const int radius = this->Class->SpawnRadius;
		CoordStruct spawnPos {
			this->obj.Coord.X + Random2Class::operator()(&Scen->RandomNumber) % radius - radius / 2,
			this->obj.Coord.Y + Random2Class::operator()(&Scen->RandomNumber) % radius - radius / 2,
			this->obj.Coord.Z + Random2Class::operator()(&Scen->RandomNumber) % 100
		};

		ParticleClass* spore = operator new(sizeof(ParticleClass));
		if (spore)
		{
			ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
			spore = ParticleClass::ParticleClass(spore, type, &spawnPos, &spawnPos, this);
			if (spore)
			{
				DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &spore);

				// Random initial direction
				const float angle = (Random2Class::operator()(&Scen->RandomNumber) % 360) * 0.01745f;
				spore->vector3_10C.X = FastMath::Cos(angle);
				spore->vector3_10C.Y = FastMath::Sin(angle);
				spore->vector3_10C.Z = 0.0f;
			}
		}
	}
};

//========================================================================================================================================

// Dust Particle AI - Dust clouds and vehicle trails
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Dust = 13
//
// INI Properties Usage:
// - Velocity: Initial expansion speed
// - Deacc: How fast dust settles
// - Radius: Maximum expansion size
// - WindEffect: Very high (dust is light)
// - Translucency: Fade as dust dissipates

class ParticleClass
{
public:
	void Dust_AI()
	{
		// Expand outward from source
		this->ExpandDustCloud();

		// Settle downward
		this->SettleDust();

		// Dissipate over time
		this->DissipateIntoDust();

		// Remove when fully dissipated
		if (this->Translucency >= 100 || this->Velocity <= 0.0f)
		{
			this->hasremaining = 1;
		}
	}

	void Dust_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Expand horizontally
		newPos.X += static_cast<int>(this->vector3_10C.X * this->Velocity);
		newPos.Y += static_cast<int>(this->vector3_10C.Y * this->Velocity);

		// Settle downward
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &newPos);
		if (newPos.Z > terrainHeight + 10)
		{
			newPos.Z -= 2; // Slow settling
		}

		// Heavy wind influence
		if (this->Type->WindEffect > 0)
		{
			const int windDir = Rule->WindDirection;
			const Point2D wind = Wind[windDir];
			newPos.X += wind.X * this->Type->WindEffect;
			newPos.Y += wind.Y * this->Type->WindEffect;
		}

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void ExpandDustCloud()
	{
		// Gradually slow expansion
		this->Velocity -= this->Type->Deacc;
		if (this->Velocity < 0.0f)
		{
			this->Velocity = 0.0f;
		}
	}

	void SettleDust()
	{
		// Handled in Coord_AI
	}

	void DissipateIntoDust()
	{
		// Fade out as dust dissipates
		if ((Frame & 3) == 0)
		{
			this->Translucency = std::min(100, this->Translucency + 3);
		}
	}
};

class ParticleSystemClass
{
public:
	char Dust_AI()
	{
		// Spawn dust burst
		if (!this->TimeToDie && this->particles.ActiveCount == 0)
		{
			this->SpawnDustBurst();
			this->TimeToDie = 1;
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnDustBurst()
	{
		if (this->Class->HoldsWhat == -1) return;

		const int dustCount = std::min(15, this->Class->ParticleCap);

		for (int i = 0; i < dustCount; ++i)
		{
			const float angle = (i * 360.0f) / dustCount;

			CoordStruct spawnPos = this->obj.Coord;
			ParticleClass* dust = operator new(sizeof(ParticleClass));
			if (dust)
			{
				ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
				dust = ParticleClass::ParticleClass(dust, type, &spawnPos, &spawnPos, this);
				if (dust)
				{
					DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &dust);

					dust->vector3_10C.X = FastMath::Cos(angle * 0.01745f);
					dust->vector3_10C.Y = FastMath::Sin(angle * 0.01745f);
					dust->vector3_10C.Z = 0.0f;
					dust->Velocity = type->Velocity;
				}
			}
		}
	}
};

//========================================================================================================================================

// Tracer Particle AI - Fast bullet trails and tracers
// ADHD-Friendly Code Style:
// - Explicit this-> for all member access (non-static)
// - No this-> means static/global function
//
// Enum Value: ParticleTypeBehavesLike::Tracer = 14
//
// INI Properties Usage:
// - Velocity: Bullet speed (very high, 50+)
// - MaxEC: Lifetime in frames (very short, 5-15)
// - ColorList: Trail colors (bright to dim)
// - Translucency: Glow effect

class ParticleClass
{
public:
	void Tracer_AI()
	{
		// Very short lifetime - quick fade
		if (--this->RemainingEC <= 0)
		{
			this->hasremaining = 1;
			return;
		}

		// Fade out quickly
		this->UpdateTracerFade();
	}

	void Tracer_Coord_AI()
	{
		CoordStruct newPos = this->o.Coord;

		// Move very fast in straight line (no physics)
		newPos.X += static_cast<int>(this->vector3_10C.X * this->Velocity);
		newPos.Y += static_cast<int>(this->vector3_10C.Y * this->Velocity);
		newPos.Z += static_cast<int>(this->vector3_10C.Z * this->Velocity);

		// Check if hit ground or left map
		const int terrainHeight = MapClass::Get_Z_Pos(&Map.sc.t.sb.p.r.d.m, &newPos);
		if (newPos.Z <= terrainHeight)
		{
			this->hasremaining = 1;
			return;
		}

		this->o.a.vftable->t.r.m.o.Set_Coord(this, &newPos);
	}

private:
	void UpdateTracerFade()
	{
		// Rapid fade based on remaining lifetime
		const float lifetimePercent = static_cast<float>(this->RemainingEC) /
			static_cast<float>(this->Type->MaxEC);
		this->Translucency = static_cast<int>((1.0f - lifetimePercent) * 80.0f);
	}
};

class ParticleSystemClass
{
public:
	char Tracer_AI()
	{
		// Spawn continuous stream of tracers
		if (!this->TimeToDie && this->obj.IsActive)
		{
			if ((Frame % this->Class->SpawnFrames) == 0)
			{
				this->SpawnTracerBurst();
			}
		}

		for (int i = 0; i < this->particles.ActiveCount; ++i)
		{
			ParticleClass::AI(this->particles.Vector_Item[i]);
			ParticleClass::Coord_AI(this->particles.Vector_Item[i]);
		}

		for (int i = this->particles.ActiveCount - 1; i >= 0; --i)
		{
			if (this->particles.Vector_Item[i]->hasremaining)
			{
				this->particles.Vector_Item[i]->o.a.vftable->t.r.m.o.Remove_This_deletethis(
					&this->particles.Vector_Item[i]->o.a);
			}
		}

		return static_cast<char>(this->particles.ActiveCount);
	}

private:
	void SpawnTracerBurst()
	{
		if (this->Class->HoldsWhat == -1) return;

		// Calculate direction to target
		const Vector3 direction {
			static_cast<float>(this->TargetCoords.X - this->obj.Coord.X),
			static_cast<float>(this->TargetCoords.Y - this->obj.Coord.Y),
			static_cast<float>(this->TargetCoords.Z - this->obj.Coord.Z)
		};

		const float length = FastMath::Sqrt(
			direction.X * direction.X +
			direction.Y * direction.Y +
			direction.Z * direction.Z
		);

		if (length == 0.0f) return;

		// Normalized direction
		Vector3 normalized {
			direction.X / length,
			direction.Y / length,
			direction.Z / length
		};

		// Spawn multiple tracers for stream effect
		const int tracerCount = 3;
		for (int i = 0; i < tracerCount; ++i)
		{
			CoordStruct spawnPos = this->obj.Coord;

			// Slight spread for visual effect
			spawnPos.X += (Random2Class::operator()(&Scen->RandomNumber) % 20) - 10;
			spawnPos.Y += (Random2Class::operator()(&Scen->RandomNumber) % 20) - 10;

			ParticleClass* tracer = operator new(sizeof(ParticleClass));
			if (tracer)
			{
				ParticleTypeClass* type = ParticleTypes.Vector_Item[this->Class->HoldsWhat];
				tracer = ParticleClass::ParticleClass(tracer, type, &spawnPos, &this->TargetCoords, this);
				if (tracer)
				{
					DynamicVectorClass_ParticleSystemClass_630250(&this->particles, &tracer);
					tracer->vector3_10C = normalized;
					tracer->Velocity = type->Velocity;
					tracer->RemainingEC = type->MaxEC;
				}
			}
		}
	}
};

// =============================================================================
// INI Example Configurations for ALL New Particles
// =============================================================================
/*
; ===== PRECIPITATION =====
[RainParticle]
BehavesLike=5  ; Precipitation
Velocity=12.0
WindEffect=2
NextParticle=RainSplash
Translucency=40

[SnowParticle]
BehavesLike=5
Velocity=2.0
WindEffect=8
NextParticle=SnowPuff
Translucency=20

; ===== ELECTRIC =====
[LightningParticle]
BehavesLike=6  ; Electric
Velocity=15.0
Damage=50
Warhead=Electric
Radius=200
NextParticle=LightningBranch

; ===== LIQUID =====
[WaterDroplet]
BehavesLike=7  ; Liquid
Velocity=8.0
Deacc=0.5
NextParticle=WaterSplash
Radius=40

; ===== DEBRIS =====
[MetalDebris]
BehavesLike=8  ; Debris
Velocity=10.0
Deacc=0.3
Damage=15
Warhead=AP
Radius=50

; ===== PLASMA =====
[PlasmaOrb]
BehavesLike=9  ; Plasma
Velocity=5.0
Radius=100
Damage=30
Warhead=Special

; ===== EMBER =====
[EmberParticle]
BehavesLike=10  ; Ember
Velocity=3.0
WindEffect=5
Translucency=30

; ===== BUBBLE =====
[BubbleParticle]
BehavesLike=11  ; Bubble
Velocity=1.0
Radius=10
NextParticle=BubblePop

; ===== SPORE =====
[SporeParticle]
BehavesLike=12  ; Spore
Velocity=2.0
WindEffect=10
Damage=5
Warhead=Poison
Radius=150

; ===== DUST =====
[DustParticle]
BehavesLike=13  ; Dust
Velocity=5.0
Deacc=0.2
WindEffect=8
Translucency=40

; ===== TRACER =====
[TracerParticle]
BehavesLike=14  ; Tracer
Velocity=80.0
MaxEC=10
Translucency=20
*/

//========================================================================================================================================

Image Requirements :

Particle Images : Yes, you need SHP files for the particles themselves

Located in art.ini under[ParticleTypeName]
Can reuse existing particle graphics
Can be simple shapes(dots, streaks, sprites)


Animation States : Defined by frames in the SHP

StartFrame, NumLoopFrames, EndStateAI control which frames play
More frames = smoother animation


Can Reuse Existing :

Rain: Use existing smoke / gas particles, just make them thin / vertical
Snow : Reuse spark / debris particles
Lightning : Reuse railgun trail
Most effects can adapt existing art


Minimal Approach :

Single - frame particles work fine
Color cycling can provide visual variety without multiple frames
Translucency changes add animation effect



TL; DR: Yes you need images, but you can mostly reuse / adapt existing particle art from the game.Only if you want fancy custom visuals do you need to create new SHP files.