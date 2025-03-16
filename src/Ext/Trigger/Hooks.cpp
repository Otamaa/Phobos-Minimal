#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/TEvent/Body.h>

ASMJIT_PATCH(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= (int)PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= (int)PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::LocalSet) ?
		0x72706E :
		0x727069;
}

ASMJIT_PATCH(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrue ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable ||
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= (int)PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::GlobalSet) ?
		0x72702E :
		0x727029;
}