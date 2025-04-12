#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/SkilledLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>

template<typename T>
class TClassFactory : public IClassFactory
{
public:
	TClassFactory()
	{
		this->nRefCount = 0;
	}

	virtual HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject) override
	{
		if (!ppvObject)
			return E_POINTER;

		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown))
			*ppvObject = static_cast<IUnknown*>(this);

		if (riid == __uuidof(IClassFactory))
			*ppvObject = static_cast<IClassFactory*>(this);

		if (!*ppvObject)
			return E_NOINTERFACE;

		this->AddRef();

		return S_OK;
	}

	virtual ULONG __stdcall AddRef() override
	{
		return Imports::InterlockedIncrementFunc.get()(&this->nRefCount);
	}

	virtual ULONG __stdcall Release() override
	{
		int nNewRef = Imports::InterlockedDecrementFunc.get()(&this->nRefCount);
		if (!nNewRef)
			GameDelete(this);
		return nNewRef;
	}

	virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, const IID& riid, void** ppvObject) override
	{
		if (!ppvObject)
			return E_INVALIDARG;

		*ppvObject = nullptr;
		if (pUnkOuter)
			return CLASS_E_NOAGGREGATION;

		T* pThis = GameCreate<T>();
		if (!pThis)
			return E_OUTOFMEMORY;

		HRESULT hr = pThis->QueryInterface(riid, ppvObject);

		if (FAILED(hr))
			GameDelete(pThis);

		return hr;
	}

	virtual HRESULT __stdcall LockServer(BOOL fLock) override
	{
		if (fLock)
			Imports::InterlockedIncrementFunc.get()(&this->nRefCount);
		else
			Imports::InterlockedDecrementFunc.get()(&this->nRefCount);

		return S_OK;
	}

private:
	int nRefCount { 0 };
};

// Registers a manually created factory for a class.
template<typename T>
void RegisterFactoryForClass(IClassFactory* pFactory)
{
	DWORD dwRegister = 0;
	CLSID clsid = __uuidof(T);
	HRESULT hr = CoRegisterClassObject(clsid, pFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister);

	const std::string name = typeid(T).name();

	if (FAILED(hr))
		Debug::Log("CoRegisterClassObject for %s class factory failed with error code %s.\n", name.c_str(), GetLastError());
	else
		Debug::Log("Class factory for %s registered.\n", name.c_str());

	Game::ClassFactories->AddItem((ULONG)dwRegister);


	//LPOLESTR str = nullptr;
	//StringFromCLSID(clsid, &str);
	//Debug::LogInfo("Validating {} CLSID: {}", name , PhobosCRT::WideStringToString(str));
	//CoTaskMemFree(str);
}

// Registers an automatically created factory for a class.
template<typename T>
void RegisterFactoryForClass()
{
	RegisterFactoryForClass<T>(GameCreate<TClassFactory<T>>());
}

ASMJIT_PATCH(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	// Add new classes to be COM-registered below
	RegisterFactoryForClass<LevitateLocomotionClass>();
	RegisterFactoryForClass<SkilledLocomotionClass>();
	RegisterFactoryForClass<CustomRocketLocomotionClass>();

	Debug::Log("COM registration done!\n");

	return 0;
}
