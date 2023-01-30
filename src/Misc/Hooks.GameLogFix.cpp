#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <ExtraHeaders/ExtraAudio.h>
#ifndef fuck
#include <Ext/Rules/Body.h>

static	void __fastcall _Log_LocalMix(const char* pFormat, ...)
{
	Debug::Log("%s \n", pFormat);
}

DEFINE_JUMP(CALL,0x530439, GET_OFFSET(_Log_LocalMix));

static	void __fastcall _Log_CacheMix(const char* pFormat, ...)
{
	Debug::Log("%s \n", pFormat);
}

DEFINE_JUMP(CALL,0x5303B5, GET_OFFSET(_Log_CacheMix));

static void* __cdecl _YR_Allocate_Localmix(size_t size)
{
	Debug::Log("%s \n" , GameStrings::LOCALMD_MIX());
	return YRMemory::Allocate(size);
}

DEFINE_JUMP(CALL,0x5303E8, GET_OFFSET(_YR_Allocate_Localmix));

static	void __fastcall _Log_CacheMdMix(const char* pFormat, ...)
{
	Debug::Log("%s\n", pFormat);
}

DEFINE_JUMP(CALL,0x530349,GET_OFFSET(_Log_CacheMdMix));

static	void __fastcall _Log_Ra2Mix(const char* pFormat, ...)
{
	Debug::Log("%s\n", pFormat);
}

DEFINE_JUMP(CALL,0x530307, GET_OFFSET(_Log_Ra2Mix));

static void* __cdecl _YR_Allocate_Ra2mix(size_t size)
{
	Debug::Log("%s \n" ,  GameStrings::LOCALMD_MIX());
	return YRMemory::Allocate(size);
}

DEFINE_JUMP(CALL,0x5302BA, GET_OFFSET(_YR_Allocate_Ra2mix));

namespace AnnoyingAudioLogSutffs
{
	bool Disable = true;
	bool bDisableNotTiberiumLog = true;
	bool bDisableNoDigestLog = true;
}

////40A55D
//static	void __fastcall _Log_soundFrameOrBufferSize(const char* pFormat, ...)
//{
//	if (!AnnoyingAudioLogSutffs::Disable)
//		Debug::Log(pFormat);
//}
//
//DEFINE_JUMP(CALL,0x40A55D, GET_OFFSET(_Log_soundFrameOrBufferSize));
//DEFINE_JUMP(CALL,0x40A5BC, GET_OFFSET(_Log_soundFrameOrBufferSize));

static	void __fastcall _Log_PathFailere_1(const char* pFormat, ...)
{
	if (Phobos::Otamaa::IsAdmin)
		return;

	if (!RulesExt::Global()->DisablePathfindFailureLog.Get())
		Debug::Log(pFormat);
}

DEFINE_JUMP(CALL,0x42CBDE, GET_OFFSET(_Log_PathFailere_1));

