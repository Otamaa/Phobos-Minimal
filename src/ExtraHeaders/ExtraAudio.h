#pragma once
#include <Base/Always.h>
#include <Audio.h>
#include <CCFileClass.h>

#include <DSound.h>

static constexpr reference<IDirectSound, 0x87E89Cu> const AUD_sound_object {};

struct AudioFormatTag
{
	int Flags;
	int Compression;
	int Rate;
	int Channels;
	int SampleWidth;
	int BytesPerSecond;
	int BlockSize;
	int framedatabuffer;
};

struct AudioLevel
{
	int flags;
	unsigned int level;
	int newLevel;
	int change;
	unsigned long long duration;
	unsigned long long lastTime;
	int field_20;
	int field_24;
};

struct AudioAttribs
{
	AudioLevel VolumeLevel;
	AudioLevel PitchLevel;
	AudioLevel PanPosition;
	int field_78;
	int field_7C;
};

struct _ListNode
{
	_ListNode* next;
	_ListNode* prev;
	int pri;
};


struct AudioSampleTag
{
	char* Data;
	int Bytes;
	_ListNode Frames;
	AudioFormatTag* Format;
	AudioAttribs* Attribs;
	int field_1C;
};

struct SBufferDataStruct
{
	int databuffer;
	int bytes_left;
};

struct __declspec(align(4)) STM_SBUFFER;
struct __declspec(align(4)) _stm_stream;
struct __declspec(align(4)) _stm_access
{
	_stm_stream* stream;
	char* databuffer;
	int datasize;
	STM_SBUFFER* sbufferptr1;
	int bitfield;
	int id;
	int mode;
	DWORD refcount;
	int field_20;
	int lasterror;
	STM_SBUFFER* sbufferptr2;
	int somepos;
	int position;
	int bytes1;
	int bytes2;
	int field_3C;
};

struct __declspec(align(4)) _stm_stream
{
	int streamcount;
	_ListNode buffernodes;
	int flags;
	int totalbytes;
	_stm_access accessors[2];
	DWORD refcount;
	int field_9C;
	int field_A0;
};


struct __declspec(align(4)) STM_SBUFFER
{
	_ListNode buffernode;
	int field_C;
	DWORD buffer;
	DWORD dword14;
	DWORD somedata;
	DWORD somebytes;
	SBufferDataStruct datastructs[2];
	_stm_stream* streampointer;
	int counter_34;
	int field_38;
};

struct LockTag
{
	int count;
	int dbg_struct_type;
};

struct MemoryItemTag
{
	MemoryItemTag* next;
	int dbg_struct_type;
};

struct MemoryPoolTag
{
	MemoryItemTag* next;
	int dbg_struct_type;
};

