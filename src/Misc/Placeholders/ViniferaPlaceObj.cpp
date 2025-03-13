
#ifdef ViniferaPlaceObj

static Cell Clip_Scatter(Cell cell, int maxdist)
{
	/**
	 *  Get X & Y coords of given starting cell.
	 */
	int x = cell.X;
	int y = cell.Y;

	/**
	 *  Compute our x & y limits
	 */
	int xmin = Map.MapCellX;
	int xmax = xmin + Map.MapCellWidth - 1;
	int ymin = Map.MapCellY;
	int ymax = ymin + Map.MapCellHeight - 1;

	/**
	 *  Adjust the x-coordinate.
	 */
	int xdist = Random_Pick(0, maxdist);
	if (Percent_Chance(50))
	{
		x += xdist;
		if (x > xmax)
		{
			x = xmax;
		}
	}
	else
	{
		x -= xdist;
		if (x < xmin)
		{
			x = xmin;
		}
	}

	/**
	 *  Adjust the y-coordinate.
	 */
	int ydist = Random_Pick(0, maxdist);
	if (Percent_Chance(50))
	{
		y += ydist;
		if (y > ymax)
		{
			y = ymax;
		}
	}
	else
	{
		y -= ydist;
		if (y < ymin)
		{
			y = ymin;
		}
	}

	return XY_Cell(x, y);
}

static Cell Clip_Move(Cell cell, FacingType facing, int dist)
{
	/**
	 *  Get X & Y coords of given starting cell.
	 */
	int x = cell.X;
	int y = cell.Y;

	/**
	 *  Compute our x & y limits.
	 */
	int xmin = Map.MapCellX;
	int xmax = xmin + Map.MapCellWidth - 1;
	int ymin = Map.MapCellY;
	int ymax = ymin + Map.MapCellHeight - 1;

	/**
	 *  Adjust the x-coordinate.
	 */
	switch (facing)
	{
	case FACING_N:
		y -= dist;
		break;

	case FACING_NE:
		x += dist;
		y -= dist;
		break;

	case FACING_E:
		x += dist;
		break;

	case FACING_SE:
		x += dist;
		y += dist;
		break;

	case FACING_S:
		y += dist;
		break;

	case FACING_SW:
		x -= dist;
		y += dist;
		break;

	case FACING_W:
		x -= dist;
		break;

	case FACING_NW:
		x -= dist;
		y -= dist;
		break;
	}

	/**
	 *  Clip to the map
	 */
	if (x > xmax) x = xmax;
	if (x < xmin) x = xmin;

	if (y > ymax) y = ymax;
	if (y < ymin) y = ymin;

	return XY_Cell(x, y);
}

static int Scan_Place_Object(ObjectClass* obj, Cell cell, int min_dist = 1, int max_dist = 31, bool no_scatter = false)
{
	int dist;               // for object placement
	FacingType rot;         // for object placement
	FacingType fcounter;    // for object placement
	int tryval;
	Cell newcell;
	TechnoClass* techno;
	bool skipit;

	/**
	 *  First try to unlimbo the object in the given cell.
	 */
	if (Map.In_Radar(cell))
	{
		techno = Map[cell].Cell_Techno();
		if (!techno || (techno->What_Am_I() == RTTI_INFANTRY &&
			obj->What_Am_I() == RTTI_INFANTRY))
		{
			Coordinate coord = Cell_Coord(newcell, true);
			coord.Z = Map.Get_Cell_Height(coord);
			if (obj->Unlimbo(coord, DIR_N))
			{
				return true;
			}
		}
	}

	/**
	 *  Loop through distances from the given center cell; skip the center cell.
	 *  For each distance, try placing the object along each rotational direction;
	 *  if none are available, try each direction with a random scatter value.
	 *  If that fails, go to the next distance.
	 *  This ensures that the closest coordinates are filled first.
	 */
	for (dist = min_dist; dist <= max_dist; dist++)
	{

		/**
		 *  Pick a random starting direction
		 */
		rot = Random_Pick(FACING_N, FACING_NW);

		/**
		 *  Try all directions twice
		 */
		for (tryval = 0; tryval < 2; tryval++)
		{

			/**
			 *  Loop through all directions, at this distance.
			 */
			for (fcounter = FACING_N; fcounter <= FACING_NW; fcounter++)
			{

				skipit = false;

				/**
				 *  Pick a coordinate along this directional axis
				 */
				newcell = Clip_Move(cell, rot, dist);

				/**
				 *  If this is our second try at this distance, add a random scatter
				 *  to the desired cell, so our units aren't all aligned along spokes.
				 */
				if (!no_scatter && tryval > 0)
				{
					newcell = Clip_Scatter(newcell, 1);
				}

				/**
				 *  If, by randomly scattering, we've chosen the exact center, skip
				 *  it & try another direction.
				 */
				if (newcell == cell)
				{
					skipit = true;
				}

				if (Map.In_Radar(newcell) && !skipit)
				{

					/**
					 *  Only attempt to Unlimbo the object if:
					 *  - there is no techno in the cell
					 *  - the techno in the cell & the object are both infantry
					 */
					techno = Map[newcell].Cell_Techno();
					if (!techno || (techno->What_Am_I() == RTTI_INFANTRY &&
						obj->What_Am_I() == RTTI_INFANTRY))
					{
						Coordinate coord = Cell_Coord(newcell, true);
						coord.Z = Map.Get_Cell_Height(coord);
						if (obj->Unlimbo(coord, DIR_N))
						{
							return true;
						}
					}
				}

				rot++;
				if (rot > FACING_NW)
				{
					rot = FACING_N;
				}
			}
		}
	}

	return false;
}

