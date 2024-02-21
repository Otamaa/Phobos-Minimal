--
--  Change the following strings
--  Do not make your new strings longer than the original strings.
--  Use the line of ); as a guide, if they aren't all lined up
--  then your new string isn't the correct length.
--

Replaces = {
	{Addr = 0x0082668C, To = "EXPANDMD%02d.MIX"       }, --1
	{Addr = 0x0082621C, To = "AIMD.INI"               }, --2
	{Addr = 0x00826254, To = "ARTMD.INI"              }, --3
	{Addr = 0x00826198, To = "BATTLEMD.INI"           }, --4
	{Addr = 0x008261A8, To = "BATTLEMD*.INI"          }, --5
	{Addr = 0x008295E8, To = "%sMD.INI"               }, --6
	{Addr = 0x00825DF0, To = "EVAMD.INI"              }, --7
	{Addr = 0x00830370, To = "MAPSELMD.INI"           }, --8
	{Addr = 0x00839724, To = "MISSIONMD.INI"          }, --9
	{Addr = 0x00826260, To = "RULESMD.INI"            }, --10
	{Addr = 0x0082626C, To = "RULEMD*.INI"            }, --11
	{Addr = 0x00825E50, To = "SOUNDMD.INI"            }, --12
	{Addr = 0x0081C24C, To = "THEMEMD.MIX"            }, --13
	{Addr = 0x00825D94, To = "THEMEMD.INI"            }, --14
	{Addr = 0x00826444, To = "RA2MD.INI"              }, --15
	{Addr = 0x00826614, To = "ELOCAL*.MIX"            }, --16
	{Addr = 0x00826620, To = "ECACHE*.MIX"            }, --17
	{Addr = 0x0081C284, To = "MULTIMD.MIX"            }, --18
	{Addr = 0x00826780, To = "MULTIMD.MIX"            }, --19
	{Addr = 0x0081C2EC, To = "MAPSMD%02d.MIX"         }, --20
	{Addr = 0x0082679C, To = "MAPSMD*.MIX"            }, --21
	{Addr = 0x0081C210, To = "MOVMD%02d.MIX"          }, --22
	{Addr = 0x008266A0, To = "MIXFILES\\MOVMD03.MIX"  }, --23
	{Addr = 0x008266B8, To = "MOVMD03.MIX"            }, --24
	{Addr = 0x008266C4, To = "MIXFILES\\MOVMD*.MIX"   }, --25
	{Addr = 0x008266D8, To = "MIXFILES\\MOVMD01.MIX"  }, --26
	{Addr = 0x008266F0, To = "MOVMD01.MIX"            }, --27
	{Addr = 0x00826748, To = "MOVMD*.MIX"             }	 --28
}
--Replace Main Window String is put in here due to character storage limit
MainWindowString = "Yuri's Revenge"