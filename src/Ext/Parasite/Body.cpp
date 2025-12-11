#include "Body.h"


#include <Utilities/Macro.h>
#include <Misc/Ares/Hooks/Header.h>

#include <Ext/Rules/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <InfantryClass.h>

bool FakeParasiteClass::IsSpecialOverlay(int overlayIndex) const
{
	return (overlayIndex >= ParasiteConstants::SpecialOverlayRangeStart1 && 
	        overlayIndex <= ParasiteConstants::SpecialOverlayRangeEnd1) ||
	       (overlayIndex >= ParasiteConstants::SpecialOverlayRangeStart2 && 
	        overlayIndex <= ParasiteConstants::SpecialOverlayRangeEnd2);
}

void FakeParasiteClass::ResetOwnerMission(FootClass* owner)
{
	if (!owner)
		return;
		
	// Restore player control if applicable
	if (owner->ShouldBeReselectOnUnlimbo && owner->Owner->ControlledByCurrentPlayer())
	{
		auto nCoord = owner->GetCoords();
		VocClass::PlayIndexAtPos(
			TechnoTypeExtContainer::Instance.Find(owner->GetTechnoType())->ParasiteExit_Sound.Get(),
			&nCoord, 
			false
		);
		owner->ShouldBeReselectOnUnlimbo = false;
		owner->Select();
	}

	// Rejoin team if applicable
	if (TeamClass* team = owner->OldTeam)
	{
		team->AddMember(owner, false);
	}

	// Reset mission if not busy
	if (!owner->HaveMegaMission())
	{
		owner->SetArchiveTarget(nullptr);
		owner->SetTarget(nullptr);
		owner->SetDestination(nullptr, true);
	}

	// Enter idle mode
	owner->EnterIdleMode(false, true);
}

void TechnoExtData::DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
#ifdef PARASITE_PIPS
	{
		//bool IsHost = false;
		//bool IsSelected = false;					//Red         //Green           //White
		//ColorScheme Color = IsSelected ? (IsHost ? Drawings::ColorRed : Drawings::ColorGreen) : Drawings::ColorWhite;
		int xOffset = 0;
		int yOffset = 0;

		nBracket = TechnoExtData::GetDisguiseType, (pThis).first->PixelSelectionBracketDelta;

		switch ((((DWORD*)pThis)[0]))
		{
		case UnitClass::vtable:
		case AircraftClass::vtable:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Units_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::vtable:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Infantry_Offset.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		}

		int pipFrame = 4;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
#endif
}

// =============================
// load / save
//template <typename T>
//void ParasiteExt::ExtData::Serialize(T& Stm) {
//	//Debug::LogInfo("Processing Element From ParasiteExt ! ");
//	Stm
//		.Process(this->Initialized)
//		.Process(this->LastVictimLocation)
//		;
//}

// =============================
// container
//ParasiteExt::ExtContainer ParasiteExt::ExtMap;

// =============================
// container hooks

//ASMJIT_PATCH_AGAIN(0x62924C , ParasiteClass_CTOR,0x5 )
//ASMJIT_PATCH(0x62932E, ParasiteClass_CTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x62AFFE , ParasiteClass_DTOR, 0x6)
//ASMJIT_PATCH(0x62946E, ParasiteClass_DTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
//ASMJIT_PATCH(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
//{
//
//	GET_STACK(ParasiteClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x62969D, ParasiteClass_Load_Suffix, 0x5)
//{
//	ParasiteExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//ASMJIT_PATCH(0x6296BC, ParasiteClass_Save_Suffix, 0x8)
//{
//	GET(ParasiteClass*, pThis, ECX);
//	GET(IStream*, pStream, EAX);
//	GET(BOOL, bClearDirty, EAX);
//
//	auto const nRes = AbstractClass::_Save(pThis, pStream, bClearDirty);
//
//	if (SUCCEEDED(nRes))
//		ParasiteExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x6296C4;
//}
//detach

#include <Notifications.h>
#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

