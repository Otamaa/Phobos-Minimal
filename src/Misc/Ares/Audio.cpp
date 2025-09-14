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
		WavName.reserve(Name.length() + 6);  // Pre-allocate
		WavName = Name + ".wav";
	}

	LooseAudioCache(const LooseAudioCache&) = delete;
	LooseAudioCache& operator=(const LooseAudioCache&) = delete;
	LooseAudioCache(LooseAudioCache&&) = delete;
	LooseAudioCache& operator=(LooseAudioCache&&) = delete;

	~LooseAudioCache() = default;

	FileStruct GetFileStruct()
	{
		CCFileClass* pFile = pFile = GameCreate<CCFileClass>(WavName.c_str());

		if (!pFile->Exists()) {
			if (Phobos::Otamaa::IsAdmin) {
				Debug::Log("LooseAudioCache: File does not exist: %s\n",
				WavName.c_str());
			}

			GameDelete<true, false>(pFile);
			pFile = nullptr;
			return { Data.Size, Data.Offset, pFile, pFile != nullptr };
		}

		if (!pFile->Open(FileAccessMode::Read)) {
			if (Phobos::Otamaa::IsAdmin)
				Debug::Log("LooseAudioCache: Failed to open file: %s\n", WavName.c_str());

			GameDelete<true, false>(pFile);
			pFile = nullptr;
			return { Data.Size, Data.Offset, pFile, pFile != nullptr };
		}

		if (Data.Size < 0 && Audio::ReadWAVFile(pFile, &Data.Data, &Data.Size)) {
			Data.Offset = pFile->Seek(0, FileSeekMode::Current);
		}

		return { Data.Size, Data.Offset, pFile, pFile != nullptr };
	}

	AudioSampleData* GetAudioSampleData()
	{
		if (Data.Size < 0)
		{
			if (Phobos::Otamaa::IsAdmin)
				Debug::Log("LooseAudioCache: Failed to parse WAV file: %s\n", WavName.c_str());

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
};

class LooseAudioCacheManager
{
	static std::vector<std::unique_ptr<LooseAudioCache>> Array;
	static std::mutex arrayMutex;

public:

	static int NameToIndex(const char* Title)
	{
		std::lock_guard<std::mutex> lock(arrayMutex);  // Lock here

		const auto it = std::find_if(Array.begin(), Array.end(), [&](const auto& ptr) {
			return ptr->GetName() == Title;
		});

		if (it == Array.end())
		{
			Array.emplace_back(std::move(std::make_unique<LooseAudioCache>(Title)));
			return (int)Array.back()->GetName().c_str(); // fuckers
		}

		return  (int)it->get()->GetName().c_str();
	}

	static LooseAudioCache* FindByIndexPtr(UINT_PTR idxptr)
	{
		std::lock_guard<std::mutex> lock(arrayMutex);  // Lock here

		if (idxptr >= 0x10000)
		{
			const auto it = std::find_if(Array.begin(), Array.end(), [&](const auto& ptr)	{
				return (UINT_PTR)(ptr->GetName().c_str()) == idxptr;
			});

			if (it == Array.end())
			{
				Debug::FatalErrorAndExit("Invalid LooseAudioCache index: %d", idxptr);
			}

			return it->get();
		}

		return nullptr;
	}
};

std::vector<std::unique_ptr<LooseAudioCache>> LooseAudioCacheManager::Array;
std::mutex LooseAudioCacheManager::arrayMutex;

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
			if(!this->Open(pFilename) && Phobos::Otamaa::IsAdmin)
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

				auto pBag = UniqueGamePtr<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename.c_str()));

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
		UniqueGamePtr<CCFileClass> Bag; //big file that contains the audios
		std::vector<AudioIDXEntry> Entries; //every audio data that sit inside the file above
	};

	AudioIDXData* Pack(const char* pPath = nullptr)
	{
		std::lock_guard<std::mutex> lock(luggageMutex);  // Lock here
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
		this->Files.reserve(size);  // Pre-allocate Files vector

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
		std::lock_guard<std::mutex> lock(luggageMutex);  // Lock here
		this->Bags.emplace_back(pFileBase);
	}

	COMPILETIMEEVAL std::optional<FileStruct> GetFileStruct(int idx) {
		std::lock_guard<std::mutex> lock(luggageMutex);  // Lock here

		const auto& files = this->Files;
		if (size_t(idx) < files.size()) {
			const auto sample = &AudioIDXData::Instance->Samples[idx];
			return FileStruct { sample->Size, sample->Offset, files[idx].second, false };
		}

		if (Phobos::Otamaa::IsAdmin)
			Debug::Log("LooseAudioCache: Failed to get audio file at index %d \n", idx);

		return {};
	}

	COMPILETIMEEVAL size_t TotalSampleSizes() const {
		std::lock_guard<std::mutex> lock(luggageMutex);  // Lock here
		return this->Files.size();
	}

private:

	std::vector<AudioBag> Bags;

	//contains linked real index of bags with files within
	std::vector<std::pair<int , CCFileClass*>> Files;
	mutable std::mutex luggageMutex;

public:
	static AudioLuggage Instance;
};

AudioLuggage AudioLuggage::Instance;

