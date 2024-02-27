#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>
#include <TriggerTypeClass.h>
#include <IsometricTileTypeClass.h>
#include <HouseClass.h>
#include <Utilities/Debug.h>


DEFINE_HOOK(0x547043, IsometricTileTypeClass_ReadFromFile, 0x6)
{
	GET(int, FileSize, EBX);
	GET(IsometricTileTypeClass*, pTileType, ESI);

	if (FileSize == 0)
	{
		if (strlen(pTileType->ID) > 9)
		{
			Debug::FatalErrorAndExit("Maximum allowed length for tile names, excluding the extension, is 9 characters.\n"
					"The tileset using filename '%s' exceeds this limit - the game cannot proceed.", pTileType->ID);
		}
		else
		{
			Debug::FatalErrorAndExit("The tileset '%s' contains a file that could not be loaded for some reason - make sure the file exists.", pTileType->ID);
		}
	}

	return 0;
}

DEFINE_HOOK(0x41088D, AbstractTypeClass_CTOR_IDTooLong, 0x6)
{
	GET(const char*, ID, EAX);

	if (strlen(ID) > 25)
		Debug::FatalErrorAndExit("Tried to create a type with ID '%s' which is longer than the maximum length of 24 .", ID);

	return 0;
}

DEFINE_HOOK(0x7272B5, TriggerTypeClass_LoadFromINI_House, 6)
{
	GET(int const, index, EAX);
	GET(TriggerTypeClass* const, pTrig, EBP);
	GET(const char*, pHouse, ESI);

	if (index < 0) {
		Debug::FatalError("TriggerType '%s' refers to a house named '%s', which does not exist. In case no house is needed, use '<none>' explicitly.", pTrig->ID, pHouse);
		R->EDX<HouseTypeClass*>(nullptr);
	} else {
		R->EDX<HouseTypeClass*>(HouseTypeClass::Array->Items[index]);
	}

	return 0x7272C1;
}

static bool CounterLog = false;
DEFINE_HOOK(0x749088, FixedWidthCounter_ResetWithGivenCount, 6)
{
	GET(unsigned int, Width, EAX);

	if (Width > 512) {

		if(CounterLog)
			Debug::Log("Counter attempted to overflow (given width of %d exceeds maximum allowed width of 512).\n", Width);

		R->EAX(512);
	}

	return 0;
}