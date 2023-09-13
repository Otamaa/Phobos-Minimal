// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include <Phobos.h>

#include <Ext/HouseType/Body.h>
#include <SessionClass.h>

struct FileStruct
{
	int Size;
	int Offset;
	RawFileClass* File;
	bool Allocated;
};

class LooseAudioCache
{
private:
	static std::unique_ptr<LooseAudioCache> InternalData;

public:

	struct LooseAudioFile
	{
		std::string wavName {};
		int Offset { -1 };
		int Size { -1 };
		AudioSampleData Data {};
	};

	static void Allocate()
	{
		InternalData = std::make_unique<LooseAudioCache>();
	}

	static LooseAudioCache* Instance()
	{
		return InternalData.get();
	}

	//not guarantee but will allocate new
	uintptr_t GetPointerAsindex(const char* pFilename)
	{
		auto it = this->Files.find(pFilename);
		if (it == this->Files.end()) {
			it = this->Files.emplace(pFilename, LooseAudioFile()).first;
		}

		if (it->second.wavName.empty()) {
			it->second.wavName = pFilename;
			it->second.wavName += ".wav";
		}

		return (uintptr_t)it->first.c_str();
	}

	//not guarantee
	FileStruct GetFileStructFromIndexOfRawPointer(uintptr_t rawptr) {
		auto iter = this->Files.find(reinterpret_cast<const char*>(rawptr));

		// Replace the construction of the RawFileClass with one of a CCFileClass
		auto pFile = GameCreate<CCFileClass>(iter->second.wavName.c_str());

		if (pFile->Exists()) {
			if( pFile->Open(FileAccessMode::Read)){
				if (iter->second.Size < 0 && Audio::ReadWAVFile(pFile, &iter->second.Data, &iter->second.Size)) {
						iter->second.Offset = pFile->Seek(0, FileSeekMode::Current);
				}
			}
		}
		else
		{
			GameDelete(pFile);
			pFile = nullptr;
		}

		return { iter->second.Size, iter->second.Offset, pFile, true };
	 }

	 static FileStruct GetFileStructFromName(LooseAudioFile& cache , const char* name)
	 {
		 // Replace the construction of the RawFileClass with one of a CCFileClass

		 auto pFile = GameCreate<CCFileClass>(cache.wavName.c_str());

		 if (pFile->Exists() && pFile->Open(FileAccessMode::Read)) {
			 if (cache.Size < 0 && Audio::ReadWAVFile(pFile, &cache.Data, &cache.Size)) {
				 cache.Offset = pFile->Seek(0, FileSeekMode::Current);
			 }
		 }
		 else
		 {
			 GameDelete(pFile);
			 pFile = nullptr;
		 }

		 return { cache.Size, cache.Offset, pFile, true };
	 }

	 //not guarantee
	 AudioSampleData* GetAudioSampleDataFromIndexOfRawPointer(uintptr_t rawptr)
	 {
		 auto iter = this->Files.find(reinterpret_cast<const char*>(rawptr));

		 if (iter != this->Files.end())
		 {
			 if (iter->second.Size < 0)
			 {
				 auto file = GetFileStructFromName(iter->second , iter->first.c_str());
				 if (file.File && file.Allocated)  {
					 GameDelete(file.File);
				 }
			 }

			 return &iter->second.Data;
		 }

		 return nullptr;
	 }
private:
	std::map<std::string, LooseAudioFile> Files;
};

std::unique_ptr<LooseAudioCache> LooseAudioCache::InternalData;

class AudioLuggage
{
	static std::unique_ptr<AudioLuggage> InternalData;
public:

	static void Allocate()
	{
		InternalData = std::make_unique<AudioLuggage>();
	}

	static AudioLuggage* Instance()
	{
		return InternalData.get();
	}

	class AudioBag
	{
	public:

		AudioBag() = default;

		explicit AudioBag(const char* pFilename) : AudioBag() {
			this->Open(pFilename);
		}

		AudioBag(AudioBag&& other) noexcept {
			this->Entries = std::move(other.Entries);
			this->Bag = std::move(other.Bag);
		};

	private:
		void Open(const char* fileBase)
		{
			char filename[0x100];
			_snprintf_s(filename, _TRUNCATE, "%s.idx", fileBase);

			CCFileClass pIndex { filename };

			if (pIndex.Exists() && pIndex.Open(FileAccessMode::Read))
			{
				_snprintf_s(filename, _TRUNCATE, "%s.bag", fileBase);
				auto pBag = UniqueGamePtrB<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename));

