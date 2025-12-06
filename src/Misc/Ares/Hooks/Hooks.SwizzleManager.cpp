#include <Utilities/Swizzle.h>
#include <Utilities/Macro.h>

#include <Phobos.h>

#include <Helpers/Macro.h>

struct SwizzleWrapperClass : public SwizzleManagerClass
{
	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Reset() {
		PhobosSwizzleManager.Reset();
		return S_OK;
	}

	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Swizzle(void** pointer) {
		return PhobosSwizzleManager.Swizzle(pointer);
	}

	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Fetch_Swizzle_ID(void* pointer, LONG* id) {
		if (pointer == nullptr || id == nullptr)
		{
			return E_POINTER;
		}

		*id = reinterpret_cast<uintptr_t>(pointer);

		return S_OK;
	}

	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Here_I_Am(LONG id, void* pointer) {
		return PhobosSwizzleManager.Here_I_Am(id, pointer);
	}

};

//DEFINE_FUNCTION_JUMP(LJMP, 0x6CF350, SwizzleWrapperClass::_Reset)
//DEFINE_FUNCTION_JUMP(LJMP, 0x6CF2C0, SwizzleWrapperClass::_Here_I_Am)
//DEFINE_FUNCTION_JUMP(LJMP, 0x6CF240, SwizzleWrapperClass::_Swizzle)
//DEFINE_FUNCTION_JUMP(LJMP, 0x6CF490, SwizzleWrapperClass::_Fetch_Swizzle_ID)

ASMJIT_PATCH(0x6CF350, SwizzleManagerClass_ConvertNodes, 7)
{
	PhobosSwizzleManager.Reset();
	return 0x6CF400;
}

ASMJIT_PATCH(0x6CF2C0, SwizzleManagerClass_Here_I_Am, 5)
{
	//GET_STACK(DWORD, caller, 0x0);
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