bool FakeParasiteClass::__Update_GrappleAnim_Frame() {

	DirStruct victimFacing = this->Victim->PrimaryFacing.Current();

	int baseFrame = 0;
	int delayThreshold = 0;

	// Determine animation parameters based on state
	switch (this->GrappleState)
	{
	case ParasiteState::Start:
	case ParasiteState::Grab:
		baseFrame = 0;
		delayThreshold = 3;
		break;

	case ParasiteState::PushLeft:
		baseFrame = 80;
		delayThreshold = 4;
		break;

	case ParasiteState::Damage:
	case ParasiteState::PushRight:
		baseFrame = 160;
		delayThreshold = 4;
		break;

	default:
		break;
	}

	// Update animation delay counter
	++this->GrappleAnimDelay;

	if (this->GrappleAnimDelay >= delayThreshold)
	{
		this->GrappleAnimDelay = 0;
		++this->GrappleAnimFrame;

		// Check if animation sequence is complete (10 frames)
		if (this->GrappleAnimFrame >= ParasiteConstants::AnimationFrameCount) {
			return true; // Animation phase complete
		}

		// Update the actual animation object if it exists
		if (AnimClass* anim = this->GrappleAnim) {

			anim->Animation.Start(ParasiteConstants::AnimationTimingBase, ParasiteConstants::AnimationTimingBase);
			// Calculate animation stage based on facing direction
			const int facingIndex = (((victimFacing.Raw >> 12) + 1) >> 1) & 7;
			anim->Animation.Stage = baseFrame + this->GrappleAnimFrame + (ParasiteConstants::AnimationFrameCount * facingIndex);
		}
	}

	return false; // Animation phase still in progress
}

void FakeParasiteClass::__ClearAnim()
{
	// Reset animation state
	this->GrappleAnimFrame = 0;
	this->GrappleAnimDelay = 0;
	this->GrappleState = ParasiteState::Start;

	// Clean up the grapple animation if it exists
	if (this->GrappleAnim) {
		this->GrappleAnim->UnInit();
		this->GrappleAnim = nullptr;
	}

	// Remove from invalid animation array if registered
	if (this->GrappleAnimGotInvalid) {
		ParasiteClass* searchItem = this;
		PointerExpiredNotification::NotifyInvalidAnim->Remove(searchItem);
		this->GrappleAnimGotInvalid = false;
	}
}

