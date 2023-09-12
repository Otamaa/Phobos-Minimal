#pragma once

#include <CRT.h>
#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <Helpers/String.h>

class RawFileClass;
class CCFileClass;
class VocClass;
class AudioEventClassTag;

struct VolumeStruct	//pretty uncreative name, but it's all I can come up with atm
{
	int			Volume;		//Between 0 and 16384, lowest bit is forced to 1, default is 0
	DWORD		unknown_4;	//uninitialized
	int			unknown_int_8;	//Volume * 1024, default is unknown_int_1C * 1024
	int			unknown_int_C;	//default is 1073741
	int			unknown_int_10;	//default is 1000
	int			unknown_int_14;	//default is 0
	DWORD		unknown_18;		//default is ds:87E848h
	int			unknown_int_1C;	//default is 16384
};

struct AudioLevel
{
	int flags;
	unsigned int level;
	int newLevel;
	int change;
	unsigned __int64 duration;
	unsigned __int64 lastTime;
	int field_20;
	int field_24;
};

static_assert(sizeof(AudioLevel) == 0x28, "Invalid Size!");

struct LockTag {
  int count;
  int dbg_struct_type;
};

static_assert(sizeof(LockTag) == 0x8, "Invalid Size!");

struct AudioIDXHeader {
	unsigned int Magic;
	unsigned int Version;
	unsigned int numSamples;
};

template<class T>
struct SomeNodes {
	T* Next;
	T* Prev;
	DWORD Magic;

	void ListNodeInit() {
		JMP_THIS(0x4072D0);
	}

	void ListNodeInsert(SomeNodes<T>* pNode) {
		JMP_THIS(0x407420);
	}
};


struct AudioIDXEntry { // assert (IDXHeader.version != 1);
	FixedString<16> Name;
	int Offset;
	int Size;
	unsigned int SampleRate;
	unsigned int Flags;
	unsigned int ChunkSize;

	//AudioIDXEntry(AudioIDXEntry&& other) noexcept
	//{
	//	std::memcpy(this->Name, other.Name, 16u);
	//	this->Offset = other.Offset;
	//	this->Size = other.Size;
	//	this->SampleRate = other.SampleRate;
	//	this->Flags = other.Flags;
	//	this->ChunkSize = other.ChunkSize;
	//};
	///AudioIDXEntry& operator = (const AudioIDXEntry& src) {
	//	std::memcpy(this, &src, sizeof(*this));
	//	return *this;
	//}

	//AudioIDXEntry& operator=(AudioIDXEntry other) {
	//	std::memcpy(this, &other, sizeof(*this));
	//	return *this;
	//}

	bool operator < (const AudioIDXEntry& rhs) const {
		return CRT::strcmpi(this->Name, rhs.Name) < 0;
	}
};

static constexpr inline size_t AudioIDXEntry_Size = sizeof(AudioIDXEntry);
static_assert(AudioIDXEntry_Size == 0x24, "Invalid Size!");

struct AudioSampleData {
	AudioSampleData() :
		Data(0),
		Format(0),
		SampleRate(0),
		NumChannels(0),
		BytesPerSample(0),
		ByteRate(0),
		BlockAlign(0),
		Flags(0)
	{ }

	unsigned int Data;
	unsigned int Format;
	unsigned int SampleRate;
	unsigned int NumChannels;
	unsigned int BytesPerSample;
	unsigned int ByteRate;
	unsigned int BlockAlign;
	unsigned int Flags;
};

static_assert(sizeof(AudioSampleData) == 0x20, "Invalid Size!");

class AudioIDXData {
public:
	static constexpr reference<AudioIDXData*, 0x87E294u> const Instance{};

	static AudioIDXData* __fastcall Create(const char* pFilename, const char* pPath)
		{ JMP_STD(0x4011C0); };

	AudioIDXData() {
		memset(this, 0, sizeof(*this));
	}

	~AudioIDXData()
		{ JMP_THIS(0x401580); }

