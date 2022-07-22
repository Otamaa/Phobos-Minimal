#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#define Blowfishdll_name reinterpret_cast<const char*>(0x840A78)

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
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = LoadLibraryA(Blowfishdll_name);
	if (hDll) {
		if (const auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject")) {

			IClassFactory* pIFactory;
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result)) {
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result)) {

		if(hDll) FreeLibrary(hDll);
		const char* Message = "File Blowfish.dll was not found\n";
		MessageBoxA(0, Message, "Fatal error ", MB_ICONERROR);
		Debug::FatalErrorAndExit(Message);
	}

	return result;
}

DEFINE_JUMP(CALL6,0x6BEDDD, GET_OFFSET(Blowfish_Loader));
DEFINE_JUMP(CALL6,0x437F6E, GET_OFFSET(Blowfish_Loader));

#undef Blowfishdll_name