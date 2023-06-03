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