				if (pBag->Exists()
					&& pBag->Open(FileAccessMode::Read))
				{
					AudioIDXHeader headerIndex;
					if(pIndex.ReadBytes(&headerIndex, sizeof(AudioIDXHeader)) == sizeof(AudioIDXHeader))
					{
						//std::vector<AudioIDXEntry> entries;

						if (headerIndex.numSamples > 0)
						{
							this->Entries.resize(headerIndex.numSamples, {});

							constexpr size_t const IdxEntrysize = sizeof(AudioIDXEntry);
							constexpr size_t const readBytes = IdxEntrysize - 4;

							if (headerIndex.Version == 1)
							{
								for (auto& entry : this->Entries)
								{
									if (pIndex.ReadBytes(&entry, readBytes) != readBytes)
									{
										return;
									}
									entry.ChunkSize = 0;
								}
							}
							else
							{
								auto const headerSize = headerIndex.numSamples * IdxEntrysize;
								if (pIndex.ReadBytes(&this->Entries[0], static_cast<int>(headerSize)) != headerSize)
								{
									return;
								}
							}

							std::sort(this->Entries.begin(), this->Entries.end());
						}
					}

					this->Bag = std::move(pBag);
				}
			}
		}

	public:
		UniqueGamePtrB<CCFileClass> Bag; //big file that contains the audios
		std::vector<AudioIDXEntry> Entries; //every audio data that sit inside the file above
	};

	AudioIDXData* Pack(const char* pPath = nullptr)
	{
		std::map<AudioIDXEntry ,int> map;

		for (size_t i = 0; i < this->Bags.size(); ++i) {

			if (!this->Bags[i].Bag.get())
				continue;

			for (const auto& ent : this->Bags[i].Entries) {
				map[ent] = i;
			}
		}

		//std::sort(map.begin(), map.end(), [](auto& left, auto& right) {
		//	return left.first < right.first;
		//});

		AudioIDXData* Indexes = GameCreateUnchecked<AudioIDXData>();
		const int size = static_cast<int>(map.size());
		Indexes->SampleCount = size;
		Indexes->Samples = GameCreateArray<AudioIDXEntry>(size);

		int i = 0;
		for (auto const& [entry, indx] : map) {
			std::memcpy(&Indexes->Samples[i++], &entry, sizeof(AudioIDXEntry));
			//Debug::Log("Samples[%d] Name [%s]\n", i, entry.Name.data());
			this->Files.emplace_back(indx, this->Bags[indx].Bag.get());
		}

		//AudioIDXEntry* test = static_cast<AudioIDXEntry*>(CRT::bsearch(
		//	"bconsela",
		//	Indexes->Samples,
		//	size,
		//	sizeof(AudioIDXEntry),
		//	(int(__cdecl*)(const void*, const void*))CRT::strcmpi));

		//auto iter = std::find_if(ret->Samples, ret->Samples + size, [](const AudioIDXEntry& item) {
		//		Debug::Log("Samples Name [%s]\n", item.Name.data());
		//		const auto res = CRT::strcmpi(item.Name.data(), "bconsela");
		//		return (res == 0);
		//
		//});
		//
		//if (iter != (ret->Samples + size)) {
		//	Debug::Log("Iter result [%d]\n", std::distance(ret->Samples, iter));
		//}
		//
		//if(test) {
		//	int idx = (test - Indexes->Samples) / sizeof(AudioIDXEntry);
		//	Debug::Log("result %d\n", idx);
		//}

		return Indexes;
	}

	void Append(const char* pFileBase) {
		this->Bags.emplace_back(pFileBase);
	}

	bool GetFileStruct(FileStruct& file , int idx) {

		if (size_t(idx) < this->Files.size()) {
			const auto sample = &AudioIDXData::Instance->Samples[idx];
			file = { sample->Size, sample->Offset, this->Files[idx].second, false };
			return true;
		}

		return false;
	}

	CCFileClass* GetFileFromIndex(int idx)
	{
		return size_t(idx) < Files.size() ?
			this->Files[idx].second : nullptr;
	}

	size_t TotalSampleSizes() const {
		return this->Files.size();
	}

private:

	std::vector<AudioBag> Bags;

	//contains linked real index of bags with files within
	std::vector<std::pair<int , CCFileClass*>> Files;
};

std::unique_ptr<AudioLuggage> AudioLuggage::InternalData;

//only for testings
//DEFINE_HOOK(0x536355, TauntCommandClass_IgnoreGameTypeA, 6)
//{
//	R->EAX(3);
//	return 0;
//}
//
//DEFINE_HOOK(0x5363A2, TauntCommandClass_IgnoreGameTypeB , 6)
//{
//	R->ECX(3);
//	return 0x5363A8;
//}

// author : Richard Hodges
// https://stackoverflow.com/questions/40973464/parse-replace-in-c-stdstring
template<class...Args>
std::string replace(const char* format, Args const&... args)
{
    // determine number of characters in output
    size_t len = std::snprintf(nullptr, 0, format, args...);

    // allocate buffer space
    std::string result = std::string(std::size_t(len), ' ');

    // write string into buffer. Note the +1 is allowing for the implicit trailing
    // zero in a std::string
    std::snprintf(&result[0], len + 1, format, args...);

    return result;
};

