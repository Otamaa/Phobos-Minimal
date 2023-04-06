#include <Syringe.h>

#pragma warning( push )
#pragma warning (disable : 4245)
#pragma warning (disable : 4838)

//DEFINE_DISABLE_HOOK(0x424538, Ares_AnimClass_Update_DamageDelay)
DEFINE_DISABLE_HOOK(0x6E232E, ActionClass_PlayAnimAt_Ares)
DEFINE_DISABLE_HOOK(0x6FC339, TechnoClass_GetFireError_OpenToppedGunnerTemporal_Ares)
DEFINE_DISABLE_HOOK(0x763226, WaveClass_DTOR_Ares)
DEFINE_DISABLE_HOOK(0x4393F2, BombClass_SDDTOR_removeUnused1_Ares)
DEFINE_DISABLE_HOOK(0x438843, BombClass_Detonate_removeUnused2_Ares)
DEFINE_DISABLE_HOOK(0x438799, BombClass_Detonate_removeUnused3_Ares)
#pragma warning( pop )