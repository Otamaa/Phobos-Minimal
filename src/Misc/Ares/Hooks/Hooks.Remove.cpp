#include <Syringe.h>

//DEFINE_DISABLE_HOOK(0x424538, Ares_AnimClass_Update_DamageDelay)
//DEFINE_DISABLE_HOOK(0x451A28, BuildingClass_PlayAnim_Destroy_replaceDestructorcall_ares)
//DEFINE_DISABLE_HOOK(0x451E40, BuildingClass_DestroyNthAnim_Destroy_replaceDestructorcall_ares)
//DEFINE_DISABLE_HOOK(0x477007, INIClass_GetSpeedType_ares)
//DEFINE_DISABLE_HOOK(0x702cfe, TechnoClass_ReceiveDamage_PreventScatter_ares)
//DEFINE_DISABLE_HOOK(0x5673A0, MapClass_RevealArea0_ares)
//DEFINE_DISABLE_HOOK(0x5678E0, MapClass_RevealArea1_ares)
//DEFINE_DISABLE_HOOK(0x567DA0, MapClass_RevealArea2_ares)

//DEFINE_DISABLE_HOOK(0x615BD3, Handle_Static_Messages_LoopingMovie_ares)

//DEFINE_DISABLE_HOOK(0x4f8440, HouseClass_Update_TogglePower_ares)

DEFINE_DISABLE_HOOK(0x41BE80, ObjectClass_DrawRadialIndicator_ares)
DEFINE_DISABLE_HOOK(0x6FC339, TechnoClass_GetFireError_OpenToppedGunnerTemporal_ares)

DEFINE_DISABLE_HOOK(0x6F5388, TechnoClass_DrawExtras_Submerged_ares)
DEFINE_DISABLE_HOOK(0x468379, BulletClass_DrawSHP_SetAnimPalette_ares)

DEFINE_DISABLE_HOOK(0x51E748, InfantryClass_GetActionOnObject_NoSelfGuardArea_ares)

DEFINE_DISABLE_HOOK(0x4d5782, FootClass_ApproachTarget_Passive_ares)

DEFINE_DISABLE_HOOK(0x5f698f, ObjectClass_GetCell_ares)

DEFINE_DISABLE_HOOK(0x629bc0, ParasiteClass_UpdateSquiddy_Culling_ares)
DEFINE_DISABLE_HOOK(0x65b5fb, RadSiteClass_Radiate_UnhardcodeSnow_ares)
DEFINE_DISABLE_HOOK(0x6b752e, SpawnManagerClass_Update_CustomMissileTakeoff_ares)
DEFINE_DISABLE_HOOK(0x6fc417, TechnoClass_CanFire_PsionicsImmune_ares)

DEFINE_DISABLE_HOOK(0x70a990, TechnoClass_DrawVeterancy_ares)
DEFINE_DISABLE_HOOK(0x71a900, TemporalClass_Update_WarpAway_ares)

DEFINE_DISABLE_HOOK(0x744216, UnitClass_UnmarkOccupationBits_ares)
DEFINE_DISABLE_HOOK(0x7441b6, UnitClass_MarkOccupationBits_ares)
DEFINE_DISABLE_HOOK(0x70cbda, TechnoClass_DealParticleDamage_ares)

DEFINE_DISABLE_HOOK(0x4D99AA, FootClass_PointerGotInvalid_Parasite_ares)

DEFINE_DISABLE_HOOK(0x47F9A4, CellClass_DrawOverlay_WallRemap_ares)

DEFINE_DISABLE_HOOK(0x5240bd, InfantryTypeClass_LoadFromINI_DamageSparks_ares)
DEFINE_DISABLE_HOOK(0x4da584, FootClass_Update_RadImmune_ares)
DEFINE_DISABLE_HOOK(0x457DB7, BuildingClass_CanBeOccupied_SpecificAssaulters_ares)
DEFINE_DISABLE_HOOK(0x5F6515, AbstractClass_Distance2DSquared_1_ares)
DEFINE_DISABLE_HOOK(0x47C8AB, CellClass_CanThisExistHere_GateOnWall_ares)

DEFINE_DISABLE_HOOK(0x52CA37, InitGame_Delay_ares)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x42784B, AnimTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x428EA8, AnimTypeClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x428970, AnimTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x428800, AnimTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x42892C, AnimTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x428958, AnimTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x42898A, AnimTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x4287E9, AnimTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI_ares)