bool NOINLINE PlayWavWrapper(int HouseTypeIdx , size_t SampleIdx)
{
	const auto pAudioStream = AudioStream::Instance();
	if(!pAudioStream || Unsorted::ScenarioInit_Audio() || SampleIdx > 9 || HouseTypeIdx <= -1) {
		return false;
	}

	const auto pExt = HouseTypeExt::ExtMap.Find(
		HouseTypeClass::Array->Items[HouseTypeIdx]
	);

	std::string buffer = pExt->TauntFile;
	const auto nPos = buffer.find("~~");

	if (nPos != std::string::npos) {
		//only set the 2 characters without the terminator string
		std::string number = "0";
		number += std::to_string(SampleIdx);
		buffer.replace(nPos, 2, number);

		//Debug::Log("Country [%s] with Taunt Name at idx [%d - %s]\n",
		//	pExt->OwnerObject()->ID, SampleIdx , buffer.c_str());
	} else {
		Debug::FatalErrorAndExit("Country [%s] Have Invalid Taunt Name Format [%s]\n",
		pExt->OwnerObject()->ID, pExt->TauntFile.c_str());
	}

	return pAudioStream->PlayWAV(buffer.c_str(), false);
}

DEFINE_OVERRIDE_HOOK(0x752b70 , PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	R->EAX(PlayWavWrapper(data.countryIdx, data.tauntIdx));
	return 0x752C68;
}

DEFINE_OVERRIDE_HOOK(0x536438 , TauntCommandClass_Execute , 5)
{
   GET(TauntDataStruct, data , ECX);
  const auto house =  NodeNameType::Array->Items[0]->Country;
  R->Stack(0x4D , house);
  PlayWavWrapper(house, data.tauntIdx);
  return 0x53643D;
}

DEFINE_OVERRIDE_HOOK(0x48da3b , sub_48D1E0_PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	PlayWavWrapper(GlobalPacketType::Instance->Command, data.tauntIdx);
	return 0x48DAD3;
}

#ifndef DISABLE_AUDIO_OVERRIDE

DEFINE_HOOK(0x406B10, Audio_InitPhobosAudio, 0x6) {
	LooseAudioCache::Allocate();
	AudioLuggage::Allocate();
	return 0x0;
}

// load more than one audio bag and index.
// this replaces the entire old parser.
DEFINE_OVERRIDE_HOOK(0x4011C0, Audio_Load, 6)
{
	// audio.bag and ares.bag
	AudioLuggage::Instance()->Append(GameStrings::audio());
	AudioLuggage::Instance()->Append("ares");

	// audio01.bag to audio99.bag
	char buffer[0x100];
	for(auto i = 1; i < 100; ++i) {
		_snprintf_s(buffer, _TRUNCATE, "audio%02d", i);
		AudioLuggage::Instance()->Append(buffer);
	}

	// cram all luggage datas onto single AudioIdxData pointer
	R->EAX(AudioLuggage::Instance()->Pack());
	return 0x401578;
}

DEFINE_OVERRIDE_HOOK(0x4016F0, IDXContainer_LoadSample, 6)
{
	GET(AudioIDXData* const, pThis, ECX);
	GET(int const, index, EDX);

	pThis->ClearCurrentSample();

	FileStruct file;
	if (!AudioLuggage::Instance()->GetFileStruct(file, index))
		file = LooseAudioCache::Instance()->GetFileStructFromIndexOfRawPointer(index);

	pThis->CurrentSampleFile = file.File;
	pThis->CurrentSampleSize = file.Size;
	if (file.Allocated)
	{
		pThis->ExternalFile = file.File;
	}

	auto const ret = file.File && file.Size
		&& file.File->Seek(file.Offset, FileSeekMode::Set) == file.Offset;

	R->EAX(ret);
	return 0x4018B8;
}

// add saple is assemble an idex then put it onto some list
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

			auto idxSample = !AudioIDXData::Instance() ? -1
				: AudioIDXData::Instance->FindSampleIndex(pSampleName);

			if(idxSample == -1) {
				idxSample = LooseAudioCache::Instance()->GetPointerAsindex(pSampleName);
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

DEFINE_OVERRIDE_HOOK(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if ((size_t)idxSample < AudioLuggage::Instance()->TotalSampleSizes())
		return 0x0;

	if(auto const pData = LooseAudioCache::Instance()->GetAudioSampleDataFromIndexOfRawPointer(idxSample)) {
		if(pData->SampleRate) {
			std::memcpy(pAudioSample, pData, sizeof(AudioSampleData));
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