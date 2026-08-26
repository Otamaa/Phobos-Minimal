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
-- All three examples below are "tier 1" — buildable entirely from
-- bindings that already exist (Techno/House/Team methods), no new C++
-- primitives required. Search/geometry-heavy missions (nearest-building
-- gathering, formation math) are NOT good fits for this pattern yet —
-- see the earlier design discussion for why.
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
--   Demonstrates: per-team STATE across multiple ticks. Since this
--   handler only gets (team, argument) each call — no persistent slot
--   from the engine — state lives in a GetPtr()-keyed table here,
--   exactly the pattern from advanced_object_registry_example.lua.
--   Cleaned up via OnInvalidatePointer so a team destroyed mid-scatter
--   doesn't leak an entry.
-- -----------------------------------------------------------------
local scatterState = {} -- team ptr -> ticksRemaining

Dispatcher.onTeamMission(5001, "ScatterAndRegroup", function(team, waitFrames)
	local ptr = team:GetPtr()

	if not scatterState[ptr] then
		-- First tick on this line: issue the scatter once.
		for _, member in ipairs(team:GetMembers()) do
			member:Scatter()
		end
		scatterState[ptr] = waitFrames
	end

	scatterState[ptr] = scatterState[ptr] - 1
	if scatterState[ptr] <= 0 then
		scatterState[ptr] = nil
		team:Complete()
	end

	return true
end)

-- Required cleanup partner for the state table above. Registered as a
-- plain OnUpdate-family broadcast handler (not a team mission handler) —
-- OnInvalidatePointer fires for ANY destroyed object, so this checks
-- whether it's one of OUR tracked teams and no-ops otherwise.
Dispatcher.on("OnInvalidatePointer", "TeamMissionsExample.ScatterAndRegroup", function(ptr, removed)
	if scatterState[ptr] then
		scatterState[ptr] = nil
	end
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