void FakeParasiteClass::__Grapple_AI()
{
	// Validate owner and victim
	if (!this->Owner || !this->Victim) {
		return;
	}

	// Get weapon information
	TechnoExtData* pOwnerExt = TechnoExtContainer::Instance.Find(this->Owner);
	WeaponStruct* weapon = this->Owner->GetWeapon(pOwnerExt->idxSlot_Parasite);
	WeaponTypeClass* weaponType = weapon ? weapon->WeaponType : nullptr;

	if (!weaponType) {
		return;
	}

	FootClass* victim = this->Victim;
	CoordStruct victimCoord = victim->Location;

	if (!weaponType->Warhead)
	{
		// Not in water - uninfect and paralyze owner
		this->__Uninfect();

		if (auto pOwner = this->Owner) {
			pOwner->ParalysisTimer.Start(0);
		}

		return;
	}

	// Update victim's paralysis timer
	this->Owner->ParalysisTimer.Start(weaponType->Warhead->Paralyzes);

	// State machine for grapple animation
	switch (this->GrappleState)
	{
	case ParasiteState::Start:
	{ // Initialize grapple
		this->Victim->AngleRotatedSideways = 0.0f;

		auto const pWeaponExt = WarheadTypeExtContainer::Instance.Find(weaponType->Warhead);
		auto const pAnimType = pWeaponExt->Parasite_GrappleAnim.Get(RulesExtData::Instance()->DefaultSquidAnim.Get());

		if (pAnimType) {
			if (AnimClass* newAnim = GameCreate<AnimClass>(pAnimType, victimCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0)) {
					this->GrappleAnim = newAnim;
					auto const Invoker = (this->Owner) ? this->Owner->GetOwningHouse() : nullptr;
					AnimExtData::SetAnimOwnerHouseKind(newAnim, Invoker, (this->Victim) ? this->Victim->GetOwningHouse() : nullptr, this->Owner, false, false);

				// Add to invalid animation array if space available
				PointerExpiredNotification::NotifyInvalidAnim->Add(this);
			}
		}

		this->GrappleState = ParasiteState::Grab;
		this->GrappleAnimDelay = 0;
		this->GrappleAnimFrame = 0;
		this->__Update_GrappleAnim_Frame(); // Start animation
		break;
	}

	case ParasiteState::Grab:
	{ // Initial grapple phase
		if (this->__Update_GrappleAnim_Frame())
		{ // Check if animation phase complete
			int randomChoice = ScenarioClass::Instance->Random.RandomBool();
			this->GrappleAnimDelay = 0;
			this->GrappleState = ParasiteState(3 - (randomChoice != 0)); // Randomly choose state 2 or 3
			this->GrappleAnimFrame = 0;
			this->__Update_GrappleAnim_Frame(); // Start next animation phase
			this->Victim->CreateWakes(victimCoord);
		}
		break;
	}

	case ParasiteState::PushLeft:
	{ // Swing one direction
		if (this->__Update_GrappleAnim_Frame())
		{ // Check if animation phase complete
			this->GrappleState = ParasiteState::Damage;
			this->GrappleAnimDelay = 0;
			this->GrappleAnimFrame = 0;
			this->__Update_GrappleAnim_Frame(); // Start next animation phase
		}

		// Calculate sideways angle (negative direction)
		double angleRadians = this->GrappleAnimFrame * -1.0f * Math::GAME_PI;
		this->Victim->AngleRotatedSideways = Math::sin(angleRadians) * Math::PI_BY_FOUR_F;
		break;
	}

	case ParasiteState::PushRight:
	{ // Swing opposite direction
		if (this->__Update_GrappleAnim_Frame())
		{ // Check if animation phase complete
			this->GrappleState = ParasiteState::Damage;
			this->GrappleAnimDelay = 0;
			this->GrappleAnimFrame = 0;
			this->__Update_GrappleAnim_Frame(); // Start next animation phase
		}

		// Calculate sideways angle (positive direction)
		double angleRadians = this->GrappleAnimFrame * 1.0f * Math::GAME_PI;
		this->Victim->AngleRotatedSideways = Math::sin(angleRadians) * Math::PI_BY_FOUR_F;
		break;
	}

	case ParasiteState::Damage:
	{ // Submerge or damage victim
		this->Victim->AngleRotatedSideways = 0.0f;

		// Create splash effects

		auto const AnimType = WarheadTypeExtContainer::Instance.Find(weapon->WeaponType->Warhead)->SquidSplash.GetElements(RulesClass::Instance->SplashList);

		if(AnimType) {

			DirStruct facingDir = this->Victim->PrimaryFacing.Current();

			double facingAngle = -((facingDir.Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC);
			float cosAngle = Math::cos(facingAngle);
			float sinAngle = Math::sin(facingAngle);

			for (int i = 0; i < 3; ++i)
			{
				float offsetY = 64.0f;
				float offsetX = i * 128.0f - 128.0f;

				// Randomize Y offset
				if (ScenarioClass::Instance->Random.RandomBool()) {
					offsetY = -64.0f;
				}

				// Calculate splash position
				CoordStruct splashCoord {
					.X = int((offsetX * cosAngle - offsetY * sinAngle) + victimCoord.X),
					.Y = int(victimCoord.Y + (offsetX * sinAngle + offsetY * cosAngle)),
					.Z = victimCoord.Z 
				};

				if (!MapClass::Instance->GetCellAt(splashCoord)->Tile_Is_Water())
					continue;

				// Create splash animation
				if (auto pSplash = AnimType[ScenarioClass::Instance->Random.RandomRanged(0 , AnimType.size() - 1)]) {
					auto const Invoker = (this->Owner) ? this->Owner->GetOwningHouse() : nullptr;
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pSplash, splashCoord, 2, 1, AnimFlag::AnimFlag_600, -10, 0), Invoker,
						(this->Victim) ? this->Victim->GetOwningHouse() : nullptr, this->Owner, false, false);
				}
			}
		}

		// Check if victim should be submerged
		const bool culling = WarheadTypeExtContainer::Instance.Find(weaponType->Warhead)
			->applyCulling(this->Owner, this->Victim);

		if (culling) {
			// Award experience if trainable
			if (this->Owner->GetTechnoType()->Trainable && !this->Owner->Owner->IsAlliedWith(this->Victim)) {
				TechnoTypeClass* victimType = this->Victim->GetTechnoType();
				TechnoTypeClass* ownerType = this->Owner->GetTechnoType();
				this->Owner->Veterancy.Add(ownerType->GetActualCost(this->Owner->Owner), victimType->GetCost());
			}

			// Save references before uninfect
			FootClass* victimToSink = this->Victim;
			FootClass* ownerSaved = this->Owner;
			auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(victimToSink->GetTechnoType());

			// Submerge victim
			this->__Uninfect();

			if (pVictimTypeExt->Sinkable_SquidGrab){
				victimToSink->IsSinking = true;
				victimToSink->Destroyed(ownerSaved);
				victimToSink->Stun();
			}
			else {
				int damage = victimToSink->GetTechnoType()->Strength;
				victimToSink->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, ownerSaved, true, false, ownerSaved ? ownerSaved->Owner : nullptr);
			}

			// Clean up grapple animation
			this->__ClearAnim(); // Potentially inlined version of ClearAnim()

			// Paralyze owner briefly
			this->Owner->ParalysisTimer.Start(0);
		} else {
			// Damage victim instead of submerging
			int damage = weaponType->Damage; // Weapon damage
			this->Victim->ReceiveDamage(&damage, 0, weaponType->Warhead, this->Owner, 0, 1, 0);

			if (this->Victim)
			{
				// Continue grappling with random direction
				int randomChoice = ScenarioClass::Instance->Random.RandomBool();
				this->GrappleAnimDelay = 0;
				this->GrappleState = ParasiteState(3 - (randomChoice != 0));
				this->GrappleAnimFrame = 0;
				this->__Update_GrappleAnim_Frame(); // Start next animation phase
				this->Victim->CreateWakes(victimCoord);
			}
			else
			{
				// Victim died - clean up
				this->__ClearAnim(); // Potentially inlined version of ClearAnim()
			}
		}
		break;
	}

	default:
		this->__Uninfect();
		break;
	}

	// Update grapple animation attachment
	AnimClass* grappleAnim = this->GrappleAnim;
	if (grappleAnim && this->Owner && this->Victim) {
		if (this->Victim != grappleAnim->OwnerObject) {
			grappleAnim->SetOwnerObject(this->Victim);
			grappleAnim->LightConvert = this->Owner->GetRemapColour();
			grappleAnim->TintColor = this->Victim->GetCell()->Color1.Red;
		}
	}
}

