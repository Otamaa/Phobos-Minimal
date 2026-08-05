// Allows WAV files being placed in Mixes
#include "Audio.h"

#include <CCFileClass.h>
#include <VocClass.h>
#include <Phobos.h>

#include <Ext/HouseType/Body.h>
#include <SessionClass.h>

#include <mutex>
#include <shared_mutex>
#include <Memory.h>
#include <Audio.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

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
	CCFileClass* FileHandle { nullptr };
};

class LooseAudioCache
{
public:
	LooseAudioCache(const char* Title)
		: Name(Title), WavName(Title), Data {}, IsFileNotAvaible { false }
	{
		WavName.reserve(Name.length() + 6);  // Pre-allocate
		WavName = Name + ".wav";
	}

	LooseAudioCache(const LooseAudioCache&) = delete;
	LooseAudioCache& operator=(const LooseAudioCache&) = delete;
	LooseAudioCache(LooseAudioCache&&) = delete;
	LooseAudioCache& operator=(LooseAudioCache&&) = delete;

	~LooseAudioCache()
	{
		// Clean up the cached file handle if any
		if (Data.FileHandle)
			GameDelete<true, false>(Data.FileHandle);
	}

	// Opens the WAV file and parses its header into `Data` if not already
	// cached. Locked: the read-check (`Data.Size < 0`) and the subsequent
	// writes to `Data.Size`/`Data.Offset`/`Data.Data` happen atomically
	// with respect to other callers on this same LooseAudioCache instance.
	FileStruct GetFileStruct()
	{
		std::lock_guard<std::mutex> lock(EntryMutex);
		return GetFileStructLocked();
	}

	// Returns a pointer to this entry's cached AudioSampleData, loading it
	// first if necessary. Locked for the same reason as GetFileStruct().
	//
	// NOTE: the returned pointer aliases `Data.Data`, which lives for the
	// lifetime of the LooseAudioCache (never destroyed/reallocated), so it
	// remains valid after the lock is released -- the CALLER's subsequent
	// memcpy from this pointer is safe PROVIDED no other thread is
	// CONCURRENTLY calling GetAudioSampleData()/GetFileStruct() on this
	// same entry and re-writing `Data.Data` mid-copy.
	//
	// If that residual race matters for your engine's calling pattern
	// (e.g. the audio thread re-validates/reloads samples while another
	// thread is mid-memcpy), see the alternative `CopySampleDataInto()`
	// below, which does the memcpy WHILE HOLDING the lock.
	AudioSampleData* GetAudioSampleData()
	{
		std::lock_guard<std::mutex> lock(EntryMutex);

		if (Data.Size < 0)
		{
			// Attempt to parse the WAV header
			if (!GetFileStructLocked().Allocated)   // Allocated = file opened successfully 
			{
				if (Phobos::Otamaa::IsAdmin)
					Debug::Log("LooseAudioCache: Failed to parse WAV file: %s\n", WavName.c_str());
				// FileStructLocked already tried to open; if it failed, Data.Size stays -1
			}
		}
		return &Data.Data;
	}

	// Preferred entry point for 0x401640 (AudioIndex_GetSampleInformation):
	// loads (if needed) AND copies the sample data into `out` while holding
	// the lock for the ENTIRE operation, eliminating the residual
	// "valid pointer but data changes mid-memcpy" race described above.
	//
	// Returns true if `out->SampleRate` is non-zero after the copy (i.e.
	// a real cached sample was found), matching the original hook's check.
	bool CopySampleDataInto(AudioSampleData* out)
	{
		std::lock_guard<std::mutex> lock(EntryMutex);

		if (Data.Size < 0)
		{
			auto file = GetFileStructLocked();
			if (!file.Allocated)
			{
				if (Phobos::Otamaa::IsAdmin)
					Debug::Log("LooseAudioCache: Failed to parse WAV file: %s\n", WavName.c_str());
			}
		}

		if (Data.Data.SampleRate)
		{
			std::memcpy(out, &Data.Data, sizeof(AudioSampleData));
			return true;
		}
		return false;
	}

