#pragma once

struct AutosaveClass
{
	static char save_filename[32];
	static wchar_t save_description[32];
	static int NextAutosaveFrame;
	static int NextAutosaveNumber;

	int AutoSaveCount { 5 };
	int AutoSaveInterval { 7200 };
	struct External {
		int AutoSaveInterval { 0 };
	};

	static External Config;

	void AI(bool SpawnerActive);

	void Save(bool SpawnerActive);
};