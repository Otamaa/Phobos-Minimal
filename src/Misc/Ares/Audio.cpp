// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include <Phobos.h>

#include <Ext/HouseType/Body.h>
#include <SessionClass.h>

#include <mutex>
#include <shared_mutex>

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
	LooseAudioCache(const char* Title)
		: Name(Title), WavName(Title), Data {}
	{
		WavName += ".wav";
	}

	LooseAudioCache(const LooseAudioCache&) = delete;
	LooseAudioCache& operator=(const LooseAudioCache&) = delete;
	LooseAudioCache(LooseAudioCache&&) = delete;
	LooseAudioCache& operator=(LooseAudioCache&&) = delete;

	~LooseAudioCache() = default;

	FileStruct GetFileStruct()
	{
		std::lock_guard<std::mutex> lock(ObjectMutex);
		auto pFile = GameCreate<CCFileClass>(WavName.c_str());

		if (pFile->Exists() && pFile->Open(FileAccessMode::Read))
		{
			if (Data.Size < 0 && Audio::ReadWAVFile(pFile, &Data.Data, &Data.Size))
			{
				Data.Offset = pFile->Seek(0, FileSeekMode::Current);
			}
		}
		else
		{
			GameDelete<true, false>(pFile);
			pFile = nullptr;
		}

		return { Data.Size, Data.Offset, pFile, pFile != nullptr };
	}

	AudioSampleData* GetAudioSampleData()
	{
		std::lock_guard<std::mutex> lock(ObjectMutex);
		if (Data.Size < 0)
		{
			auto file = GetFileStruct();
			if (file.File && file.Allocated)
			{
				GameDelete<true, false>(file.File);
			}
		}
		return &Data.Data;
	}

	const std::string& GetName() const { return Name; }

private:
	std::string Name;
	std::string WavName;
	LooseAudioFile Data;
	std::mutex ObjectMutex;
};

class LooseAudioCacheManager
{
	static std::vector<std::unique_ptr<LooseAudioCache>> Array;

public:

	static int FindOrAllocateIndex(const char* Title)
	{
		auto& array = Array;
		auto it = std::find_if(array.begin(), array.end(), [&](const auto& ptr)
 {
	 return ptr->GetName() == Title;
		});

		if (it != array.end())
		{
			return static_cast<int>(std::distance(array.begin(), it));
		}

		array.emplace_back(std::make_unique<LooseAudioCache>(Title));
		return static_cast<int>(array.size() - 1);
	}

	static LooseAudioCache* Find(int idx)
	{
		auto& array = Array;
		if (idx < 0 || static_cast<size_t>(idx) >= array.size())
		{
			Debug::FatalErrorAndExit("Invalid LooseAudioCache index: %d", idx);
			return nullptr;
		}
		return array[idx].get();
	}

	static FileStruct GetFileStructFromIndex(int idx)
	{
		auto* entry = Find(idx);
		return entry->GetFileStruct();
	}

	static AudioSampleData* GetAudioSampleDataFromIndex(int idx)
	{
		auto* entry = Find(idx);
		return entry->GetAudioSampleData();
	}

};

std::vector<std::unique_ptr<LooseAudioCache>> LooseAudioCacheManager::Array;

class AudioLuggage
{
public:

	class AudioBag
	{
		COMPILETIMEEVAL AudioBag(const AudioBag&) = delete;
		COMPILETIMEEVAL AudioBag& operator=(const AudioBag& other) = delete;
	public:

		COMPILETIMEEVAL AudioBag() = default;
		COMPILETIMEEVAL ~AudioBag() = default;

		explicit AudioBag(const char* pFilename) : AudioBag() {
			if(!this->Open(pFilename))
				Debug::LogInfo("Failed To open AudioBag {}" , pFilename);
		}

		AudioBag(AudioBag&& other) noexcept {
			this->Entries = std::move(other.Entries);
			this->Bag = std::move(other.Bag);
			this->BagFile = std::move(other.BagFile);
		};

	private:
		bool Open(const char* fileBase)
		{
			std::string filename = fileBase;
			const size_t filebase_len = filename.size();
			filename += ".idx";
			CCFileClass pIndex { filename.c_str() };
			if (Phobos::Otamaa::OutputAudioLogs)
				Debug::LogInfo("Reading {}" , filename);

			if (pIndex.Exists() && pIndex.Open(FileAccessMode::Read))
			{
				filename[filebase_len + 1] = 'b';
				filename[filebase_len + 2] = 'a';
				filename[filebase_len + 3] = 'g';

				if(Phobos::Otamaa::OutputAudioLogs)
					Debug::LogInfo("Reading {}" , filename);

				auto pBag = UniqueGamePtrC<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename.c_str()));

