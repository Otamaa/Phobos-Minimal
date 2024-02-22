#include <Phobos.h>

#include <Utilities/Constructs.h>
#include "Header.h"

class MoviesList
{
	struct Item : public MovieUnlockableInfo
	{
		std::string FilenameBuffer;
		std::string DescriptionBuffer;
		bool Unlockable = true;
		bool Unlocked = false;

		// then read all items
		void ReadFromINI(CCINIClass* const pINI)
		{
			char buffer[0x20];
			auto const pID = this->FilenameBuffer.c_str();
			if (int len = pINI->ReadString(pID, "Description", Phobos::readDefval, buffer)) {
				this->DescriptionBuffer.assign(buffer, buffer + len);
			}

			this->DiskRequired = pINI->ReadInteger(pID, "DiskRequired", this->DiskRequired);
			this->Unlockable = pINI->ReadBool(pID, "Unlockable", this->Unlockable);

			// update the pointers (required because of small string optimization
			// and vector reallocation)
			this->Filename = this->FilenameBuffer.c_str();
			this->Description = this->DescriptionBuffer.c_str();
		}
	};

public:
	static MoviesList Instance;

	bool PopulateMovieList(HWND const hWnd) const;
	void Unlock(char const* pFilename);
	void LoadListFromINI();
	void WriteToINI() const;

private:
	void AddMovie(HWND const hWnd, MovieUnlockableInfo const& movie) const;
	Item* FindMovie(const char* pFilename);

	std::vector<Item> Array;
};

MoviesList MoviesList::Instance;

bool MoviesList::PopulateMovieList(HWND const hWnd) const
{
	for (auto const& item : this->Array)
	{
		// add commons and unlocked, in order
		if (!item.Unlockable || item.Unlocked)
		{
			this->AddMovie(hWnd, item);
		}
	}

	return !this->Array.empty();
}

void MoviesList::Unlock(char const* const pFilename)
{
	if (auto const pItem = this->FindMovie(pFilename))
	{
		pItem->Unlocked = true;
	}
}

void MoviesList::LoadListFromINI()
{
	Debug::Log("Reading %s\n" , StaticVars::MovieMDINI.c_str());

	CCFileClass file { StaticVars::MovieMDINI.c_str() };

	if (file.Exists())
	{
		CCINIClass ini {};
		ini.ReadCCFile(&file);

		// create a list of all movies first
		auto const count = ini.GetKeyCount(GameStrings::Movies());
		this->Array.reserve(static_cast<size_t>(count));

		for (int i = 0; i < count; ++i)
		{
			char buffer[0x20];
			auto const pKey = ini.GetKeyName(GameStrings::Movies(), i);
			bool read = true;
			if (int len = ini.ReadString(GameStrings::Movies(), pKey, Phobos::readDefval, buffer)) {
				if (!this->FindMovie(buffer)) {
					this->Array.emplace_back().FilenameBuffer.assign(buffer, buffer + len);
				}
			}
		}

		for (auto& item : this->Array) {
			item.ReadFromINI(&ini);
		}
	}

	// load unlocked state
	for (auto& item : this->Array) {
		item.Unlocked = CCINIClass::INI_RA2MD().ReadBool("UnlockedMovies", item.Filename, item.Unlocked);
	}
}

void MoviesList::WriteToINI() const
{
	if (auto const pINI = &CCINIClass::INI_RA2MD())
	{
		for (auto& item : this->Array)
		{
			// only write if unlocked, to not reveal movie names
			if (auto const value = item.Unlockable && item.Unlocked)
			{
				pINI->WriteBool("UnlockedMovies", item.Filename, value);
			}
		}
	}
}

void MoviesList::AddMovie(HWND const hWnd, MovieUnlockableInfo const& movie) const
{
	if (movie.Filename && movie.Description)
	{
		auto const pName = StringTable::LoadString(movie.Description);
		auto const lparam = reinterpret_cast<LPARAM>(pName);
		auto const res = SendMessage(hWnd, WW_LB_ADDITEM, 0, lparam);
		if (res != -1)
		{
			auto const index = static_cast<WPARAM>(res);
			auto const data = reinterpret_cast<LPARAM>(&movie);
			SendMessage(hWnd, LB_SETITEMDATA, index, data);
		}
	}
}

MoviesList::Item* MoviesList::FindMovie(const char* const pFilename)
{
	auto const it = std::find_if(
		this->Array.begin(), this->Array.end(),
		[=](Item const& item) { return item.FilenameBuffer == pFilename; });

	return (it == this->Array.end()) ? nullptr : &*it;
}

DEFINE_OVERRIDE_HOOK(0x52C939, InitGame_MoviesList, 5)
{
	MoviesList::Instance.LoadListFromINI();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5FBF80, GameOptionsClass_UnlockMovieIfNeeded_MoviesList, 5)
{
	GET_STACK(char const* const, pMovieName, STACK_OFFS(0x0, -0x4));
	MoviesList::Instance.Unlock(pMovieName);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5FAFFB, Options_SaveToINI_MoviesList, 6)
{
	MoviesList::Instance.WriteToINI();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5FC000, GameOptionsClass_PopulateMovieList, 6)
{
	//GET(GameOptionsClass* const, pThis, ECX);
	GET_STACK(HWND const, hWnd, STACK_OFFS(0x0, -0x4));
	return MoviesList::Instance.PopulateMovieList(hWnd) ? 0x5FC11Cu : 0u;
}