void NOINLINE TakeDamage(FootClass* pVictiom , FootClass* pOwner , WeaponTypeClass* pWeapon) {

	auto const pWarheadTypeExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (pWarheadTypeExt->Parasite_Damaging_Chance.isset()
		&& ScenarioClass::Instance->Random.RandomDouble() >=
		Math::abs(pWarheadTypeExt->Parasite_Damaging_Chance.Get())
	)
	{
		return;
	}

	if (auto const pInvestationWP = pWarheadTypeExt->Parasite_InvestationWP.Get(nullptr)) {
		WeaponTypeExtData::DetonateAt1(pInvestationWP, pVictiom, pOwner, true, nullptr);
		return;
	}

	int weaponDamage = pVictiom->Health;
	pVictiom->ReceiveDamage(&weaponDamage, 0, pWeapon->Warhead, pOwner, 1, 1, pOwner->Owner);
}

void FakeParasiteClass::__AI()
{
	// Validate owner and victim
	if (!this->Owner || !this->Victim) {
		return;
	}

	// Check if this is a naval organic unit (uses grapple AI)
	TechnoTypeClass* ownerType = this->Owner->GetTechnoType();
	auto const pOwnerTypeExt = TechnoTypeExtContainer::Instance.Find(ownerType);

	if (pOwnerTypeExt->GrapplingAttack.Get(ownerType->Naval && ownerType->Organic)) {
		this->__Grapple_AI();
		return;
	}

	TechnoExtData* pOwnerExt = TechnoExtContainer::Instance.Find(this->Owner);
	// Get weapon and victim information
	WeaponStruct* weapon = this->Owner->GetWeapon(pOwnerExt->idxSlot_Parasite);
	WeaponTypeClass* weaponType = weapon ? weapon->WeaponType : nullptr;

	FootClass* victim = this->Victim;
	CoordStruct victimCoord = victim->Location;

	// Check damage delivery timer
	if (this->DamageDeliveryTimer.GetTimeLeft() > 0) {
		return; // Not time to deliver damage yet
	}

	// Validate weapon and warhead
	if (!weaponType || !weaponType->Warhead)
	{
		return;
	}

	const bool isInfantry = this->Victim->WhatAmI() == AbstractType::Infantry;

	// Reset timer with weapon ROF
	this->DamageDeliveryTimer.Start(weaponType->ROF);

	// Update victim's paralysis timer
	victim->ParalysisTimer.Start(weaponType->Warhead->Paralyzes);
	const bool InfantryInstantKill = isInfantry && !WarheadTypeExtContainer::Instance.Find(weaponType->Warhead)->Parasite_TreatInfantryAsVehicle
		.Get(static_cast<InfantryClass*>(this->Victim)->Type->Cyborg);

	// Handle infantry differently (direct damage to health)
	if (WarheadTypeExtContainer::Instance.Find(weaponType->Warhead)->Parasite_DisableRocking.Get() || InfantryInstantKill) {
		TakeDamage(this->Victim, this->Owner, weaponType);
		return;
	}

	// Non-infantry: create spark effects and apply damage

	// Create particle system
	if (auto pParticle = WarheadTypeExtContainer::Instance.Find(weaponType->Warhead)->Parasite_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem)) {
		CoordStruct nLocHere = victimCoord;
		if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
			nLocHere.Z += ParasiteConstants::SmokeZOffset;

		GameCreate<ParticleSystemClass>(pParticle, nLocHere, this->Victim, this->Owner, CoordStruct::Empty, this->Owner->Owner);
	}

	// Get victim facing direction
	DirStruct facingDir = this->Victim->PrimaryFacing.Current();

	// Create weapon animation if available
	if (weaponType->Anim.Count > 0) {
		int animIndex = (((facingDir.Raw >> 12) + 1) >> 1) & 7;
		if (AnimTypeClass* animType = weaponType->Anim.Items[animIndex]) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(animType, victimCoord, 0, 1, AnimFlag::AnimFlag_600, 0, 0),
					this->Owner ? this->Owner->GetOwningHouse() : nullptr,
					this->Victim ? this->Victim->GetOwningHouse() : nullptr,
					this->Owner,
					false, false
			);
		}
	}

	// Calculate spread effect position
	int randomOffset = ScenarioClass::Instance->Random.RandomBool() ? -64 : 64;

	double facingRadians = (facingDir.Raw - Math::BINARY_ANGLE_MASK) * Math::DIRECTION_FIXED_MAGIC;
	float cosAngle = Math::cos(facingRadians);
	float sinAngle = Math::sin(facingRadians);

	CoordStruct spreadCoord {
		.X = int(cosAngle * randomOffset + victimCoord.X),
		.Y = int(victimCoord.Y - sinAngle * randomOffset),
		.Z = victimCoord.Z
	};

	this->Victim->RockByValue(&spreadCoord, 1.5f, 0);

	// Apply weapon damage
	TakeDamage(this->Victim, this->Owner, weaponType);
}

