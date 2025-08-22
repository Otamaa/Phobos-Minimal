#include <Utilities/Swizzle.h>
#include <Phobos.h>

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x6CF350, SwizzleManagerClass_ConvertNodes, 7)
{
	PhobosSwizzle::Instance.ApplyFixups();
	PhobosSwizzle::Instance.Clear();
	return 0x6CF400;
}

ASMJIT_PATCH(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
	GET_STACK(DWORD, caller, 0x0);
	GET_STACK(void*, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);

	if(Phobos::Otamaa::IsAdmin)
		Debug::Log("Caller %x\n", caller);

	R->EAX(PhobosSwizzle::Instance.DeclareMapping(oldP, newP));
	return 0x6CF316;
}

ASMJIT_PATCH(0x6CF240, SwizzleManagerClass_Swizzle, 7)
{
	GET_STACK(void**, ptr, 0x8);
	R->EAX(PhobosSwizzle::Instance.RegisterForFixup(ptr));
	return 0x6CF2B3;
}
