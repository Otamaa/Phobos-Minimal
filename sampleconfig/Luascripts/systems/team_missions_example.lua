-- =================================================================
-- systems/team_missions_example.lua
--
-- Custom TeamMissionType actions, dispatched via core/dispatcher.lua's
-- onTeamMission/dispatchTeamMission (see there for why this is a
-- named-lookup, not a broadcast like OnUpdate).
--
-- To use one of these from a TeamType's Script list, pick an unused
-- TeamMissionType action id (this file uses 5000+, an arbitrary range
-- assumed free — confirm nothing native/Ares/Phobos claims it in your
-- build before shipping) and set Argument to whatever that action needs.
--
-- All examples below are "tier 1" — buildable entirely from bindings
-- that already exist (Techno/House/Team methods), no new C++ primitives
-- required. Search/geometry-heavy missions (nearest-building gathering,
-- formation math) are NOT good fits for this pattern yet — see the
-- earlier design discussion for why.
--
-- State-lifetime patterns shown across these examples:
--   - WaitForCredits / HuntWhenMostlyIdle are STATELESS — every tick
--     recomputes its answer from live House/Techno data, nothing to
--     remember between calls.
--   - ScatterAndRegroup / SetRallyPoint+ReturnToRallyPoint /
--     RallyOnNearestEnemyBuilding all need to remember something ACROSS
--     ticks that must survive a save/load — so they're stored via the
--     typed team:Set*/Get* slot API (backed by TeamExtData's own
--     PhobosLuaSlots, which has its own Load/Save — see
--     Phobos.LuaSlots.h/.cpp), never a Lua-side table.
-- =================================================================

local Dispatcher = require("core.dispatcher")

-- -----------------------------------------------------------------
-- Action 5000: WaitForCredits
--   Argument = credit threshold.
--   Team does nothing and stays on this script line until its owning
--   house's credits reach the threshold, then completes.
--
--   Demonstrates: reading House state as a completion condition. No
--   per-team bookkeeping needed — the check is stateless every tick.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(5000, "WaitForCredits", function(team, threshold)
	local owner = team:GetOwner()
	if not owner then
		team:Complete() -- no owner (edge case) — don't stall forever waiting on nothing
		return true
	end

	if owner:GetCredits() >= threshold then
		team:Complete()
	end
	-- else: stay on this line, we'll be called again next tick

	return true
end)

