#include "Locomotions.h"

#include <New/Interfaces/LevitateLocomotionClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>
#include <YRCom.h>
#include <TClassFactory.h>
#include <Utilities/INIParser.h>

template<typename T>
void REG_CLASS()
{
	DWORD dwRegister = 0;
	TClassFactory<T>* ptr = GameCreate<TClassFactory<T>>();
	HRESULT hr = CoRegisterClassObject(__uuidof(T), ptr, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister);

	if (FAILED(hr)) {
		Debug::Log("CoRegisterClassObject(TClassFactory<%s>) failed with error code % d.\n", typeid(T).name(), GetLastError());
	} else {
		Debug::Log("%s factory registered.\n", typeid(T).name());
	}

	Game::ClassFactories->AddItem((ULONG)dwRegister);
}

//DEFINE_HOOK(0x6BC4AE, WinMain_CoRegisterClassObjects_Locomotions, 0x6)
//{
//	Debug::Log("Registering new com objects...\n");
//	Debug::Log("Registering LevitateLocomotionClass\n");
//	REG_CLASS<LevitateLocomotionClass>();
//	Debug::Log("Registering done !\n");
//	return 0;
//}