bool PlayWavWrapper(int HouseTypeIdx , size_t SampleIdx)
{
	if(!AudioStreamerTag::Instance() || Unsorted::ScenarioInit_Audio() || SampleIdx > 9 || HouseTypeIdx <= -1) {
		return false;
	}

	const auto pExt = HouseTypeExtContainer::Instance.Find(
		HouseTypeClass::Array->Items[HouseTypeIdx]
	);

	const auto& vec = pExt->TauntFile;

	if (vec.empty() || vec[SampleIdx - 1].empty()) {
		Debug::FatalErrorAndExit("Country [%s] Have Invalid Taunt Name Format [%s]",
		pExt->Name(), vec[SampleIdx - 1].c_str());
	}

	return AudioStreamerTag::PlayWAV(AudioStreamerTag::Instance() ,vec[SampleIdx - 1].c_str(), false);
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
ASMJIT_PATCH(0x720C39, Theme_PlaySong_DisableStopLog, 0x9) // skip Theme::PlaySong
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

// load more than one audio bag and index.
// this replaces the entire old parser.
ASMJIT_PATCH(0x4011C0, Audio_Load, 6)
{
	auto& instance = AudioLuggage::Instance;
	// audio.bag and ares.bag
	instance.Append(GameStrings::audio());
	instance.Append("ares");

	// audio01.bag to audio99.bag
	static fmt::basic_memory_buffer<char, 20> buffer {};
	for(auto i = 1; i < 100; ++i) {
		buffer.clear();
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

	std::optional<FileStruct> file = std::nullopt;

	if (auto pLose = LooseAudioCacheManager::FindByIndexPtr(index)) {
		file = pLose->GetFileStruct();
	}

	if (!file) {
		file = AudioLuggage::Instance.GetFileStruct(index);
	}

	if (!file) Debug::FatalErrorAndExit("Cannot find audio with idx %d !", index);

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
				idxSample = LooseAudioCacheManager::NameToIndex(pSampleName);
			}

			if (Phobos::Otamaa::OutputAudioLogs && idxSample == -1) {
				Debug::LogInfo("Cannot Find [{}] sample!.", pSampleName);
				pVoc->SamplesOK = false;
			} else {
				// Set sample index or string pointer
				pVoc->SampleIndex[pVoc->NumSamples++] = idxSample;
			}
			// return true
			R->EAX(1);
		}
	}

	return 0x40651E;
}

//ASMJIT_PATCH(0x401E4F, AudioCacheLoadItem_Invalid, 0x5)
//{
//	GET(AudioCacheTag*, pTag, EBX);
//	GET(AudioCacheItem*, pItem, EBP);
//
//	auto& format = pItem->format;
//	const size_t idx = (size_t)pItem->itemindex;
//	bool isValid = false;
//
//	if (idx < AudioLuggage::Instance.TotalSampleSizes())
//	{
//		auto& idx_ = pTag->idxdata->Samples[pItem->itemindex];
//		format.Flags = 4;
//		format.SampleRate = idx_.SampleRate;
//		format.NumChannels = ((idx_.Flags & 1) != 0) + 1;
//		format.BlockAlign = idx_.ChunkSize;
//
//		if ((idx_.Flags & 8) != 0)
//		{
//			format.Format = 1;
//			format.BytesPerSample = 2;
//		}
//		else
//		{
//			format.Format = 0;
//			format.BytesPerSample = ((idx_.Flags & 4) != 0) + 1;
//		}
//
//		isValid = true;
//	}
//	else if (auto const pData = LooseAudioCacheManager::Find(pItem->itemindex - AudioIDXData::Instance->SampleCount))
//	{
//
//		if (auto pSampleData = pData->GetAudioSampleData())
//		{
//			if (pSampleData->SampleRate)
//			{
//				std::memcpy(&pItem->format, pSampleData, sizeof(AudioSampleData));
//			}
//			else
//			{
//				format.Data = 4;
//				format.Format = 0;
//				format.SampleRate = 22050;
//				format.NumChannels = 1;
//				format.BytesPerSample = 2;
//				format.BlockAlign = 0;
//			}
//
//			isValid = true;
//		}
//
//		if (!isValid && Phobos::Otamaa::IsAdmin)
//			Debug::LogInfo("Cannot find sample at idx [{} - {}]", pItem->itemindex, pData->GetName());
//
//	}
//	else if (Phobos::Otamaa::IsAdmin)
//	{
//		Debug::LogInfo("Cannot find sample at idx [{}]", pItem->itemindex);
//	}
//
//	if (isValid)
//	{
//		SomeNodes<DWORD>::ListNodeAppend(&pTag->items, &pItem->listnode);
//		pItem->valid = true;
//		pTag->idxdata->ClearCurrentSample();
//		return 0x401E75; // return audiocache item
//	}
//
//	return 0x401CFA; //ret null;
//}

ASMJIT_PATCH(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if (auto pData = LooseAudioCacheManager::FindByIndexPtr(idxSample)) {

		auto sampleData = pData->GetAudioSampleData();

		if (sampleData->SampleRate) {
			std::memcpy(pAudioSample, sampleData, sizeof(AudioSampleData));
		}
		else
		{
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

	return 0x0;
}

//ASMJIT_PATCH(0x750E4A, VocCClass_Play_DebugMem, 0x6)
//{
//	GET(int, _IDX, EBP);
//	Debug::LogInfo("Playing Audio at idx {}", _IDX);
//	return 0x0;
//}