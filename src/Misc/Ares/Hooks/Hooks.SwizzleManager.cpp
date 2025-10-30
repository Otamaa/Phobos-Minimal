#include <Utilities/Swizzle.h>
#include <Utilities/Macro.h>

#include <Phobos.h>

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x6CF350, SwizzleManagerClass_ConvertNodes, 7)
{
	PhobosSwizzleManager.Reset();
	return 0x6CF400;
}

ASMJIT_PATCH(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
	GET_STACK(DWORD, caller, 0x0);
	GET_STACK(LONG, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);

	//if(Phobos::Otamaa::IsAdmin)
	//	Debug::Log("SwizzleManagerClass_Here_I_Am Caller %x\n", caller);

	R->EAX(PhobosSwizzleManager.Here_I_Am(oldP, newP));
	return 0x6CF316;
}

ASMJIT_PATCH(0x6CF240, SwizzleManagerClass_Swizzle, 7)
{
	GET_STACK(void**, ptr, 0x8);
	R->EAX(PhobosSwizzleManager.Swizzle(ptr));
	return 0x6CF2B3;
}