struct __declspec(align(4)) AudioCacheTag
{
	AudioIDXData* idxdata;
	MemoryPoolTag* framePool;
	int itemPool;
	_ListNode items;
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

struct AudioChannelTag;
struct AudioDeviceTag;
struct AudioStreamerTag
{
	_ListNode listnode;
	int bitfield;
	AudioDeviceTag* _AudioDeviceTag;
	AudioChannelTag* _AudioChannelTag;
	DWORD valforAudioFormatBytes;
	DWORD timing1C;
	long long endtimestamp;
	long long endtimestamp2;
	_stm_stream* stream;
	_stm_access* streamaccess1;
	_stm_access* streamaccess2;
	AudioSampleTag sample;
	AudioFormatTag format;
	FileClass* fileclass;
	DWORD pauses;
	int field_84;
	DWORD locks;
	float gap8C;
	int samplebytes;
	int field_94;
	int field_98;
	int samplebytes2;
	int formatbytes;
	char streamname[80];
	char field_F4;
	char field_F5;
	char field_F6;
	char field_F7;
};

struct AudioServiceInfoTag
{
	long long serviceInterval;
	long long mustServiceInterval;
	long long lastInterval;
	long long longestInterval;
	long long longestReset;
	long long lastServiceTime;
	long long longestIntervalforPeriod;
	long long periodStart;
	long long periodInterval;
	DWORD count;
	DWORD missCount;
	DWORD lastcount;
	DWORD animPos;
	int field_58;
	int field_5C;
};

struct AudioSystemTag;
struct AudioDriverTag;
struct AudioDeviceTag
{
	_ListNode nd;
	int flags;
	LockTag lock;
	AudioSystemTag* _AudioSystemTag;
	int systemUnit;
	int stamp;
	AudioFormatTag Format;
	AudioFormatTag DefaultFormat;
	int field_64;
	AudioAttribs Attribs;
	AudioAttribs* GroupAttribs;
	int Unit;
	int channels;
	int maxChannels;
	_ListNode channelList;
	LockTag channelAccess;
	int(__fastcall* deviceHandler)(struct AudioDeviceTag*);
	AudioServiceInfoTag attribsUpdate;
	AudioServiceInfoTag mixerUpdate;
	AudioDriverTag* _AudioDriverTag;
	int data;
	int frames;
	int over_sample;
	int frame_lag;
	int dbg_struct_type;
};

struct AudioControlTag
{
	int Priority;
	int Status;
	int LoopCount;
};

struct AudioEventTag;
struct AudioFrameTag;
struct AudioDriverChannelTag;
struct AudioChannelTag
{

	static int __fastcall AudioChannelSetFormat(AudioChannelTag* chan, AudioFormatTag* format) {
		JMP_STD(0x402800);
	}

	int AudioDriver_update() {
		JMP_THIS(0x40A6D0);
	}

	_ListNode nd;
	int Type;
	AudioAttribs ChannelAttribs;
	AudioAttribs* SfxAttribs;
	AudioAttribs* GroupAttribs;
	AudioAttribs* CompAttribs;
	AudioAttribs* FadeAttribs;
	AudioControlTag Control;
	AudioDeviceTag* Device;
	int(__fastcall* CB_NextFrame)(struct AudioChannelTag*);
	int(__fastcall* CB_NextSample)(struct AudioChannelTag*);
	int(__fastcall* CB_SampleDone)(struct AudioChannelTag*);
	int(__fastcall* CB_Stop)(struct AudioChannelTag*);
	AudioEventTag* _AudioEventTag;
	int UserField[4];
	AudioDriverTag* _AudioDriverTag;
	AudioAttribs attribs;
	AudioDriverChannelTag* drvData;
	int drvCBNextFrame;
	int drvCBNextSample;
	int drvCBSampleDone;
	AudioSampleTag* sample;
	AudioFrameTag* frame;
	int frameData;
	int bytesInFrame;
	int bytesRemaining;
	AudioFormatTag current_format;
	int format_changed;
	int drv_format_changed;
	long long time_min_frame;
	double time_buffer;
	int field_1B4;
	int field_1B8;
	int field_1BC;
};

struct AudioSystemMasterTag;
struct __declspec(align(4)) AudioSystemTag
{
	_ListNode nd;
	AudioSystemMasterTag* master;
	LockTag lock;
	int numUnits;
	AudioDeviceTag* unit[16];
	int dbg_struct_type;
};

struct AudioDriverTag
{
	int data;
	int(__fastcall* openChannel)(AudioChannelTag*);
	int(__fastcall* closeChannel)(AudioChannelTag*);
	int(__fastcall* start)(AudioChannelTag*);
	int(__fastcall* stop)(AudioChannelTag*);
	int(__fastcall* pause)(AudioChannelTag*);
	int(__fastcall* resume)(AudioChannelTag*);
	int(__fastcall* check)(AudioChannelTag*);
	int(__fastcall* update)(AudioChannelTag*);
	int(__fastcall* queueIt)(AudioChannelTag*);
	int(__fastcall* lock)(AudioChannelTag*);
	int(__fastcall* unlock)(AudioChannelTag*);
};


struct AudioEventClassTag
{
	float gap0;
	int field_4;
	int field_8;
	int valid;
	int Control;
	int TypeFlags;
	AudioLevel audiolevel;
	int Priority;
	int somecount;
	int Limit;
	int LoopCount;
	int Range;
	float MinVolume;
	int MinDelay;
	int MaxDelay;
	int MinFrequenceDelta;
	int MaxFrequencyDelta;
	int VolumeShift;
	char Name[32];
	_ListNode listnode;
	int someid;
	int field_9C;
	int priority_A0;
	_ListNode listnode_A4;
	int volumelevle_B0;
	int Sounds[32];
	int SoundCount;
	int AttackCount;
	int DecayCount;
	int field_140;
	int field_144;
};

struct __declspec(align(4)) AudioCacheItem
{
	_ListNode listnode;
	LockTag locks;
	void* itemindex;
	int valid;
	AudioSampleTag sampletag;
	AudioCacheTag* cachetag;
	AudioFormatTag format;
	int dbg_struct_type;
};

struct AudioEventHandleTag;
struct AudioEventTag
{
	int nodemaybe_0;
	char field_4;
	char field_5;
	char field_6;
	char field_7;
	int field_8;
	_ListNode listnode;
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

struct __declspec(align(4)) AudioEventHandleTag
{
	AudioEventTag* _AudioEventTag;
	DWORD stamp;
	AudioEventClassTag* eclass;
	DWORD id;
	int field_10;
};

struct __declspec(align(4)) imastruct
{
	DWORD Predicted;
	DWORD Index;
};

struct AudioDriverChannelTag
{

