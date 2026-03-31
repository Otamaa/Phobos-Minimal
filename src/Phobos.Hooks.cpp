#include "Phobos.h"
#include "Phobos.COM.h"
#include "Phobos.Ext.h"

#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/AdvancedDriveLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>
#include <New/Interfaces/TSJumpJetLocomotionClass.h>
//#include <New/Interfaces/AttachmentLocomotionClass.h>

ASMJIT_PATCH(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	// Add new classes to be COM-registered below
	//RegisterFactoryForClass<LevitateLocomotionClass>();
	//RegisterFactoryForClass<TSJumpJetLocomotionClass>();
	PhobosCOM::RegisterFactoryForClass<AdvancedDriveLocomotionClass>();
	//RegisterFactoryForClass<CustomRocketLocomotionClass>();
	//RegisterFactoryForClass<AttachmentLocomotionClass>();

	Debug::Log("COM registration done!\n");

	return 0;
}

ASMJIT_PATCH(0x52FE55, Scenario_Start, 0x6)
{
	PhobosExt::EnsureSeeded((DWORD)Game::Seed());
	return 0;
}ASMJIT_PATCH_AGAIN(0x52FEB7, Scenario_Start, 0x6)

ASMJIT_PATCH(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	Phobos::ClearAll();
	return 0;
}

ASMJIT_PATCH(0x6BBE6A, WinMain_AllowMultipleInstances, 0x6)
{
	return Phobos::Otamaa::AllowMultipleInstance ? 0x6BBED6 : 0x0;
}

ASMJIT_PATCH(0x7258DE, AnnounceInvalidPointer_PhobosGlobal, 0x7)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);
	GET(AbstractType, type, EAX);

	if (Phobos::Otamaa::ExeTerminated)
		return 0;

	PhobosExt::InvalidatePointers(pInvalid, removed, type);
	return 0;
}

ASMJIT_PATCH(0x55DBCD, MainLoop_SaveGame, 0x6)
{
	// This happens right before LogicClass::Update()
	enum { SkipSave = 0x55DC99, InitialSave = 0x55DBE6 };

	if (SessionClass::IsSingleplayer() && !ScenarioClass::ScenarioSaved())
	{
		ScenarioClass::ScenarioSaved = true;
		if (Phobos::ShouldQuickSave)
		{
			Phobos::PassiveSaveGame();
			Phobos::ShouldQuickSave = false;
			Phobos::CustomGameSaveDescription.clear();
		}
		else if (Phobos::Config::SaveGameOnScenarioStart && SessionClass::IsCampaign())
		{
			Debug::Log("Saving Game [Filename : %s , UI : %s , LoadedUI : %ls]",
			ScenarioClass::Instance->FileName,
			ScenarioClass::Instance->UIName,
			ScenarioClass::Instance->UINameLoaded
			);
			return InitialSave;
		}
	}

	return SkipSave;
}