DEFINE_DISABLE_HOOK(0x721876, TiberiumClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x72193A, TiberiumClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x7220D0, TiberiumClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x721E80, TiberiumClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x72208C, TiberiumClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x72212C, TiberiumClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x721CDC, TiberiumClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x721CE9, TiberiumClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x721C7B, TiberiumClass_LoadFromINI_ares)
#endif

DEFINE_DISABLE_HOOK(0x5F4FF9, ObjectClass_Put_IsFlammable_ares)

DEFINE_DISABLE_HOOK(0x6F407D, TechnoClass_Init_1_ares);
DEFINE_DISABLE_HOOK(0x6F4103, TechnoClass_Init_2_ares);
DEFINE_DISABLE_HOOK(0x65B5FB, RadSiteClass_Radiate_UnhardcodeSnow_ares)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x55afb3, LogicClass_Update_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x771ee9, WeaponTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x7729b0, WeaponTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x7729c7, WeaponTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x7729d6, WeaponTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x772cd0, WeaponTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x772ea6, WeaponTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x772eb0, WeaponTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x772f8c, WeaponTypeClass_Save_ares)
DEFINE_DISABLE_HOOK(0x77311d, WeaponTypeClass_SDDTOR_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x75d1a9, WarheadTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x75dea0, WarheadTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x75deaf, WarheadTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x75e0c0, WarheadTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x75e2ae, WarheadTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x75e2c0, WarheadTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x75e39c, WarheadTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x6873ab, INIClass_ReadScenario_EarlyLoadRules_ares)
DEFINE_DISABLE_HOOK(0x6d4684, TacticalClass_Draw_FlyingStrings_ares)
DEFINE_DISABLE_HOOK(0x7258d0, AnnounceInvalidPointer_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x533058, CommandClassCallback_Register_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x679CAF, RulesData_LoadAfterTypeData_ares)
DEFINE_DISABLE_HOOK(0x667a1d, RulesClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x667a30, RulesClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x668bf0, RulesClass_Addition_ares)
DEFINE_DISABLE_HOOK(0x674730, RulesClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x675205, RulesClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x675210, RulesClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x678841, RulesClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x679a15, RulesData_LoadBeforeTypeData_ares)
#endif

DEFINE_DISABLE_HOOK(0x565215, MapClass_CTOR_NoInit_Crates_ares)//, 0x6, 56522D)

DEFINE_DISABLE_HOOK(0x6AD0ED, Game_AllowSinglePlay_ares)//, 0x5, 6AD16C);
DEFINE_DISABLE_HOOK(0x483BF1, CellClass_Load_Crates_ares)//, 0x7, 483BFE)

DEFINE_DISABLE_HOOK(0x4BA61B, DSurface_CTOR_SkipVRAM_ares)//, 0x6, 4BA623)
DEFINE_DISABLE_HOOK(0x78997B, sub_789960_RemoveWOLResolutionCheck_ares)//, 0x5, 789A58)
DEFINE_DISABLE_HOOK(0x615BD3, Handle_Static_Messages_LoopingMovie_ares)//, 0x5, 615BE0)

DEFINE_DISABLE_HOOK(0x53AF40, PsyDom_Update_ares)//, 6, 53B060)
DEFINE_DISABLE_HOOK(0x715857, TechnoTypeClass_LoadFromINI_LimitPalettes_ares);

DEFINE_DISABLE_HOOK(0x6BB9DD, WinMain_LogGameClasses_ares)//, 5, 6BBE2B)
DEFINE_DISABLE_HOOK(0x70CAD8, TechnoClass_DealParticleDamage_DontDestroyCliff_ares)//, 9, 70CB30)

DEFINE_DISABLE_HOOK(0x531726, Game_BulkDataInit_MultipleDataInitFix1_ares) //, 5, 53173A)
DEFINE_DISABLE_HOOK(0x53173F, Game_BulkDataInit_MultipleDataInitFix2_ares)//, 5, 531749)

DEFINE_DISABLE_HOOK(0x67F281, LoadGame_LateSkipSides_ares) //, 7, 67F2BF)

DEFINE_DISABLE_HOOK(0x6BD7E3, Expand_MIX_Reorg_ares)
DEFINE_DISABLE_HOOK(0x52bb64, Expand_MIX_Deorg_ares)

DEFINE_DISABLE_HOOK(0x414D36, AircraftClass_Update_DontloseTargetInAir_ares)//, 0x5 , 414D4D)
DEFINE_DISABLE_HOOK(0x425002, AnimClass_Expired_SpawnsParticle_ares)