	void ClearCurrentSample()
		{ JMP_THIS(0x401910); }

	int __fastcall FindSampleIndex(const char* pName) const
		{ JMP_STD(0x4015C0); }

	const char* __fastcall GetSampleName(int index) const
		{ JMP_STD(0x401600); }

	int __fastcall GetSampleSize(int index) const
		{ JMP_STD(0x401620); }

	AudioSampleData* __fastcall GetSampleInformation(int index, AudioSampleData* pBuffer) const
		{ JMP_STD(0x401640); }

	AudioIDXEntry* Samples;
	int SampleCount;
	char Path[MAX_PATH];
	CCFileClass* BagFile;
	RawFileClass* ExternalFile;
	BOOL PathFound;
	RawFileClass* CurrentSampleFile;
	int CurrentSampleSize;
	DWORD unknown_120;
};
static_assert(sizeof(AudioIDXData) == 0x124, "Invalid Size!");

class Audio {
public:

	static bool __fastcall ReadWAVFile(RawFileClass* pFile, AudioSampleData* pAudioSample, int* pDataSize)
		{ JMP_STD(0x408610); }
};

class AudioStream {
public:
	static constexpr reference<AudioStream*, 0xB1D4D8u> const Instance{};

	bool __fastcall PlayWAV(const char* pFilename, bool bUnk)
		{ JMP_STD(0x407B60); }
};

struct TauntDataStruct {
	DWORD tauntIdx : 4;
	DWORD countryIdx : 4;
};

typedef AudioSampleData AudioFormatTag ;

static_assert(sizeof(AudioFormatTag) == 0x20, "Invalid Size!");

struct AudioAttribs
{
  AudioLevel VolumeLevel;
  AudioLevel PitchLevel;
  AudioLevel PanPosition;
  int field_78;
  int field_7C;
};

static_assert(sizeof(AudioAttribs) == 0x80, "Invalid Size!");

struct AudioSampleTag
{
  char *Data;
  int Bytes;
  SomeNodes<DWORD> Frames;
  AudioFormatTag* Format;
  AudioAttribs* Attribs;
  int field_1C;
};

static_assert(sizeof(AudioSampleTag) == 0x20, "Invalid Size!");

struct MemoryItemTag
{
  MemoryItemTag* next;
  int dbg_struct_type;
};
//static_assert(sizeof(MemoryItemTag) == 0x8, "Invalid Size!");

struct MemoryPoolTag
{
  MemoryItemTag* next;
  int dbg_struct_type;

  static void* __fastcall MemoryPoolGetItem(MemoryPoolTag* tag)
  { JMP_STD(0x407520); }
};
//static_assert(sizeof(MemoryPoolTag) == 0x8, "Invalid Size!");

