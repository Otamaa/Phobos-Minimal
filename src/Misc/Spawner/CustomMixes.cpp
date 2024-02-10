#include <MixFileClass.h>

#include<Utilities/GameUniquePointers.h>

#include "Main.h"

static UniqueGamePtrB<MixFileClass> CnCnetMix;

void SpawnerMain::InitMixes()
{
	CnCnetMix.reset(GameCreate<MixFileClass>("cncnet.mix"));
}