	const std::string& GetName() const { return Name; }

private:
	// Must be called with EntryMutex held.
	FileStruct GetFileStructLocked()
	{
		// If we already have a cached file handle and the data is valid, reuse it
		if (Data.Size >= 0 && Data.FileHandle) {
			return { Data.Size, Data.Offset, Data.FileHandle, false }; // false = not newly allocated
		}

		if(IsFileNotAvaible) { //the file one parsed and not found  , just bail
			return { -1, -1, nullptr, false };
		}

		// First time: open and parse the WAV
		auto pFile = GameCreate<CCFileClass>(WavName.c_str());

		if (!pFile->IsAvaible()) {
			if (Phobos::Otamaa::IsAdmin){
				Debug::Log("LooseAudioCache: File does not exist: %s\n", WavName.c_str());
			}

			IsFileNotAvaible = true;

			GameDelete<true, false>(pFile);
			return { -1, -1, nullptr, false };
		}

		if (!pFile->Open1(FileAccessMode::Read))
		{
			GameDelete<true, false>(pFile);
			return { -1, -1, nullptr, false };
		}

		// Parse WAV header
		if (!Audio::ReadWAVFile(pFile, &Data.Data, &Data.Size))
		{
			if (Phobos::Otamaa::IsAdmin)
				Debug::Log("LooseAudioCache: Failed to parse WAV header: %s\n", WavName.c_str());
			pFile->Close();
			GameDelete<true, false>(pFile);
			return { -1, -1, nullptr, false };
		}

		// Record the data start position and keep the file open
		Data.Offset = pFile->Seek(0, FileSeekMode::Current);

		// Sanity check: offset should be within the file
		if (Data.Offset <= 0 || Data.Size <= 0)
		{
			Debug::Log("LooseAudioCache: Invalid WAV data offset/size (%d/%d) for %s\n",
				Data.Offset, Data.Size, WavName.c_str());
			pFile->Close();
			GameDelete<true, false>(pFile);
			Data.Size = -1;
			return { -1, -1, nullptr, false };
		}

		// Store the open file handle for future streaming
		Data.FileHandle = pFile;

		if (Phobos::Otamaa::IsAdmin)
			Debug::Log("LooseAudioCache: successfully opened and cached WAV: %s\n", WavName.c_str());

		return { Data.Size, Data.Offset, Data.FileHandle, false }; // not newly allocated in the caller's sense
	}
	
	std::string Name;
	std::string WavName;
	LooseAudioFile Data;
	std::mutex EntryMutex;
	bool IsFileNotAvaible;
};

class LooseAudioCacheManager
{
	static std::vector<std::unique_ptr<LooseAudioCache>> Array;
	static std::mutex arrayMutex;

public:

	static int NameToIndex(const char* Title)
	{
		std::lock_guard<std::mutex> lock(arrayMutex);  // Lock here

		const auto it = std::ranges::find_if(Array, [&](const auto& ptr) {
			return ptr->GetName() == Title;
		});

		if (it == Array.end())
		{
			Array.emplace_back((std::make_unique<LooseAudioCache>(Title)));
			return (int)Array.back()->GetName().c_str(); // fuckers
		}

		return  (int)it->get()->GetName().c_str();
	}

