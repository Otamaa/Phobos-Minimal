// list of hooks that not yet backported with some explanation with 

// sub_48D1E0_PlayTaunt , idk what the f is happening there atm
// hWnd_UpdatePlayerColors , dont really care
// hWnd_SetPlayerColor , dont really care
// hWnd_PopulateWithCountryNames , dont really care
// hWnd_PopulateWithColors , dont really care
// hWnd_IsAvailableColor , dont really care
// hWnd_GetSlotColorIndex , dont really care

// VoxClass_SetEVAIndex , later maybe
// VoxClass_PlayEVASideSpecific , later maybe
// VoxClass_LoadFromINI , later maybe
// VoxClass_GetFilename
// VoxClass_DeleteAll
// VoxClass_CreateFromINIList
// VocClassData_AddSample

//TODO : 
// better port these
// DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)

/* TODO : Addition Weapon shenanegans , need to port whole TechnoClass::Update
DEFINE_HOOK(717890, TechnoTypeClass_SetWeaponTurretIndex, 8)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nTurIdx, 0x4);
	GET_STACK(int, nWeaponIdx, 0x8);

	if (nWeaponIdx < TechnoTypeClass::MaxWeapons)
	{
		pThis->TurretWeapon[nWeaponIdx] = nTurIdx;
	}
	else
	{
		auto const pExt = TechnoTypeExt::ExtMap.Find(pThis);
		pExt->AdditionalTurrentWeapon[nWeaponIdx - TechnoTypeClass::MaxWeapons] = nTurIdx;
	}

	return 0x71789F;
}
*/

//TacticalClass_Draw_TimerVisibility  , the default value sometime initialized at NewSuper init function , and not readed from ini
	// if i want to replace this , i need to figure out the current NewSuper index doing 
	// it is mostlikely change since there is new super were added !

// DEFINE_HOOK(6D4E79, TacticalClass_DrawOverlay_GraphicalText, 6) , hmm not now

// [Ares.dll - 80][0x420960 = AlphaShapeClass_CTOR , 5] 
// [Ares.dll - 89][0x421730 = AlphaShapeClass_SDDTOR , 8]
// [Ares.dll - 103][0x423b0b = AnimClass_Update_AlphaLight , 6]

// [Ares.dll - 147] [0x435820 = BuildingLightClass_CTOR, 6]
// [Ares.dll - 148] [0x435bfa = BuildingLightClass_Draw_Start, 6]
// [Ares.dll - 149] [0x435cd3 = BuildingLightClass_Draw_Spotlight, 6]
// [Ares.dll - 150] [0x436072 = BuildingLightClass_Draw_430, 6]
// [Ares.dll - 151] [0x4360d8 = BuildingLightClass_Draw_400, 6]
// [Ares.dll - 152] [0x4360ff = BuildingLightClass_Draw_250, 6]
// [Ares.dll - 153] [0x436459 = BuildingLightClass_Update, 6]
// [Ares.dll - 156] [0x4370c0 = BuildingLightClass_SDDTOR, 10]