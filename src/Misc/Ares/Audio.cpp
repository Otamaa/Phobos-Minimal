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

struct LooseAudioFile
{
	int Offset { -1 };
	int Size { -1 };
	AudioSampleData Data {};
};

class LooseAudioCache
{
public:
	static std::vector<LooseAudioCache> Array;

	static int FindOrAllocateIndex(const char* Title)
	{
		const auto nResult = FindIndexById(Title);

		if (nResult < 0)
		{
			AllocateNoCheck(Title);
			return Array.size() - 1;
		}

		return nResult;
	}

	static int FindIndexById(const char* Title)
	{
		for (auto pos = Array.begin();
			pos != Array.end();
			++pos) {
			if (IS_SAME_STR_((*pos).Name.data(), Title))
			{
				return std::distance(Array.begin(), pos);
			}
		}

		return -1;
	}

	static void AllocateNoCheck(const char* Title)
	{
		Array.emplace_back(Title);
	}

	static inline LooseAudioCache* Find(int idx)
	{
		if ((size_t)idx > Array.size())
			Debug::FatalErrorAndExit("Trying To Get LoseAudioCache with Index [%d] but the array size is only [%d]\n", idx , Array.size());
		return &Array[idx];
	}

	static 	FileStruct GetFileStructFromIndex(int idx)
	{
		auto iter = Find(idx);

		// Replace the construction of the RawFileClass with one of a CCFileClass
		auto pFile = GameCreate<CCFileClass>(iter->WavName.c_str());

		if (pFile->Exists()) {
			if (pFile->Open(FileAccessMode::Read)) {
				if (iter->Data.Size < 0 && Audio::ReadWAVFile(pFile, &iter->Data.Data, &iter->Data.Size)) {
					iter->Data.Offset = pFile->Seek(0, FileSeekMode::Current);
				}
			}
		}
		else
		{
			GameDelete<true , false>(std::exchange(pFile , nullptr));
		}

		return { iter->Data.Size, iter->Data.Offset, pFile, true };
	}

	static AudioSampleData* GetAudioSampleDataFromIndex(int idx)
	{
		const auto iter = Find(idx);

		if (iter)
		{
			if (iter->Data.Size < 0)
			{
				auto file = GetFileStructFromIndex(idx);
				if (file.File && file.Allocated) {
					GameDelete<true , false>(std::exchange(file.File , nullptr));
				}
			}

			return &iter->Data.Data;
		}

		return nullptr;
	}

	LooseAudioCache(const char* Title) : Name { Title }, WavName { Title }, Data {}
	{
		WavName += ".wav";
	}

	~LooseAudioCache() = default;
private:
	std::string Name;
	std::string WavName;
	LooseAudioFile Data;

	//LooseAudioCache(const LooseAudioCache&) = default;
	//LooseAudioCache(LooseAudioCache&&) = default;
	//LooseAudioCache& operator=(const LooseAudioCache& other) = default;
};

std::vector<LooseAudioCache> LooseAudioCache::Array;

class AudioLuggage
{
public:
	static AudioLuggage Instance;

	class AudioBag
	{
		AudioBag(const AudioBag&) = delete;
		AudioBag& operator=(const AudioBag& other) = delete;
	public:

		AudioBag() = default;
		~AudioBag() = default;

		explicit AudioBag(const char* pFilename) : AudioBag() {
			this->Open(pFilename);
		}

		AudioBag(AudioBag&& other) noexcept {
			this->Entries = std::move(other.Entries);
			this->Bag = std::move(other.Bag);
			this->BagFile = std::move(other.BagFile);
		};

	private:
		void Open(const char* fileBase)
		{
			std::string filename = fileBase;
			const size_t filebase_len = filename.size();
			filename += ".idx";
			CCFileClass pIndex { filename.c_str() };

			if (pIndex.Exists() && pIndex.Open(FileAccessMode::Read))
			{
				filename[filebase_len + 1] = 'b';
				filename[filebase_len + 2] = 'a';
				filename[filebase_len + 3] = 'g';
				auto pBag = UniqueGamePtrB<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename.c_str()));

