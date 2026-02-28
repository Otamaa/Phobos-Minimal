#include <exception>
#include <Windows.h>

#include <GeneralDefinitions.h>
#include <ObjectClass.h>
#include <FootClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Helpers/Macro.h>

#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

#ifndef _ENABLE_HOOKS

// class PointerExpireHook
// {
// public:
// 	PointerExpireHook()
// 	{
// 		EventSystems::General.AddHandler(Events::DetachAll, DetachAll);
// 		EventSystems::General.AddHandler(Events::PointerExpireEvent, InvalidatePointer);
// 	}
// };

// static PointerExpireHook _pointerExpireHook;

ASMJIT_PATCH(0x4101F0, AbstractClass_DTOR, 0x6)
{
	GET(ObjectClass*, pObject, ECX);
	EventSystems::General.Broadcast(Events::PointerExpireEvent, pObject);
	return 0;
}

ASMJIT_PATCH(0x5F65F1, ObjectClass_UnInit, 0x5)
{
	GET(ObjectClass*, pObject, ECX);
	// Debug::Log("ObjectClass [%s]%d ready to delete.\n", pObject->GetType()->ID, pObject);
	EventSystems::General.Broadcast(Events::ObjectUnInitEvent, pObject);
	return 0;
}

// this function is a Object want Detach_All when Limbo or Delete
ASMJIT_PATCH(0x7258D0, DetachThisFromAll, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);
	void* args[] = { pInvalid, (void*)removed };

	// 该广播会执行三次
	// (01) DetachAll
	// (01) Limbo
	// (01) DetachAll
	// (02) DetachAll
	// (02) Delete
	EventSystems::General.Broadcast(Events::DetachAll, &args);

	return 0;
}

#endif