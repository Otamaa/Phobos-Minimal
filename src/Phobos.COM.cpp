#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <New/Interfaces/LevitateLocomotionClass.h>


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
		int nNewRef = Imports::InterlockedIncrementFunc.get()(&this->nRefCount);
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
		this->nRefCount += fLock ? 1 : -1;

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
	HRESULT hr = CoRegisterClassObject(__uuidof(T), pFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister);

	if (FAILED(hr))
		Debug::LogInfo("CoRegisterClassObject for {} class factory failed with error code {}.", typeid(T).name(), GetLastError());
	else
		Debug::LogInfo("Class factory for {} registered.", typeid(T).name());

	Game::ClassFactories->AddItem((ULONG)dwRegister);
}

// Registers an automatically created factory for a class.
template<typename T>
void RegisterFactoryForClass()
{
	RegisterFactoryForClass<T>(GameCreate<TClassFactory<T>>());
}

DEFINE_HOOK(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::LogInfo("Starting COM registration...");

	// Add new classes to be COM-registered below
	RegisterFactoryForClass<LevitateLocomotionClass>();

	Debug::LogInfo("COM registration done!");

	return 0;
}