void FakeParasiteClass::__Detach(AbstractClass* detachingObject, bool permanent) {
	// Handle owner detachment
	if (detachingObject == this->Owner) {
		this->Owner = nullptr;
		return;
	}

	// Handle victim detachment
	if (permanent && detachingObject == this->Victim) {
		if (!Game::InScenario2) {
			this->Victim = nullptr;
			return;
		}

		// Validate owner before proceeding
		if (!this->Owner) {
			this->Victim = nullptr;
			return;
		}

		// Check suppression timer
		// If suppression timer active and owner not iron curtained, destroy owner
		if (this->SuppressionTimer.GetTimeLeft() > 0 &&
			!this->Owner->IsIronCurtained()) {
			this->Victim->ParasiteEatingMe = nullptr;
			this->Victim = nullptr;
			this->Owner->UnInit();
			return;
		}

		// Detach from victim and try to place owner back in world
		CoordStruct detachCoord = this->__Detach_From_Victim();
		auto const pWhat = this->Owner->WhatAmI();

		bool allowed = false;
		if (pWhat == UnitClass::AbsID) {
			allowed = !this->Owner->GetTechnoType()->Naval;
		} else if (pWhat == AbstractType::Infantry) {
			allowed = true;
		}

		if (allowed) {
			if (this->Owner->GetHeight() > ParasiteConstants::MaxHeightForFalling) {
				detachCoord = this->Owner->Location;
				this->Owner->IsFallingDown = this->Owner->IsABomb = true;
			}
			else if (this->Owner->GetHeight() < 0) //inside ground
				detachCoord = CoordStruct::Empty;
		}

		if(!detachCoord.IsValid() || CellClass::Coord2Cell(detachCoord) == CellStruct::Empty) {
			Debug::LogInfo("Parasite[{} : {}] With Invalid Location ! , Removing ! ", (void*)this, Owner->get_ID());
			this->Owner->Health = 0;
			this->Owner->UnInit();
			this->Victim = nullptr;
			return;
		}

		// Get victim facing direction
		DirStruct facingDir= this->Victim->PrimaryFacing.Current();
		DirType ownerDirection = DirType(((facingDir.Raw >> 7) + 1) >> 1);

		bool canPlace = false;

		++Unsorted::ScenarioInit; // Prevent certain global updates during placement

		// Check if placement is valid
		if (detachCoord.IsValid()) {
			canPlace = this->Owner->Unlimbo(detachCoord, ownerDirection);
		}

		--Unsorted::ScenarioInit;

		if (!canPlace) {
			// Failed to place - destroy owner
			this->Owner->Health = 0;
			this->Owner->UnInit();
			this->Victim = nullptr;
			return;
		}

		// Successfully placed owner back in world
		FootClass* owner = this->Owner;
		this->ResetOwnerMission(owner);
		owner->UpdateSight(0, 0, 0, 0, 0);

		// Update map visibility
		CoordStruct ownerCoord = owner->Location;
		MapClass::Instance->RevealArea3(&ownerCoord, owner->LastSightRange - 3, owner->LastSightRange + 3, 0);

		this->Victim = nullptr;
		return;
	}

	// Handle grapple animation detachment
	if (detachingObject == this->GrappleAnim)
	{
		this->GrappleAnim = nullptr;
		this->GrappleAnimGotInvalid = true;
	}
}

