#include <Utilities/Swizzle.h>
#include <Phobos.h>

/// i forgot that ares swizzle manager system is tightly integrated
/// so replacing the hook mean we need to replace absolutely everything , lmao
///
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
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterChange_Hook(oldP, newP));
	return 0x6CF316;
}

DEFINE_OVERRIDE_HOOK(0x6CF240, SwizzleManagerClass_Swizzle, 7)
{
	GET_STACK(void**, ptr, 0x8);
	R->EAX<HRESULT>(PhobosSwizzle::Instance.RegisterForChange_Hook(ptr));
	return 0x6CF2B3;
}