DEFINE_DISABLE_HOOK(0x4393F2, BombClass_SDDTOR_removeUnused1_ares)
DEFINE_DISABLE_HOOK(0x438843, BombClass_Detonate_removeUnused2_ares)
DEFINE_DISABLE_HOOK(0x438799, BombClass_Detonate_removeUnused3_ares)
DEFINE_DISABLE_HOOK(0x6FCBAD, TechnoClass_GetObjectActivityState_IvanBomb_ares)
DEFINE_DISABLE_HOOK(0x438e86, BombListClass_Plant_AllTechnos_ares)
DEFINE_DISABLE_HOOK(0x438FD7, BombListClass_Plant_AttachSound_ares)//, 7 , 439022);
DEFINE_DISABLE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_ares)

DEFINE_DISABLE_HOOK(0x4471D5, BuildingClass_Sell_DetonateNoBuildup_ares)

DEFINE_DISABLE_HOOK(0x75DAE6, WarheadTypeClass_LoadFromINI_SkipLists_ares)//, 9, 75DDCC)

DEFINE_DISABLE_HOOK(0x67062F, Buf_AnimToInf_Paradrop_ares)//, 6, 6707FE)
DEFINE_DISABLE_HOOK(0x66FA13, Buf_SecretBoons_ares)//, 6, 66FAD6)
DEFINE_DISABLE_HOOK(0x66f589, Buf_Shipyard_ares)//, 6, 66F68C)


DEFINE_DISABLE_HOOK(0x66DD13, Buf_WeatherArt_ares)//, 6, 66DF19)
DEFINE_DISABLE_HOOK(0x66DB93, Buf_BridgeExplosions_ares)//, 6, 66DC96)

DEFINE_DISABLE_HOOK(0x4E3792, HTExt_Unlimit1_ares)//, 0){ return 0x4E37AD; }
DEFINE_DISABLE_HOOK(0x4E3A9C, HTExt_Unlimit2_ares)//, 0){ return 0x4E3AA1; }
DEFINE_DISABLE_HOOK(0x4E3F31, HTExt_Unlimit3_ares)//, 0){ return 0x4E3F4C; }
DEFINE_DISABLE_HOOK(0x4E412C, HTExt_Unlimit4_ares)//, 0){ return 0x4E4147; }
DEFINE_DISABLE_HOOK(0x4E41A7, HTExt_Unlimit5_ares)//, 0){ return 0x4E41C3; }

DEFINE_DISABLE_HOOK(0x56017A, OptionsDlg_WndProc_RemoveResLimit_ares)//, 0x5, 560183)
DEFINE_DISABLE_HOOK(0x5601E3, OptionsDlg_WndProc_RemoveHiResCheck_ares)//, 0x9, 5601FC)

DEFINE_DISABLE_HOOK(0x679caf, RulesClass_LoadAfterTypeData_CompleteInitialization_ares)

DEFINE_DISABLE_HOOK(0x441D25, BuildingClass_Destroy_ares)//, 0xA, 441D37);
DEFINE_DISABLE_HOOK(0x4449DF, BuildingClass_KickOutUnit_PreventClone_ares)//, 0x6, 444A53)

DEFINE_DISABLE_HOOK(0x446E9F, BuildingClass_Place_FreeUnit_Mission_ares);

DEFINE_DISABLE_HOOK(0x441163, BuildingClass_Put_DontSpawnSpotlight_ares)//, 0x6, 441196)
DEFINE_DISABLE_HOOK(0x451132, BuildingClass_ProcessAnims_SuperWeaponsB_ares)//, 0x6, 451145)
DEFINE_DISABLE_HOOK(0x44656D, BuildingClass_Place_SuperWeaponAnimsB_ares)//, 0x6, 446580)

DEFINE_DISABLE_HOOK(0x46A5B2, BulletClass_Shrapnel_WeaponType1_ares)
DEFINE_DISABLE_HOOK(0x46AA27, BulletClass_Shrapnel_WeaponType2_ares)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x4664ba, BulletClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x4665e9, BulletClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x46ae70, BulletClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46af97, BulletClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46af9e, BulletClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46afb0, BulletClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46afc4, BulletClass_Save_Suffix_ares)

DEFINE_DISABLE_HOOK(0x46BDD9, BulletTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x46C730, BulletTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46C722, BulletTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46C74A, BulletTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x46C429, BulletTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI_ares)
#endif

DEFINE_DISABLE_HOOK(0x69A310, SessionClass_GetPlayerColorScheme_ares)

