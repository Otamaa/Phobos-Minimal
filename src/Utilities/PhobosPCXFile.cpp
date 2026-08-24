#include "PhobosPCXFile.h"
#include <PCX.h>

#include "SavegameDef.h"
#include "Debug.h"

#include <CCINIClass.h>
#include <Utilities/Swizzle.h>

#include <vector>
#include <stack>
#include <memory>
#include <mutex>

PhobosPCXFile::PhobosPCXFile() : Surface(nullptr), filename() {}

PhobosPCXFile::PhobosPCXFile(const char* pFilename) : PhobosPCXFile()
{
	this->Insert(pFilename);
}

void PhobosPCXFile::Erase()
{
	this->Surface = nullptr;
}

PhobosPCXFile& PhobosPCXFile::AssignNoCheck(const char* pFilename)
{
	this->filename = pFilename;

	BSurface* pSource = PCXImages::Instance->GetSurface(this->filename);
	if (!pSource && PCXImages::Instance->ForceLoadFile(this->filename))
		pSource = PCXImages::Instance->GetSurface(this->filename);

	this->Surface = pSource;

	return *this;
}

PhobosPCXFile& PhobosPCXFile::Assign(const char* pFilename)
{
	// fucker
	if (!pFilename || !*pFilename || !strlen(pFilename)) {
		this->Clear();
		return *this;
	}

	this->filename = pFilename;

	BSurface* pSource = PCXImages::Instance->GetSurface(this->filename);
	if (!pSource && PCXImages::Instance->ForceLoadFile(this->filename))
		pSource = PCXImages::Instance->GetSurface(this->filename);

	this->Surface = pSource;

	return *this;
}

void PhobosPCXFile::Insert(const char* pFilename)
{
	std::string cachedWithExt = pFilename;

	if (!cachedWithExt.empty() && cachedWithExt[0]) {

		this->Assign(_strlwr(cachedWithExt.data()));
	}
}

bool PhobosPCXFile::Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault)
{
	char buffer[Capacity];
	if (pINI->ReadString(pSection, pKey, pDefault, buffer) > 0)
	{
		std::string cachedWithExt = _strlwr(buffer);

		this->Assign(cachedWithExt.c_str());

		if (this->filename && !this->Surface) {
			Debug::INIParseFailed(pSection, pKey, this->filename.c_str(), "PCX file not found.");
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
		if (!this->Surface) {
			Debug::LogInfo("PCX file[{}] not found.", this->filename.data());
		}

		std::string _reg = "BSurface ";
		_reg += this->filename.data();

		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, this->Surface, _reg.c_str());

		BSurface* pSource = PCXImages::Instance->GetSurface(this->filename);
		if (!pSource && PCXImages::Instance->ForceLoadFile(this->filename))
			pSource = PCXImages::Instance->GetSurface(this->filename, nullptr);

		this->Surface = pSource;
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