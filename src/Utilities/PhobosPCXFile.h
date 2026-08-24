#pragma once

#include <Helpers/String.h>

class BSurface;
class INIClass;
class PhobosStreamReader;
class PhobosStreamWriter;
// pcx filename storage with optional automatic loading
class PhobosPCXFile
{
	static COMPILETIMEEVAL const size_t Capacity = 0x20;

public:

	explicit PhobosPCXFile();

	PhobosPCXFile(const char* pFilename);

	~PhobosPCXFile() = default;

	PhobosPCXFile(const PhobosPCXFile& other) = default;
	PhobosPCXFile& operator=(const PhobosPCXFile& other) = default;

	PhobosPCXFile& operator=(const char* pFilename) = delete;
	PhobosPCXFile& operator=(std::string& pFilename) = delete;

	void Insert(const char* pFilename);

	const char* GetFilename() const
	{
		return this->filename.data();
	}

	COMPILETIMEEVAL BSurface* GetSurface() const
	{
		return this->Surface;
	}

	COMPILETIMEEVAL void SetSurface(BSurface* pSurface)
	{
		this->Surface = pSurface;
	}

	COMPILETIMEEVAL bool Exists() const
	{
		return this->Surface;
	}

	COMPILETIMEEVAL auto GetCachedFilename() const {
		return this->filename.c_str();
	}

	COMPILETIMEEVAL auto IsCachedFilenameValid() const {
		return !this->filename.empty();
	}

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "");
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	void Erase();
	

	void Clear() {
		this->Surface = nullptr;
		this->filename = nullptr;
	}

	PhobosPCXFile& AssignNoCheck(const char* pFilename);

private:

	PhobosPCXFile& Assign(const char* pFilename);

	BSurface* Surface;
	FixedString<Capacity> filename;
};