DEFINE_DISABLE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning_ares)
DEFINE_DISABLE_HOOK(0x49F620, CopyProtection_NotifyLauncher_ares)
DEFINE_DISABLE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData_ares)

DEFINE_DISABLE_HOOK(0x4DB37C, FootClass_Remove_Airspace_ares)//, 0x6, 4DB3A4)


DEFINE_DISABLE_HOOK(0x465d4a, BuildingTypeClass_IsUndeployable_ares) //, 6)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x43bcbd, BuildingClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x43c022, BuildingClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x453e20, BuildingClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x45417e, BuildingClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x454190, BuildingClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x454244, BuildingClass_Save_Suffix_ares)

DEFINE_DISABLE_HOOK(0x45e50c, BuildingTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x45e707, BuildingTypeClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x464a49, BuildingTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x464a56, BuildingTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x4652ed, BuildingTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x465300, BuildingTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46536a, BuildingTypeClass_Save_Suffix_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x4f6532, HouseClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x4f7371, HouseClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x50114d, HouseClass_InitFromINI_ares)
DEFINE_DISABLE_HOOK(0x503040, HouseClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x504069, HouseClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x504080, HouseClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x5046de, HouseClass_Save_Suffix_ares)

DEFINE_DISABLE_HOOK(0x511635, HouseTypeClass_CTOR_1_ares)
DEFINE_DISABLE_HOOK(0x511643, HouseTypeClass_CTOR_2_ares)
DEFINE_DISABLE_HOOK(0x51214f, HouseTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x51215a, HouseTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x512290, HouseTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x51246d, HouseTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x512480, HouseTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x51255c, HouseTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x5127cf, HouseTypeClass_DTOR_ares)
#endif

DEFINE_DISABLE_HOOK(0x4E38D8, LoadPlayerCountryString_ares)
DEFINE_DISABLE_HOOK(0x508F91, HouseClass_SpySat_Update_CheckEligible_ares) //, 6)
DEFINE_DISABLE_HOOK(0x50928C, HouseClass_Update_Factories_Queues_SkipBrokenDTOR_ares)//, 0x5, 5092A3)

DEFINE_DISABLE_HOOK(0x51E5BB, InfantryClass_GetActionOnObject_MultiEngineerA_ares)//, 0x5, 51E5D9)
DEFINE_DISABLE_HOOK(0x6FCFA4, TechnoClass_GetROF_BuildingHack_ares)//, 0x5, 6FCFC1)
DEFINE_DISABLE_HOOK(0x51F1D8, InfantryClass_ActionOnObject_IvanBombs_ares)//, 0x6, 51F1EA)

DEFINE_DISABLE_HOOK(0x4B5F9E, DropPodLocomotionClass_ILocomotion_Process_Report_ares)
DEFINE_DISABLE_HOOK(0x4B619F, DropPodLocomotionClass_ILocomotion_MoveTo_AtmosphereEntry_ares)

//DEFINE_DISABLE_HOOK(0x653CA6, RadarClass_GetMouseAction_AllowMinimap_ares) //, 5)
DEFINE_DISABLE_HOOK(0x5BDDC0, MouseClass_Update_Reset_ares) //, 5)
DEFINE_DISABLE_HOOK(0x4AB35A, DisplayClass_SetAction_CustomCursor_ares) //, 6)
DEFINE_DISABLE_HOOK(0x5BDC8C, MouseClass_UpdateCursor_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDADF, MouseClass_UpdateCursorMinimapState_UseCursor_ares) //, 0
DEFINE_DISABLE_HOOK(0x5BDDC8, MouseClass_Update_AnimateCursor_ares) //, 6
DEFINE_DISABLE_HOOK(0x5BDE64, MouseClass_Update_AnimateCursor2_ares) //, 6
DEFINE_DISABLE_HOOK(0x5BDB90, MouseClass_GetCursorFirstFrame_Minimap_ares) //, B
DEFINE_DISABLE_HOOK(0x5BE974, MouseClass_GetCursorFirstFrame_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BE994, MouseClass_GetCursorFrameCount_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDBC4, MouseClass_GetCursorCurrentFrame_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDC1B, MouseClass_GetCursorHotSpot_ares) //, 7

//;\Ext\ParticleSystem\Body.cpp
DEFINE_DISABLE_HOOK(0x62DF05, ParticleSystemClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x62E26B, ParticleSystemClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x630090, ParticleSystemClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x62FF20, ParticleSystemClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x630088, ParticleSystemClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6300F3, ParticleSystemClass_Save_Suffix_ares)

