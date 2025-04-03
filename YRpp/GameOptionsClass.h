#pragma once

#include <Helpers/CompileTime.h>
#include <ASMMacros.h>

class GameOptionsClass
{
public:
	static COMPILETIMEEVAL reference<GameOptionsClass, 0xA8EB60u> const Instance{};
	static COMPILETIMEEVAL reference<bool, 0x89F978u> const WindowedMode {};

	int GetAnimSpeed(int rate)
		{ JMP_THIS(0x5FB2E0); }

	//RA2MD.ini
	void LoadFromINIFile() const
		{ JMP_THIS(0x5FA620); }

	//RA2MD.ini
	void SaveToINIFile() const
		{ JMP_THIS(0x5FAD10); }

	void Set() const
		{ JMP_THIS(0x5FB160); }

	void Set(bool repeat) const
		{ JMP_THIS(0x5FA470); }

	void SetScoreVolume(float volume, bool feedback) const
	 	{ JMP_THIS(0x5FA4A0); }

	void SetShuffle(bool shuffle) const
		{ JMP_THIS(0x5FA440); }

	void SetSoundVolume(float volume, bool feedback) const
		{ JMP_THIS(0x5FA510); }

	void SetVoiceVolume(float volume, bool feedback) const
		{ JMP_THIS(0x5FA590); }

public:
	int GameSpeed;
	int Difficulty;
	int CampDifficulty;
	int ScrollMethod;
	int ScrollRate;
	bool AutoScroll;
	int DetailLevel;
	bool SidebarMode; // True -> right False -> left
	bool SidebarCameoText;
	bool UnitActionLines;
	bool ShowHidden;
	bool Tooltips;
	int ScreenWidth;
	int ScreenHeight;
	int ShellWidth;
	int ShellHeight;
	bool StretchMovies;
	bool AllowHiResModes;
	bool AllowVRAMSidebar;
	float SoundVolume;
	float VoiceVolume;
	float MovieVolume;
	bool IsScoreRepeat;
	bool InGameMusic;
	bool IsScoreShuffle;
	short SoundLatency;
	short Socket;
	int LastUnlockedSovMovie;
	int LastUnlockedAllMovie;
	int NetCard;
  	char DestNet[64];

	// virtual key constants, each of them doubled
	// defaulting to VK_MENU, VK_CONTROL and VK_SHIFT

	int KeyForceMove1; //98
	int KeyForceMove2; //9C
	int KeyForceFire1;  //A0
	int KeyForceFire2;  //A4
	int KeyForceSelect1; //A8
	int KeyForceSelect2; //AC

  	int KeyQueueMove1; //B0
  	int KeyQueueMove2; //B4

};
static_assert(offsetof(GameOptionsClass, KeyForceMove1) == 0x98 , "Class Member Shifted !");
typedef GameOptionsClass OptionsClass;
