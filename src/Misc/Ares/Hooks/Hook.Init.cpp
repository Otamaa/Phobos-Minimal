#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <ParticleClass.h>

ASMJIT_PATCH(0x52BA78, _YR_GameInit_Pre, 5) {
	TheaterTypeClass::Array.clear();
	TheaterTypeClass::AddDefaults();
	TheaterTypeClass::LoadAllTheatersToArray();
	return 0;
}