	static LooseAudioCache* FindByIndexPtr(UINT_PTR idxptr)
	{
		std::lock_guard<std::mutex> lock(arrayMutex);  // Lock here

		if (idxptr >= 0x10000)
		{
			const auto it = std::ranges::find_if(Array, [&](const auto& ptr)	{
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
			if(this->Open(pFilename) && Phobos::Otamaa::IsAdmin)
				Debug::LogInfo("Opening AudioBag {}" , pFilename);
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

			if (pIndex.IsAvaible() && pIndex.Open1(FileAccessMode::Read))
			{
				filename[filebase_len + 1] = 'b';
				filename[filebase_len + 2] = 'a';
				filename[filebase_len + 3] = 'g';

				if(Phobos::Otamaa::OutputAudioLogs)
					Debug::LogInfo("Reading {}" , filename);

				auto pBag = UniqueGamePtr<CCFileClass>(GameCreateUnchecked<CCFileClass>(filename.c_str()));

				if (pBag->IsAvaible()
					&& pBag->Open1(FileAccessMode::Read))
				{
					AudioIDXHeader headerIndex {};
					if(pIndex.Read(&headerIndex, sizeof(AudioIDXHeader)) == sizeof(AudioIDXHeader))
					{
						if (Phobos::Otamaa::OutputAudioLogs) {
							Debug::LogInfo("Reading [{} from {}] file with [{}] samples!.",
								filename.c_str(), pIndex.FileName(), headerIndex.numSamples);
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
									if (pIndex.Read(&entry, readBytes) != readBytes)
										break; // handle error

									entry.ChunkSize = 0;
								}
							}
							else
							{
								const auto headerSize = headerIndex.numSamples * IdxEntrysize;
								const auto readed = pIndex.Read(&this->Entries[0], static_cast<int>(headerSize));

								if (readed != (int)headerSize)
								{
									if(Phobos::Otamaa::OutputAudioLogs)
										Debug::LogInfo("Failed Reading [{} from {}] file with [{}] samples , due to missmatch header size [readed {} vs intended {}]].",
											filename.c_str(), pIndex.FileName(), headerIndex.numSamples , readed, headerSize);
									return false;
								}
							}

							std::ranges::sort(this->Entries, std::less<>());
							//std::sort(this->Entries.begin(), this->Entries.end());
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
								file->Filename ,
								bagFileName.c_str(),
								i,
								this->Bags[i].Bag->Filename,
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

	void Append(const char* pFileBase) {
		std::lock_guard<std::mutex> lock(luggageMutex);  // Lock here
		this->Bags.emplace_back(pFileBase);
	}

	std::optional<FileStruct> GetFileStruct(int idx) {
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

	size_t TotalSampleSizes() const {
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
		pExt->Name.data(), vec[SampleIdx - 1].c_str());
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

std::mutex g_vocAddSampleMutex;

// add saple is assemble an idex then put it onto some list
ASMJIT_PATCH(0x4064A0, VocClassData_AddSample, 6) // Complete rewrite of VocClass::AddSample
{
	GET(AudioEventClassTag*, pVoc, ECX);
	GET(const char*, pSampleName, EDX);

	PHOBOS_AUDIO_THREAD_GUARD(g_vocAddSampleMutex);

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
				Debug::Log("[Developer warning] VocClass [%s] has missing sample '%s'\n", pVoc->Name, pSampleName);
				pVoc->SamplesOK = false;
			} else {
				// Set sample index or string pointer
				pVoc->SampleIndex[pVoc->NumSamples++] = idxSample;
			}
			R->EAX(1);
		}
	}

	return 0x40651E;
}

ASMJIT_PATCH(0x401640, AudioIndex_GetSampleInformation, 5)
{
	GET(const int, idxSample, EDX);
	GET_STACK(AudioSampleData*, pAudioSample, 0x4);

	if (auto pData = LooseAudioCacheManager::FindByIndexPtr(idxSample))
	{
		if (!pData->CopySampleDataInto(pAudioSample))
		{
			// No cached sample available -- fall back to default PCM format,
			// same defaults as the original implementation.
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

ASMJIT_PATCH(0x40A5B3, AudioDriverStart_AnnoyingBufferLogDisable_A, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	pAudioChannelTag->dwBufferBytes = R->EAX<int>();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::LogInfo("Sound frame size = {} bytes", pAudioChannelTag->dwBufferBytes);

	return 0x40A5C4;
}

ASMJIT_PATCH(0x40A554, AudioDriverStart_AnnoyingBufferLogDisable_B, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	LEA_STACK(DWORD*, ptr, STACK_OFFS(0x40, 0x28));
	pAudioChannelTag->soundframesize1 = R->EAX();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::LogInfo("Sound frame size = {} bytes", pAudioChannelTag->soundframesize1);

	R->EDX(R->EAX());
	R->EAX(ptr);
	return 0x40A56C;
}