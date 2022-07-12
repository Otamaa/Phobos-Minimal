#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#ifdef fuck
#include <Ext/Rules/Body.h>

static	void __fastcall _Log_LocalMix(const char* pFormat, ...)
{
	Debug::Log("LOCAL.MIX\n");
}

DEFINE_JUMP(CALL,0x530439, GET_OFFSET(_Log_LocalMix));

static	void __fastcall _Log_CacheMix(const char* pFormat, ...)
{
	Debug::Log("CACHE.MIX\n");
}

DEFINE_JUMP(CALL,0x5303B5, GET_OFFSET(_Log_CacheMix));

static void* __cdecl _YR_Allocate_Localmix(size_t size)
{
	Debug::Log("LOCALMD.MIX\n");
	return YRMemory::Allocate(size);
}

DEFINE_JUMP(CALL,0x5303E8, GET_OFFSET(_YR_Allocate_Localmix));

static	void __fastcall _Log_CacheMdMix(const char* pFormat, ...)
{
	Debug::Log("CACHEMD.MIX\n");
}

DEFINE_JUMP(CALL,0x530349,GET_OFFSET(_Log_CacheMdMix));

static	void __fastcall _Log_Ra2Mix(const char* pFormat, ...)
{
	Debug::Log("RA2.MIX\n");
}

DEFINE_JUMP(CALL,0x530307, GET_OFFSET(_Log_Ra2Mix));

static void* __cdecl _YR_Allocate_Ra2mix(size_t size)
{
	Debug::Log("RA2MD.MIX\n");
	return YRMemory::Allocate(size);
}

DEFINE_JUMP(CALL,0x5302BA, GET_OFFSET(_YR_Allocate_Ra2mix));

namespace AnnoyingAudioLogSutffs
{
	bool Disable = true;
	bool bDisableNotTiberiumLog = true;
	bool bDisableNoDigestLog = true;
}

//40A55D
static	void __fastcall _Log_soundFrameOrBufferSize(const char* pFormat, ...)
{
	if (!AnnoyingAudioLogSutffs::Disable)
		Debug::Log(pFormat);
}

DEFINE_JUMP(CALL,0x40A55D, GET_OFFSET(_Log_soundFrameOrBufferSize));
DEFINE_JUMP(CALL,0x40A5BC, GET_OFFSET(_Log_soundFrameOrBufferSize));


static	void __fastcall _Log_PathFailere_1(const char* pFormat, ...)
{
	if (!RulesExt::Global()->DisablePathfindFailureLog.Get())
		Debug::Log(pFormat);
}

DEFINE_JUMP(CALL,0x42CBDE, GET_OFFSET(_Log_PathFailere_1));

static	void __fastcall _Log_PathFailere_2(const char* pFormat, ...)
{
	if (!RulesExt::Global()->DisablePathfindFailureLog.Get())
		Debug::Log(pFormat);
}

DEFINE_JUMP(CALL,0x42CC65, GET_OFFSET(_Log_PathFailere_2));

DEFINE_HOOK(0x530277, MixFile_BoostTrap_FixLog, 0x6)
{
	LEA_STACK(char*, pFilename, STACK_OFFS(0x78, 0x40));

	Debug::Log("%s Loaded! \n", pFilename);

	return 0x530289;
}

static	void __fastcall _Log_NotTib(const char* pFormat, ...)
{
	if (!AnnoyingAudioLogSutffs::bDisableNotTiberiumLog)
		Debug::Log(pFormat);
}

static	void __fastcall _Log_NoDigest(const char* pFormat, ...)
{
	if (!AnnoyingAudioLogSutffs::bDisableNoDigestLog)
		Debug::Log(pFormat);
}
DEFINE_JUMP(CALL,0x5FDDB9, GET_OFFSET(_Log_NotTib));
DEFINE_JUMP(CALL,0x69A79D, GET_OFFSET(_Log_NoDigest));

//To prevent Compiler Optimization
static bool Dummy = false;
static	void __fastcall _Log_Disable_These(const char* pFormat, ...) {
	if(!Dummy)
		Dummy = true;
}

DEFINE_JUMP(CALL,0x40A55D, GET_OFFSET(_Log_Disable_These));
DEFINE_JUMP(CALL,0x40A5BC, GET_OFFSET(_Log_Disable_These));
DEFINE_JUMP(CALL,0x69A79D, GET_OFFSET(_Log_Disable_These));
//DEFINE_JUMP(CALL,0x42CC65, GET_OFFSET(_Log_Disable_These));
//DEFINE_JUMP(CALL,0x42CBDE, GET_OFFSET(_Log_Disable_These));
#endif