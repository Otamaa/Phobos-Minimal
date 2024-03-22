function OnExecute (team)
	 _args = _TeamClass_GetCurrentScriptArg(team)
	
		 if _args <= 0 then
			 _TeamClass_SetStepCompleted(team)
		 else

			 _IsTicking = _TeamClass_IsGuardAreaTimerTicking(team)
			 _Timeleft = _TeamClass_GetAreaGuardTimerTimeLeft(team)

			if not _IsTicking and _Timeleft == 0 then
				_Unit = _TeamClass_GetFirstUnit(team)

				while _Unit > 0 do
					if not _TechnoExtData_IsInWarfactory(_Unit) then
						_MissionClass_QueueMission(_Unit, Mission_Area_Guard,true)
					end

					_Unit = _Unit_GetNextTeamMember()
				end

				_TeamClass_StartGuardAreaTimer(team , 15 * _args)
			else  if _IsTicking and _Timeleft == 0 then
				_TeamClass_StopGuardAreaTimer(team)
				_TeamClass_SetStepCompleted(team)
			end
		end
	end
end