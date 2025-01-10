#pragma once

#include "GameUniquePointers.h"
#include <CCINIClass.h>
#include <Utilities/LambdaFunctionArgCount.h>

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
		if (this->OpenINI(mode)) {
			if constexpr (lambda_details<decltype(action)>::argument_count == 1)
				action(Ini.get());
			else if constexpr (lambda_details<decltype(action)>::argument_count == 2)
				action(Ini.get(), File.get());
			else
				static_assert(true, "fail!");
		}
	}

	template <typename Func>
	void OpenOrCreateAction(Func&& action, FileAccessMode mode = FileAccessMode::ReadWrite) noexcept
	{
		if(this->OpenOrCreate(mode)){
			if constexpr (lambda_details<decltype(action)>::argument_count == 1)
				action(Ini.get());
			else if constexpr (lambda_details<decltype(action)>::argument_count == 2)
				action(Ini.get(), File.get());
			else
				static_assert(true, "fail!");
		}
	}

	bool OpenOrCreate(FileAccessMode mode = FileAccessMode::ReadWrite) noexcept;

	FORCEINLINE void WriteCCFile()
	{
		Ini->WriteCCFile(File.get());
	}

	FORCEINLINE constexpr const char* filename() noexcept
	{
		return File->FileName;
	}

	FORCEINLINE constexpr CCINIClass* get() noexcept
	{
		return Ini.get();
	}

	FORCEINLINE constexpr CCINIClass* operator->() noexcept
	{
		return Ini.get();
	}

protected:
	UniqueGamePtr<CCFileClass> File;
	UniqueGamePtr<CCINIClass> Ini;
};