struct AudioCacheTag
{
  AudioIDXData* idxdata;
  MemoryPoolTag* framePool;
  int itemPool;
  SomeNodes<DWORD> items;
  int frameSize;
  char field_1C;
  char field_1D;
  char field_1E;
  char field_1F;
  char field_20;
  char field_21;
  char field_22;
  char field_23;
  char field_24;
  char field_25;
  char field_26;
  char field_27;
  char field_28;
  char field_29;
  char field_2A;
  char field_2B;
  char field_2C;
  char field_2D;
  char field_2E;
  char field_2F;
  char field_30;
  char field_31;
  char field_32;
  char field_33;
  char field_34;
  char field_35;
  char field_36;
  char field_37;
  char field_38;
  char field_39;
  char field_3A;
  char field_3B;
  char field_3C;
  char field_3D;
  char field_3E;
  char field_3F;
  char field_40;
  char field_41;
  char field_42;
  char field_43;
  char field_44;
  char field_45;
  char field_46;
  char field_47;
  char field_48;
  char field_49;
  char field_4A;
  char field_4B;
  char field_4C;
  char field_4D;
  char field_4E;
  char field_4F;
  char field_50;
  char field_51;
  char field_52;
  char field_53;
  char field_54;
  char field_55;
  char field_56;
  char field_57;
  char field_58;
  char field_59;
  char field_5A;
  char field_5B;
  char field_5C;
  char field_5D;
  char field_5E;
  char field_5F;
  char field_60;
  char field_61;
  char field_62;
  char field_63;
  char field_64;
  char field_65;
  char field_66;
  char field_67;
  char field_68;
  char field_69;
  char field_6A;
  char field_6B;
  char field_6C;
  char field_6D;
  char field_6E;
  char field_6F;
  char field_70;
  char field_71;
  char field_72;
  char field_73;
  char field_74;
  char field_75;
  char field_76;
  char field_77;
  char field_78;
  char field_79;
  char field_7A;
  char field_7B;
  char field_7C;
  char field_7D;
  char field_7E;
  char field_7F;
  char field_80;
  char field_81;
  char field_82;
  char field_83;
  char field_84;
  char field_85;
  char field_86;
  char field_87;
  char field_88;
  char field_89;
  char field_8A;
  char field_8B;
  char field_8C;
  char field_8D;
  char field_8E;
  char field_8F;
  char field_90;
  char field_91;
  char field_92;
  char field_93;
  char field_94;
  char field_95;
  char field_96;
  char field_97;
  char field_98;
  char field_99;
  char field_9A;
  char field_9B;
  char field_9C;
  char field_9D;
  char field_9E;
  char field_9F;
  char field_A0;
  char field_A1;
  char field_A2;
  char field_A3;
  char field_A4;
  char field_A5;
  char field_A6;
  char field_A7;
  char field_A8;
  char field_A9;
  char field_AA;
  char field_AB;
  char field_AC;
  char field_AD;
  char field_AE;
  char field_AF;
  char field_B0;
  char field_B1;
  char field_B2;
  char field_B3;
  char field_B4;
  char field_B5;
  char field_B6;
  char field_B7;
  char field_B8;
  char field_B9;
  char field_BA;
  char field_BB;
  char field_BC;
  char field_BD;
  char field_BE;
  char field_BF;
  char field_C0;
  char field_C1;
  char field_C2;
  char field_C3;
  char field_C4;
  char field_C5;
  char field_C6;
  char field_C7;
  char field_C8;
  char field_C9;
  char field_CA;
  char field_CB;
  char field_CC;
  char field_CD;
  char field_CE;
  char field_CF;
  char field_D0;
  char field_D1;
  char field_D2;
  char field_D3;
  char field_D4;
  char field_D5;
  char field_D6;
  char field_D7;
  char field_D8;
  char field_D9;
  char field_DA;
  char field_DB;
  char field_DC;
  char field_DD;
  char field_DE;
  char field_DF;
  char field_E0;
  char field_E1;
  char field_E2;
  char field_E3;
  char field_E4;
  char field_E5;
  char field_E6;
  char field_E7;
};

static_assert(sizeof(AudioCacheTag) == 0xE8, "Invalid Size!");

struct AudioCacheItem
{
  SomeNodes<DWORD>listnode;
  LockTag locks;
  AudioIDXData* itemindex; // Maybe ?
  int valid;
  AudioSampleTag sampletag;
  AudioCacheTag* cachetag;
  AudioFormatTag format;
  int dbg_struct_type;
};

static_assert(sizeof(AudioCacheItem) == 0x64, "Invalid Size!");

struct AudioEventHandleTag;
class AudioChannelTag; // Big boi
class AudioEventTag
{
public:

	void AudioEventAdjustVolume(int amount)
	{ JMP_THIS(0x4061D0); }

	void AudioEventAdjustPan(Panning adjustment)
	{ JMP_THIS(0x406270); }

	AudioEventClassTag* AudioEventGetClass()
	{ JMP_THIS(0x406310); }

