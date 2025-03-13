
/*
*	Original Backport code author : ZivDero
*	Otamaa : do some modification to adapt YRpp and Ares stuffs
*/
struct _SpawnManager
{
	//static void AI(SpawnManagerClass* pThis)
	//{
	//	/**
	//	 *  The spawner only does logic every 10 frames.
	//	 */
	//	if (!pThis->UpdateTimer.Expired())
	//		return;
	//
	//	pThis->UpdateTimer.Start(10);
	//
	//	for (int i = 0; i < SpawnControls.Count(); i++)
	//	{
	//		SpawnControl* control = SpawnControls[i];
	//		AircraftClass* spawnee = control->Spawnee;
	//		const auto owner_ext = Extension::Fetch<TechnoClassExtension>(Owner);
	//		const auto owner_type_ext = Extension::Fetch<TechnoTypeClassExtension>(Owner->Techno_Type_Class());
	//
	//		switch (control->Status)
	//		{
	//			/**
	//			 *  The spawn is currently idle.
	//			 */
	//		case SpawnControlStatus::Idle:
	//		{
	//			/**
	//			 *  If we don't have a target, no need to do anything about it.
	//			 */
	//			if (!Target)
	//				continue;
	//
	//			/**
	//			 *  If it's not yet time to respawn, skip.
	//			 */
	//			if (!SpawnTimer.Expired())
	//				continue;
	//
	//			/**
	//			 *  If we're on cooldown.
	//			 */
	//			if (Status == SpawnManagerStatus::Cooldown)
	//				continue;
	//
	//			/**
	//			 *  No spawning during an Ion Storm.
	//			 */
	//			if (IonStorm_Is_Active())
	//				continue;
	//
	//			/**
	//			 *  If the spawner can move (i. e. is not a building), don't allow spawning while it's on the move.
	//			 */
	//			if (control->IsSpawnedMissile && Owner->Is_Foot())
	//			{
	//				if (static_cast<FootClass*>(Owner)->Locomotion->Is_Moving() || static_cast<FootClass*>(Owner)->Locomotion->Is_Moving_Now())
	//					continue;
	//			}
	//
	//			/**
	//			 *  Not quite sure what's up here.
	//			 *  Maybe should check the missile instead, huh?
	//			 */
	//			SpawnTimer = owner_type_ext->IsMissileSpawn ? 9 : 20;
	//
	//			/**
	//			 *  We can spawn 2 missiles using the burst logic.
	//			 */
	//			const auto weapon = Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon;
	//			if (control->IsSpawnedMissile && weapon->Burst > 1 && i < weapon->Burst)
	//				Owner->CurrentBurstIndex = i;
	//			else
	//				Owner->CurrentBurstIndex = 0;
	//
	//			/**
	//			 *  Update our status.
	//			 */
	//			control->Status = SpawnControlStatus::Preparing;
	//
	//			WeaponSlotType weapon_slot = Extension::Fetch<WeaponTypeClassExtension>(Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon)->IsSpawner ? WEAPON_SLOT_PRIMARY : WEAPON_SLOT_SECONDARY;
	//
	//			/**
	//			 *  Apply SecondSpawnOffset if this is the second missile in a burst.
	//			 */
	//			Coordinate fire_coord;
	//			if (Owner->CurrentBurstIndex % 2 == 0)
	//				fire_coord = owner_ext->Fire_Coord(weapon_slot);
	//			else
	//				fire_coord = owner_ext->Fire_Coord(weapon_slot, owner_type_ext->SecondSpawnOffset);
	//
	//			Coordinate spawn_coord = Coordinate(fire_coord.X, fire_coord.Y, fire_coord.Z + 10);
	//
	//			const auto rocket = RocketTypeClass::From_AircraftType(SpawnType);
	//			if (rocket && rocket->IsCruiseMissile)
	//			{
	//				spawn_coord.X -= 40;
	//				spawn_coord.Y -= 40;
	//			}
	//
	//			/**
	//			 *  Place the spawn in the world.
	//			 */
	//			DirStruct dir = Owner->PrimaryFacing.Current();
	//			spawnee->Unlimbo(spawn_coord, dir.Get_Dir());
	//
	//			/**
	//			 *  Cruise missiles spawn their takeoff animation.
	//			 */
	//			if (rocket && rocket->IsCruiseMissile && rocket->TakeoffAnim)
	//				new AnimClass(rocket->TakeoffAnim, spawnee->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
	//
	//			/**
	//			 *  Reset burst since if we're done with this volley.
	//			 */
	//			if (i == SpawnControls.Count() - 1)
	//				Owner->CurrentBurstIndex = 0;
	//
	//			/**
	//			 *  Missiles only take a destination once, so they go straight to the target.
	//			 */
	//			if (control->IsSpawnedMissile)
	//			{
	//				Next_Target();
	//				spawnee->Assign_Destination(Target);
	//				spawnee->Assign_Mission(MISSION_MOVE);
	//			}
	//			/**
	//			 *  Aircraft first "organize" next to the spawner.
	//			 */
	//			else
	//			{
	//				CellClass* owner_cell = Owner->Get_Cell_Ptr();
	//				CellClass* adjacent_cell = &owner_cell->Adjacent_Cell(FACING_S);
	//				spawnee->Assign_Destination(adjacent_cell);
	//				spawnee->Assign_Mission(MISSION_MOVE);
	//			}
	//
	//			break;
	//		}
	//
	//		/**
	//		 *  The rocket is taking off (handled by the locomotor), so wait until it's done, then let it go.
	//		 */
	//		case SpawnControlStatus::Takeoff:
	//		{
	//			if (control->ReloadTimer.Expired())
	//				Detach(spawnee);
	//			break;
	//		}
	//
	//		/**
	//		 *  The aircraft is preparing to attack.
	//		 */
	//		case SpawnControlStatus::Preparing:
	//		{
	//			/**
	//			 *  Missiles don't do this.
	//			 */
	//			if (control->IsSpawnedMissile)
	//				break;
	//
	//			/**
	//			 *  If there's not target, return to base.
	//			 */
	//			Next_Target();
	//			if (Target != nullptr)
	//			{
	//				spawnee->Assign_Destination(Owner);
	//				spawnee->Assign_Target(nullptr);
	//				spawnee->Assign_Mission(MISSION_MOVE);
	//				spawnee->Commence();
	//				control->Status = SpawnControlStatus::Returning;
	//			}
	//
	//			/**
	//			 *  Send the aircraft to attack.
	//			 */
	//          CellClass* adjacent_cell = &Owner->Get_Cell_Ptr()->Adjacent_Cell(FACING_S);
	//			spawnee->Assign_Destination(adjacent_cell);
	//			spawnee->Assign_Mission(MISSION_MOVE);
	//			break;
	//		}
	//
	//		/**
	//		 *  The aircraft is currently attacking.
	//		 */
	//		case SpawnControlStatus::Attacking:
	//		{
	//			/**
	//			 *  If there's still ammo and a target, attack.
	//			 */
	//			Next_Target();
	//			if (spawnee->Ammo > 0 && Target)
	//			{
	//				spawnee->Assign_Target(Target);
	//				spawnee->Assign_Mission(MISSION_ATTACK);
	//			}
	//			/**
	//			 *  Otherwise, return to base.
	//			 */
	//			else
	//			{
	//				spawnee->Assign_Destination(Owner);
	//				spawnee->Assign_Target(nullptr);
	//				spawnee->Assign_Mission(MISSION_MOVE);
	//				control->Status = SpawnControlStatus::Returning;
	//			}
	//			break;
	//		}
	//
	//		/**
	//		 *  The aircraft is retuning back to the spawner.
	//		 */
	//		case SpawnControlStatus::Returning:
	//		{
	//			/**
	//			 *  Check if we've got ammo and there's a target now.
	//			 *  If so, attack it.
	//			 */
	//			Next_Target();
	//			if (spawnee->Ammo > 0 && Target)
	//			{
	//				control->Status = SpawnControlStatus::Attacking;
	//				spawnee->Assign_Target(Target);
	//				spawnee->Assign_Mission(MISSION_ATTACK);
	//				break;
	//			}
	//
	//			/**
	//			 *  If we've arrived at the spawner, "land" (despawn).
	//			 *  Otherwise, keep going towards the spawner.
	//			 */
	//			Cell owner_coord = Owner->Get_Cell();
	//			Cell spawnee_coord = spawnee->Get_Cell();
	//
	//			if (owner_coord == spawnee_coord && spawnee->Coord.Z - Owner->Coord.Z < 20)
	//			{
	//				spawnee->Limbo();
	//				control->Status = SpawnControlStatus::Reloading;
	//				control->ReloadTimer = ReloadRate;
	//			}
	//			else
	//			{
	//				spawnee->Assign_Destination(Owner);
	//				spawnee->Assign_Target(nullptr);
	//				spawnee->Assign_Mission(MISSION_MOVE);
	//			}
	//
	//			break;
	//		}
	//
	//		/**
	//		 *  The aircraft has expended its ammo and is reloading.
	//		 */
	//		case SpawnControlStatus::Reloading:
	//		{
	//			/**
	//			 *  Wait until the reload timer expires.
	//			 */
	//			if (!control->ReloadTimer.Expired())
	//				break;
	//
	//			/**
	//			 *  Then reset the spawn to max ammo and health.
	//			 */
	//			control->Status = SpawnControlStatus::Idle;
	//			spawnee->Ammo = spawnee->Class->MaxAmmo;
	//			spawnee->Strength = spawnee->Class->MaxStrength;
	//			break;
	//		}
	//
	//		/**
	//		 *  The spawn has been destroyed and is respawning.
	//		 */
	//		case SpawnControlStatus::Dead:
	//		{
	//			/**
	//			 *  Wait until the reload timer expires.
	//			 */
	//			if (!control->ReloadTimer.Expired())
	//				break;
	//
	//			/**
	//			 *  Create a new spawn and set it to idle.
	//			 */
	//			control->Spawnee = static_cast<AircraftClass*>(SpawnType->Create_One_Of(Owner->Owning_House()));
	//			control->IsSpawnedMissile = RocketTypeClass::From_AircraftType(SpawnType) != nullptr;
	//			control->Spawnee->Limbo();
	//			Extension::Fetch<AircraftClassExtension>(control->Spawnee)->SpawnOwner = Owner;
	//			control->Status = SpawnControlStatus::Idle;
	//			break;
	//		}
	//		}
	//	}
	//
	//	switch (pThis->Status)
	//	{
	//	case SpawnManagerStatus::Idle: {
	//		Next_Target();
	//
	//		if (Target) {
	//			WeaponSlotType weapon = Owner->What_Weapon_Should_I_Use(Target);
	//			if (Owner->In_Range_Of(Target, weapon))
	//				Status = SpawnManagerStatus::Launching;
	//			else
	//				Abandon_Target();
	//		}
	//	}break;
	//
	//	case SpawnManagerStatus::Launching: {
	//		/**
	//		 *  If we're launching spawns, but there isn't a target anymore, stop it.
	//		 */
	//		if (Target == nullptr)
	//		{
	//			Abandon_Target();
	//			return;
	//		}
	//
	//		/**
	//		 *  Check to make sure all of our spawns are currently preparing to launch.
	//		 *  This should only happen when the spawns are missiles, I believe.
	//		 */
	//		for (int i = 0; i < SpawnControls.Count(); i++)
	//		{
	//			const SpawnControl* control = SpawnControls[i];
	//			if (control->Status != SpawnControlStatus::Preparing && control->Status != SpawnControlStatus::Dead)
	//				return;
	//		}
	//
	//		/**
	//		 *  Process all our missiles.
	//		 */
	//		bool is_missile_launcher = false;
	//		for (int i = 0; i < SpawnControls.Count(); i++)
	//		{
	//			SpawnControl* control = SpawnControls[i];
	//			AircraftClass* spawnee = control->Spawnee;
	//
	//			/**
	//			 *  Don't process dead spawns.
	//			 */
	//			if (control->Status == SpawnControlStatus::Preparing)
	//			{
	//				/**
	//				 *  If the spawn is a missile, add it to the kamikaze tracker and set it to take off.
	//				 *  Also set the reload timer to the missile's takeoff time.
	//				 */
	//				if (Extension::Fetch<AircraftTypeClassExtension>(spawnee->Techno_Type_Class())->IsMissileSpawn)
	//				{
	//					is_missile_launcher = true;
	//					KamikazeTracker->Add(spawnee, Target);
	//					KamikazeTracker->UpdateTimer = 2;
	//
	//					if (control->IsSpawnedMissile)
	//					{
	//						control->Status = SpawnControlStatus::Takeoff;
	//						const auto atype = control->Spawnee->Class;
	//						const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);
	//						control->ReloadTimer = rocket->PauseFrames + rocket->TiltFrames;
	//					}
	//					else
	//					{
	//						Detach(spawnee);
	//					}
	//				}
	//				/**
	//				 *  On the off chance it's not a missile, just set it to attack.
	//				 */
	//				else
	//				{
	//					control->Status = SpawnControlStatus::Attacking;
	//					spawnee->Assign_Target(Target);
	//					spawnee->Assign_Mission(MISSION_ATTACK);
	//				}
	//			}
	//		}
	//
	//		/**
	//		 *  If this is a missile launcher,
	//		 *  abandon the target.
	//		 */
	//		if (is_missile_launcher)
	//			Abandon_Target();
	//
	//		/**
	//		 *  Phew, time to go on cooldown.
	//		 */
	//		Status = SpawnManagerStatus::Cooldown;
	//	}break;
	//	case SpawnManagerStatus::Cooldown :
	//	{
	//		bool is_idle = true;
	//		for (int i = 0; i < SpawnControls.Count(); i++)
	//		{
	//			SpawnControl* control = SpawnControls[i];
	//			if (control->Status == SpawnControlStatus::Attacking || control->Status == SpawnControlStatus::Returning)
	//			{
	//				is_idle = false;
	//				break;
	//			}
	//		}
	//
	//		if (is_idle)
	//			Status = SpawnManagerStatus::Idle;
	//	}break
	//	default:
	//		break;
	//	}
	//}
};