				if (pBag->Exists()
					&& pBag->Open(FileAccessMode::Read))
				{
					AudioIDXHeader headerIndex {};
					if(pIndex.ReadBytes(&headerIndex, sizeof(AudioIDXHeader)) == sizeof(AudioIDXHeader))
					{
						if (Phobos::Otamaa::OutputAudioLogs) {
							Debug::Log("Reading [%s from %s] file with [%d] samples!.\n", filename.c_str(), pIndex.GetFileName(), headerIndex.numSamples);
						}

						if (headerIndex.numSamples > 0)
						{
							this->Entries.resize(headerIndex.numSamples, {});

							constexpr size_t const IdxEntrysize = sizeof(AudioIDXEntry);
							constexpr size_t const readBytes = IdxEntrysize - 4;

							if (headerIndex.Magic == 1)
							{
								for (auto& entry : this->Entries)
								{
									while (pIndex.ReadBytes(&entry, readBytes) == readBytes)
									{
										entry.ChunkSize = 0;
									}
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
					this->BagFile = std::move(filename);
				}
			}
		}

	public:
		std::string BagFile;
		UniqueGamePtrB<CCFileClass> Bag; //big file that contains the audios
		std::vector<AudioIDXEntry> Entries; //every audio data that sit inside the file above
	};

	AudioIDXData* Pack(const char* pPath = nullptr)
	{
		std::map<AudioIDXEntry , std::tuple<int, CCFileClass*, std::string>,std::less<AudioIDXEntry>> map;

		for (size_t i = 0; i < this->Bags.size(); ++i) {
			if (this->Bags[i].Bag.get()) {
				for (const auto& ent : this->Bags[i].Entries) {
					auto find = map.find(ent);

					//no entry , put one
					if (find == map.end()) {
						map.emplace(ent, std::make_tuple(i , this->Bags[i].Bag.get() , this->Bags[i].BagFile));
					}
					else
					{
						//update the data with the new one
						auto node = map.extract(find);
						node.key().update(ent);
						auto& [idx, file , bagFileName] = node.mapped();

						if(Phobos::Otamaa::OutputAudioLogs) {
							Debug::Log("Replacing audio `%s` from : [%d - (%s - %s)] to : [%d - (%s - %s)].\n",
								ent.Name,
								idx,
								file->FileName ,
								bagFileName.c_str(),
								i,
								this->Bags[i].Bag->FileName,
								this->Bags[i].BagFile.c_str()
							);
						}

						idx = i;
						file = this->Bags[i].Bag.get();
						bagFileName = this->Bags[i].BagFile;
						map.insert(std::move(node));
					}
				}
			}
		}

		AudioIDXData* Indexes = GameCreateUnchecked<AudioIDXData>();
		const int size = static_cast<int>(map.size());
		Indexes->SampleCount = size;
		Indexes->Samples = GameCreateArray<AudioIDXEntry>(size);

		int i = 0;
		for (auto const& [entry, data] : map) {
			//Debug::Log("Samples[%d] Name [%s][%d , %d , %d ,  %d , %d]\n",
			//	i,
			//	entry.Name,
			//	entry.Offset,
			//	entry.Size,
			//	entry.SampleRate,
			//	entry.Flags,
			//	entry.ChunkSize
			//);
			std::memcpy(&Indexes->Samples[i++], &entry, sizeof(AudioIDXEntry));

			this->Files.emplace_back(std::get<0>(data) , std::get<1>(data));
		}

		return Indexes;
	}

	void Append(const char* pFileBase) {
		this->Bags.emplace_back(pFileBase);
	}

	bool GetFileStruct(FileStruct& file , int idx , AudioIDXEntry*& sample) {

		if (size_t(idx) < this->Files.size()) {
			sample = &AudioIDXData::Instance->Samples[idx];
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

AudioLuggage AudioLuggage::Instance;

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
	IMPL_SNPRNINTF(&result[0], len + 1, format, args...);

    return result;
};

bool PlayWavWrapper(int HouseTypeIdx , size_t SampleIdx)
{
	const auto pAudioStream = AudioStream::Instance();
	if(!pAudioStream || Unsorted::ScenarioInit_Audio() || SampleIdx > 9 || HouseTypeIdx <= -1) {
		return false;
	}

	const auto pExt = HouseTypeExtContainer::Instance.Find(
		HouseTypeClass::Array->Items[HouseTypeIdx]
	);

	if (pExt->TauntFile.empty() || pExt->TauntFile[SampleIdx - 1].empty()) {
		Debug::FatalErrorAndExit("Country [%s] Have Invalid Taunt Name Format [%s]\n",
		pExt->AttachedToObject->ID, pExt->TauntFile[SampleIdx - 1].c_str());
	}

	return pAudioStream->PlayWAV(pExt->TauntFile[SampleIdx - 1].c_str(), false);
}

#ifndef aaa

DEFINE_HOOK(0x752b70 , PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	R->EAX(PlayWavWrapper(data.countryIdx, data.tauntIdx));
	return 0x752C68;
}

DEFINE_HOOK(0x536438 , TauntCommandClass_Execute , 5)
{
   GET(TauntDataStruct, data , ECX);
  const auto house =  NodeNameType::Array->Items[0]->Country;
  R->Stack(0x4D , house);
  PlayWavWrapper(house, data.tauntIdx);
  return 0x53643D;
}

DEFINE_HOOK(0x48da3b , sub_48D1E0_PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	PlayWavWrapper(GlobalPacketType::Instance->Command, data.tauntIdx);
	return 0x48DAD3;
}

#include <ThemeClass.h>

//DEFINE_HOOK(0x406B10, Audio_InitPhobosAudio, 0x6) {
//	LooseAudioCache::Allocate();
//	//AudioLuggage::Allocate();
//	return 0x0;
//}

// skip theme log lines
DEFINE_HOOK(0x720C3C, Theme_PlaySong_DisableStopLog, 0x6) // skip Theme::PlaySong
{
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720C4D;
}

DEFINE_HOOK(0x720DBF, ThemeClass_PlaySong_DisablePlaySongLog, 0x5)
{
	GET(ThemeClass*, pThis, ESI);
	R->AL(pThis->IsScoreRepeat);
	return 0x720DF3;
}

DEFINE_HOOK(0x720F2E, ThemeClass_Stop_DisableStopLog, 0x9)
{
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720F42;
}

DEFINE_HOOK(0x720A58, ThemeClass_AI_DisableLog, 0x6)
{
	GET(ThemeClass*, pThis, ESI);
	pThis->QueuedTheme = R->EAX<int>();
	return 0x720A69;
}

// load more than one audio bag and index.
// this replaces the entire old parser.
DEFINE_HOOK(0x4011C0, Audio_Load, 6)
{
	auto& instance = AudioLuggage::Instance;
	// audio.bag and ares.bag
	instance.Append(GameStrings::audio());
	instance.Append("ares");

	// audio01.bag to audio99.bag
	//char buffer[0x100];
	for(auto i = 1; i < 100; ++i) {
		instance.Append(std::format("audio{:02}", i).c_str());
	}

	// cram all luggage datas onto single AudioIdxData pointer
	R->EAX(instance.Pack());
	return 0x401578;
}

DEFINE_HOOK(0x4016F0, IDXContainer_LoadSample, 6)
{
	GET(AudioIDXData*, pThis, ECX);
	GET(int const, index, EDX);

	pThis->ClearCurrentSample();

	FileStruct file;
	AudioIDXEntry* ptr = nullptr;
	if (!AudioLuggage::Instance.GetFileStruct(file, index , ptr))
		file = LooseAudioCache::GetFileStructFromIndex(index - pThis->SampleCount);

	pThis->CurrentSampleFile = file.File;
	pThis->CurrentSampleSize = file.Size;
	if (file.Allocated) {
		pThis->ExternalFile = file.File;
	}

	R->EAX(file.File && file.Size
		&& file.File->Seek(file.Offset, FileSeekMode::Set) == file.Offset);
	return 0x4018B8;
}

// add saple is assemble an idex then put it onto some list
DEFINE_HOOK(0x4064A0, VocClassData_AddSample, 6) // Complete rewrite of VocClass::AddSample
{
	GET(AudioEventClassTag*, pVoc, ECX);
	GET(const char*, pSampleName, EDX);

	if (!AudioIDXData::Instance())
		Debug::FatalError("AudioIDXData is missing!\n");

	if(pVoc->NumSamples == 0x20) {
		// return false
		R->EAX(0);
	} else {
		const bool AutoEventSet = *reinterpret_cast<int*>(0x87E2A0);

		if(AutoEventSet) { // I dunno
			while(*pSampleName == '$' || *pSampleName == '#') {
				++pSampleName;
			}

			auto idxSample = AudioIDXData::Instance->FindSampleIndex(pSampleName);

			if(idxSample == -1) {
				idxSample = LooseAudioCache::FindOrAllocateIndex(pSampleName) + AudioIDXData::Instance->SampleCount;
			}

			if (Phobos::Otamaa::OutputAudioLogs && idxSample == -1) {
				Debug::Log("Cannot Find [%s] sample!.\n", pSampleName);
			}

			// Set sample index or string pointer
			pVoc->SampleIndex[pVoc->NumSamples++] = idxSample;

			// return true
			R->EAX(1);
		}
	}

	return 0x40651E;
}


DEFINE_HOOK(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if ((size_t)idxSample < AudioLuggage::Instance.TotalSampleSizes())
	{
		//const auto& pIdx = AudioIDXData::Instance()->Samples[idxSample];
		//Debug::Log("SampleInfo [%s]\n", pIdx.Name.data());
		return 0x0;
	}

	if(auto const pData = LooseAudioCache::GetAudioSampleDataFromIndex(idxSample - AudioIDXData::Instance->SampleCount)) {
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