#include <Utilities/Swizzle.h>
#include <Utilities/Parser.h>
#include <Utilities/Macro.h>

#include <Utilities/Container.h>

DWORD LastKnown;
AbstractClass* pAbs;
DWORD Origin;

ASMJIT_PATCH(0x410380 , AbstractClass_Load_FetchOrigi, 0x5){
	GET_STACK(DWORD, caller, 0x0);
	Origin = caller;
	return 0x0;
}

ASMJIT_PATCH(0x4103D0, AbstractClass_Load_LogValue, 0x5)
{
	GET(AbstractClass*, pThis, ESI);
	//GET_STACK(IStream*, pStream, 0x0);

	//immedietely update the extension pointer value and the extension AttachedToObject itself !
	ExtensionSwizzleManager::SwizzleExtensionPointer(reinterpret_cast<void**>(&pThis->unknown_18), pThis , Origin);
	Origin = 0u;
	LastKnown = pThis->unknown_18;
	pAbs = pThis;

	return 0x0;
}

//more specific
//ASMJIT_PATCH(0x41096D, AbstractTypeClass_NoInt_cleaupPtr,0x6)
//{
   //  GET(AbstractClass*, pThis, EAX);

   //  if (Phobos::Otamaa::DoingLoadGame) {
   //	  if (pAbs != pThis)  //avoid missmatching
   //		  LastKnown = 0;
   //  }

   //  pThis->unknown_18 = std::exchange(LastKnown, 0u);
   //  return 0x0;
//}

ASMJIT_PATCH(0x410182, AbstractClass_cleaupPtr_B, 0x6)
{
	GET(AbstractClass*, pThis, EAX);

	if (Phobos::Otamaa::DoingLoadGame) {
		if (pAbs != pThis) //avoid missmatching
			LastKnown = 0;
	}

	pThis->unknown_18 = std::exchange(LastKnown, 0u);
	pThis->RefCount = 0l;
	return 0x410188;
}

ASMJIT_PATCH(0x4101E4, AbstractClass_cleaupPtr, 0x7)
{
	GET(AbstractClass*, pThis, EAX);

	if (Phobos::Otamaa::DoingLoadGame) {

		if (pAbs != pThis) //avoid missmatching
			LastKnown = 0;
	}

	pThis->unknown_18 = std::exchange(LastKnown, 0u);
	return 0x0;
}