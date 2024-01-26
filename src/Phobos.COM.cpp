#include <Utilities/Debug.h>

#include <Helpers/Macro.h>


DEFINE_HOOK(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	// Add new classes to be COM-registered below

	Debug::Log("COM registration done!\n");

	return 0;
}
