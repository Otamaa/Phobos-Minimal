function OnExecute()
	local _t = GetTeam()
	local _args = _t::GetCurrentScriptArg()

	if _args <= 0 then
		_t::SetStepComplete()
	else

		local _IsTicking = _t::IsGuardAreaTimerTicking()
		local _Timeleft = _t::GetAreaGuardTimerTimeLeft()

		if _IsTicking == 0 and _Timeleft == 0 then
			for _Unit = _t::GetFirstUnit() do
				if _Unit == 0 then break end

				if TechnoExtData::IsInWarfactory(_Unit) == 0 then
					_Unit::QueueMission(Mission_Area_Guard,true)
				end

				_Unit = _Unit::GetNextTeamMember()
			end

			_t::StartGuardAreaTimer(15 * _args)
		else if _IsTicking == 1 and _Timeleft == 0 then
			_t::StopGuardAreaTimer()
			_t::SetStepComplete()
		end
	end
end
