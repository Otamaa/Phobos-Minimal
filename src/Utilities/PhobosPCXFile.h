#pragma once

#include <PCX.h>
#include <Helpers/String.h>

#include <string>
#include "PhobosMap.h"

class BSurface;
class INIClass;
class PhobosStreamReader;
class PhobosStreamWriter;
// pcx filename storage with optional automatic loading
class PhobosPCXFile
{
	static COMPILETIMEEVAL const size_t Capacity = 0x20;

public:
	struct GlobalMarker
	{
		BSurface* surface;
		bool logged;
	};

	static std::map<std::string, GlobalMarker> LoadedMap;

	explicit PhobosPCXFile() : Surface(nullptr), filename() { }

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

	COMPILETIMEEVAL bool Exists() const
	{
		return this->Surface;
	}

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "");
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	void Erase();

private:

	void Clear() {
		this->Surface = nullptr;
		this->filename = nullptr;
	}

	PhobosPCXFile& Assign(const char* pFilename);
	BSurface* Surface { nullptr };
	FixedString<Capacity> filename;
};