#include <AircraftClass.h>
#include <AircraftTypeClass.h>

#include <Phobos.h>

/*
		An experimental code  , recovered from GScript.ext (dll) from the `TC2` mod
		Since it based on Phobos , they suppose to share the dll source code as per License says
		instead they hiding it behind new names , and re-using Phobos member codes
		without following the GPL license

		so  , i guess i need to brakeforce like what i did with ares :p

		This codes is here in purpose to understand the internal working of the
		`TC2` bootleg `Phobos.dll`.
*/

DEFINE_HOOK(0x4179f7, AircraftClass_AssumeTaskComplete_DontCrash, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Type->Spawned || pThis->Type->Carryall)
		return 0x0;

	pThis->SetDestination(nullptr, true);
	return 0x417B69;
}