				if (pBag->Exists()
					&& pBag->Open(FileAccessMode::Read))
				{
					AudioIDXHeader headerIndex {};
					if(pIndex.ReadBytes(&headerIndex, sizeof(AudioIDXHeader)) == sizeof(AudioIDXHeader))
					{
						if (Phobos::Otamaa::OutputAudioLogs) {
							Debug::LogInfo("Reading [{} from {}] file with [{}] samples!.", filename.c_str(), pIndex.GetFileName(), headerIndex.numSamples);
						}

						if (headerIndex.numSamples > 0)
						{
							this->Entries.resize(headerIndex.numSamples, {});

							COMPILETIMEEVAL size_t const IdxEntrysize = sizeof(AudioIDXEntry);
							COMPILETIMEEVAL size_t const readBytes = IdxEntrysize - 4;

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
								const auto headerSize = headerIndex.numSamples * IdxEntrysize;
								const auto readed = pIndex.ReadBytes(&this->Entries[0], static_cast<int>(headerSize));

								if (readed != (int)headerSize)
								{
									if(Phobos::Otamaa::OutputAudioLogs)
										Debug::LogInfo("Failed Reading [{} from {}] file with [{}] samples , due to missmatch header size [readed {} vs intended {}]].", filename.c_str(), pIndex.GetFileName(), headerIndex.numSamples , readed, headerSize);
									return false;
								}
							}

							std::sort(this->Entries.begin(), this->Entries.end());
						}
					}

					this->Bag = std::move(pBag);
					this->BagFile = std::move(filename);
					return true;
				}
			}

			return false;
		}

	public:
		std::string BagFile;
		UniqueGamePtrC<CCFileClass> Bag; //big file that contains the audios
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
							Debug::LogInfo("Replacing audio `{}` from : [{} - ({} - {})] to : [{} - ({} - {})].",
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
			//Debug::LogInfo("Samples[%d] Name [%s][%d , %d , %d ,  %d , %d]",
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

	COMPILETIMEEVAL void Append(const char* pFileBase) {
		this->Bags.emplace_back(pFileBase);
	}

	COMPILETIMEEVAL std::optional<FileStruct> GetFileStruct(int idx) {

		const auto& files = this->Files;
		if (size_t(idx) < files.size()) {
			const auto sample = &AudioIDXData::Instance->Samples[idx];
			return FileStruct { sample->Size, sample->Offset, files[idx].second, false };
		}

		return {};
	}

	COMPILETIMEEVAL size_t TotalSampleSizes() const {
		return this->Files.size();
	}

private:

	std::vector<AudioBag> Bags;

	//contains linked real index of bags with files within
	std::vector<std::pair<int , CCFileClass*>> Files;

public:
	static AudioLuggage Instance;
};

AudioLuggage AudioLuggage::Instance;

bool PlayWavWrapper(int HouseTypeIdx , size_t SampleIdx)
{
	const auto pAudioStream = AudioStream::Instance();
	if(!pAudioStream || Unsorted::ScenarioInit_Audio() || SampleIdx > 9 || HouseTypeIdx <= -1) {
		return false;
	}

	const auto pExt = HouseTypeExtContainer::Instance.Find(
		HouseTypeClass::Array->Items[HouseTypeIdx]
	);

	const auto& vec = pExt->TauntFile;

	if (vec.empty() || vec[SampleIdx - 1].empty()) {
		Debug::FatalErrorAndExit("Country [%s] Have Invalid Taunt Name Format [%s]",
		pExt->AttachedToObject->ID, vec[SampleIdx - 1].c_str());
	}

	return pAudioStream->PlayWAV(vec[SampleIdx - 1].c_str(), false);
}

ASMJIT_PATCH(0x752b70 , PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	R->EAX(PlayWavWrapper(data.countryIdx, data.tauntIdx));
	return 0x752C68;
}

ASMJIT_PATCH(0x536438 , TauntCommandClass_Execute , 5)
{
   GET(TauntDataStruct, data , ECX);
  const auto house =  NodeNameType::Array->Items[0]->Country;
  R->Stack(0x4D , house);
  PlayWavWrapper(house, data.tauntIdx);
  return 0x53643D;
}

ASMJIT_PATCH(0x48da3b , sub_48D1E0_PlayTaunt , 5)
{
	GET(TauntDataStruct, data , ECX);
	PlayWavWrapper(GlobalPacketType::Instance->Command, data.tauntIdx);
	return 0x48DAD3;
}

