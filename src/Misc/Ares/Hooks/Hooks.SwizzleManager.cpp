#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

DEFINE_OVERRIDE_HOOK(0x6CF350, SwizzleManagerClass_ConvertNodes, 7)
{
	PhobosSwizzle::Instance.ConvertNodes();
	PhobosSwizzle::Instance.Clear();

	return 0x6CF400;
}

DEFINE_OVERRIDE_HOOK(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
	GET_STACK(void*, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterChange(oldP, newP));
	return 0x6CF316;
}

DEFINE_OVERRIDE_HOOK(0x6CF240, SwizzleManagerClass_Swizzle, 7)
{
	GET_STACK(void**, ptr, 0x8);
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterForChange(ptr));
	return 0x6CF2B3;
}
