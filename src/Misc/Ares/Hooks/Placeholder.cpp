
//static std::array<std::string, 3> whos{
//	{
//		"GACNST" , "NACNST" , "YACNST"
//	}
//};
//
//static std::map <const BuildingClass*, std::vector<unsigned int>> CRCRecords {};

void PrintBld(const BuildingClass* pThis, FILE* stream)
{
	/*
	const auto iter = CRCRecords.find(pThis);

	if (iter == CRCRecords.end())
		return;

	for (int i = 0; i < (int)iter->second.size(); ++i) {
		fprintf(stream, "LasrRecorded CRCs [%d] [%08X]\n",i , iter->second.data() + i);
	}

	for (auto const& who : whos)
	{
		if (IS_SAME_STR_(pThis->Type->ID, who.c_str()))
		{
			auto printTimer = [=](const CDTimerClass& timer)
				{
					fprintf(stream, "%d %d\n", timer.TimeLeft, timer.StartTime);
				};

			//ObjectClass]
			fprintf(stream, "\n");
			fprintf(stream, "%d\n", pThis->unknown_24);
			fprintf(stream, "%d\n", pThis->unknown_28);
			fprintf(stream, "%d\n", pThis->FallRate);
			fprintf(stream, "%p\n", pThis->NextObject);
			//AmbientSoundController
			//CustomSoundController
			fprintf(stream, "%d\n", pThis->CustomSound);
			fprintf(stream, "%d\n", pThis->BombVisible);
			fprintf(stream, "%d\n", pThis->Health);
			fprintf(stream, "%d\n", pThis->EstimatedHealth);
			fprintf(stream, "%d\n", pThis->IsOnMap);
			fprintf(stream, "%d\n", pThis->NeedsRedraw);
			fprintf(stream, "%d\n", pThis->InLimbo);
			fprintf(stream, "%d\n", pThis->InOpenToppedTransport);
			fprintf(stream, "%d\n", pThis->IsSelected);

			fprintf(stream, "%p\n", pThis->Parachute);
			fprintf(stream, "%d\n", pThis->OnBridge);
			fprintf(stream, "%d\n", pThis->IsFallingDown);
			fprintf(stream, "%d\n", pThis->WasFallingDown);
			fprintf(stream, "%d\n", pThis->IsABomb);
			fprintf(stream, "%d\n", pThis->IsAlive);
			fprintf(stream, "%d\n", (int)pThis->LastLayer);
			fprintf(stream, "%d\n", pThis->IsInLogic);
			fprintf(stream, "%d\n", pThis->IsVisible);
			fprintf(stream, "%d %d %d\n", pThis->Location.X, pThis->Location.Y, pThis->Location.Z);
			fprintf(stream, "%p\n", pThis->LineTrailer);
			fprintf(stream, "%d\n", pThis->IsSelected);


			//MissionClass
			fprintf(stream, "%d\n", pThis->CurrentMission);
			fprintf(stream, "%d\n", pThis->SuspendedMission);
			fprintf(stream, "%d\n", pThis->QueuedMission);
			fprintf(stream, "%d\n", pThis->unknown_bool_B8);
			fprintf(stream, "%d\n", pThis->MissionStatus);
			fprintf(stream, "%d\n", pThis->CurrentMissionStartTime);
			fprintf(stream, "%d\n", pThis->unknown_C4);
			printTimer(pThis->UpdateTimer);


			//RadioClass
			fprintf(stream, "%d %d %d\n", (int)pThis->LastCommands[0], (int)pThis->LastCommands[1], (int)pThis->LastCommands[2]);
			fprintf(stream, "%d %d %d\n", pThis->RadioLinks.Capacity, pThis->RadioLinks.IsInitialized, pThis->RadioLinks.IsAllocated);

			fprintf(stream, "%d %d\n", pThis->Flashing.DurationRemaining, pThis->Flashing.FlashingNow);
			fprintf(stream, "%d %d %d\n", pThis->Animation.Step, pThis->Animation.Value, pThis->Animation.Timer.Duration); // how the unit animates
			fprintf(stream, "%d\n", pThis->Passengers.NumPassengers);
			fprintf(stream, "%p\n", pThis->Transporter); // unit carrying me
			fprintf(stream, "%d\n", pThis->__LastGuardAreaTargetingFrame_120);
			fprintf(stream, "%d\n", pThis->CurrentTurretNumber); // for IFV/gattling/charge turrets
			fprintf(stream, "%d\n", pThis->__TurretWeapon2_128);
			fprintf(stream, "%p\n", pThis->BehindAnim);
			fprintf(stream, "%p\n", pThis->DeployAnim);
			fprintf(stream, "%d\n", pThis->InAir);
			fprintf(stream, "%d\n", pThis->CurrentWeaponNumber); // for IFV/gattling 138
			fprintf(stream, "%d\n", pThis->CurrentRanking); // only used for promotion detection
			fprintf(stream, "%d\n", pThis->CurrentGattlingStage);
			fprintf(stream, "%d\n", pThis->GattlingValue); // sum of RateUps and RateDowns
			fprintf(stream, "%d\n", pThis->TurretAnimFrame);
			fprintf(stream, "%p\n", pThis->InitialOwner); // only set in ctor
			fprintf(stream, "%fl\n", pThis->Veterancy.Veterancy);
			fprintf(stream, "%d\n", pThis->align_154);
			fprintf(stream, "%fl\n", pThis->ArmorMultiplier);
			fprintf(stream, "%fl\n", pThis->FirepowerMultiplier);
			printTimer(pThis->IdleActionTimer); // MOO
			printTimer(pThis->RadarFlashTimer);
			printTimer(pThis->TargetingTimer); //Duration = 45 on init!
			printTimer(pThis->IronCurtainTimer);
			printTimer(pThis->IronTintTimer); // how often to alternate the effect color
			fprintf(stream, "%d\n", pThis->IronTintStage); // ^
			printTimer(pThis->AirstrikeTimer);
			printTimer(pThis->AirstrikeTintTimer); // tracks alternation of the effect color
			fprintf(stream, "%d\n", pThis->AirstrikeTintStage); //  ^
			fprintf(stream, "%d\n", pThis->ProtectType);	//0 or 1, NOT a bool - is this under ForceShield as opposed to IC?
			fprintf(stream, "%d\n", pThis->Deactivated); //Robot Tanks without power for instance
			fprintf(stream, "%p\n", pThis->DrainTarget); // eg Disk -> PowerPlant, this points to PowerPlant
			fprintf(stream, "%p\n", pThis->DrainingMe);  // eg Disk -> PowerPlant, this points to Disk
			fprintf(stream, "%p\n", pThis->DrainAnim);
			fprintf(stream, "%d\n", pThis->Disguised);
			fprintf(stream, "%d\n", pThis->DisguiseCreationFrame);
			printTimer(pThis->InfantryBlinkTimer); // Rules->InfantryBlinkDisguiseTime , detects mirage firing per description
			printTimer(pThis->DisguiseBlinkTimer); // disguise disruption timer
			fprintf(stream, "%d\n", pThis->UnlimboingInfantry); //1F8
			printTimer(pThis->ReloadTimer);
			fprintf(stream, "%d\n", pThis->unknown_208);
			fprintf(stream, "%d\n", pThis->unknown_20C);

			// WARNING! this is actually an index of HouseTypeClass es, but it's being changed to fix typical WW bugs.
			//DECLARE_PROPERTY(IndexBitfield<HouseClass*>, DisplayProductionTo); // each bit corresponds to one player on the map, telling us whether that player has (1) or hasn't (0) spied this building, and the game should display what's being produced inside it to that player. The bits are arranged by player ID, i.e. bit 0 refers to house #0 in HouseClass::Array, 1 to 1, etc.; query like ((1 << somePlayer->ArrayIndex) & someFactory->DisplayProductionToHouses) != 0

			fprintf(stream, "%d\n", pThis->Group);
			fprintf(stream, "%p\n", pThis->ArchiveTarget);
			fprintf(stream, "%p\n", pThis->Owner);
			fprintf(stream, "%d\n", pThis->CloakState);
			//DECLARE_PROPERTY(StageClass, CloakProgress); // phase from [opaque] -> [fading] -> [transparent] , [General]CloakingStages= long
			printTimer(pThis->CloakDelayTimer); // delay before cloaking again
			fprintf(stream, "%fl\n", pThis->WarpFactor); // don't ask! set to 0 in CTOR, never modified, only used as ((this->Fetch_ID) + this->WarpFactor) % 400 for something in cloak ripple
			fprintf(stream, "%d\n", pThis->unknown_bool_250);
			//CoordStruct      LastSightCoords;
			fprintf(stream, "%d\n", pThis->LastSightRange);
			fprintf(stream, "%d\n", pThis->LastSightHeight);
			fprintf(stream, "%d\n", pThis->GapSuperCharged); // GapGenerator, when SuperGapRadiusInCells != GapRadiusInCells, you can deploy the gap to boost radius
			fprintf(stream, "%d\n", pThis->GeneratingGap); // is currently generating gap
			fprintf(stream, "%d\n", pThis->GapRadius);
			fprintf(stream, "%d\n", pThis->BeingWarpedOut); // is being warped by CLEG used , for 70C5B0
			fprintf(stream, "%d\n", pThis->WarpingOut); // phasing in after chrono-jump used , for 70C5C0
			fprintf(stream, "%d\n", pThis->unknown_bool_272);
			fprintf(stream, "%d\n", pThis->unused_273);
			fprintf(stream, "%p\n", pThis->TemporalImUsing); // CLEG attacking Power Plant : CLEG's this
			fprintf(stream, "%p\n", pThis->TemporalTargetingMe); 	// CLEG attacking Power Plant : PowerPlant's this
			fprintf(stream, "%d\n", pThis->IsImmobilized); // by chrono aftereffects ,27C
			fprintf(stream, "%d\n", pThis->unknown_280);
			fprintf(stream, "%d\n", pThis->ChronoLockRemaining); // 284 countdown after chronosphere warps things around
			//CoordStruct      ChronoDestCoords; // teleport loco and chsphere set this
			fprintf(stream, "%p\n", pThis->Airstrike); //Boris
			fprintf(stream, "%d\n", pThis->Berzerk);
			fprintf(stream, "%d\n", pThis->BerzerkDurationLeft);
			fprintf(stream, "%d\n", pThis->SprayOffsetIndex); // hardcoded array of xyz offsets for sprayattack, 0 - 7, see 6FE0AD
			fprintf(stream, "%d\n", pThis->Uncrushable); // DeployedCrushable fiddles this, otherwise all 0
			fprintf(stream, "%p\n", pThis->DirectRockerLinkedUnit);
			fprintf(stream, "%p\n", pThis->LocomotorTarget); // mag->LocoTarget = victim
			fprintf(stream, "%p\n", pThis->LocomotorSource); // victim->LocoSource = mag
			fprintf(stream, "%p\n", pThis->Target); //if attacking ,tarcom
			fprintf(stream, "%p\n", pThis->LastTarget); //suspendedtarcom
			fprintf(stream, "%p\n", pThis->CaptureManager); //for Yuris
			fprintf(stream, "%p\n", pThis->MindControlledBy);
			fprintf(stream, "%d\n", pThis->MindControlledByAUnit);
			fprintf(stream, "%p\n", pThis->MindControlRingAnim);
			fprintf(stream, "%p\n", pThis->MindControlledByHouse); //used for a TAction
			fprintf(stream, "%p\n", pThis->SpawnManager);
			fprintf(stream, "%p\n", pThis->SpawnOwner); // on DMISL , points to DRED and such
			fprintf(stream, "%p\n", pThis->SlaveManager);
			fprintf(stream, "%p\n", pThis->SlaveOwner); // on SLAV, points to YAREFN
			fprintf(stream, "%p\n", pThis->OriginallyOwnedByHouse); //used for mind control

			//units point to the Building bunkering them, building points to Foot contained within
			fprintf(stream, "%p\n", pThis->BunkerLinkedItem);

			fprintf(stream, "%fl\n", pThis->PitchAngle); // not exactly, and it doesn't affect the drawing, only internal state of a dropship
			printTimer(pThis->RearmTimer);
			fprintf(stream, "%d\n", pThis->ROF);
			fprintf(stream, "%d\n", pThis->Ammo);
			fprintf(stream, "%d\n", pThis->Value); //,PurchasePrice set to actual cost when this gets queued in factory, updated only in building's 42C


			fprintf(stream, "%p\n", pThis->FireParticleSystem);
			fprintf(stream, "%p\n", pThis->SparkParticleSystem);
			fprintf(stream, "%p\n", pThis->NaturalParticleSystem);
			fprintf(stream, "%p\n", pThis->DamageParticleSystem);
			fprintf(stream, "%p\n", pThis->RailgunParticleSystem);
			fprintf(stream, "%p\n", pThis->unk1ParticleSystem);
			fprintf(stream, "%p\n", pThis->unk2ParticleSystem);
			fprintf(stream, "%p\n", pThis->FiringParticleSystem);

			fprintf(stream, "%p\n", pThis->Wave); //Beams


			// rocking effect
			fprintf(stream, "%fl\n", pThis->AngleRotatedSideways); // in this frame, in radians - if abs() exceeds pi/2, it dies
			fprintf(stream, "%fl\n", pThis->AngleRotatedForwards); // same

			// set these and leave the previous two alone!
			// if these are set, the unit will roll up to pi/4, by this step each frame, and balance back
			fprintf(stream, "%fl\n", pThis->RockingSidewaysPerFrame); // left to right - positive pushes left side up
			fprintf(stream, "%fl\n", pThis->RockingForwardsPerFrame); // back to front - positive pushes ass up

			fprintf(stream, "%d\n", pThis->HijackerInfantryType); // mutant hijacker

			//DECLARE_PROPERTY(StorageClass, Tiberium);
			fprintf(stream, "%d\n", pThis->unknown_34C);

			//DECLARE_PROPERTY(DoorClass, UnloadTimer); // times the deploy, unload, etc. cycles ,DoorClass

			//DECLARE_PROPERTY(FacingClass, BarrelFacing);
			//DECLARE_PROPERTY(FacingClass, PrimaryFacing); //Facing
			//DECLARE_PROPERTY(FacingClass, SecondaryFacing); // TurretFacing
			fprintf(stream, "%d\n", pThis->CurrentBurstIndex);
			printTimer(pThis->TargetLaserTimer);
			fprintf(stream, "%d\n", pThis->weapon_sound_randomnumber_3C8);
			fprintf(stream, "%d\n", pThis->__shipsink_3CA);
			fprintf(stream, "%d\n", pThis->CountedAsOwned); // is this techno contained in OwningPlayer->Owned... counts?
			fprintf(stream, "%d\n", pThis->IsSinking);
			fprintf(stream, "%d\n", pThis->WasSinkingAlready); // if(IsSinking && !WasSinkingAlready) { play SinkingSound; WasSinkingAlready = 1; }
			fprintf(stream, "%d\n", pThis->__ProtectMe_3CF);
			fprintf(stream, "%d\n", pThis->IsUseless); //3D0
			fprintf(stream, "%d\n", pThis->IsTickedOff); //HasBeenAttacked //3D1
			fprintf(stream, "%d\n", pThis->Cloakable); //3D2
			fprintf(stream, "%d\n", pThis->IsPrimaryFactory); //3D3 IsLoaner
			//BYTE			 IsALoaner; // 3D4
			//BYTE			 IsLocked; // 3D5
			fprintf(stream, "%d\n", pThis->Spawned); // 3D6
			fprintf(stream, "%d\n", pThis->IsInPlayfield); // 3D7
			//DECLARE_PROPERTY(RecoilData, TurretRecoil);
			//DECLARE_PROPERTY(RecoilData, BarrelRecoil);
			fprintf(stream, "%d\n", pThis->IsTethered); //418
			fprintf(stream, "%d\n", pThis->RADIO_26_27_419);
			fprintf(stream, "%d\n", pThis->IsOwnedByCurrentPlayer);
			fprintf(stream, "%d\n", pThis->DiscoveredByCurrentPlayer);
			fprintf(stream, "%d\n", pThis->DiscoveredByComputer);
			fprintf(stream, "%d\n", pThis->unknown_bool_41D);
			fprintf(stream, "%d\n", pThis->unknown_bool_41E);
			fprintf(stream, "%d\n", pThis->unknown_bool_41F);
			fprintf(stream, "%d\n", pThis->SightIncrease); // used for LeptonsPerSightIncrease
			fprintf(stream, "%d\n", pThis->RecruitableA); // these two are like Lenny and Carl, weird purpose and never seen separate
			fprintf(stream, "%d\n", pThis->RecruitableB); // they're usually set on preplaced objects in maps
			fprintf(stream, "%d\n", pThis->IsRadarTracked);
			fprintf(stream, "%d\n", pThis->IsOnCarryall);
			fprintf(stream, "%d\n", pThis->IsCrashing);
			fprintf(stream, "%d\n", pThis->WasCrashingAlready);
			fprintf(stream, "%d\n", pThis->IsBeingManipulated);
			fprintf(stream, "%p\n", pThis->BeingManipulatedBy); // set when something is being molested by a locomotor such as magnetron
			// the pointee will be marked as the killer of whatever the victim falls onto
			fprintf(stream, "%p\n", pThis->ChronoWarpedByHouse);
			fprintf(stream, "%d\n", pThis->_Mission_Patrol_430);
			fprintf(stream, "%d\n", pThis->IsMouseHovering);
			fprintf(stream, "%d\n", pThis->ShouldBeReselectOnUnlimbo);
			//	BYTE			 byte_433;
			fprintf(stream, "%p\n", pThis->OldTeam);
			fprintf(stream, "%d\n", pThis->CountedAsOwnedSpecial); // for absorbers, infantry uses this to manually control OwnedInfantry count
			fprintf(stream, "%d\n", pThis->Absorbed); // in UnitAbsorb/InfantryAbsorb or smth, lousy memory
			fprintf(stream, "%d\n", pThis->forceattackforcemovefirendlytarget_bool_43A);
			fprintf(stream, "%d\n", pThis->__RadialFireCounter_43C);
			//DECLARE_PROPERTY(DynamicVectorClass<int>, CurrentTargetThreatValues);
			//DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, CurrentTargets);

			// if DistributedFire=yes, this is used to determine which possible targets should be ignored in the latest threat scan
			//DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, AttackedTargets);

			//DECLARE_PROPERTY(AudioController, Audio3);

			fprintf(stream, "%d\n", pThis->__IsTurretTurning_49C); // Turret is moving?
			fprintf(stream, "%d\n", pThis->TurretIsRotating);

			//DECLARE_PROPERTY(AudioController, Audio4);

			fprintf(stream, "%d\n", pThis->GattlingAudioPlayed); //4B8
			fprintf(stream, "%d\n", pThis->unknown_4BC);

			//DECLARE_PROPERTY(AudioController, Audio5);

			fprintf(stream, "%d\n", pThis->gattlingsound_4D4);
			fprintf(stream, "%d\n", pThis->unknown_4D8);

			//DECLARE_PROPERTY(AudioController, Audio6);

			fprintf(stream, "%d\n", pThis->QueuedVoiceIndex);
			fprintf(stream, "%d\n", pThis->__LastVoicePlayed); //4F4
			fprintf(stream, "%d\n", pThis->deploy_bool_4F8);
			fprintf(stream, "%d\n", pThis->__creationframe_4FC);	//gets initialized with the current Frame, but this is NOT a CDTimerClass!
			fprintf(stream, "%p\n", pThis->QueueUpToEnter); // 500 BuildingClass*
			fprintf(stream, "%d\n", pThis->EMPLockRemaining);
			fprintf(stream, "%d\n", pThis->ThreatPosed); // calculated to include cargo etc
			fprintf(stream, "%d\n", pThis->ShouldLoseTargetNow); //the rest is padded for sure
			fprintf(stream, "%p\n", pThis->FiringRadBeam);
			fprintf(stream, "%p\n", pThis->PlanningToken);
			fprintf(stream, "%p\n", pThis->Disguise);
			fprintf(stream, "%p\n", pThis->DisguisedAsHouse);


			//BuildingClass
			fprintf(stream, "%p\n", pThis->Type);
			fprintf(stream, "%p\n", pThis->Factory);
			printTimer(pThis->GoingToBlowTimer);  // used for warhead DelayKill and also C4
			fprintf(stream, "%d\n", pThis->BState);
			fprintf(stream, "%d\n", pThis->QueueBState);
			fprintf(stream, "%d\n", pThis->OwnerCountryIndex);
			fprintf(stream, "%p\n", pThis->C4AppliedBy);
			fprintf(stream, "%d\n", pThis->LastStrength); //544
			fprintf(stream, "%p\n", pThis->FirestormAnim); //pointer
			fprintf(stream, "%p\n", pThis->PsiWarnAnim); //pointer
			printTimer(pThis->PlacementDelay); //550

			//AnimClass* Anims[0x15];
			//bool AnimStates[0x15]; // one flag for each of the above anims (whether the anim was enabled when power went offline?)

			//PROTECTED_PROPERTY(BYTE, align_5C5[3]);

			//DWORD DamageFireAnims1; //0x5C8
			//AnimClass* DamageFireAnims[0x8];
			fprintf(stream, "%d\n", pThis->RequiresDamageFires); // if set, ::Update spawns damage fire anims and zeroes it

			//5E8 - 5F8 ????????
			//BuildingTypeClass* Upgrades[0x3];

			fprintf(stream, "%d\n", pThis->FiringSWType); // type # of sw being launched
			fprintf(stream, "%d\n", pThis->upgrade_5FC);
			fprintf(stream, "%p\n", pThis->Spotlight);
			//RateTimer GateTimer;
			fprintf(stream, "%p\n", pThis->LightSource); // tiled light , LightIntensity > 0
			fprintf(stream, "%d\n", pThis->LaserFenceFrame); // 0-7 for active directionals, 8/12 for offline ones, check ntfnce.shp or whatever
			fprintf(stream, "%d\n", pThis->FirestormWallFrame); // anim data for firestorm active animations
			//StageClass RepairProgress; // for hospital, armory, unitrepair etc
			fprintf(stream, "%d %d %d %d\n", pThis->unknown_rect_63C.X, pThis->unknown_rect_63C.Y, pThis->unknown_rect_63C.Width, pThis->unknown_rect_63C.Height);
			fprintf(stream, "%d %d %d\n", pThis->unknown_coord_64C.X, pThis->unknown_coord_64C.Y, pThis->unknown_coord_64C.Z);
			fprintf(stream, "%d\n", pThis->unknown_int_658);
			fprintf(stream, "%d\n", pThis->unknown_65C);
			fprintf(stream, "%d\n", pThis->HasPower);
			fprintf(stream, "%d\n", pThis->IsOverpowered);

			// each powered unit controller building gets this set on power activation and unset on power outage
			fprintf(stream, "%d\n", pThis->RegisteredAsPoweredUnitSource);

			fprintf(stream, "%d\n", pThis->SupportingPrisms);
			fprintf(stream, "%d\n", pThis->HasExtraPowerBonus);
			fprintf(stream, "%d\n", pThis->HasExtraPowerDrain);
			//DynamicVectorClass<InfantryClass*> Overpowerers;
			//DynamicVectorClass<InfantryClass*> Occupants;
			fprintf(stream, "%d\n", pThis->FiringOccupantIndex); // which occupant should get XP, which weapon should be fired (see 6FF074)

			//AudioController Audio7;
			//AudioController Audio8;

			fprintf(stream, "%d\n", pThis->WasOnline); // the the last state when Update()ing. if this changed since the last Update(), UpdatePowered is called.
			fprintf(stream, "%d\n", pThis->ShowRealName); // is also NOMINAL under [Structures]
			fprintf(stream, "%d\n", pThis->BeingProduced); // is also AI_REBUILDABLE under [Structures]
			fprintf(stream, "%d\n", pThis->ShouldRebuild);// is also AI_REPAIRABLE under [Structures]
			fprintf(stream, "%d\n", pThis->HasEngineer); // used to pass the NeedsEngineer check
			printTimer(pThis->CashProductionTimer);
			fprintf(stream, "%d\n", pThis->IsAllowedToSell); //6DC bool AI_Sellable; AI_SELLABLE under [Structures]
			fprintf(stream, "%d\n", pThis->IsReadyToCommence); //6DD
			fprintf(stream, "%d\n", pThis->NeedsRepairs); // AI handholder for repair logic,
			fprintf(stream, "%d\n", pThis->IsGoingToBlow); // used for warhead DelayKill and also C4
			fprintf(stream, "%d\n", pThis->NoCrew);
			fprintf(stream, "%d\n", pThis->IsCharging); //6E1
			fprintf(stream, "%d\n", pThis->IsCharged);	//6E2
			fprintf(stream, "%d\n", pThis->HasBeenCaptured); // has this building changed ownership at least once? affects crew and repair.
			fprintf(stream, "%d\n", pThis->ActuallyPlacedOnMap);
			fprintf(stream, "%d\n", pThis->unknown_bool_6E5);
			fprintf(stream, "%d\n", pThis->IsDamaged); // AI handholder for repair logic,
			fprintf(stream, "%d\n", pThis->IsFogged);
			fprintf(stream, "%d\n", pThis->IsBeingRepaired); // show animooted repair wrench
			fprintf(stream, "%d\n", pThis->HasBuildup);
			fprintf(stream, "%d\n", pThis->StuffEnabled); // status set by EnableStuff() and DisableStuff()
			fprintf(stream, "%d\n", pThis->HasCloakingData); // some fugly buffers
			fprintf(stream, "%d\n", pThis->CloakRadius); // from Type->CloakRadiusInCells
			fprintf(stream, "%d\n", pThis->Translucency);
			fprintf(stream, "%d\n", pThis->StorageFilledSlots); // the old "silo needed" logic
			fprintf(stream, "%p\n", pThis->SecretProduction); // randomly assigned secret lab bonus, used if SecretInfantry, SecretUnit, and SecretBuilding are null
			fprintf(stream, "%d %d %d\n", pThis->ColorAdd.R, pThis->ColorAdd.G, pThis->ColorAdd.B);
			fprintf(stream, "%d\n", pThis->IsAirstrikeTargetingMe); //6FC
			fprintf(stream, "%d\n", pThis->unknown_short_700);
			fprintf(stream, "%d\n", pThis->UpgradeLevel); // as defined by Type->UpgradesToLevel=
			fprintf(stream, "%d\n", pThis->GateStage);
			fprintf(stream, "%d\n", pThis->PrismStage);
			fprintf(stream, "%d %d %d\n", pThis->PrismTargetCoords.X, pThis->PrismTargetCoords.Y, pThis->PrismTargetCoords.Z);
			fprintf(stream, "%d\n", pThis->DelayBeforeFiring); //714

			fprintf(stream, "%d\n", pThis->BunkerState); // used in UpdateBunker and friends 0x718
		}
	}
	*/
}
//void NOINLINE BuildingClass_CalculateCRC(const BuildingClass* pThis, CRCEngine* pEngine)
//{
//	pThis->AbstractClass::Compute_CRC_Impl(pEngine);
//	std::vector<unsigned int> record {};
//
//	record.reserve(200);
//	record.push_back((unsigned int)pEngine->CRC);
//
//	if (auto pNext = pThis->NextObject) {
//		pEngine->Compute_int(pNext->Fetch_ID()); record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	if (auto pTag = pThis->AttachedTag) { pEngine->Compute_int(pTag->Fetch_ID());
//		record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	pEngine->Compute_int(pThis->Health); record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsOnMap); record.push_back((unsigned int)pEngine->CRC);
//	if (SessionClass::Instance->GameMode == GameMode::Campaign
//		|| SessionClass::Instance->GameMode == GameMode::Skirmish)
//	{
//		pEngine->Compute_bool(pThis->NeedsRedraw); record.push_back((unsigned int)pEngine->CRC);
//		pEngine->Compute_bool(pThis->IsSelected); record.push_back((unsigned int)pEngine->CRC);
//	}
//	pEngine->Compute_bool(pThis->InLimbo); record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->HasParachute); record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->OnBridge);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsFallingDown);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsABomb);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsAlive);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->Location.X);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->Location.Y);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->Location.Z);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->CurrentMission);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->SuspendedMission);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->QueuedMission);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->MissionStatus);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->UpdateTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->CurrentMissionStartTime);		record.push_back((unsigned int)pEngine->CRC);
//
//	pEngine->Compute_int(pThis->RadioLinks.Capacity);		record.push_back((unsigned int)pEngine->CRC);
//	for (int i = 0; i < pThis->RadioLinks.Capacity; ++i) {
//		if (auto v4 = pThis->RadioLinks.Items[i]) {
//			pEngine->Compute_int(v4->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
//			pEngine->Compute_int((int)v4->WhatAmI());		record.push_back((unsigned int)pEngine->CRC);
//		}
//	}
//
//	pEngine->Compute_int(pThis->Group);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_double(pThis->ArmorMultiplier);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_double(pThis->FirepowerMultiplier);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->IdleActionTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->DisplayProductionTo.data);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->CloakState);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->CloakDelayTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->WarpFactor);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->unknown_bool_250);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->LastSightRange);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->PitchAngle);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->RearmTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->Ammo);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->ReloadTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->Value);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->AngleRotatedSideways);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->AngleRotatedForwards);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->RockingSidewaysPerFrame);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_float(pThis->RockingForwardsPerFrame);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsSinking);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->__ProtectMe_3CF);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsUseless);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsTickedOff);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->Cloakable);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsPrimaryFactory);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->Spawned);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsInPlayfield);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsTethered);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->RADIO_26_27_419);		record.push_back((unsigned int)pEngine->CRC);
//	if (SessionClass::Instance->GameMode == GameMode::Campaign
//		|| SessionClass::Instance->GameMode == GameMode::Skirmish)
//	{
//		pEngine->Compute_bool(pThis->IsOwnedByCurrentPlayer);		record.push_back((unsigned int)pEngine->CRC);
//		pEngine->Compute_bool(pThis->DiscoveredByCurrentPlayer);		record.push_back((unsigned int)pEngine->CRC);
//		pEngine->Compute_bool(pThis->DiscoveredByComputer);		record.push_back((unsigned int)pEngine->CRC);
//	}
//	pEngine->Compute_bool(pThis->unknown_bool_41D);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->EMPLockRemaining);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->unknown_bool_41E);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->unknown_bool_41F);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_uchar(pThis->SightIncrease);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->CurrentTargets.Count);		record.push_back((unsigned int)pEngine->CRC);
//	for (int i = 0; i < pThis->CurrentTargets.Count; ++i) {
//		pEngine->Compute_int(pThis->CurrentTargets.Items[i]->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
//		pEngine->Compute_int(pThis->CurrentTargetThreatValues.Items[i]);		record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	pEngine->Compute_int(pThis->AttackedTargets.Count);		record.push_back((unsigned int)pEngine->CRC);
//	for (int j = 0; j < pThis->AttackedTargets.Count; ++j) {
//		pEngine->Compute_int(pThis->AttackedTargets.Items[j]->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	pEngine->Compute_int(pThis->GoingToBlowTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->CashProductionTimer.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->BState);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->QueueBState);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->OwnerCountryIndex);		record.push_back((unsigned int)pEngine->CRC);
//	if (auto WhomToRepay  = pThis->C4AppliedBy) {
//		pEngine->Compute_int(WhomToRepay->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	pEngine->Compute_int(pThis->LastStrength);		record.push_back((unsigned int)pEngine->CRC);
//
//	if (auto FirestormAnim = pThis->FirestormAnim) {
//		pEngine->Compute_int(FirestormAnim->Fetch_ID());		record.push_back((unsigned int)pEngine->CRC);
//	}
//
//	pEngine->Compute_int(pThis->PlacementDelay.GetTimeLeft());		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->upgrade_5FC);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->LaserFenceFrame);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int(pThis->FirestormWallFrame);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->HasPower);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->StuffEnabled);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->BeingProduced);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->ShouldRebuild);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsAllowedToSell);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsReadyToCommence);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsBeingRepaired);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->NeedsRepairs);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsGoingToBlow);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->NoCrew);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsCharging);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsCharged);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->HasBeenCaptured);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->ActuallyPlacedOnMap);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->HasEngineer);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->unknown_bool_6E5);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_bool(pThis->IsDamaged);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_int((int)pThis->unknown_short_700);		record.push_back((unsigned int)pEngine->CRC);
//	pEngine->Compute_uchar(pThis->UpgradeLevel);		record.push_back((unsigned int)pEngine->CRC);
//
//	//CRCRecords[pThis] = std::move(record);
//}