	AudioSampleTag*  AudioDriverChannelTag_noname_setup() {
		JMP_THIS(0x409C40);
	}

	AudioChannelTag* audiochannel;
	int field_4;
	int soundframesize2;
	int soundframesizedivplaycursor;
	LARGE_INTEGER audiogettimeresult;
	LARGE_INTEGER audiogettimeresult2;
	int frameData1;
	int bytesInFrame1;
	int frameData2;
	int bytesInFrame2;
	DWORD playcursor;
	DWORD writecursor;
	int buffersizeminusplaycursor;
	int soundframesizetimes4;
	int buffersize2;
	AudioFormatTag audioformat;
	IDirectSoundBuffer* soundriverpointer;
	int dwBufferBytes;
	int soundframesize1;
	int decompression_func;
	int decomptype;
	int pendingdecomptype;
	char* IMA_InBuffer;
	int blocksize5;
	int blocksize6;
	DWORD somebuffer2;
	int blocksize4;
	WORD* IMA_OutBuffer;
	int IMA_BitsProcessed;
	DWORD somedecompcount;
	int blocksize3;
	int blocksizetimes4_plus128;
	int Channels;
	int decompwas0;
	int blocksize1;
	int ima_function;
	imastruct imastruct[2];
	int field_C4;
	int field_C8;
	int skip_drvCBNextFrame;
	int somecount;
	int soundframesize3;
	int soundframesizedivplaycursor2;
	int playcursor2;
};

static int __fastcall AudioDriverChannelTag_noname_prefil(AudioDriverChannelTag* a1, int a2) {
	JMP_STD(0x409880);
}

struct __declspec(align(4)) AudioFrameTag
{
	_ListNode nd;
	DWORD Data;
	DWORD Bytes;
	AudioSampleTag* _AudioSampleTag;
	int field_18;
};

struct __declspec(align(4)) AudioSystemMasterTag
{
	char* Name;
	int Id;
	int Properties;
	int flags;
	int stamp;
	signed int(__fastcall* load)(AudioSystemTag* AudioSystemTag);
	void(__fastcall* unload)(AudioSystemTag* AudioSystemTag);
	int(__fastcall* open)(AudioDeviceTag* AudioDeviceTag);
	void(__fastcall* close)(AudioDeviceTag* AudioDeviceTag);
	AudioDriverTag* _AudioDriverTag;
};
