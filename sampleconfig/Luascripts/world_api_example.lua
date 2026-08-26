-- =================================================================
-- world_api_example.lua — World / Techno / House API reference
--
-- require this from init.lua (package.path is already extended to
-- Luascripts/ by RunInitScript):
--     require("world_api_example")
-- =================================================================

-- =================================================================
-- House — global namespace, no instance needed to call these
-- =================================================================
-- House.GetPlayer()        -> house | nil   (the local human player's house)
-- House.GetCount()         -> int           (total houses in the match)
-- House.GetByIndex(idx)    -> house | nil   (0-based index into HouseClass::Array)

local function printPlayerInfo()
	local player = House.GetPlayer()
	if not player then
		return -- no human player this session (e.g. pure spectator/AI match)
	end

	print(string.format(
		"[Example] Player house: %s  credits=%d  power=%d/%d",
		player:GetName(),
		player:GetCredits(),
		player:GetPowerOutput(),
		player:GetPowerDrain()
	))
end

-- house:GetCredits()              -> int
-- house:SetCredits(amount)        -> sets to an exact value (computes the delta internally)
-- house:AddCredits(delta)         -> adds/subtracts a relative amount
-- house:GetPowerOutput()          -> int
-- house:GetPowerDrain()           -> int
-- house:GetName()                 -> string (house ID, e.g. "GermanY")
-- house:IsHuman()                 -> bool
-- house:IsAlliedWith(otherHouse)  -> bool

-- =================================================================
-- World — global namespace for bulk object queries
-- =================================================================
-- World.GetBuildings()  -> table of Techno objects, all buildings
-- World.GetUnits()      -> table of Techno objects, everything that is
--                          NOT a building (vehicles, infantry, aircraft)

local function countFriendlyBuildings()
	local player = House.GetPlayer()
	if not player then return 0 end

	local count = 0
	for _, building in ipairs(World.GetBuildings()) do
		local owner = building:GetOwner()
		if owner and owner:GetName() == player:GetName() then
			count = count + 1
		end
	end
	return count
end

-- =================================================================
-- Techno — instance methods, called on any object returned by
-- World.GetBuildings()/GetUnits() or House methods that return objects
-- =================================================================
-- obj:GetTypeName()         -> string        (e.g. "GAPOWR")
-- obj:GetHealth()           -> int
-- obj:GetMaxHealth()        -> int
-- obj:GetOwner()            -> house | nil
-- obj:GetPosition()         -> { x=, y=, z= }  (map cell coordinates)
-- obj:IsAlive()             -> bool
-- obj:GetId()               -> int            (engine-wide unique ID)
-- obj:GetKind()             -> "building"|"unit"|"infantry"|"aircraft"|"other"
-- obj:GetDistanceTo(other)  -> number | nil   (map cells, nil if either is invalid)
-- obj:TakeDamage(amount, [warheadName])  -> int (remaining health)
--     goes through the real damage pipeline (fire/splash anims, kill credit),
--     not a raw Health write; warheadName defaults to Rules' C4/Fire warhead
-- obj:Disable(frames)       -> locks the object (no power/firing, or
--                              paralysis for units) for N frames, auto-restores
--
-- FootClass-only (units/infantry/aircraft — no-ops for buildings):
-- obj:Scatter([cellX, cellY])  -> flee current position, or flee toward a cell
-- obj:MoveTo(cellX, cellY)     -> bool success
-- obj:Hunt()                  -> enter aggressive auto-target mode
-- obj:IsIdle()                -> bool (Guard/Stop/Sleep mission)

local function logOwnedUnitCount()
	local player = House.GetPlayer()
	if not player then return end

	local mine = 0
	for _, unit in ipairs(World.GetUnits()) do
		local owner = unit:GetOwner()
		if owner and owner:GetName() == player:GetName() and unit:IsAlive() then
			mine = mine + 1
		end
	end

	print(string.format("[Example] You control %d live units", mine))
end

-- =================================================================
-- Run once at load, matching init.lua's log-once pattern.
-- =================================================================
local hasRunOnce = false

function OnUpdate(frame)
	if not hasRunOnce then
		printPlayerInfo()
		print(string.format("[Example] Friendly buildings: %d", countFriendlyBuildings()))
		logOwnedUnitCount()
		hasRunOnce = true
	end
end