#include <ThemeClass.h>

//ASMJIT_PATCH(0x406B10, Audio_InitPhobosAudio, 0x6) {
//	LooseAudioCache::Allocate();
//	//AudioLuggage::Allocate();
//	return 0x0;
//}

// skip theme log lines
ASMJIT_PATCH(0x720C3C, Theme_PlaySong_DisableStopLog, 0x6) // skip Theme::PlaySong
{
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720C4D;
}

ASMJIT_PATCH(0x720DBF, ThemeClass_PlaySong_DisablePlaySongLog, 0x5)
{
	GET(ThemeClass*, pThis, ESI);
	R->AL(pThis->IsScoreRepeat);
	return 0x720DF3;
}

ASMJIT_PATCH(0x720F2E, ThemeClass_Stop_DisableStopLog, 0x9)
{
	GET(ThemeClass*, pThis, ESI);
	R->ECX(pThis->Stream);
	return 0x720F42;
}

ASMJIT_PATCH(0x720A58, ThemeClass_AI_DisableLog, 0x6)
{
	GET(ThemeClass*, pThis, ESI);
	pThis->QueuedTheme = R->EAX<int>();
	return 0x720A69;
}

// load more than one audio bag and index.
// this replaces the entire old parser.
ASMJIT_PATCH(0x4011C0, Audio_Load, 6)
{
	auto& instance = AudioLuggage::Instance;
	// audio.bag and ares.bag
	instance.Append(GameStrings::audio());
	instance.Append("ares");

	// audio01.bag to audio99.bag
	fmt::memory_buffer buffer {};
	for(auto i = 1; i < 100; ++i) {
		fmt::format_to(std::back_inserter(buffer), "audio{:02}", i);
		buffer.push_back('\0');
		instance.Append(buffer.data());
		buffer.clear();
	}

	// cram all luggage datas onto single AudioIdxData pointer
	R->EAX(instance.Pack());
	return 0x401578;
}

ASMJIT_PATCH(0x4016F0, IDXContainer_LoadSample, 6)
{
	GET(AudioIDXData*, pThis, ECX);
	GET(int const, index, EDX);

	pThis->ClearCurrentSample();

	std::optional<FileStruct> file = AudioLuggage::Instance.GetFileStruct(index);
	if (!file) file = LooseAudioCacheManager::GetFileStructFromIndex(index - pThis->SampleCount);

	pThis->CurrentSampleFile = file->File;
	pThis->CurrentSampleSize = file->Size;
	if (file->Allocated) {
		pThis->ExternalFile = file->File;
	}

	R->EAX(file->File && file->Size
		&& file->File->Seek(file->Offset, FileSeekMode::Set) == file->Offset);

	return 0x4018B8;
}

// add saple is assemble an idex then put it onto some list
ASMJIT_PATCH(0x4064A0, VocClassData_AddSample, 6) // Complete rewrite of VocClass::AddSample
{
	GET(AudioEventClassTag*, pVoc, ECX);
	GET(const char*, pSampleName, EDX);

	if (!AudioIDXData::Instance())
		Debug::FatalError("AudioIDXData is missing!");

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
				idxSample = LooseAudioCacheManager::FindOrAllocateIndex(pSampleName) + AudioIDXData::Instance->SampleCount;
			}

			if (Phobos::Otamaa::OutputAudioLogs && idxSample == -1) {
				Debug::LogInfo("Cannot Find [{}] sample!.", pSampleName);
			}

			// Set sample index or string pointer
			pVoc->SampleIndex[pVoc->NumSamples++] = idxSample;

			// return true
			R->EAX(1);
		}
	}

	return 0x40651E;
}

ASMJIT_PATCH(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if ((size_t)idxSample < AudioLuggage::Instance.TotalSampleSizes())
	{
		//const auto& pIdx = AudioIDXData::Instance()->Samples[idxSample];
		//Debug::LogInfo("SampleInfo [%s]", pIdx.Name.data());
		return 0x0;
	}

	if(auto const pData = LooseAudioCacheManager::GetAudioSampleDataFromIndex(idxSample - AudioIDXData::Instance->SampleCount)) {
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

//ASMJIT_PATCH(0x750E4A, VocCClass_Play_DebugMem, 0x6)
//{
//	GET(int, _IDX, EBP);
//	Debug::LogInfo("Playing Audio at idx {}", _IDX);
//	return 0x0;
//}