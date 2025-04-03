#pragma once

#include <PCX.h>
#include <Helpers/String.h>

#include <string>

class BSurface;
class INIClass;
class PhobosStreamReader;
class PhobosStreamWriter;
// pcx filename storage with optional automatic loading
class PhobosPCXFile
{
	static COMPILETIMEEVAL const size_t Capacity = 0x20;
public:
	explicit PhobosPCXFile() : Surface(nullptr), filename() { }

	PhobosPCXFile(const char* pFilename) : PhobosPCXFile()
	{
		*this = pFilename;
	}

	~PhobosPCXFile() = default;

	PhobosPCXFile(const PhobosPCXFile& other) = default;
	PhobosPCXFile& operator=(const PhobosPCXFile& other) = default;

	PhobosPCXFile& operator=(const char* pFilename)
	{

		// fucker
		if (!pFilename || !*pFilename || !strlen(pFilename))
		{
			this->Clear();
			return *this;
		}

		this->filename = pFilename;
		auto& data = this->filename.data();
		_strlwr_s(data);

		BSurface* pSource = PCX::Instance->GetSurface(this->filename);
		if (!pSource && PCX::Instance->LoadFile(this->filename))
			pSource = PCX::Instance->GetSurface(this->filename);

		this->Surface = pSource;

		return *this;
	}

	PhobosPCXFile& operator=(std::string& pFilename)
	{

		// fucker
		if (pFilename.empty() || !*pFilename.data())
		{
			this->Clear();
			return *this;
		}

		this->filename = pFilename.c_str();
		auto& data = this->filename.data();
		_strlwr_s(data);

		BSurface* pSource = PCX::Instance->GetSurface(this->filename);
		if (!pSource && PCX::Instance->ForceLoadFile(this->filename, 2, 0))
			pSource = PCX::Instance->GetSurface(this->filename);

		this->Surface = pSource;

		return *this;
	}

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

private:

	void Clear()
	{
		this->Surface = nullptr;
		this->filename = nullptr;
	}

	BSurface* Surface { nullptr };
	FixedString<Capacity> filename;
};
