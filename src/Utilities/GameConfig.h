#pragma once

#include "GameUniquePointers.h"
#include "LambdaFunctionArgCount.h"

#include <CCINIClass.h>

struct GameConfig
{

	GameConfig(const char* pFilename) noexcept : File { nullptr }
		, Ini { nullptr }
	{
		this->File.reset(GameCreate<CCFileClass>(pFilename));
	}

	~GameConfig() noexcept = default;

	GameConfig(GameConfig&&) noexcept = default;
	GameConfig& operator=(GameConfig&&) noexcept = default;
	GameConfig(const GameConfig&) = delete;
	GameConfig& operator=(const GameConfig&) = delete;

	bool OpenINI(FileAccessMode mode = FileAccessMode::Read) noexcept;

	template <typename Func>
	COMPILETIMEEVAL void OpenINIAction(Func&& action, FileAccessMode mode = FileAccessMode::Read) noexcept
	{
		if (this->OpenINI(mode)) {
			if COMPILETIMEEVAL (lambda_details<decltype(action)>::argument_count == 1)
				action(Ini.get());
			else if COMPILETIMEEVAL (lambda_details<decltype(action)>::argument_count == 2)
				action(Ini.get(), File.get());
			else
				static_assert(true, "fail!");
		}
	}

	template <typename Func>
	COMPILETIMEEVAL void OpenOrCreateAction(Func&& action, FileAccessMode mode = FileAccessMode::ReadWrite) noexcept
	{
		if(this->OpenOrCreate(mode)){
			if COMPILETIMEEVAL (lambda_details<decltype(action)>::argument_count == 1)
				action(Ini.get());
			else if COMPILETIMEEVAL (lambda_details<decltype(action)>::argument_count == 2)
				action(Ini.get(), File.get());
			else
				static_assert(true, "fail!");
		}
	}

	bool OpenOrCreate(FileAccessMode mode = FileAccessMode::ReadWrite) noexcept;

	FORCEDINLINE void WriteCCFile()
	{
		Ini->WriteCCFile(File.get());
	}

	FORCEDINLINE const char* filename() noexcept
	{
		return File->FileName;
	}

	FORCEDINLINE CCINIClass* get() noexcept
	{
		return Ini.get();
	}

	FORCEDINLINE CCINIClass* operator->() noexcept
	{
		return Ini.get();
	}

protected:
	UniqueGamePtr<CCFileClass> File;
	UniqueGamePtr<CCINIClass> Ini;
};