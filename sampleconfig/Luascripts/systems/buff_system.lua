-- =================================================================
-- systems/buff_system.lua
--
-- A simple damage-over-time ("burning") buff system. Demonstrates the
-- full lifecycle a real gameplay system needs:
--   - registers into the SHARED OnUpdate/OnInvalidatePointer via the
--     dispatcher, instead of defining those globals itself
--   - a fast, GetPtr()-keyed live registry for per-frame ticking
--   - GetId()-keyed persistence for save/load, since GetPtr() values
--     are raw pointers and are NOT stable across a save/load (the
--     engine reassigns memory on load) — only GetId() is
--   - reconnects persisted data to real objects on load via the
--     World.GetById() lookup, since a saved pointer is meaningless
--     once loaded into a new process/session
-- =================================================================

local Dispatcher = require("core.dispatcher")

local BuffSystem = {}

-- Live registry: ptr (from obj:GetPtr()) -> { ticksRemaining, damagePerTick, objectId }
local active = {}

-- Call this to start burning an object.
function BuffSystem.applyBurning(obj, durationFrames, damagePerTick)
	local ptr = obj:GetPtr()
	if not ptr then
		return -- object already invalid, nothing to attach data to
	end

	active[ptr] = {
		ticksRemaining = durationFrames,
		damagePerTick = damagePerTick,
		objectId = obj:GetId(), -- stable identifier, used only for persistence
	}
end

-- -----------------------------------------------------------------
-- Per-frame tick, registered into the shared OnUpdate broadcast.
-- -----------------------------------------------------------------
Dispatcher.on("OnUpdate", "BuffSystem", function(frame)
	for ptr, buff in pairs(active) do
		buff.ticksRemaining = buff.ticksRemaining - 1
		if buff.ticksRemaining <= 0 then
			active[ptr] = nil
		end
	end
end)

-- -----------------------------------------------------------------
-- Cleanup on object destruction, registered into the shared
-- OnInvalidatePointer broadcast. Without this, a unit that dies
-- mid-burn would leave its entry in `active` forever — a permanent
-- leak for the rest of the match.
-- -----------------------------------------------------------------
Dispatcher.on("OnInvalidatePointer", "BuffSystem", function(ptr, removed)
	if active[ptr] then
		active[ptr] = nil
	end
end)

-- -----------------------------------------------------------------
-- Persistence. Encodes each active buff by objectId (stable), one
-- line per buff: "id,ticksRemaining,damagePerTick"
-- -----------------------------------------------------------------
Dispatcher.onSave("BuffSystem", function()
	local lines = {}
	for _, buff in pairs(active) do
		table.insert(lines, string.format("%d,%d,%d", buff.objectId, buff.ticksRemaining, buff.damagePerTick))
	end
	if #lines == 0 then
		return nil -- nothing active, don't bother writing a chunk
	end
	return table.concat(lines, "\n")
end)

Dispatcher.onLoad("BuffSystem", function(data, scriptsChanged)
	if scriptsChanged then
		-- The buff data format itself might have changed along with the
		-- script — safest default is to discard rather than guess.
		print("[BuffSystem] scripts changed since save, discarding buff data")
		return
	end

	for line in data:gmatch("[^\n]+") do
		local idStr, ticksStr, dmgStr = line:match("(%d+),(%d+),(%d+)")
		if idStr then
			local obj = World.GetById(tonumber(idStr))
			if obj then
				BuffSystem.applyBurning(obj, tonumber(ticksStr), tonumber(dmgStr))
				print(string.format("[BuffSystem] restored burning on %s (id=%s)", obj:GetTypeName(), idStr))
			else
				-- Object was destroyed between save and load (or the save is
				-- from a different scenario entirely) — just drop this entry.
				print(string.format("[BuffSystem] object id=%s no longer exists, skipping", idStr))
			end
		end
	end
end)

return BuffSystem
