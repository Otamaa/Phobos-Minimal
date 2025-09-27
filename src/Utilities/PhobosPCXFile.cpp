#include "PhobosPCXFile.h"

#include "SavegameDef.h"
#include "Debug.h"

#include <CCINIClass.h>
#include <Utilities/Swizzle.h>

#include <vector>
#include <stack>
#include <memory>
#include <mutex>

class BSurfacePool
{
private:
	std::vector<std::unique_ptr<BSurface>> allSurfaces;
	std::stack<BSurface*> available;
	std::mutex poolMutex;

public:

	BSurface* Acquire()
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		if (!available.empty())
		{
			BSurface* surf = available.top();
			available.pop();
			return surf;
		}

		// Create a new one if pool is empty
		auto newSurface = std::make_unique<BSurface>();
		BSurface* ptr = newSurface.get();
		allSurfaces.push_back(std::move(newSurface));
		return ptr;
	}

	void Release(BSurface* surface)
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		surface->Clear();
		available.push(surface);
	}

	void ClearPool()
	{
		std::lock_guard<std::mutex> lock(poolMutex);
		available = std::stack<BSurface*>(); // Clear the stack
		allSurfaces.clear();
	}
};

std::map<std::string, PhobosPCXFile::GlobalMarker> PhobosPCXFile::LoadedMap;

PhobosPCXFile::PhobosPCXFile(const char* pFilename) : PhobosPCXFile()
{
	this->Insert(pFilename);
}

void PhobosPCXFile::Erase()
{
	auto& marker = LoadedMap[this->filename.data()];
	marker.logged = false;
	marker.surface = nullptr;
	this->Surface = nullptr;
}

PhobosPCXFile& PhobosPCXFile::Assign(const char* pFilename)
{
	// fucker
	if (!pFilename || !*pFilename || !strlen(pFilename)) {
		this->Clear();
		return *this;
	}

	this->filename = pFilename;

	BSurface* pSource = PCX::Instance->GetSurface(this->filename);
	if (!pSource && PCX::Instance->LoadFile(this->filename))
		pSource = PCX::Instance->GetSurface(this->filename);

	this->Surface = pSource;

	return *this;
}

void PhobosPCXFile::Insert(const char* pFilename)
{
	std::string cachedWithExt = pFilename;

	if (!cachedWithExt.empty() && cachedWithExt[0]) {
		_strlwr(cachedWithExt.data());

		if (cachedWithExt.find(".pcx") == std::string::npos)
			cachedWithExt += ".pcx";

		auto& iter = LoadedMap[cachedWithExt];

		if (!iter.surface) {
			this->Assign(cachedWithExt.c_str());
			iter.surface = this->Surface;
		} else {
			this->Surface = iter.surface;
		}
	}
}

bool PhobosPCXFile::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	char buffer[Capacity];
	if (pINI->ReadString(pSection, pKey, pDefault, buffer) > 0)
	{
		std::string cachedWithExt = _strlwr(buffer);

		if (cachedWithExt.find(".pcx") == std::string::npos)
			cachedWithExt += ".pcx";

		auto& iter = LoadedMap[cachedWithExt];

		if (!iter.surface) {
			this->Assign(cachedWithExt.c_str());
			iter.surface  = this->Surface;

			if (!iter.logged && this->filename && !this->Surface) {
				iter.logged = true;
				Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
			}

		} else {
			this->Surface = iter.surface;
		}

	}

	return buffer[0] != 0;
}


bool PhobosPCXFile::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{

	this->Clear();
	long oldPtr = 0l;

	if (!Stm.Load(oldPtr))
		return false;

	if (!Stm.Process(this->filename))
		return false;

	if (oldPtr && this->filename)
	{
		auto& iter = LoadedMap[this->filename.data()];

		if(!iter.surface){

			BSurface* pSource = PCX::Instance->GetSurface(this->filename);
			if (!pSource && PCX::Instance->LoadFile(this->filename))
				pSource = PCX::Instance->GetSurface(this->filename, nullptr);

			this->Surface = pSource;
			iter.surface = pSource;

			if (!iter.logged && !this->Surface) {
				iter.logged = true;
				Debug::LogInfo("PCX file[{}] not found.", this->filename.data());
			}

			PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr,  this->Surface, "BSurface")
		} else {
			this->Surface = iter.surface;
		}
	}

	return true;
}

bool PhobosPCXFile::Save(PhobosStreamWriter& Stm) const
{
	if(!Stm.Save((long)this->Surface))
		return false;

	return Stm
		.Process(this->filename)
		.Success()
		;
}