  int nodemaybe_0;
  char field_4;
  char field_5;
  char field_6;
  char field_7;
  int field_8;
  SomeNodes<DWORD> listnode;
  int bitfield;
  DWORD sometype1;
  int sometype2;
  AudioEventClassTag* evclass;
  AudioCacheItem* caches[32];
  int cachecount;
  int cacheentry;
  AudioChannelTag* channeltag;
  int field_B4;
  AudioLevel volume;
  AudioLevel audiolevel2;
  AudioLevel pan;
  char field_130;
  char field_131;
  char field_132;
  char field_133;
  int field_134;
  int stamp;
  int field_13C;
  int field_140;
  int field_144;
  AudioSampleTag* sampletag;
  int PitchShift;
  int randlevelval2;
  int priority_154;
  DWORD pauses;
  char field_15C;
  char field_15D;
  char field_15E;
  char field_15F;
  int randvals1_160[32];
  int randvals1counter;
  int loops_done;
  int randvals2_1E8[32];
  int randvals_268;
  int randvals2counter;
  int randvals_270;
  int randval_274;
  AudioEventHandleTag* eventhandletag;
  int field_27C;
};

static_assert(sizeof(AudioEventTag) == 0x280, "Invalid Size!");

struct AudioController
{
	void* f_0;
	DWORD f_4;
	VocClass* Voice;
	AudioIDXData** AudioIndex;
	DWORD f_10;

	AudioController() :
		f_0(nullptr),
		f_4(0),
		Voice(nullptr),
		AudioIndex(&AudioIDXData::Instance),
		f_10(0)
	{
	}

	void AudioEventHandleDeInit()
	{ JMP_THIS(0x405C00); }

	void AudioEventHandleStop() // DTOR_1
	{ JMP_THIS(0x405D40); }

	void AudioEventHandleEnd() // DTOR_2
	{ JMP_THIS(0x405FD0); }

	void ShutUp()
	{ JMP_THIS(0x406060); }

	void AudioEventHandleEndLooping()
	{ JMP_THIS(0x406060); }

	void __fastcall sub_4060F0(int a1, int a2)
	{ JMP_STD(0x4060F0); }

	void sub_406130()
	{ JMP_THIS(0x406130); }

	void sub_406170()
	{ JMP_THIS(0x406170); }

	void __fastcall sub_4061D0(unsigned int arg)
	{ JMP_STD(0x4061D0); }

	void __fastcall sub_406270(unsigned int arg)
	{ JMP_THIS(0x406270); }

	void sub_406310()
	{ JMP_THIS(0x406310); }
};

struct AudioEventHandleTag
{
	AudioEventTag* event;
	DWORD stamp;
	AudioEventClassTag* Voice;
	AudioIDXData** AudioIndex;
	DWORD f_10;

	//AudioEventHandleTag() noexcept :
	//	event(nullptr),
	//	stamp(0),
	//	Voice(nullptr),
	//	AudioIndex(&AudioIDXData::Instance()),
	//	f_10(0)
	//{  }

	void AudioEventHandleDeInit()
		{ JMP_THIS(0x405C00); }

	void AudioEventHandleStop()
		{ JMP_THIS(0x405D40); }

	void AudioEventHandleEnd()
		{ JMP_THIS(0x405FD0); }

	void ShutUp()
		{ JMP_THIS(0x406060); }

	void AudioEventHandleEndLooping()
		{ JMP_THIS(0x406060); }

	void SetHandle(AudioEventTag* pAudioEventTag = nullptr, AudioEventClassTag* pAudioEventClassTag = nullptr)
		{ JMP_THIS(0x4060F0); }

	AudioEventTag* AudioEventHandleGet()
		{ JMP_THIS(0x406130); }

	AudioEventClassTag* AudioEventHandleGetClass()
		{ JMP_THIS(0x406170); }

//protected :
//	~AudioEventHandleTag() = delete;
};

static_assert(sizeof(AudioEventHandleTag) == 0x14, "Invalid Size!");