static bool Is_Adjacent_Cell_Empty(Cell cell, FacingType facing, int dist)
{
	Cell newcell;
	TechnoClass* techno;

	/**
	 *  Pick a coordinate along this directional axis
	 */
	newcell = Clip_Move(cell, facing, dist);

	/**
	 *  Is there already an object on this cell?
	 */
	techno = Map[newcell].Cell_Techno();
	if (!techno)
	{
		return true;
	}

	/**
	 *  Is there any free infantry spots?
	 */
	if (techno->What_Am_I() == RTTI_INFANTRY
		&& Map[newcell].Is_Any_Spot_Free())
	{

		return true;
	}

	return false;
}

static bool Are_Starting_Cells_Full(Cell cell, int dist)
{
	static bool empty_flag[FACING_COUNT];
	std::memset(empty_flag, false, FACING_COUNT);

	for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; ++facing)
	{
		if (Is_Adjacent_Cell_Empty(cell, facing, dist))
		{
			return false;
		}
	}

	return true;
}

static bool Place_Object(ObjectClass* obj, Cell cell, FacingType facing, int dist)
{
	Cell newcell;
	TechnoClass* techno;

	/**
	 *  Pick a coordinate along this directional axis
	 */
	newcell = Clip_Move(cell, facing, dist);

	/**
	 *  Try to unlimbo the object in the given cell.
	 */
	if (Map.In_Radar(newcell))
	{
		techno = Map[newcell].Cell_Techno();
		if (!techno)
		{
			Coordinate coord = Cell_Coord(newcell, true);
			coord.Z = Map.Get_Cell_Height(coord);
			if (obj->Unlimbo(coord, DIR_N))
			{
				return true;
			}
		}
	}

	return false;
}

static DynamicVectorClass<Cell> Build_Starting_Waypoint_List(bool official)
{
	DynamicVectorClass<Cell> waypts;

	/**
	 *  Find first valid player spawn waypoint.
	 */
	int min_waypts = 0;
	for (int i = 0; i < 8; i++)
	{
		if (!Scen->Is_Valid_Waypoint(i))
		{
			break;
		}
		min_waypts++;
	}

	/**
	 *  Calculate the number of waypoints (as a minimum) that will be lifted from the
	 *  mission file. Bias this number so that only the first 4 waypoints are used
	 *  if there are 4 or fewer players. Unofficial maps will pick from all the
	 *  available waypoints.
	 */
	int look_for = std::max(min_waypts, Session.Players.Count() + Session.Options.AIPlayers);
	if (!official)
	{
		look_for = MAX_PLAYERS;
	}

	for (int waycount = 0; waycount < look_for; ++waycount)
	{
		if (Scen->Is_Valid_Waypoint(waycount))
		{
			Cell waycell = Scen->Get_Waypoint_Location(waycount);
			waypts.Add(waycell);
			DEBUG_INFO("Multiplayer start waypoint found at cell %d,%d.", waycell.X, waycell.Y);
		}
	}

	/**
	 *  If there are insufficient waypoints to account for all players, then randomly
	 *  assign starting points until there is enough.
	 */
	int deficiency = look_for - waypts.Count();
	if (deficiency > 0)
	{
		DEBUG_WARNING("Multiplayer start waypoint deficiency - looking for more start positions.");
		for (int index = 0; index < deficiency; ++index)
		{

			Cell trycell = XY_Cell(Map.MapCellX + Random_Pick(10, Map.MapCellWidth - 10),
								   Map.MapCellY + Random_Pick(0, Map.MapCellHeight - 10) + 10);

			trycell = Map.Nearby_Location(trycell, SPEED_TRACK, -1, MZONE_NORMAL, false, 8, 8);
			if (trycell)
			{
				waypts.Add(trycell);
				DEBUG_INFO("Random multiplayer start waypoint added at cell %d,%d.", trycell.X, trycell.Y);
			}
		}
	}

	return waypts;
}

