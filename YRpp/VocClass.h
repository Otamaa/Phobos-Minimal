/*
	Sound effects!
*/

#pragma once

#include <ArrayClasses.h>
#include <CoordStruct.h>
#include <Audio.h>

struct VocAudioStruct {
  SomeNodes<DWORD> listnode;
  AudioEventHandleTag eventhandletag;
  CoordStruct position;
  int somebool;
  DWORD pauses;
  int field_34;

  VocAudioStruct() = delete;
  ~VocAudioStruct() = delete;
};

static_assert(sizeof(VocAudioStruct) == 0x38, "Invalid Size!");

class AudioEventClassTag {
public:
	SomeNodes<DWORD> Header;
	int SamplesOK;	//0 or 1, determines whether all samples are OK to use
	SoundControl Control;
	SoundType Type;
	VolumeStruct Volume;
	DWORD unknown_38;
	DWORD unknown_3C;
	SoundPriority Priority;
	DWORD unknown_44;
	int Limit;	//as in sound.ini
	int Loop;	//as in sound.ini
	int Range;	//as in sound.ini
	float MinVolume;	//as in sound.ini
	int MinDelay;	//as in sound.ini
	int MaxDelay;	//as in sound.ini
	int MinFDelta;	//as in sound.ini
	int MaxFDelta;	//as in sound.ini
	int VShift;	//as in sound.ini
	char Name [0x20]; //as in sound.ini
	DWORD unknown_8C;
	DWORD unknown_90;
	DWORD unknown_94;
	DWORD unknown_98;
	DWORD unknown_9C;
	DWORD unknown_A0;
	DWORD unknown_A4;
	DWORD unknown_A8;
	DWORD unknown_AC;
	DWORD unknown_B0;

	int SampleIndex [0x20];

	int NumSamples;
	int Attack;
	int Decay;
	DWORD unknown_140;
	DWORD unknown_144;
};

static_assert(sizeof(AudioEventClassTag) == 0x148 , "Invalid Size !");

class AudioEventTag;
class VocClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<VocClass*>, 0xB1D378u> const Array{};

	static constexpr reference<bool, 0x8464ACu> const VoicesEnabled{};

	static NOINLINE VocClass* __fastcall Find(const char* pName)
	{
		for(int i = 0; i < Array->Count; ++i) {
			if(!CRT::strcmpi(Array->Items[i]->EventClassTag->Name, pName)) {
				return Array->Items[i];
			}
		}
		return nullptr;
	}

	static int __fastcall FindIndexById(const char *pName)
		{ JMP_STD(0x7514D0); }

	/* Play a sound independant of the position.
	   n = Index of VocClass in Array to be played
	   Volume = 0.0f to 1.0f
	   Panning = 0x0000 (left) to 0x4000 (right) (0x2000 is center)
	   */
	//static AudioEventTag* __fastcall PlayGlobal(int n, Panning Panning, float Volume, AudioEventHandleTag* pCtrl = nullptr)
	//	{ JMP_STD(0x750920); }

	/* Play a sound at a certain Position.
       n = Index of VocClass in Array to be played */
	//static void __fastcall PlayAt(int n, const CoordStruct &coords, AudioEventHandleTag* pCtrl = nullptr)
	//	{ JMP_STD(0x7509E0); }

	static DWORD __fastcall PlayGlobal(int n, Panning Panning, float Volume, AudioController* pCtrl = nullptr)
		{ JMP_STD(0x750920); }

	/* Play a sound at a certain Position.
       n = Index of VocClass in Array to be played */
	static void __fastcall PlayAt(int n, const CoordStruct &coords, AudioController* pCtrl = nullptr)
		{ JMP_STD(0x7509E0); }

	static void __fastcall PlayAt(int n, const CoordStruct* coords, AudioController* pCtrl)
		{ JMP_STD(0x7509E0); }

	// calls `PlayAt` - with sanity check
	static VocAudioStruct* __fastcall PlayIndexAtPos(int n, const CoordStruct& coords, int a3 = 0)
		{ JMP_STD(0x750E20); }

	static VocAudioStruct* __fastcall PlayIndexAtPos(int n, const CoordStruct& coords,  AudioController* pCtrl)
		{ JMP_STD(0x750E20); }

	static VocAudioStruct* __fastcall PlayIndexAtPos(int n, const CoordStruct* coords, AudioController* pCtrl)
		{ JMP_STD(0x750E20); }
	//Properties

public:
	AudioEventClassTag* EventClassTag;

	//constructor and destructor should never be needed
	VocClass() = delete;
	~VocClass() = delete;
};

static_assert(sizeof(VocClass) == 0x4, "Invalid Size !");