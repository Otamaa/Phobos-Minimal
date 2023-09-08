// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include <Phobos.h>

LooseAudioCache LooseAudioCache::Instance;
AudioLuggage AudioLuggage::Instance;

AudioSampleData* AresAudioHelper::GetData(int const index)
{
	if(auto pFilename = ToLooseFile(index)) {
		auto& loose = LooseAudioCache::Instance.GetData(pFilename);

		if(loose.Size < 0) {
			auto file = GetFile(index);
			if(file.File && file.Allocated) {
				GameDelete<true , false>(file.File);
				file.File = nullptr;
			}
		}

		return &loose.Data;
	}

	return nullptr;
}

AresAudioHelper::FileStruct AresAudioHelper::GetFile(int const index)
{
	if(auto const pSampleName =  AresAudioHelper::ToLooseFile(index)) {
		auto& loose = LooseAudioCache::Instance.GetData(pSampleName);

		// Replace the construction of the RawFileClass with one of a CCFileClass
		char filename[0x100];
		_snprintf_s(filename, _TRUNCATE, "%s.wav", pSampleName);

		auto pFile = GameCreateUnchecked<CCFileClass>(filename);
		if(pFile->Exists() && pFile->Open(FileAccessMode::Read)) {
			if(loose.Size < 0 && Audio::ReadWAVFile(pFile, &loose.Data, &loose.Size)) {
				loose.Offset = pFile->Seek(0, FileSeekMode::Current);
			}
		} else {
			GameDelete<true, false>(pFile);
			pFile = nullptr;
		}

		return { loose.Size, loose.Offset, pFile, true };
	} else {
		auto& sample = AudioIDXData::Instance->Samples[index];
		auto const pFile = AudioLuggage::Instance.GetFile(index);
		return { sample.Size, sample.Offset, pFile, false };
	}
}

// change done
void AudioBag::Open(const char* fileBase) {
	char filename[0x100];
	_snprintf_s(filename, _TRUNCATE, "%s.idx", fileBase);

	CCFileClass pIndex { filename };

	if(pIndex.Exists() && pIndex.Open(FileAccessMode::Read)) {
		_snprintf_s(filename, _TRUNCATE, "%s.bag", fileBase);
		auto pBag = UniqueGamePtr<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename));

		AudioIDXHeader headerIndex;
		if(pBag->Exists()
			&& pBag->Open(FileAccessMode::Read)
			&& pIndex.ReadBytes(&headerIndex,sizeof(headerIndex)) == sizeof(headerIndex)) {{

				std::vector<AudioIDXEntry> entries;

				if(headerIndex.numSamples > 0) {
					entries.resize(headerIndex.numSamples, {});

					constexpr auto const IdxEntrysize = sizeof(AudioIDXEntry);

					if(headerIndex.Magic == 1) {
						for(auto& entry : entries) {
							if(!pIndex.ReadBytes(&entry, IdxEntrysize - 4)) {
								return;
							}
							entry.ChunkSize = 0;
						}
					} else {
						auto const headerSize = headerIndex.numSamples * IdxEntrysize;
						if(pIndex.ReadBytes(&entries[0], static_cast<int>(headerSize)) != headerSize) {
							return;
						}
					}
				}

				std::sort(entries.begin(), entries.end());
				this->Bag = std::move(pBag);
				this->Entries = std::move(entries);
			}
		}
	}
}

// there is big changes here
AudioIDXData* AudioLuggage::Create(const char* pPath) {
	std::map<AudioIDXEntry, CCFileClass*> map;

	for(auto const& bag : this->Bags) {
		for(auto const& entry : bag.entries()) {
			map[entry] = bag.file();
		}
	}

	auto ret = GameCreateUnchecked<AudioIDXData>();
	//ret->BagFile = nullptr; // this->Bags.front().file(); // not needed
	const int size = static_cast<int>(map.size());
	ret->SampleCount = size;
	ret->Samples = GameCreateArray<AudioIDXEntry>(size);

	this->Files.clear();
	this->Files.reserve(size);

	auto pEntry = ret->Samples;

	for(const auto&[entry ,pFiles] : map) {
		*pEntry++ = entry;
		this->Files.push_back(pFiles);
	}

	return ret;
}

/*
*	Otamaa : there quite some internal change that happen
*			not sure what it is but seems crucial , because enabling these will lead to
*			`Failed to prefill buffer` audio error
*/
#ifndef DISABLE_AUDIO_OVERRIDE
//NoChange
// load more than one audio bag and index.
// this replaces the entire old parser.
DEFINE_OVERRIDE_HOOK(0x4011C0, Audio_Load, 6)
{
	auto& luggage = AudioLuggage::Instance;

	// audio.bag and ares.bag
	luggage.Append(GameStrings::audio());
	luggage.Append("ares");

	// audio01.bag to audio99.bag
	char buffer[0x100];
	for(auto i = 1; i < 100; ++i) {
		_snprintf_s(buffer, _TRUNCATE, "audio%02d", i);
		luggage.Append(buffer);
	}

	// generate index
	R->EAX(luggage.Create());
	return 0x401578;
}

//NameChange
DEFINE_OVERRIDE_HOOK(0x4064A0, VocClassData_AddSample, 0) // Complete rewrite of VocClass::AddSample
{
	GET(AudioEventClassTag*, pVoc, ECX);
	GET(const char*, pSampleName, EDX);

	if(pVoc->NumSamples == 0x20) {
		// return false
		R->EAX(0);
	} else {
		const bool AutoEventSet = *reinterpret_cast<int*>(0x87E2A0);

		if(AutoEventSet) { // I dunno
			while(*pSampleName == '$' || *pSampleName == '#') {
				++pSampleName;
			}

			auto idxSample = !AudioIDXData::Instance ? -1
				: AudioIDXData::Instance->FindSampleIndex(pSampleName);

			if(idxSample == -1) {
				idxSample = LooseAudioCache::Instance.GetIndex(pSampleName);
			}

			// Set sample index or string pointer
			pVoc->SampleIndex[pVoc->NumSamples] = idxSample;
			++pVoc->NumSamples;

			// return true
			R->EAX(1);
		}
	}

	return 0x40651E;
}

//NoChange
DEFINE_OVERRIDE_HOOK(0x4016F0, IDXContainer_LoadSample, 6)
{
	GET(AudioIDXData* const, pThis, ECX);
	GET(int const, index, EDX);

	pThis->ClearCurrentSample();

	auto const file = AresAudioHelper::GetFile(index);
	pThis->CurrentSampleFile = file.File;
	pThis->CurrentSampleSize = file.Size;
	if(file.Allocated) {
		pThis->ExternalFile = file.File;
	}

	auto const ret = file.File && file.Size
		&& file.File->Seek(file.Offset, FileSeekMode::Set) == file.Offset;

	R->EAX(ret);
	return 0x4018B8;
}

//NoChange
DEFINE_OVERRIDE_HOOK(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if(auto const pData = AresAudioHelper::GetData(idxSample)) {
		if(pData->SampleRate) {
			*pAudioSample = *pData;
		} else {
			pAudioSample->Data = 4;
			pAudioSample->Format = 0;
			pAudioSample->SampleRate = 22050;
			pAudioSample->NumChannels = 1;
			pAudioSample->BytesPerSample = 2;
			pAudioSample->BlockAlign = 0;
		}

		R->EAX(pAudioSample);
		return 0x40169E;
	}

	return 0;
}
#endif