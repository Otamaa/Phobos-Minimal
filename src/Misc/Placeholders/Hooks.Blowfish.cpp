#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

static HRESULT __stdcall
Blowfish_Loader(
	REFCLSID  rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD	 dwClsContext,
	REFIID	riid,
	LPVOID* ppv)
{
	typedef HRESULT(__stdcall *pDllGetClassObject)(const IID&, const IID&, IClassFactory**);

	auto result = REGDB_E_KEYMISSING;

	// First, let's try to run the vanilla function
	result = Imports::CoCreateInstance.get()(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = Imports::LoadLibraryA.get()(GameStrings::BLOWFISH_DLL());
	if (hDll) {
		if (const auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject")) {

			IClassFactory* pIFactory {};
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result)) {
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result)) {

		if(hDll) Imports::FreeLibrary.get()(hDll);
		Debug::FatalErrorAndExit("File Blowfish.dll was not found");
	}

	return result;
}

DEFINE_FUNCTION_JUMP(CALL6,0x6BEDDD, Blowfish_Loader));
DEFINE_FUNCTION_JUMP(CALL6,0x437F6E, Blowfish_Loader));