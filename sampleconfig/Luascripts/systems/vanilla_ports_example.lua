-- =================================================================
-- systems/vanilla_ports_example.lua
--
-- Lua re-implementations of a handful of SIMPLE vanilla TeamMissionType
-- actions (see Team/Body.cpp's _TMission_Guard / _TMission_Move_To_Cell /
-- _TMission_Player_wins / _TMission_Player_loses / _TMission_Talk_bubble),
-- for testing how equivalent behavior feels under Lua.
--
-- IMPORTANT — these are NOT the native missions. The C++ fallback dispatch
-- (see LuaAPI::OnTeamMission / Team/Body.cpp's default: cases) is only
-- ever reached for action ids the native switch does NOT already
-- recognize. Nothing here overrides or replaces vanilla Guard/Move To
-- Cell/etc — the real ones still run exactly as before for their real
-- action ids. These are separate, new action ids (6000+) that a TeamType
-- script can opt into instead, purely for side-by-side comparison/testing.
--
-- FIDELITY NOTES (read before assuming these behave identically):
--   - GuardArea omits the native _CoordinateRegroup() call every tick.
--     That's hot-path per-tick movement math (see the earlier design
--     discussion on why _Regroup-family functions stay native) -
--     deliberately not reimplemented here. Members may drift apart while
--     "guarding" in this version; vanilla pulls them back together.
--   - MoveToCell always uses the modern cell-encoding divisor (1000).
--     Vanilla picks 128 instead for older-format maps
--     (ScenarioClass::NewINIFormat() < 4), which isn't exposed to Lua
--     yet - old-format maps will decode the wrong cell here.
--   - PlayerWins/PlayerLoses call House.GetPlayer():Win()/Lose() - this
--     matches vanilla's actual code, which always targets
--     HouseClass::CurrentPlayer specifically, NOT the triggering team's
--     own owner. That looks like it only makes sense in a single-player
--     campaign context; preserved verbatim for fidelity rather than
--     "corrected" to team:GetOwner().
--   - TalkBubble's argument meaning (likely a CSF string-table id) is
--     unverified - passed through as-is.
--
-- NOT ported here:
--   - _TMission_Loop: needs a script-position-jump primitive the Team
--     binding doesn't expose yet.
--   - _TMission_Attack_Waypoint / TeamMissionType::Hound_dog: both lean
--     on native _Coordinate_Attack()/_CoordinateMove()/_Calc_Center() -
--     exactly the hot-path per-tick movement/targeting functions this
--     whole design keeps out of Lua. A "port" would be a materially
--     different algorithm wearing the same name, which would be
--     misleading to call a faithful port.
--   - TeamMissionType::Force_facing: needs FootClass::Locomotor->
--     Do_Turn(), a gradual per-tick physical rotation. An instant
--     "snap to facing" Lua wrapper would NOT be faithful - vanilla turns
--     smoothly over several ticks, a snap would look and behave
--     differently. Same category of skip as the two above: the honest
--     answer is this needs deeper native involvement, not a loose
--     approximation.
-- =================================================================

local Dispatcher = require("core.dispatcher")

-- -----------------------------------------------------------------
-- Action 6000: GuardArea (vanilla: Guard)
--   Argument = vanilla's own unit, multiplied by 15 to get frames
--   (GuardAreaTimer.Start(nNode->Argument * 15) in the original) -
--   preserved verbatim; the exact reasoning for *15 isn't re-derived
--   here, just carried over.
-- -----------------------------------------------------------------
local GUARD_SLOT = 4

Dispatcher.onTeamMission(6000, "GuardArea", function(team, argument)
	local ticksRemaining = team:GetInt(GUARD_SLOT)

	if not ticksRemaining then
		ticksRemaining = argument * 15
	end

	ticksRemaining = ticksRemaining - 1
	if ticksRemaining <= 0 then
		team:ClearSlot(GUARD_SLOT)
		team:Complete()
	else
		team:SetInt(GUARD_SLOT, ticksRemaining)
	end

	return true
end)

-- -----------------------------------------------------------------
-- Action 6001: MoveToCell (vanilla: Move_To_Cell)
--   Argument packs a cell as (Y * divisor) + X, matching vanilla's own
--   encoding - see the fidelity note at the top about the divisor.
-- -----------------------------------------------------------------
local MOVE_ISSUED_SLOT = 5

Dispatcher.onTeamMission(6001, "MoveToCell", function(team, argument)
	local divisor = 1000 -- see fidelity notes: vanilla uses 128 for older map formats
	local cellX = argument % divisor
	local cellY = math.floor(argument / divisor)

	local members = team:GetMembers()
	if #members == 0 then
		team:ClearSlot(MOVE_ISSUED_SLOT)
		team:Complete()
		return true
	end

	if not team:GetBool(MOVE_ISSUED_SLOT) then
		for _, member in ipairs(members) do
			member:MoveTo(cellX, cellY)
		end
		team:SetBool(MOVE_ISSUED_SLOT, true)
	end

	-- Vanilla's _CoordinateMove() also handles arrival detection natively;
	-- approximated here with a plain distance check, same pattern as
	-- team_missions_example.lua's ReturnToRallyPoint.
	local pos = members[1]:GetPosition()
	local dx, dy = pos.x - cellX, pos.y - cellY
	local distance = math.sqrt(dx * dx + dy * dy)

	if distance <= 1 then -- "arrived" threshold, in cells
		team:ClearSlot(MOVE_ISSUED_SLOT)
		team:Complete()
	end

	return true
end)