-- -----------------------------------------------------------------
-- Action 5001: ScatterAndRegroup
--   Argument = frames to stay scattered before regrouping.
--   On first tick: every member scatters once.
--   Waits `argument` frames, then completes (native follow-up mission
--   lines, e.g. a Move/Attack, handle the actual regrouping).
--
--   Demonstrates: per-team state that must SURVIVE a save/load, stored
--   in a typed slot (team:SetInt/GetInt) backed by TeamExtData's own
--   native Serialize<T> chain in Team/Body.cpp — rather than a Lua-side
--   GetPtr()-keyed table. Two direct benefits over the table approach:
--     1. The countdown itself round-trips through a save/load instead
--        of silently resetting.
--     2. No OnInvalidatePointer cleanup needed here at all - the data
--        lives inside TeamExtData, which is freed automatically when
--        the team is destroyed (see TeamExtContainer::Instance.Remove
--        in the team dtor hook).
--
--   Uses slot 0 for the countdown. Slots are per-team GLOBAL storage
--   (see Phobos.LuaSlots.h's caveat) — if another custom mission on the
--   same TeamType also wants slot 0, they'll collide. This mission
--   clears its slot on completion specifically to avoid leaving stale
--   data behind for whatever runs next.
-- -----------------------------------------------------------------
local SCATTER_SLOT = 0

Dispatcher.onTeamMission(5001, "ScatterAndRegroup", function(team, waitFrames)
	local ticksRemaining = team:GetInt(SCATTER_SLOT)

	if not ticksRemaining then
		-- First tick on this line for this team: issue the scatter once.
		for _, member in ipairs(team:GetMembers()) do
			member:Scatter()
		end
		ticksRemaining = waitFrames
	end

	ticksRemaining = ticksRemaining - 1
	if ticksRemaining <= 0 then
		team:ClearSlot(SCATTER_SLOT) -- done - avoid leaving stale data for whatever mission runs next
		team:Complete()
	else
		team:SetInt(SCATTER_SLOT, ticksRemaining)
	end

	return true
end)

-- -----------------------------------------------------------------
-- Action 5002: HuntWhenMostlyIdle
--   Argument = minimum idle-member count required.
--   Waits until at least `argument` members are idle (Guard/Stop/Sleep),
--   then sends every member into Hunt (aggressive auto-target) and
--   completes.
--
--   Demonstrates: aggregating a per-member condition (IsIdle) across
--   team:GetMembers() to decide when to act — the kind of team-level
--   decision-making that's awkward to express as a single native
--   TMission but is a few lines of Lua.
-- -----------------------------------------------------------------
Dispatcher.onTeamMission(5002, "HuntWhenMostlyIdle", function(team, minIdle)
	local members = team:GetMembers()

	local idleCount = 0
	for _, member in ipairs(members) do
		if member:IsIdle() then
			idleCount = idleCount + 1
		end
	end

	if idleCount >= minIdle then
		for _, member in ipairs(members) do
			member:Hunt()
		end
		team:Complete()
	end

	return true
end)

-- -----------------------------------------------------------------
-- Actions 5003 + 5004: SetRallyPoint / ReturnToRallyPoint
--
-- A deliberate PAIR of actions rather than one, showing that a Team's
-- slots persist across DIFFERENT script lines, not just across ticks
-- of the SAME line — SetRallyPoint might run early in a TeamType's
-- script, with ReturnToRallyPoint triggered much later (even after a
-- save/load in between) by a completely different line.
--
-- SetRallyPoint (5003): single-tick — captures the first member's
-- current position into a Cell slot and completes immediately.
--
-- ReturnToRallyPoint (5004): multi-tick — argument is the arrival
-- distance (in cells). Issues one MoveTo per member (tracked via a
-- Bool slot so it's only issued ONCE, not re-issued every tick), then
-- waits until the team is within range before completing.
--
-- Slots used: 1 = rally Cell, 2 = "move already issued" Bool. Chosen to
-- avoid colliding with ScatterAndRegroup's slot 0 if both ever run on
-- the same TeamType — see Phobos.LuaSlots.h's collision caveat.
-- -----------------------------------------------------------------
local RALLY_CELL_SLOT = 1
local RALLY_ISSUED_SLOT = 2

Dispatcher.onTeamMission(5003, "SetRallyPoint", function(team, argument)
	local members = team:GetMembers()
	if #members > 0 then
		local pos = members[1]:GetPosition()
		team:SetCell(RALLY_CELL_SLOT, pos.x, pos.y)
	end

	team:Complete() -- single-tick: nothing to wait for
	return true
end)

Dispatcher.onTeamMission(5004, "ReturnToRallyPoint", function(team, arrivalDistance)
	local rallyX, rallyY = team:GetCell(RALLY_CELL_SLOT)
	if not rallyX then
		team:Complete() -- SetRallyPoint was never run for this team - nothing to return to
		return true
	end

	local members = team:GetMembers()
	if #members == 0 then
		team:ClearSlot(RALLY_ISSUED_SLOT)
		team:Complete()
		return true
	end

	if not team:GetBool(RALLY_ISSUED_SLOT) then
		for _, member in ipairs(members) do
			member:MoveTo(rallyX, rallyY)
		end
		team:SetBool(RALLY_ISSUED_SLOT, true)
	end

	-- Distance check uses plain Lua math against the remembered cell -
	-- there's no Techno:GetDistanceTo(cell) binding, only
	-- Techno:GetDistanceTo(otherTechno), so a bare Cell target needs its
	-- own arithmetic here rather than a direct binding call.
	local pos = members[1]:GetPosition()
	local dx, dy = pos.x - rallyX, pos.y - rallyY
	local distance = math.sqrt(dx * dx + dy * dy)

	if distance <= arrivalDistance then
		team:ClearSlot(RALLY_ISSUED_SLOT) -- allow a LATER ReturnToRallyPoint run to re-issue MoveTo
		team:Complete()
	end

	return true
end)

-- -----------------------------------------------------------------
-- Action 5005: RallyOnNearestEnemyBuilding
--
--   Finds the nearest non-allied building ONCE, remembers it by
--   GetId() (never a raw pointer - see Phobos.LuaSlots.h for why),
--   and moves the team there every tick. If the remembered target no
--   longer resolves via World.GetById() (destroyed or captured), it
--   picks a new one automatically rather than getting stuck.
--
--   Demonstrates: ObjectId slot usage — persisting a REFERENCE to a
--   specific object (not a snapshot of its position, which would go
--   stale the moment it moves) and re-resolving it every tick.
--
--   Also demonstrates a DIFFERENT mission shape from every example
--   above: this one never calls team:Complete() in its normal path.
--   It's an open-ended "hold/contest this position" mission that stays
--   active indefinitely - a real script would typically pair this with
--   a native timeout/ConditionalJump line rather than expecting Lua
--   itself to decide when enough is enough.
-- -----------------------------------------------------------------
local TARGET_BUILDING_SLOT = 3

Dispatcher.onTeamMission(5005, "RallyOnNearestEnemyBuilding", function(team, argument)
	local owner = team:GetOwner()
	local members = team:GetMembers()
	local anchor = members[1]

	if not owner or not anchor then
		team:Complete() -- no owner or no members left - nothing meaningful to do
		return true
	end

	local targetId = team:GetObjectId(TARGET_BUILDING_SLOT)
	local target = targetId and World.GetById(targetId)

	if not target then
		-- First tick, or our remembered target no longer exists: find one.
		local best, bestDist = nil, nil

		for _, building in ipairs(World.GetBuildings()) do
			local buildingOwner = building:GetOwner()
			if buildingOwner and not owner:IsAlliedWith(buildingOwner) then
				local dist = anchor:GetDistanceTo(building)
				if dist and (not bestDist or dist < bestDist) then
					best, bestDist = building, dist
				end
			end
		end

		if not best then
			team:ClearSlot(TARGET_BUILDING_SLOT)
			team:Complete() -- nothing hostile found at all - give up rather than spin forever
			return true
		end

		team:SetObjectId(TARGET_BUILDING_SLOT, best:GetId())
		target = best
	end

	local pos = target:GetPosition()
	for _, member in ipairs(members) do
		member:MoveTo(pos.x, pos.y)
	end

	return true -- intentionally never completes on its own - see header note above
end)

-- -----------------------------------------------------------------
-- Debug utility: dumps every non-empty slot for a team, using
-- GetSlotType() for introspection rather than assuming any one
-- mission's slot conventions. Not wired to anything by default - call
-- TeamMissionsExample.DumpSlots(team) from your own debug tooling (a
-- console command, an OnUpdate hook while testing, etc).
-- -----------------------------------------------------------------
local TeamMissionsExample = {}

-- NOTE: 8 must match kPhobosLuaSlotCount in Phobos.LuaSlots.h - Lua has
-- no visibility into that C++ compile-time constant. If you change one,
-- change the other (or expose it via a binding if this becomes a
-- recurring source of drift).
local SLOT_COUNT = 8

function TeamMissionsExample.DumpSlots(team)
	for slot = 0, SLOT_COUNT - 1 do
		local slotType = team:GetSlotType(slot)
		if slotType ~= "empty" then
			local value
			if slotType == "int" then
				value = team:GetInt(slot)
			elseif slotType == "float" then
				value = team:GetFloat(slot)
			elseif slotType == "bool" then
				value = team:GetBool(slot)
			elseif slotType == "cell" then
				local x, y = team:GetCell(slot)
				value = string.format("(%d,%d)", x, y)
			elseif slotType == "objectid" then
				value = team:GetObjectId(slot)
			end
			print(string.format("[TeamDebug] %s slot %d: %s = %s",
				team:GetTypeName(), slot, slotType, tostring(value)))
		end
	end
end

return TeamMissionsExample