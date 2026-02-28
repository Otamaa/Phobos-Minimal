#pragma once

#include <optional>
#include <CStreamClass.h>

struct SavedGames
{
	static int HowManyTimesISavedForThisScenario;
	static bool CreateSubdir();
	static char* FormatPath(const char* pFileName);


	template<typename T> requires std::is_trivially_copyable_v<T>
	static inline std::optional<T> ReadFromStorage(IStorage* pStorage)
	{
		IStreamPtr pStream = nullptr;
		bool hasValue = false;
		HRESULT hr = pStorage->OpenStream(
			T::SaveName,
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			&pStream
		);

		T info {};

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			ULONG read = 0;
			hr = pStream->Read(&info, sizeof(info), &read);
			hasValue = SUCCEEDED(hr) && read == sizeof(info);
		}

		return hasValue ? std::make_optional(info) : std::nullopt;
	}

	template<typename T> requires std::is_trivially_copyable_v<T>
	static inline bool WriteToStorage(IStorage* pStorage)
	{
		IStreamPtr pStream = nullptr;
		bool ret = false;
		HRESULT hr = pStorage->CreateStream(
			T::SaveName,
			STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
			0,
			0,
			&pStream
		);

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			T info {};
			ULONG written = 0;
			hr = pStream->Write(&info, sizeof(info), &written);
			ret = SUCCEEDED(hr) && written == sizeof(info);
		}

		return ret;
	}
};

struct CustomMissionID
{
	static constexpr const wchar_t* SaveName = L"CustomMissionID";

	int Number;

	CustomMissionID();
	CustomMissionID(int num) : Number { num } { }

	operator int() const { return Number; }
};