-- -----------------------------------------------------------------
-- Action 6002: PlayerWins (vanilla: Player_wins)
-- Action 6003: PlayerLoses (vanilla: Player_loses)
--   Both single-tick. See the fidelity note above about why these
--   target House.GetPlayer() specifically, not the team's own owner.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6002, "PlayerWins", function(team, argument)
	local player = House.GetPlayer()
	if player then
		player:Win()
	end
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6003, "PlayerLoses", function(team, argument)
	local player = House.GetPlayer()
	if player then
		player:Lose()
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6004: TalkBubble (vanilla: Talk_bubble)
--   Single-tick, fires on the first (and only, since it completes
--   immediately) dispatch - no separate "did this already fire" state
--   needed, unlike vanilla's own missionChanged-gated version, since our
--   dispatch model naturally only calls this once per script line here.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6004, "TalkBubble", function(team, argument)
	local members = team:GetMembers()
	if members[1] then
		members[1]:CreateTalkBubble(argument)
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6005: ScatterAll (vanilla: Scatter)
--   Single-tick, calls member:Scatter() on every member.
--
--   FIDELITY CAVEAT: vanilla calls Scatter(CoordStruct::Empty, true,
--   false) - our existing member:Scatter() binding, when called with no
--   arguments, defaults to the member's OWN current position instead of
--   CoordStruct::Empty. If Empty is a sentinel meaning "no defined
--   threat origin, flee outward" rather than literally "flee from world
--   origin," these are NOT guaranteed equivalent. Worth watching during
--   testing - if scatter behavior looks off compared to vanilla, this is
--   the first place to look.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6005, "ScatterAll", function(team, argument)
	for _, member in ipairs(team:GetMembers()) do
		member:Scatter()
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6006: PanicAll (vanilla: Panic)
-- Action 6007: UnPanicAll (vanilla: Unpanic)
--   Both single-tick, straightforward per-member calls.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6006, "PanicAll", function(team, argument)
	for _, member in ipairs(team:GetMembers()) do
		member:Panic()
	end
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6007, "UnPanicAll", function(team, argument)
	for _, member in ipairs(team:GetMembers()) do
		member:UnPanic()
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6008: IdleAnimAll (vanilla: Idle_anim)
--   Single-tick. Argument meaning unverified - see Techno_PlayIdleAnim's
--   VERIFY note in Phobos.Lua.cpp.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6008, "IdleAnimAll", function(team, argument)
	for _, member in ipairs(team:GetMembers()) do
		member:PlayIdleAnim(argument)
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Actions 6009-6012: SetGlobalVar / ClearGlobalVar / SetLocalVar /
-- ClearLocalVar (vanilla: Set_global / Clear_global / Set_local /
-- Clear_local)
--   All single-tick. Argument = the variable index. Vanilla exposes
--   these as four separate mission actions, each hardcoding true/false
--   into the SAME underlying GlobalVarChange/LocalVarChange call - kept
--   as four separate Lua actions too here for a direct side-by-side
--   comparison against the vanilla action ids, even though
--   Scenario.SetGlobalVar/SetLocalVar could express all four with one
--   action id and an extra argument if you were designing this fresh.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6009, "SetGlobalVar", function(team, argument)
	Scenario.SetGlobalVar(argument, true)
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6010, "ClearGlobalVar", function(team, argument)
	Scenario.SetGlobalVar(argument, false)
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6011, "SetLocalVar", function(team, argument)
	Scenario.SetLocalVar(argument, true)
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6012, "ClearLocalVar", function(team, argument)
	Scenario.SetLocalVar(argument, false)
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6013: DeleteTeamMembers (vanilla: Delete_team_members)
--   Single-tick. Silently despawns every member that passes the native
--   safety guard (see Techno_LimboAndRemove) - no death animation, no
--   kill credit. Matches vanilla's own semantics: this is a scripted
--   cleanup tool, not a combat kill.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6013, "DeleteTeamMembers", function(team, argument)
	for _, member in ipairs(team:GetMembers()) do
		member:LimboAndRemove()
	end
	team:Complete()
	return true
end)

-- -----------------------------------------------------------------
-- Action 6014: RevealMap (vanilla: Reveal_map)
-- Action 6015: ReshroudMap (vanilla: Reshroud_map)
--   Both single-tick, whole-map operations - not really "team AI" in
--   character (they don't touch this team's members at all), but
--   included since they're trivially simple and a fun, very visible way
--   to confirm the whole Lua pipeline actually fired during testing.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(6014, "RevealMap", function(team, argument)
	World.RevealMap()
	team:Complete()
	return true
end)

Dispatcher.onTeamMission(6015, "ReshroudMap", function(team, argument)
	World.ReshroudMap()
	team:Complete()
	return true
end)