void FakeParasiteClass::__Uninfect()
{
	// Validate owner and victim
	if (!this->Owner || !this->Victim) {
		return;
	}

	FootClass* owner = this->Owner;
	TechnoTypeClass* ownerType = owner->GetTechnoType();
	bool Naval = ownerType->Naval;
	auto pOwnerExt = TechnoExtContainer::Instance.Find(owner);

	// Calculate exit direction based on victim facing
	DirStruct victimFacing = this->Victim->PrimaryFacing.Current();
	int facingIndex = (((victimFacing.Raw >> 12) + 1) >> 1) & 7;
	DirStruct exitDirection;

	// Choose opposite direction if facing forward
	if (facingIndex > 2) {
		exitDirection.Raw -= Math::BINARY_ANGLE_MASK;
	} else {
		exitDirection.Raw += Math::BINARY_ANGLE_MASK;
	}

	CellStruct targetCell;

	// Naval units use different logic
	if (Naval) {
		// Get adjacent cell in exit direction
		CellStruct victimCell = this->Victim->InlineMapCoords();
		int dirIndex = (((exitDirection.Raw >> 12) + 1) >> 1) & 7;
		auto neig = CellSpread::GetNeighbourOffset(dirIndex);
		targetCell.X = neig.X + victimCell.X;
		targetCell.Y = neig.Y + victimCell.Y;
	} else {
		// Check suppression timer for non-naval units
		// If suppression active, destroy owner
		if (this->SuppressionTimer.GetTimeLeft() > 0) {
			this->Owner->Health = 0;

			// Clear victim's paralysis
			this->Victim->ParalysisTimer.Start(0);
			this->Victim->ParasiteEatingMe = nullptr;
			this->Owner->UnInit();
			this->Victim = nullptr;
			return;
		}

		// Use victim's current cell
		targetCell = this->Victim->InlineMapCoords();
	}

	// Validate target cell
	CellClass* targetCellClass = MapClass::Instance->GetCellAt(targetCell);
	MovementZone mzone = ownerType->MovementZone;
	ZoneType zone = (ZoneType)MapClass::Instance->GetMapZone(targetCell, mzone, 0);

	// Check if cell is clear for owner
	if (!targetCellClass->IsClearToMove(ownerType->SpeedType, false, false, zone, mzone, -1, true)) {
		this->Victim->NearbyLocation(&targetCell , nullptr);
	}

	// Get coordinate for placement
	CoordStruct placementCoord = CoordStruct::Empty;

	if (targetCell.IsValid()) {
		placementCoord = MapClass::Instance->GetCellAt(targetCell)->GetCoords();
	}

	// Validate placement coordinate and victim cell
	const bool canPlace = targetCell.IsValid() && placementCoord.IsValid() && this->__Victims_Cell_Valid();

	if (!canPlace) {
		// Cannot place - destroy owner
		this->Owner->Health = 0;
		this->Owner->UnInit();
	}
	else {
		// Try to place owner
		DirType placementDir = DirType(((exitDirection.Raw >> 7) + 1) >> 1);

		if (!this->Owner->Unlimbo(placementCoord, placementDir))
		{
			// Placement failed - destroy owner
			this->Owner->Health = 0;
			this->Owner->UnInit();
		} else {
			// Successfully placed
			FootClass* placedOwner = this->Owner;
			this->ResetOwnerMission(placedOwner);
			placedOwner->vt_entry_48C(nullptr, 0u, false, nullptr);

			// Apply paralysis to owner
			WeaponStruct* weapon = placedOwner->GetWeapon(pOwnerExt->idxSlot_Parasite);
			int paralysisTime = weapon->WeaponType->ROF * 3;

			placedOwner->ParalysisTimer.Start(paralysisTime);

			// Clean up grapple animation
			this->__ClearAnim(); // Potentially inlined version of ClearAnim()
		}
	}

	// Reset victim state
	this->Victim->AngleRotatedSideways = 0.0f;
	this->Victim->ParalysisTimer.Start(0);
	this->Victim->ParasiteEatingMe = nullptr;
	this->Victim = nullptr;
}