void ScenarioClassExtension::Create_Units(bool official)
{
	/**
	 *  #issue-338
	 *
	 *  Change the starting unit formation to be like Red Alert 2.
	 *
	 *  This sets the desired placement distance from the base center cell.
	 *
	 *  @author: CCHyper
	 */
	const unsigned int PLACEMENT_DISTANCE = 3;

	int tot_units = Session.Options.UnitCount;
	if (Session.Options.Bases)
	{
		--tot_units;
	}

	DEBUG_INFO("NumPlayers = %d", Session.NumPlayers);
	DEBUG_INFO("AIPlayers = %d", Session.Options.AIPlayers);
	DEBUG_INFO("Creating %d starting units per house - Random seed is %08x", tot_units, Scen->RandomNumber);
	DEBUG_INFO("UniqueID is %08x", Scen->UniqueID);

	Cell centroid;          // centroid of this house's stuff.
	TechnoClass* obj;       // newly-created object.

	/**
	 *  Generate lists of all the available starting units (regardless of owner).
	 */
	int tot_inf_count = 0;
	int tot_unit_count = 0;

	for (int i = 0; i < UnitTypes.Count(); ++i)
	{
		UnitTypeClass* unittype = UnitTypes[i];
		if (unittype && unittype->IsAllowedToStartInMultiplayer)
		{
			if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID())
			{
				++tot_unit_count;
			}
		}
	}

	for (int i = 0; i < InfantryTypes.Count(); ++i)
	{
		InfantryTypeClass* infantrytype = InfantryTypes[i];
		if (infantrytype && infantrytype->IsAllowedToStartInMultiplayer)
		{
			++tot_inf_count;
		}
	}

	if (!(tot_inf_count + tot_unit_count))
	{
		DEBUG_WARNING("No starting units available!");
	}

	/**
	 *  Build a list of the valid waypoints. This normally shouldn't be
	 *  necessary because the scenario level designer should have assigned
	 *  valid locations to the first N waypoints, but just in case, this
	 *  loop verifies that.
	 */
	const unsigned int MAX_STORED_WAYPOINTS = 26;

	bool taken[MAX_STORED_WAYPOINTS];
	std::memset(taken, '\0', sizeof(taken));

	DynamicVectorClass<Cell> waypts;
	waypts = Build_Starting_Waypoint_List(official);

	/**
	 *  Loop through all houses.  Computer-controlled houses, with Session.Options.Bases
	 *  ON, are treated as though bases are OFF (since we have no base-building AI logic.)
	 */
	int numtaken = 0;
	for (HousesType house = HOUSE_FIRST; house < Houses.Count(); ++house)
	{

		/**
		 *  Get a pointer to this house; if there is none, go to the next house.
		 */
		HouseClass* hptr = Houses[house];
		if (hptr == nullptr)
		{
			DEV_DEBUG_INFO("Invalid house %d!", house);
			continue;
		}

		DynamicVectorClass<InfantryTypeClass*> available_infantry;
		DynamicVectorClass<UnitTypeClass*> available_units;

		/**
		 *  Skip passive houses.
		 */
		if (hptr->Class->IsMultiplayPassive)
		{
			DEV_DEBUG_INFO("House %d (%s - \"%s\") is passive, skipping.", house, hptr->Class->Name(), hptr->IniName);
			continue;
		}

		int owner_id = 1 << hptr->Class->ID;

		DEBUG_INFO("Generating units for house %d (Name: %s - \"%s\", Color: %s)...",
			house, hptr->Class->Name(), hptr->IniName, ColorSchemes[hptr->RemapColor]->Name);

		/**
		 *  Generate list of starting units for this house.
		 */
		DEBUG_INFO("  Creating list of available UnitTypes...");
		for (int i = 0; i < UnitTypes.Count(); ++i)
		{
			UnitTypeClass* unittype = UnitTypes[i];
			if (unittype)
			{

				/**
				 *  Is this unit allowed to be placed in multiplayer?
				 */
				if (!unittype->IsAllowedToStartInMultiplayer)
				{
					continue;
				}

				/**
				 *  Check tech level and ownership.
				 */
				if (unittype->TechLevel <= hptr->Control.TechLevel && (owner_id & unittype->Ownable) != 0)
				{

					if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID())
					{
						DEBUG_INFO("    Added %s", unittype->Name());
						available_units.Add(unittype);
					}
				}
			}
		}

		/**
		 *  Generate list of starting infantry for this house.
		 */
		DEBUG_INFO("  Creating list of available InfantryTypes...");
		for (int i = 0; i < InfantryTypes.Count(); ++i)
		{
			InfantryTypeClass* infantrytype = InfantryTypes[i];
			if (infantrytype)
			{

				/**
				 *  Is this unit allowed to be placed in multiplayer?
				 */
				if (!infantrytype->IsAllowedToStartInMultiplayer)
				{
					continue;
				}

				/**
				 *  Check tech level and ownership.
				 */
				if (infantrytype->TechLevel <= hptr->Control.TechLevel && (owner_id & infantrytype->Ownable) != 0)
				{
					available_infantry.Add(infantrytype);
					DEBUG_INFO("    Added %s", infantrytype->Name());
				}
			}
		}

		/**
		 *  Pick the starting location for this house. The first house just picks
		 *  one of the valid locations at random. The other houses pick the furthest
		 *  waypoint from the existing houses.
		 */
		if (numtaken == 0)
		{
			int pick = Random_Pick(0, waypts.Count() - 1);
			centroid = waypts[pick];
			taken[pick] = true;
			numtaken++;

		}
		else
		{

			/**
			 *  Set all waypoints to have a score of zero in preparation for giving
			 *  a distance score to all waypoints.
			 */
			int score[MAX_STORED_WAYPOINTS];
			std::memset(score, '\0', sizeof(score));

			/**
			 *  Scan through all waypoints and give a score as a value of the sum
			 *  of the distances from this waypoint to all taken waypoints.
			 */
			for (int index = 0; index < waypts.Count(); index++)
			{

				/**
				 *  If this waypoint has not already been taken, then accumulate the
				 *  sum of the distance between this waypoint and all other taken
				 *  waypoints.
				 */
				if (!taken[index])
				{
					for (int trypoint = 0; trypoint < waypts.Count(); trypoint++)
					{

						if (taken[trypoint])
						{
							score[index] += Distance(waypts[index], waypts[trypoint]);
						}
					}
				}
			}

			/**
			 *  Now find the waypoint with the largest score. This waypoint is the one
			 *  that is furthest from all other taken waypoints.
			 */
			int best = 0;
			int bestvalue = 0;
			for (int searchindex = 0; searchindex < waypts.Count(); searchindex++)
			{
				if (score[searchindex] > bestvalue || bestvalue == 0)
				{
					bestvalue = score[searchindex];
					best = searchindex;
				}
			}

			/**
			 *  Assign this best position to the house.
			 */
			centroid = waypts[best];
			taken[best] = true;
			numtaken++;
		}

		/**
		 *  Assign the center of this house to the waypoint location.
		 */
		hptr->Center = Cell_Coord(centroid, true);
		DEBUG_INFO("  Setting house center to %d,%d", centroid.X, centroid.Y);

		/**
		 *  If Bases are ON, place a base unit (MCV).
		 */
		if (Session.Options.Bases)
		{

			/**
			 *  #issue-206
			 *
			 *  Adds game option to allow construction yards to be placed on the
			 *  map at game start instead of an MCV.
			 *
			 *  @author: CCHyper
			 */
			if (SessionExtension && SessionExtension->ExtOptions.IsPrePlacedConYards)
			{

				/**
				 *  Create a construction yard (decided from the base unit).
				 */
				obj = new BuildingClass(Rule->BaseUnit->DeploysInto, hptr);
				if (obj->Unlimbo(Cell_Coord(centroid, true), DIR_N) || Scan_Place_Object(obj, centroid))
				{
					if (obj != nullptr)
					{
						DEBUG_INFO("  Construction yard %s placed at %d,%d.",
							obj->Class_Of()->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

						BuildingClass* building = reinterpret_cast<BuildingClass*>(obj);

						/**
						 *  Always reveal the construction yard to the player
						 *  that owns it.
						 */
						building->Revealed(obj->House);
						building->IsReadyToCommence = true;

						/**
						 *  Always consider production to have started for the
						 *  owning house. This ensures that in multiplay, computer
						 *  opponents will begin construction as soon as they start
						 *  their base.
						 */
						if (Session.Type != GAME_NORMAL)
						{

							if (!building->House->Is_Player_Control())
							{

								building->IsToRebuild = true;
								building->IsToRepair = true;

								if (building->Class->IsConstructionYard)
								{

									Cell cell = Coord_Cell(building->Coord);

									building->House->Begin_Construction();

									building->House->Base.Nodes[0].Where = cell;
									building->House->Base.field_50 = cell;

									building->House->IsStarted = true;
									building->House->IsAITriggersOn = true;
									building->House->IsBaseBuilding = true;
								}
							}
						}
					}
					hptr->FlagHome = Cell(0, 0);
					hptr->FlagLocation = nullptr;
				}

			}
			else
			{

				/**
				 *  For a human-controlled house:
				 *    - Create an MCV
				 *    - Attach a flag to it for capture-the-flag mode.
				 */
				obj = new UnitClass(Rule->BaseUnit, hptr);
				if (obj->Unlimbo(Cell_Coord(centroid, true), DIR_N) || Scan_Place_Object(obj, centroid))
				{
					if (obj != nullptr)
					{
						DEBUG_INFO("  Base unit %s placed at %d,%d.",
							obj->Class_Of()->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);
						hptr->FlagHome = Cell(0, 0);
						hptr->FlagLocation = nullptr;
						if (Special.IsCaptureTheFlag)
						{
							hptr->Flag_Attach((UnitClass*)obj, true);
						}

						/**
						 *  #issue-206
						 *
						 *  Adds game option to allow MCV's to auto-deploy on game start.
						 *
						 *  @author: CCHyper
						 */
						if (Session.Options.UnitCount == 1)
						{
							if (SessionExtension && SessionExtension->ExtOptions.IsAutoDeployMCV)
							{
								if (hptr->Is_Human_Control())
								{
									obj->Set_Mission(MISSION_UNLOAD);
								}
							}
						}
					}

				}
				else if (obj)
				{
					delete obj;
					obj = nullptr;
				}

			}
		}

		/**
		 *  #BUGFIX:
		 *  Make sure there are units available to place before entering the loop.
		 */
		bool units_available = (tot_inf_count + tot_unit_count) > 0;

		if (units_available)
		{

			TechnoTypeClass* technotype = nullptr;

			int inf_percent = 50;
			int unit_percent = 50;

			int inf_count = (Session.Options.UnitCount * inf_percent) / 100;
			int unit_count = (Session.Options.UnitCount * unit_percent) / 100;

			/**
			 *  Make sure we place 3 infantry per cell.
			 */
			inf_count *= 3;

			/**
			 *  Place starting units for this house.
			 */
			if (available_units.Count() > 0)
			{
				for (int i = 0; i < unit_count; ++i)
				{

					/**
					 *  #BUGFIX:
					 *  If all cells are full, we can stop placing units. This
					 *  stops any run away cases with Scan_Place_Object.
					 */
					if (Are_Starting_Cells_Full(centroid, PLACEMENT_DISTANCE))
					{
						break;
					}

					technotype = available_units[Random_Pick(0, available_units.Count() - 1)];
					if (!technotype)
					{
						DEBUG_WARNING("  Invalid unit pointer!");
						continue;
					}

					/**
					 *  Create an instance of the unit.
					 */
					obj = reinterpret_cast<TechnoClass*>(technotype->Create_One_Of(hptr));
					if (obj)
					{

						if (Scan_Place_Object(obj, centroid, PLACEMENT_DISTANCE, PLACEMENT_DISTANCE, true))
						{

							DEBUG_INFO("  House %s deployed object %s at %d,%d",
								hptr->Class->Name(), obj->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

							if (Scen->SpecialFlags.IsInitialVeteran)
							{
								obj->Veterancy.Set_Elite(true);
							}

							if (hptr->Is_Human_Control())
							{
								obj->Set_Mission(MISSION_GUARD);
							}
							else
							{
								obj->Set_Mission(MISSION_GUARD_AREA);
							}

						}
						else if (obj)
						{
							delete obj;
						}

					}

				}

			}

			/**
			 *  Place starting infantry for this house.
			 */
			if (available_infantry.Count() > 0)
			{
				for (int i = 0; i < inf_count; ++i)
				{

					/**
					 *  #BUGFIX:
					 *  If all cells are full, we can stop placing units. This
					 *  stops any run away cases with Scan_Place_Object.
					 */
					if (Are_Starting_Cells_Full(centroid, PLACEMENT_DISTANCE))
					{
						break;
					}

					technotype = available_infantry[Random_Pick(0, available_infantry.Count() - 1)];
					if (!technotype)
					{
						DEBUG_WARNING("  Invalid infantry pointer!");
						continue;
					}

					/**
					 *  Create an instance of the unit.
					 */
					obj = reinterpret_cast<TechnoClass*>(technotype->Create_One_Of(hptr));
					if (obj)
					{

						if (Scan_Place_Object(obj, centroid, PLACEMENT_DISTANCE, PLACEMENT_DISTANCE, true))
						{

							DEBUG_INFO("  House %s deployed object %s at %d,%d",
								hptr->Class->Name(), obj->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

							if (Scen->SpecialFlags.IsInitialVeteran)
							{
								obj->Veterancy.Set_Elite(true);
							}

							if (hptr->Is_Human_Control())
							{
								obj->Set_Mission(MISSION_GUARD);
							}
							else
							{
								obj->Set_Mission(MISSION_GUARD_AREA);
							}

						}
						else if (obj)
						{
							delete obj;
						}

					}

				}

			}

			/**
			 *  #issue-338
			 *
			 *  Change the starting unit formation to be like Red Alert 2.
			 *  As a result, this is no longer required as the units are
			 *  now placed neatly around the base unit.
			 *
			 *  @author: CCHyper
			 */
#if 0
			 /**
			  *  Scatter all the human placed objects to create
			  *  some space around the base unit.
			  */
			if (hptr->Is_Human_Control())
			{
				for (int i = 0; i < deployed_objects.Count(); ++i)
				{
					TechnoClass* techno = deployed_objects[i];
					if (techno)
					{
						techno->Scatter();
					}
				}
			}
#endif

#if 0
			/**
			 *  #BUGFIX:
			 *
			 *  Due to the costings of the starting units in Tiberian Sun, sometimes
			 *  there was a deficiency in the equal placement of units in the radius
			 *  around the starting unit. This code makes sure there are no blank
			 *  spaces around the base unit and that all players get 9 units.
			 */
			if (Session.Options.UnitCount)
			{
				for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; ++facing)
				{
					if (Is_Adjacent_Cell_Empty(centroid, facing, PLACEMENT_DISTANCE))
					{

						TechnoTypeClass* technotype = nullptr;

						/**
						 *  Very rarely should another unit be placed, the algorithm
						 *  above places a fair amount already...
						 */
						if (Percent_Chance(25))
						{
							technotype = available_units[Random_Pick(0, available_units.Count() - 1)];
						}
						else if (available_infantry.Count() > 0)
						{
							technotype = available_infantry[Random_Pick(0, available_infantry.Count() - 1)];
						}

						/**
						 *  Create an instance of the unit.
						 */
						obj = reinterpret_cast<TechnoClass*>(technotype->Create_One_Of(hptr));
						if (obj)
						{
							if (Place_Object(obj, centroid, facing, PLACEMENT_DISTANCE))
							{
								DEBUG_WARNING("  House %s deployed deficiency object %s at %d,%d",
									hptr->Class->Name(), obj->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

								if (Scen->SpecialFlags.InitialVeteran)
								{
									obj->Veterancy.Set_Elite(true);
								}

								if (hptr->Is_Human_Control())
								{
									obj->Set_Mission(MISSION_GUARD);
								}
								else
								{
									obj->Set_Mission(MISSION_GUARD_AREA);
								}

							}
							else if (obj)
							{
								delete obj;
							}
						}
					}
				}
			}
#endif

		}
	}

	DEBUG_INFO("Finished unit generation. Random number is %d", Scen->RandomNumber);
}

#endif
