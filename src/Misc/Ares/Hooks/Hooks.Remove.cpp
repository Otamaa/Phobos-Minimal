#include <Syringe.h>

#pragma warning( push )
#pragma warning (disable : 4245)
#pragma warning (disable : 4838)

//DEFINE_DISABLE_HOOK(0x424538, Ares_AnimClass_Update_DamageDelay)
DEFINE_DISABLE_HOOK(0x6E232E, Ares_ActionClass_PlayAnimAt)
DEFINE_DISABLE_HOOK(0x6FC339, Ares_TechnoClass_GetFireError_OpenToppedGunnerTemporal)

#pragma warning( pop )