static	void __fastcall _Log_PathFailere_2(const char* pFormat, ...)
{
	if (Phobos::Otamaa::IsAdmin)
		return;

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

// ToDO : Make Optional
DEFINE_JUMP(CALL,0x40A55D, GET_OFFSET(_Log_Disable_These));
DEFINE_JUMP(CALL,0x40A5BC, GET_OFFSET(_Log_Disable_These));
DEFINE_JUMP(CALL,0x4431D8, GET_OFFSET(_Log_Disable_These)); //Survivor unlimbo OK Log
#endif

/*
static HRESULT AudioDriver_Start_replace(AudioChannelTag* pTag)
{
	DWORD v14;

	const auto pDriverData = pTag->drvData;
	auto pSampleFormat = pTag->sample->Format;
	if (!pSampleFormat)
		pSampleFormat = &pTag->Device->DefaultFormat;

	AudioChannelTag::AudioChannelSetFormat(pDriverData->audiochannel, pSampleFormat);

	auto nDriverptr = &pDriverData->soundriverpointer;
	if (pDriverData->soundriverpointer)
	{
		if (pDriverData->audioformat.Channels == pSampleFormat->Channels
		 && pDriverData->audioformat.Rate == pSampleFormat->Rate
		 && pDriverData->audioformat.SampleWidth == pSampleFormat->SampleWidth)
		{
			pDriverData->audiochannel->drv_format_changed = 0;
			pDriverData->bytesInFrame2 = 0;
			pDriverData->soundframesize2 = 0;
			pDriverData->soundframesizetimes4 = 4 * pDriverData->soundframesize1;
			(*nDriverptr)->SetCurrentPosition(0);
			(*nDriverptr)->GetCurrentPosition(&v14, &pDriverData->writecursor);
			pDriverData->soundframesizedivplaycursor = v14 / pDriverData->soundframesize1;
			pDriverData->audiogettimeresult = Game::AudioGetTime();
			auto v4 = *nDriverptr;
			pDriverData->buffersize2 = pDriverData->dwBufferBytes;
			v4->GetCurrentPosition(&pDriverData->playcursor, &pDriverData->writecursor);
			pDriverData->buffersizeminusplaycursor = pDriverData->dwBufferBytes - pDriverData->playcursor;
			pDriverData->somecount = 0;
			pDriverData->soundframesizedivplaycursor2 = pDriverData->soundframesizedivplaycursor;
			pDriverData->soundframesize3 = 0;
			pDriverData->playcursor2 = pDriverData->playcursor;
			pDriverData->AudioDriverChannelTag_noname_setup();
			pDriverData->skip_drvCBNextFrame = 0;
			auto v5 = pDriverData->audiochannel;
			pDriverData->frameData2 = pDriverData->audiochannel->frameData;
			pDriverData->frameData1 = v5->frameData;
			auto v6 = v5->bytesInFrame;
			pDriverData->bytesInFrame1 = v6;
			pDriverData->bytesInFrame2 = v6;
			if (AudioDriverChannelTag_noname_prefil(pDriverData, pDriverData->dwBufferBytes))
			{
				pTag->AudioDriver_update();
				if ((*nDriverptr)->Play(0, 0, 1))
				{
					Debug::Log("Failed to start playback\n");
					return 0x80040000;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				Debug::Log("Failed to prefill buffer\n");
				return 0x80040000;
			}
		}
		pDriverData->audiochannel->drv_format_changed = 1;
	}

	if (*nDriverptr)
	{
		(*nDriverptr)->Release();
		*nDriverptr = nullptr;
	}

	std::memcpy(&pDriverData->audioformat, pSampleFormat, sizeof(pDriverData->audioformat));

	DSBUFFERDESC nDesc { };
	nDesc.dwBufferBytes = 0;
	nDesc.dwReserved = 0;
	nDesc.lpwfxFormat = 0;
	nDesc.dwSize = 20;
	nDesc.dwFlags = 232;
	auto const v10 = pSampleFormat->Rate * (pSampleFormat->Channels * pSampleFormat->SampleWidth);
	auto const v12 = ((8 * (3 * v10 / 0x64u) + 1023) / 1024) << 10;
	pDriverData->soundframesize1 = v12;

	WAVEFORMATEX nFormat { };

	nFormat.wFormatTag = 1;
	nFormat.nChannels = pSampleFormat->Channels;
	nFormat.nSamplesPerSec = pSampleFormat->Rate;
	nFormat.nAvgBytesPerSec = pSampleFormat->Rate * (pSampleFormat->Channels * pSampleFormat->SampleWidth);
	nFormat.nBlockAlign = (pSampleFormat->Channels * pSampleFormat->SampleWidth);
	nFormat.wBitsPerSample = 8 * pSampleFormat->SampleWidth;
	nFormat.cbSize = 0;

	nDesc.lpwfxFormat = &nFormat;
	nDesc.dwBufferBytes = 4 * pDriverData->soundframesize1;

	if (AUD_sound_object->CreateSoundBuffer(&nDesc, &pDriverData->soundriverpointer, 0))
	{
		*nDriverptr = 0;
		Debug::Log("Failed to create playback buffer\n");
		return 0x80040000;
	}

	pDriverData->dwBufferBytes = nDesc.dwBufferBytes;

	pDriverData->bytesInFrame2 = 0;
	pDriverData->soundframesize2 = 0;
	pDriverData->soundframesizetimes4 = 4 * pDriverData->soundframesize1;
	(*nDriverptr)->SetCurrentPosition(0);
	(*nDriverptr)->GetCurrentPosition(&v14, &pDriverData->writecursor);
	pDriverData->soundframesizedivplaycursor = v14 / pDriverData->soundframesize1;
	pDriverData->audiogettimeresult = Game::AudioGetTime();
	auto v4 = *nDriverptr;
	pDriverData->buffersize2 = pDriverData->dwBufferBytes;
	v4->GetCurrentPosition(&pDriverData->playcursor, &pDriverData->writecursor);
	pDriverData->buffersizeminusplaycursor = pDriverData->dwBufferBytes - pDriverData->playcursor;
	pDriverData->somecount = 0;
	pDriverData->soundframesizedivplaycursor2 = pDriverData->soundframesizedivplaycursor;
	pDriverData->soundframesize3 = 0;
	pDriverData->playcursor2 = pDriverData->playcursor;
	pDriverData->AudioDriverChannelTag_noname_setup();
	pDriverData->skip_drvCBNextFrame = 0;
	auto v5 = pDriverData->audiochannel;
	pDriverData->frameData2 = pDriverData->audiochannel->frameData;
	pDriverData->frameData1 = v5->frameData;
	auto v6 = v5->bytesInFrame;
	pDriverData->bytesInFrame1 = v6;
	pDriverData->bytesInFrame2 = v6;
	HRESULT nRes { };
	if (AudioDriverChannelTag_noname_prefil(pDriverData, pDriverData->dwBufferBytes))
	{
		pTag->AudioDriver_update();
		if ((*nDriverptr)->Play(0, 0, 1))
		{
			Debug::Log("Failed to start playback\n");
			nRes = 0x80040000;
		}
		else
		{
			nRes = 0;
		}
	}
	else
	{
		Debug::Log("Failed to prefill buffer\n");
		nRes = 0x80040000;
	}

	return nRes;
}

DEFINE_HOOK(0x40A340, AudioDriver_Start_FuckAll , 0x5)
{
	GET(AudioChannelTag*, pTag, ECX);
	R->EAX(AudioDriver_Start_replace(pTag));
	return 0x40A489;
}*/