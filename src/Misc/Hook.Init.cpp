#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <ParticleClass.h>

ASMJIT_PATCH(0x52BA78, _YR_GameInit_Pre, 5) {
	TheaterTypeClass::Array.clear();
	TheaterTypeClass::AddDefaults();
	TheaterTypeClass::LoadAllTheatersToArray();
	return 0;
}

void __fastcall GameInit()
{
	TheaterTypeClass::Array.clear();
	TheaterTypeClass::AddDefaults();
	TheaterTypeClass::LoadAllTheatersToArray();

	ScenarioClass::Instance = GameCreate<ScenarioClass>();
	RulesClass::Instance = GameCreate<RulesClass>();

	//TODO:
	/*
	if ( !Is16BitColor ){
        PaletteClass::Set(&BlackPalette, 0, 0);
    }

	if ( CD::CD_Files_Local ){
        CD::Set_Required_CD(-2);
    }

	do{
        WWKeyboardClass::Check(Keyboard);
    }
    while ( !GameInFocus );
    SurfacesRestored = 0;

	BitText::Instance = GameCreate<BitText>();
	Load_UI_Shape_Files();
    Audio_Init(SoundOn);
	WWKeyboardClass::Clear(Keyboard);

	/./......
	*/
}
