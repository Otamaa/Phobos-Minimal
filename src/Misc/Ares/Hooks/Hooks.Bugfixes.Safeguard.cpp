#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>
#include <TriggerTypeClass.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

DEFINE_OVERRIDE_HOOK(0x41088D, AbstractTypeClass_CTOR_IDTooLong, 0x6)
{
	GET(const char*, ID, EAX);

	if (strlen(ID) > 25)
		Debug::FatalErrorAndExit("Tried to create a type with ID '%s' which is longer than the maximum length of 24 .", ID);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7272B5, TriggerTypeClass_LoadFromINI_House, 6)
{
	GET(int const, index, EAX);
	GET(TriggerTypeClass* const, pTrig, EBP);
	GET(const char*, pHouse, ESI);

	if (index < 0) {
		Debug::FatalError("TriggerType '%s' refers to a house named '%s', which does not exist. In case no house is needed, use '<none>' explicitly.", pTrig->ID, pHouse);
		R->EDX<HouseTypeClass*>(nullptr);
	} else {
		R->EDX<HouseTypeClass*>(HouseTypeClass::Array->GetItem(index));
	}

	return 0x7272C1;
}