//;\Ext\ParticleType\Body.cpp
DEFINE_DISABLE_HOOK(0x644DBB, ParticleTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x645A3B, ParticleTypeClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6457A0, ParticleTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x645405, ParticleTypeClass_LoadFromINI_ares)

DEFINE_DISABLE_HOOK(0x62CDE8, ParticleClass_Update_Fire_ares) //, 5)
DEFINE_DISABLE_HOOK(0x62C2ED, ParticleClass_Update_Gas_ares) //, 6)


DEFINE_DISABLE_HOOK(0x62CDB6, ParticleClass_Update_Fire_ares)
DEFINE_DISABLE_HOOK(0x48A634, FlashbangWarheadAt_Details_ares)

DEFINE_DISABLE_HOOK(0x701A5C, TechnoClass_ReceiveDamage_IronCurtainFlash_ares)
DEFINE_DISABLE_HOOK(0x71B99E, TerrainClass_ReceiveDamage_ForestFire_ares)
DEFINE_DISABLE_HOOK(0x5185C8, InfantryClass_ReceiveDamage_InfDeath_ares)
DEFINE_DISABLE_HOOK(0x5f53e5, ObjectClass_ReceiveDamage_Relative_ares)
DEFINE_DISABLE_HOOK(0x5f5456, ObjectClass_ReceiveDamage_Culling_ares)
DEFINE_DISABLE_HOOK(0x41668B, AircraftClass_ReceiveDamage_Survivours_ares)

DEFINE_DISABLE_HOOK(0x685659, Scenario_ClearClasses_ares)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x6a4609, SideClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x6a4780, SideClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6a488b, SideClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6a48a0, SideClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6a48fc, SideClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6a499f, SideClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x679a10, SideClass_LoadAllFromINI_ares)
#endif


DEFINE_DISABLE_HOOK(0x6E9443, TeamClass_AI_HandleAres)

#ifndef aaa
DEFINE_DISABLE_HOOK(0x6f3260, TechnoClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x6f4500, TechnoClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x70bf50, TechnoClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x70c249, TechnoClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x70c250, TechnoClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x70c264, TechnoClass_Save_Suffix_ares)
#endif

#ifndef aaa
DEFINE_DISABLE_HOOK(0x711835, TechnoTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x711ae0, TechnoTypeClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x716123, TechnoTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x716132, TechnoTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x7162f0, TechnoTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x716dac, TechnoTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x716dc0, TechnoTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x717094, TechnoTypeClass_Save_Suffix_ares)
#endif


DEFINE_DISABLE_HOOK(0x6E232E, ActionClass_PlayAnimAt_ares)

DEFINE_DISABLE_HOOK(0x6DD176, TActionClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x6E4761, TActionClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6E3E30, TActionClass_Save_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6E3DB0, TActionClass_Load_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6E3E29, TActionClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6E3E4A, TActionClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71e7f8, TEventClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x71f8c0, TEventClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x71f92b, TEventClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71f930, TEventClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x71f94a, TEventClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71faa6, TEventClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6e1780, TActionClass_PlayAudioAtRandomWP_ares)

DEFINE_DISABLE_HOOK(0x73D81E, UnitClass_Mi_Unload_LastPassenger_ares);
DEFINE_DISABLE_HOOK(0x7388EB, UnitClass_ActionOnObject_IvanBombs_ares)//, 0x6, 7388FD)
DEFINE_DISABLE_HOOK(0x73C733, UnitClass_DrawSHP_SkipTurretedShadow_ares)//, 7, 73C7AC)
//DEFINE_DISABLE_HOOK(0x74653C, UnitClass_RemoveGunner_ares)
DEFINE_DISABLE_HOOK(0x73E4A2, UnitClass_Mi_Unload_Storage_ares) //, 0x6)
//{
	// because a value gets pushed to the stack in an inconvenient
	// location, we do our stuff and then mess with the stack so
	// the original functions do nothing any more.
	// GET(BuildingClass* const, pBld, EDI);
	// GET(int, idxTiberium, EBP);
	// REF_STACK(float, amountRaw, 0x1C);
	// REF_STACK(float, amountPurified, 0x34);
//
	// TechnoExt_ExtData::DepositTiberium(pBld,
	//  amountRaw,
	//  BuildingTypeExtData::GetPurifierBonusses(pBld->Owner) * amountRaw,
	//  idxTiberium
	//  );
	//
	//
	//return 0x0;
