--
--  Change the following strings
--  Do not make your new strings longer than the original strings.
--  Use the line of ); as a guide, if they aren't all lined up
--  then your new string isn't the correct length.
--

Replaces = {
	{Addr = 0x0082668C, To = "EXPANDMD%02d.MIX"                                     }, --1
	{Addr = 0x0082621C, To = "AIMD.INI"                                             }, --2
	{Addr = 0x00826254, To = "ARTMD.INI"                                            }, --3
	{Addr = 0x00826198, To = "BATTLEMD.INI"                                         }, --4
	{Addr = 0x008261A8, To = "BATTLEMD*.INI"                                        }, --5
	{Addr = 0x008295E8, To = "%sMD.INI"                                             }, --6
	{Addr = 0x00825DF0, To = "EVAMD.INI"                                            }, --7
	{Addr = 0x00830370, To = "MAPSELMD.INI"                                         }, --8
	{Addr = 0x00839724, To = "MISSIONMD.INI"                                        }, --9
	{Addr = 0x00826260, To = "RULESMD.INI"                                          }, --10
	{Addr = 0x0082626C, To = "RULEMD*.INI"                                          }, --11
	{Addr = 0x00825E50, To = "SOUNDMD.INI"                                          }, --12
	{Addr = 0x0081C24C, To = "THEMEMD.MIX"                                          }, --13
	{Addr = 0x00825D94, To = "THEMEMD.INI"                                          }, --14
	{Addr = 0x00826444, To = "RA2MD.INI"                                            }, --15
	{Addr = 0x00826614, To = "ELOCAL*.MIX"                                          }, --16
	{Addr = 0x00826620, To = "ECACHE*.MIX"                                          }, --17
	{Addr = 0x0081C284, To = "MULTIMD.MIX"                                          }, --18
	{Addr = 0x00826780, To = "MULTIMD.MIX"                                          }, --19
	{Addr = 0x0081C2EC, To = "MAPSMD%02d.MIX"                                       }, --20
	{Addr = 0x0082679C, To = "MAPSMD*.MIX"                                          }, --21
	{Addr = 0x0081C210, To = "MOVMD%02d.MIX"                                        }, --22
	{Addr = 0x008266A0, To = "MIXFILES\\MOVMD03.MIX"                                }, --23
	{Addr = 0x008266B8, To = "MOVMD03.MIX"                                          }, --24
	{Addr = 0x008266C4, To = "MIXFILES\\MOVMD*.MIX"                                 }, --25
	{Addr = 0x008266D8, To = "MIXFILES\\MOVMD01.MIX"                                }, --26
	{Addr = 0x008266F0, To = "MOVMD01.MIX"                                          }, --27
	{Addr = 0x00826748, To = "MOVMD*.MIX"                                           }, --28
	{Addr = 0x00830A18, To = "MPMODESMD.INI"                                        }, --29
	{Addr = 0x0081F3C0, To = "COOPCAMPMD.INI"                                       }, --30
	{Addr = 0x008332F4, To = "----------- Loading RA2MD.INI settings -----------\n" }, --32
	{Addr = 0x00825DA0, To = "Reading THEMEMD.INI\n"	                            }, --33
	{Addr = 0x00825DB8, To = "Failed to load EVAMD.INI\n"                           }, --34
	{Addr = 0x00825DD4, To = "Failed to find EVAMD.INI\n"                           }, --35
	{Addr = 0x00825DFC, To = "Reading EVAMD.INI\n"                                  }, --36
	{Addr = 0x00825E5C, To = "Reading SOUNDMD.INI\n"                                }, --37
	{Addr = 0x00825E10, To = "Failed to load SOUNDMD.INI!\n"                        }, --38
	{Addr = 0x00825E30, To = "Failed to find SOUNDMD.INI!\n"                        }, --39
	{Addr = 0x00827DAC, To = "Failed to load UIMD.INI!\n"                           }  --40
}
--Replace Main Window String is put in here due to character storage limit
MainWindowString = "Yuri's Revenge"

--Internal DLL name will be put here if needed
MovieMDINI = "MOVIEMD.INI"

--Activate this to enable certain compatibily settings
-- Phobos Develop `Convert` tags now readed as it is
-- "PlacementPreview.Show" -> "PlacementPreview"
-- "ShowBuildingPlacementPreview" -> "PlacementPreview"
-- "BuildingPlacementGrid.TranslucentLevel" -> "PlacementGrid.Translucency"
-- "BuildingPlacementPreview.DefaultTranslucentLevel" -> "PlacementPreview.Translucency"
-- "DockingPoseDir" -> "AircraftDockingDir" , Read From Art -> Read From Rules
-- "Gas.DriftSpeed" -> Gas.MaxDriftSpeed
-- Note : that some tag not worked due to them not implemented like :
-- "IronCurtain.EffectOnOrganics" , "IronCurtain.KillOrganicsWarhead" , "IsVoiceCreatedGlobal" ,etc that i may forgot :p
CompatibilityMode=false