void FakeParasiteClass::__Infect(FootClass* target)
{
	// Validate owner and target
	if (!this->Owner || !target) {
		return;
	}

	// Clean up any existing grapple animation
	this->__ClearAnim(); // Potentially inlined version of ClearAnim()

	// Initialize damage delivery timer
	this->DamageDeliveryTimer.Start(0);

	// Check if target can be infected
	if (this->CanInfect(target)) //TODO 
	{
		// Force locomotion to target coordinates
		this->Owner->Locomotor->Force_Track(-1, target->Location);

		// Mark target as being parasited
		target->ParasiteEatingMe = this->Owner;
		this->Victim = target;
	} else {
		// Cannot infect - place owner back on map
		CellClass* ownerLastCell = MapClass::Instance->GetCellAt(this->Owner->LastMapCoords);
		CoordStruct placementCoord = ownerLastCell->GetCoords();
		bool attempt = this->Owner->Unlimbo(placementCoord, DirType::North);

		if (!attempt) {

			if (!this->Owner)
				return;

			auto pType = this->Owner->GetTechnoType();

			auto cell = MapClass::Instance->NearByLocation(this->Owner->LastMapCoords,
				pType->SpeedType, ZoneType::None, pType->MovementZone, false, 1, 1, false,
				false, false, true, CellStruct::Empty, false, false);

			placementCoord = MapClass::Instance->GetCellAt(cell)->GetCoords();
			attempt = this->Owner->Unlimbo(placementCoord, DirType::North);
		}

		if (attempt) {
			// Successfully placed back on map
			FootClass* owner = this->Owner;

			// Reveal from limbo
			owner->UpdateSight(false, 0 ,false, nullptr, false);

			// Update map visibility
			CoordStruct ownerCoord = owner->Location;
			MapClass::Instance->RevealArea3(&owner->Location, owner->LastSightRange - 3, owner->LastSightRange + 2, 0);

			// Reset mission if not busy
			if (!owner->HaveMegaMission()) {
				owner->SetTarget(nullptr);
				owner->SetDestination(nullptr, true);
			}

			// Enter idle mode
			owner->EnterIdleMode(false, true);
		} else {
			// Failed to place - destroy owner
			if(this->Owner)
				this->Owner->UnInit();
		}
	}
}

bool FakeParasiteClass::__Victims_Cell_Valid()
{
	FootClass* victim = this->Victim;

	if (!victim) {
		return false;
	}

	// Cannot infect airborne units
	if (victim->IsInAir()) {
		return false;
	}

	// Naval parasites have different rules
	if (this->Owner->GetTechnoType()->Naval) {
		// Naval parasites work on water - check for buildings
		return victim->GetCell()->GetBuilding() == nullptr;
	}

	// Check terrain restrictions for non-naval units
	FootClass* owner = this->Owner;
	if (owner && owner->WhatAmI() == UnitClass::AbsID) {
		// Cannot operate on terrain objects
		const auto pTerrain = [](CellClass* pCell)->TerrainClass* {
			for (ObjectClass* pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject) {
					const auto pTerrain = cast_to<TerrainClass*, false>(pObject);

				if (pTerrain && !TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->IsPassable)
					return pTerrain;
			}

				return nullptr;
		}(this->Victim->GetCell());

		if (pTerrain) {
			return false;
		}
	}

	// Check land type
	CellClass* victimCell = victim->GetCell();
	LandType landType = victimCell->LandType;

	// Check water/beach/rock terrain
	if (landType == LandType::Water || landType == LandType::Beach || landType == LandType::Rock) {

		if (GroundType::GetCost(landType, owner->GetTechnoType()->SpeedType) <= 0.0) {
			// Check for specific overlay ranges (ice?)
			int overlay = victimCell->OverlayTypeIndex;
			bool hasSpecialOverlay = this->IsSpecialOverlay(overlay);

			// Invalid if it's water without bridge or special overlay
			if (!victimCell->ContainsBridgeHead() && !hasSpecialOverlay) {
				return false;
			}
		}
	}

	// Check if there's a building in the cell
	return victimCell->GetBuilding() == nullptr;
}