//ASMJIT_PATCH(0x454260, BuildingClass_CalculateCRC_Handle, 0x6)
//{
//	GET(const BuildingClass*, pThis, ECX);
//	GET_STACK(CRCEngine*, pCRCEngine, 0x4);
//
//	for (const auto& who : whos) {
//		if (IS_SAME_STR_(who.c_str(), pThis->Type->ID)) {
//			BuildingClass_CalculateCRC(pThis, pCRCEngine);
//			return 0x454498;
//		}
//	}
//
//	return 0x0;
//}


//#ifdef SellFunctionHandled
//ASMJIT_PATCH(0x4471D5, BuildingClass_Sell_DetonateNoBuildup, 6)
//{
//	GET(BuildingClass* const, pStructure, ESI);
//
//	if(const auto pBomb = pStructure->AttachedBomb){
//		if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get())
//			pBomb->Detonate();// Otamaa : detonate may kill the techno before this function
//			// so this can possibly causing some weird crashes if that happening
//	}
//
//	return 0;
//}
//#else
//DEFINE_DISABLE_HOOK(0x4471D5, BuildingClass_Sell_DetonateNoBuildup_ares)
//#endif

//DEFINE_DISABLE_HOOK(0x55CFDF, CopyProtection_DontBlowMeUp_ares);
//DEFINE_JUMP(LJMP, 0x55CFDF, 0x55D059);
