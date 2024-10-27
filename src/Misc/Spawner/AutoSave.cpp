#include "AutoSave.h"

#include <SessionClass.h>
#include <Unsorted.h>
#include <RulesClass.h>
#include <ScenarioClass.h>

char AutosaveClass::save_filename[32];
wchar_t AutosaveClass::save_description[32];
int AutosaveClass::NextAutosaveFrame;
int AutosaveClass::NextAutosaveNumber;
AutosaveClass::External AutosaveClass::Config;

void Print_Saving_Game_Message()
{
	const int message_delay = int(RulesClass::Instance->MessageDelay * TICKS_PER_MINUTE);
	MessageListClass::Instance->AddMessage(nullptr, 0, L"Saving game...", 4, TextPrintType::Point6Grad | TextPrintType::UseGradPal | TextPrintType::FullShadow, message_delay, SessionClass::IsSingleplayer());
}

void AutosaveClass::AI(bool SpawnerActive)
{
	NextAutosaveFrame = Unsorted::CurrentFrame;
	NextAutosaveFrame += (SpawnerActive && SessionClass::Instance->GameMode == GameMode::LAN) ? Config.AutoSaveInterval : 0;
}

void AutosaveClass::Save(bool SpawnerActive)
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		if (this->AutoSaveCount > 0)
		{
			if (Unsorted::CurrentFrame == NextAutosaveFrame - 1)
			{
				Print_Saving_Game_Message();
			}

			if (Unsorted::CurrentFrame >= NextAutosaveFrame)
			{
				sprintf_s(save_filename, "AUTOSAVE%d.SAV", NextAutosaveNumber);
				_swprintf(save_description, L"Mission Auto-Save (Slot %d)", NextAutosaveNumber);

				ScenarioClass::PauseGame();
				Game::CallBack();
				ScenarioClass::SaveGame(save_filename, save_description);
				ScenarioClass::ResumeGame();

				NextAutosaveNumber++;
				if (NextAutosaveNumber > this->AutoSaveCount)
				{
					NextAutosaveNumber = 1;
				}

				NextAutosaveFrame = Unsorted::CurrentFrame + this->AutoSaveInterval;
			}
		}
	}
	else if (SpawnerActive && SessionClass::Instance->GameMode == GameMode::LAN && Config.AutoSaveInterval > 0)
	{

		if (Unsorted::CurrentFrame == NextAutosaveFrame - 1)
		{
			Print_Saving_Game_Message();
		}

		if (Unsorted::CurrentFrame >= NextAutosaveFrame)
		{

			ScenarioClass::SaveGame("SAVEGAME.NET", L"Multiplayer Game");
			NextAutosaveFrame = Unsorted::CurrentFrame + Config.AutoSaveInterval;

		}
	}
}