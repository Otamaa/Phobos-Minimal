-- =================================================================
-- core/dispatcher.lua
--
-- Problem this solves: the C++ side calls ONE global function per
-- event (OnUpdate, OnRender, OnInvalidatePointer, OnScenarioClear,
-- OnGlobalGameSave, OnGlobalGameLoad). If two independent system
-- files each do `function OnUpdate(frame) ... end`, the second one
-- loaded silently overwrites the first — only the last definition
-- survives. That doesn't scale past one file.
--
-- This module gives every system a way to REGISTER a handler instead
-- of DEFINING the global directly. init.lua defines the globals
-- exactly once, each forwarding to this dispatcher, which then fans
-- out to every registered system.
--
-- Each handler runs inside its own pcall: one broken system module
-- logs an error and is skipped for that call, but never stops the
-- other registered systems (or the frame itself) from running. This
-- mirrors the "no domino failures" principle the C++ side already
-- applies at the whole-script level — here it's applied per-module.
-- =================================================================

local Dispatcher = {}

local handlers = {
	OnUpdate = {},
	OnRender = {},
	OnInvalidatePointer = {},
	OnScenarioClear = {},
}

-- Register a handler for a plain broadcast event.
-- event: one of the keys in `handlers` above.
-- name:  short identifier used only in error logs — pick something
--        that makes it obvious which system misbehaved.
-- fn:    the handler itself; receives the same arguments the
--        underlying C++ callback receives.
function Dispatcher.on(event, name, fn)
	assert(handlers[event], "Dispatcher: unknown event '" .. tostring(event) .. "'")
	table.insert(handlers[event], { name = name, fn = fn })
end

local function broadcast(event, ...)
	for _, entry in ipairs(handlers[event]) do
		local ok, err = pcall(entry.fn, ...)
		if not ok then
			print(string.format("[Dispatcher] %s handler '%s' errored: %s", event, entry.name, tostring(err)))
		end
	end
end

function Dispatcher.broadcastUpdate(frame)                    broadcast("OnUpdate", frame) end
function Dispatcher.broadcastRender()                          broadcast("OnRender") end
function Dispatcher.broadcastInvalidatePointer(ptr, removed)   broadcast("OnInvalidatePointer", ptr, removed) end
function Dispatcher.broadcastScenarioClear()                   broadcast("OnScenarioClear") end

-- -----------------------------------------------------------------
-- Save/Load are NOT broadcasts — the C++ contract is exactly one
-- string in, one string out (see OnGlobalGameSave/OnGlobalGameLoad
-- in Phobos.Lua.cpp). Instead, each module contributes a NAMED chunk;
-- the dispatcher packs every chunk into one self-describing blob on
-- save, and unpacks it back out to the matching module by name on
-- load. Modules never need to know about each other's data formats,
-- and a module that isn't present in an older save is just skipped.
--
-- Wire format, one entry per module that returned non-empty data:
--   [name: Lua string.pack "s1"][data: Lua string.pack "s4"]
-- "s1"/"s4" are Lua 5.4 string.pack length-prefixed string formats
-- (1-byte and 4-byte length prefixes respectively) — plenty for a
-- short module name and a payload up to 4GB.
-- -----------------------------------------------------------------
local saveHandlers = {} -- name -> function() -> string|nil
local loadHandlers = {} -- name -> function(data, scriptsChanged)

function Dispatcher.onSave(name, fn)
	assert(not saveHandlers[name], "Dispatcher: duplicate save handler name '" .. name .. "'")
	saveHandlers[name] = fn
end

function Dispatcher.onLoad(name, fn)
	assert(not loadHandlers[name], "Dispatcher: duplicate load handler name '" .. name .. "'")
	loadHandlers[name] = fn
end

function Dispatcher.collectSaveData()
	local parts = {}

	for name, fn in pairs(saveHandlers) do
		local ok, result = pcall(fn)
		if not ok then
			print(string.format("[Dispatcher] save handler '%s' errored: %s", name, tostring(result)))
		elseif type(result) == "string" and #result > 0 then
			table.insert(parts, string.pack("s1s4", name, result))
		end
		-- nil / non-string / empty-string result = "nothing to save", silently skipped
	end

	return table.concat(parts)
end

function Dispatcher.distributeLoadData(payload, scriptsChanged)
	local pos = 1
	local total = #payload

	while pos <= total do
		local ok, name, data, nextPos = pcall(string.unpack, "s1s4", payload, pos)
		if not ok then
			print("[Dispatcher] save blob corrupt or truncated, stopping load distribution: " .. tostring(name))
			return
		end
		pos = nextPos

		local fn = loadHandlers[name]
		if fn then
			local callOk, err = pcall(fn, data, scriptsChanged)
			if not callOk then
				print(string.format("[Dispatcher] load handler '%s' errored: %s", name, tostring(err)))
			end
		else
			print(string.format("[Dispatcher] no load handler for saved module '%s', skipping %d bytes", name, #data))
		end
	end
end

-- -----------------------------------------------------------------
-- Team missions are also NOT a broadcast — only one system should ever
-- claim a given custom TeamMissionType action id (two systems both
-- claiming action 5000 is a modder error, not something to silently
-- merge), so this is closer to onSave/onLoad's "named, looked-up-by-key"
-- shape than OnUpdate's fan-out-to-everyone shape.
--
-- init.lua's OnTeamMission(team, action, argument) forwards straight
-- into Dispatcher.dispatchTeamMission below.
-- -----------------------------------------------------------------
local teamMissionHandlers = {} -- actionId -> { name = ..., fn = function(team, argument) -> bool } }

function Dispatcher.onTeamMission(actionId, name, fn)
	assert(not teamMissionHandlers[actionId],
		string.format("Dispatcher: action id %d already claimed by '%s' (tried to register '%s')",
			actionId, teamMissionHandlers[actionId] and teamMissionHandlers[actionId].name or "?", name))
	teamMissionHandlers[actionId] = { name = name, fn = fn }
end

-- Returns true if a handler is registered for `action` AND it chose to
-- handle it (regardless of whether it completed the mission this tick).
-- Returns false if no handler is registered at all, or the handler
-- returned false itself.
function Dispatcher.dispatchTeamMission(team, action, argument)
	local entry = teamMissionHandlers[action]
	if not entry then
		return false
	end

	local ok, result = pcall(entry.fn, team, argument)
	if not ok then
		print(string.format("[Dispatcher] team mission '%s' (action=%d) errored: %s", entry.name, action, tostring(result)))
		return true -- treat as handled-but-not-complete, same fail-open philosophy as elsewhere
	end

	return result ~= false -- nil/true = handled; explicit false = declined
end

return Dispatcher