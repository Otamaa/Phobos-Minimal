#pragma once

#include "GameUniquePointers.h"
#include <CCINIClass.h>

struct GameConfig
{

	GameConfig(const char* pFilename) noexcept : File { nullptr }
		, Ini { nullptr }
	{
		this->File.reset(GameCreate<CCFileClass>(pFilename));
	}

	~GameConfig() noexcept = default;

	bool OpenINI(FileAccessMode mode = FileAccessMode::Read) noexcept;

	template <typename Func>
	void OpenINIAction(Func&& action, FileAccessMode mode = FileAccessMode::Read) noexcept
	{
		this->OpenINI(mode);
		action(Ini.get());
	}

	bool OpenOrCreate(FileAccessMode mode = FileAccessMode::ReadWrite) noexcept;

	FORCEINLINE void WriteCCFile()
	{
		Ini->WriteCCFile(File.get());
	}

	constexpr const char* filename() noexcept
	{
		return File->FileName;
	}

	constexpr CCINIClass* get() noexcept
	{
		return Ini.get();
	}

	constexpr CCINIClass* operator->() noexcept
	{
		return Ini.get();
	}

protected:
	UniqueGamePtr<CCFileClass> File;
	UniqueGamePtr<CCINIClass> Ini;
};