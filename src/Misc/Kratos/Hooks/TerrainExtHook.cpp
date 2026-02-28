#include <exception>
#include <Windows.h>

#include <TerrainClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TerrainTypeExt.h>

#include <Misc/Kratos/Ext/TerrainType/TerrainDestroyAnim.h>

#ifdef _ENABLE_HOOKS

// ASMJIT_PATCH(0x71C94C, TerrainClass_Remove_PlayDestroyAnim, 0xA)
ASMJIT_PATCH(0x71BB2C, TerrainClass_TakeDamage_NowDead_Add, 0x6) // from Phobos
{
	GET(TerrainClass*, pTerrain, ESI);
	TerrainDestroyAnim::PlayDestroyAnim(pTerrain);
	return 0;
}
#endif