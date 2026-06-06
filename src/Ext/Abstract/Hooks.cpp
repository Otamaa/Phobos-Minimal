#include <Utilities/Swizzle.h>
#include <Utilities/Parser.h>
#include <Utilities/Macro.h>

#include <Utilities/Container.h>

DWORD Origin;

ASMJIT_PATCH(0x410380 , AbstractClass_Load_FetchCaller, 0x5){
	GET_STACK(AbstractClass*, pThis, 0x4);
	GET_STACK(DWORD, caller, 0x0);
	pThis->unknown_18 = 0u; //reset first
	Origin = caller;
	return 0x0;
}

// fetch the ext pointer on second pass read
ASMJIT_PATCH(0x4103D0, AbstractClass_Load_LogValue, 0x5)
{
	GET(AbstractClass*, pThis, ESI);
	//GET_STACK(IStream*, pStream, 0x0);

	//immedietely update the extension pointer value and the extension AttachedToObject itself !
	ExtensionSwizzleManager::SwizzleExtensionPointer(reinterpret_cast<void**>(&pThis->unknown_18), pThis , Origin);
	Origin = 0u;

	return 0x0;
}

//more specific
//ASMJIT_PATCH(0x41096D, AbstractTypeClass_NoInt_cleaupPtr,0x6)
//{
//     GET(AbstractClass*, pThis, EAX);
//
//	 pThis->unknown_18 = 0u; //reset first
//     if (Phobos::Otamaa::DoingLoadGame) {
//		 ExtensionSwizzleManager::HandOverExtension(reinterpret_cast<void**>(&pThis->unknown_18), pThis);
//     }
//
//     return 0x0;
//}

ASMJIT_PATCH(0x410182, AbstractClass_cleaupPtr_B, 0x6)
{
	GET(AbstractClass*, pThis, EAX);

	pThis->unknown_18 = 0u; //reset first
	if (Phobos::Otamaa::DoingLoadGame) {
		ExtensionSwizzleManager::HandOverExtension(reinterpret_cast<void**>(&pThis->unknown_18), pThis);
	}

	pThis->RefCount = 0l;
	return 0x410188;
}

ASMJIT_PATCH(0x4101E4, AbstractClass_cleaupPtr, 0x7)
{
	GET(AbstractClass*, pThis, EAX);

	pThis->unknown_18 = 0u; //reset first
	if (Phobos::Otamaa::DoingLoadGame) {
		ExtensionSwizzleManager::HandOverExtension(reinterpret_cast<void**>(&pThis->unknown_18), pThis);
	}

	return 0x0;
}