//}

DEFINE_DISABLE_HOOK(0x522D75, InfantryClass_Slave_UnloadAt_Storage_ares)
DEFINE_DISABLE_HOOK(0x6F7FC5, TechnoClass_CanAutoTargetObject_Heal_ares) //, 7, 6F7FDF)
DEFINE_DISABLE_HOOK(0x73E66D, UnitClass_Mi_Harvest_SkipDock_ares)//, 6, 73E6CF);
DEFINE_DISABLE_HOOK(0x73C143, UnitClass_DrawVXL_Deactivated_ares)

DEFINE_DISABLE_HOOK(0x489235, GetTotalDamage_Verses_ares)

DEFINE_DISABLE_HOOK(0x5F848C, ObjectTypeClass_Load3DArt_NoSpawnAlt2_ares)//, 6, 5F8844)
DEFINE_DISABLE_HOOK(0x763226, WaveClass_DTOR_ares)

DEFINE_DISABLE_HOOK(0x715B1F, TechnoTypeClass_LoadFromINI_Weapons2_ares) //, 6

DEFINE_DISABLE_HOOK(0x6CEE96, SuperWeaponTypeClass_FindIndex_ares)
DEFINE_DISABLE_HOOK(0x46B371, BulletClass_NukeMaker_ares)
DEFINE_DISABLE_HOOK(0x44C9FF, BuildingClass_Mi_Missile_PsiWarn_6_ares)
DEFINE_DISABLE_HOOK(0x46b423, BulletClass_NukeMaker_PropagateSW_ares)
DEFINE_DISABLE_HOOK(0x6CE6F6, SuperWeaponTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x6CEFE0, SuperWeaponTypeClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6CE800, SuperWeaponTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6CE8BE, SuperWeaponTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6CE8EA, SuperWeaponTypeClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6CEE50, SuperWeaponTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x6CEE43, SuperWeaponTypeClass_LoadFromINIB_ares)
DEFINE_DISABLE_HOOK(0x44cb4c, BuildingClass_Mi_Missile_NukeTakeOff_ares)

DEFINE_DISABLE_HOOK(0x41F1A1, AITriggerClass_Chrono_Ready_ares)

DEFINE_DISABLE_HOOK(0x720C42, Theme_PlaySong_DisableStopLog_ares) // skip Theme::Stop
DEFINE_DISABLE_HOOK(0x720DE8, ThemeClass_PlaySong_DisablePlaySongLog_ares)
DEFINE_DISABLE_HOOK(0x720F37, ThemeClass_Stop_DisableStopLog_ares)
DEFINE_DISABLE_HOOK(0x720A61, skip_Theme_AI_ares)


#ifdef SellFunctionHandled
DEFINE_HOOK(0x447113, BuildingClass_Sell_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	// #754 - evict Hospital/Armory contents
	TechnoExt_ExtData::KickOutHospitalArmory(pThis);
	BuildingExtContainer::Instance.Find(pThis)->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}
#else
DEFINE_DISABLE_HOOK(0x447113, BuildingClass_Sell_PrismForward_ares)
#endif

DEFINE_DISABLE_HOOK(0x6F64CB, TechnoClass_DrawHealthBar_FirestormWall_ares) //, 6)

DEFINE_DISABLE_HOOK(0x513EAA, HoverLocomotionClass_UpdateHover_DeployToLand_ares)
DEFINE_DISABLE_HOOK(0x6FCF53, TechnoClass_SetTarget_Burst_ares)//, 0x6, 6FCF61)
// #1415844: units in open-topped transports show behind anim
DEFINE_DISABLE_HOOK(0x6FA2C7, TechnoClass_Update_DrawHidden_ares) //, 0x8)

DEFINE_DISABLE_HOOK(0x6FFF9E, TechnoClass_GetActionOnObject_IvanBombsB_ares)//, 0x5, 700006)
DEFINE_DISABLE_HOOK(0x71AF2B, TemporalClass_Fire_UnwarpableA_ares)//, 0xA, 71AF4D)

DEFINE_DISABLE_HOOK(0x44D8A7, BuildingClass_Mi_Unload_Tunnel_ares)

DEFINE_DISABLE_HOOK(0x760286, WaveClass_Draw_Magnetron3_ares)//, 0x5, 7602D3)


DEFINE_DISABLE_HOOK(0x534a4d, Theater_Init_ResetLogStatus_ares)
