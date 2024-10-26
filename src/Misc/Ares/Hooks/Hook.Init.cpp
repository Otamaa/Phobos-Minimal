#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <ParticleClass.h>

DEFINE_HOOK(0x52BA78, _YR_GameInit_Pre, 5) {
	TheaterTypeClass::LoadAllTheatersToArray();
	return 0;
}