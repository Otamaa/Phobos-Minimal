template<DWORD addr, DWORD addr_ptr>
struct MixBundle
{
	constant_ptr<const char, addr> MIXName;
	reference<MixFileClass*, addr_ptr> const MIXptr;
};

static COMPILETIMEEVAL MixBundle<0x826838, 0x884E38 > const CONQMD {};
static COMPILETIMEEVAL MixBundle<0x8267EC, 0x884E3C > const CONQUER {};

static COMPILETIMEEVAL MixBundle<0x826820, 0x884E18 > const GENERMD {};
static COMPILETIMEEVAL MixBundle<0x826814, 0x884E14 > const GENERIC {};

static COMPILETIMEEVAL MixBundle<0x826804, 0x884E28 > const ISOGENMD {};
static COMPILETIMEEVAL MixBundle<0x8267F8, 0x884E24 > const ISOGEN {};

static COMPILETIMEEVAL MixBundle<0x8267D0, 0x884E40 > const CAMEOMD {};
static COMPILETIMEEVAL MixBundle<0x8267B4, 0x884E44 > const CAMEO {};

static COMPILETIMEEVAL MixBundle<0x81C284, 0x884DD8 > const MULTIMD {};

static COMPILETIMEEVAL MixBundle<0x81C24C, 0x87E738 > const THEMEMD {};
static COMPILETIMEEVAL MixBundle<0x81C220, 0x87E738 > const THEME {};

static COMPILETIMEEVAL reference<MixFileClass*, 0x884E64> const MapsMix {};
static COMPILETIMEEVAL reference < MixFileClass*, 0x884E60> const MapsMDMix {};

template<typename TMixBundle>
OPTIONALINLINE void LoadMixFile(TMixBundle& bb)
{
	CCFileClass _file { bb.MIXName() };
	if (_file.Exists())
	{
		bb.MIXptr = GameCreate<MixFileClass>(bb.MIXName());
	}
	Debug::LogInfo(" Loading {} ... {} !!!", bb.MIXName(), !bb.MIXptr ? "FAILED" : "OK");
}

ASMJIT_PATCH(0x53046A, Game_InitSecondaryMix_handle, 0x5)
{
	Debug::LogInfo(" ");
	LoadMixFile(CONQMD);
	LoadMixFile(GENERMD);
	LoadMixFile(GENERIC);
	LoadMixFile(ISOGENMD);
	LoadMixFile(ISOGEN);
	LoadMixFile(CONQUER);
	LoadMixFile(CAMEOMD);
	LoadMixFile(CAMEO);

	int cd = Game::Get_Volume_Index(60);
	if (cd < 0) {
		cd = 0;
	}

	cd += 1;
	char buffer[260];

	if (CD::IsLocal())
	{
		std::snprintf(buffer, sizeof(buffer), "MAPSMD*.MIX");
		if (Game::File_Finder_Start(buffer)) {
			MapsMDMix = GameCreate<MixFileClass>(buffer);

			while (Game::File_Finder_Next_Name(buffer))
			{
				if (auto pMix = GameCreate<MixFileClass>(buffer)) {
					MixFileClass::Maps->AddItem(pMix);
				}
			}

			Game::File_Finder_End();
		}
		else
		{
			std::snprintf(buffer, sizeof(buffer), "MAPS*.MIX");
			if (Game::File_Finder_Start(buffer))
			{
				MapsMix = GameCreate<MixFileClass>(buffer);

				while (Game::File_Finder_Next_Name(buffer))
				{
					if (auto pMix = GameCreate<MixFileClass>(buffer))
					{
						MixFileClass::Maps->AddItem(pMix);
					}
				}

				Game::File_Finder_End();
			}
		}
	}
	else
	{
		std::snprintf(buffer, sizeof(buffer), "MAPSMD%02d.MIX", cd);
		CCFileClass MAPSMD_file { buffer };

		if (MAPSMD_file.Exists())
		{
			MapsMDMix = GameCreate<MixFileClass>(buffer);
		}

		std::snprintf(buffer, sizeof(buffer), "MAPS%02d.MIX", cd);
		CCFileClass MAPS_file { buffer };

		if (MAPS_file.Exists())
		{
			MapsMix = GameCreate<MixFileClass>(buffer);
		}
	}

	LoadMixFile(MULTIMD);
	LoadMixFile(THEMEMD);
	LoadMixFile(THEME);

	R->ESI(cd);
	return 0x530CF4;
}
