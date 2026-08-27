-- =================================================================
-- init.lua — multi-module entry point
--
-- This file's only job is to:
--   1. define the six global functions C++ actually calls, each one
--      forwarding straight into core/dispatcher.lua
--   2. require() every system module, which self-registers with the
--      dispatcher as a side effect of being loaded
--
-- init.lua itself never needs to know WHAT systems exist — adding a
-- new one is just another require() line below, with zero changes to
-- the callback wiring.
-- =================================================================

print("Engine version: " .. Engine.version)

local Dispatcher = require("core.dispatcher")

-- Order only matters if one system reads another system's state
-- within the SAME frame it's loaded. buff_system and economy_system
-- are independent, so load order is unconstrained here.
require("systems.buff_system")
require("systems.economy_system")
require("systems.team_missions_example")
require("systems.vanilla_ports_example")

function OnUpdate(frame)
	Dispatcher.broadcastUpdate(frame)
end

function OnRender()
	Dispatcher.broadcastRender()
end

function OnInvalidatePointer(ptr, removed)
	Dispatcher.broadcastInvalidatePointer(ptr, removed)
end

function OnScenarioClear()
	Dispatcher.broadcastScenarioClear()
end

function OnGlobalGameSave()
	return Dispatcher.collectSaveData()
end

function OnGlobalGameLoad(payload, scriptsChanged)
	Dispatcher.distributeLoadData(payload, scriptsChanged)
end

function OnTeamMission(team, action, argument)
	return Dispatcher.dispatchTeamMission(team, action, argument)
end