-- =================================================================
-- systems/economy_system.lua
--
-- Tracks each house's credit balance and logs notable swings. Kept
-- deliberately simple to make a point: NOT every system needs the
-- GetPtr()-keyed registry + OnInvalidatePointer cleanup pattern from
-- buff_system.lua. This module keys by house NAME (a plain string,
-- copied by value) and houses persist for the whole match, so there's
-- no dangling-pointer risk here and no save/load handler either —
-- it's pure live monitoring, not state worth persisting.
--
-- Use the pattern that matches your data's actual lifetime; reaching
-- for OnInvalidatePointer + GetPtr() everywhere "to be safe" is
-- unnecessary complexity when a plain string/number key already works.
-- =================================================================

local Dispatcher = require("core.dispatcher")

local EconomySystem = {}

local lastCredits = {} -- house name -> last known credit value
local SIGNIFICANT_DELTA = 500

Dispatcher.on("OnUpdate", "EconomySystem", function(frame)
	-- Sampling every 30 frames instead of every frame — this system only
	-- needs to notice trends, not react instantly.
	if frame % 30 ~= 0 then
		return
	end

	local count = House.GetCount()
	for i = 0, count - 1 do
		local house = House.GetByIndex(i)
		if house then
			local name = house:GetName()
			local credits = house:GetCredits()
			local last = lastCredits[name]

			if last and math.abs(credits - last) >= SIGNIFICANT_DELTA then
				print(string.format("[Economy] %s credits changed %+d (now %d)", name, credits - last, credits))
			end

			lastCredits[name] = credits
		end
	end
end)

return EconomySystem