CoordStruct FakeParasiteClass::__Detach_From_Victim()
{
	if (!this->Victim) {
		return CoordStruct::Empty;
	}

	// If victim's cell is not valid, find adjacent cell
	if (!this->__Victims_Cell_Valid()) {
		// Get victim facing direction
		DirStruct victimFacing = this->Victim->PrimaryFacing.Current();
		int facingIndex = (((victimFacing.Raw >> 12) + 1) >> 1) & 7;

		// Get adjacent cell
		CellClass* victimCell = this->Victim->GetCell();
		CellClass* adjacentCell = victimCell->GetAdjacentCell((FacingType)facingIndex);

		// Check if non-naval unit can use this cell
		if (!this->Owner->GetTechnoType()->Naval) {
			LandType landType = adjacentCell->LandType;

			// Check water/beach/rock terrain
			if (landType == LandType::Water || landType == LandType::Beach || landType == LandType::Rock) {
				int overlay = adjacentCell->OverlayTypeIndex;
				bool hasSpecialOverlay = this->IsSpecialOverlay(overlay);

				// Cannot place on water without bridge or special overlay
				if (!adjacentCell->ContainsBridgeHead() && !hasSpecialOverlay) {
					return CoordStruct::Empty;
				}
			}
		}

		// Get cell center coordinate
		CoordStruct cellCenter = adjacentCell->GetCoords();

		// Find closest free spot
		CoordStruct freeSpot;
		adjacentCell->FindInfantrySubposition(&freeSpot, cellCenter, false, false, false);
		CoordStruct result = freeSpot;

		// For units, use exact cell center
		FootClass* owner = this->Owner;
		if (owner && owner->WhatAmI() == UnitClass::AbsID) {
			CellClass* targetCell = MapClass::Instance->GetCellAt(result);
			result = targetCell->GetCoords();
		}

		const bool HasBridgeHead = MapClass::Instance->GetCellAt(result)->ContainsBridgeHead();

		// Handle bridge placement
		if (this->Victim->OnBridge) {
			if (HasBridgeHead) {
				result.Z += ParasiteConstants::BridgeHeightOffset;
				this->Owner->OnBridge = true;
			} else {
				this->Owner->OnBridge = false;
			}
		} else {
			if (HasBridgeHead) {
				// Check if we should go on bridge
				CoordStruct victimCellCenter =victimCell->GetCoords();

				if (result.Z >= victimCellCenter.Z) {
					this->Owner->OnBridge = false;
				} else {
					result.Z += ParasiteConstants::BridgeHeightOffset;
					this->Owner->OnBridge = true;
				}
			}
			else
			{
				this->Owner->OnBridge = false;
			}
		}

		// Validate result
		if (!result.IsValid()) {
			return CoordStruct::Empty;
		}

		return result;
	}

	// Victim's cell is valid - use victim's position
	CoordStruct result = this->Victim->Location;

	// For units, adjust to cell center
	FootClass* owner = this->Owner;
	if (owner && owner->WhatAmI() == UnitClass::AbsID) {

		result = MapClass::Instance->GetCellAt(result)->GetCoords();

		// Handle bridge height
		if (this->Victim->OnBridge) {
			result.Z += ParasiteConstants::BridgeHeightOffset;
		}
	}

	// Copy bridge state
	this->Owner->OnBridge = this->Victim->OnBridge;

	return result;
}

DEFINE_FUNCTION_JUMP(VTABLE ,0x7EF8EC, FakeParasiteClass::__AI)
DEFINE_FUNCTION_JUMP(LJMP, 0x629FD0 , FakeParasiteClass::__AI)
DEFINE_FUNCTION_JUMP(LJMP, 0x62A260, FakeParasiteClass::__Detach)
DEFINE_FUNCTION_JUMP(LJMP, 0x62AC30, FakeParasiteClass::__Detach_From_Victim)
DEFINE_FUNCTION_JUMP(LJMP, 0x6297F0, FakeParasiteClass::__Grapple_AI)
DEFINE_FUNCTION_JUMP(LJMP, 0x62A980, FakeParasiteClass::__Infect)
DEFINE_FUNCTION_JUMP(LJMP, 0x62A4A0, FakeParasiteClass::__Uninfect)
DEFINE_FUNCTION_JUMP(LJMP, 0x62AB40, FakeParasiteClass::__Victims_Cell_Valid)
DEFINE_FUNCTION_JUMP(LJMP, 0x629720, FakeParasiteClass::__Update_GrappleAnim_Frame)