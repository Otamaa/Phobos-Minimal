#pragma once

#pragma once

#include <Unsorted.h>
#include <unknwn.h>
#include <YRCom.h>

template<typename T>
class TClassFactory : public IClassFactory
{
public:
	COMPILETIMEEVAL TClassFactory() : nRefCount {0L}
	{}

	virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObject) override
	{
		if (ppvObject == nullptr) {
			return E_POINTER;
		}

		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown)) {
			*ppvObject = static_cast<IUnknown*>(this);
		}

		if (riid == __uuidof(IClassFactory)){
			*ppvObject = static_cast<IClassFactory*>(this);
		}

		if (*ppvObject == nullptr) {
			return E_NOINTERFACE;
		}

		reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();

		return S_OK;
	}

	virtual ULONG __stdcall AddRef() override
	{
		return InterlockedIncrement(&this->nRefCount);
	}

	virtual ULONG __stdcall Release() override
	{
		ULONG nNewRef = InterlockedIncrement(&this->nRefCount);

		if (nNewRef == 0)
			GameDelete(this);

		return nNewRef;
	}

	virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObject) override
	{
		if (ppvObject == nullptr) {
			return E_INVALIDARG;
		}

		*ppvObject = nullptr;
		if (pUnkOuter != nullptr) {
			return CLASS_E_NOAGGREGATION;
		}

		T* obj = GameCreate<T>();
		if (obj == nullptr) {
			return E_OUTOFMEMORY;
		}

		HRESULT hr = obj->QueryInterface(riid, ppvObject);

		if (FAILED(hr))
			obj->~T();

		return hr;
	}

	virtual HRESULT __stdcall LockServer(BOOL fLock) override
	{
		if (fLock) {
			++nRefCount;
		} else {
			--nRefCount;
		}

		return S_OK;
	}

private:
	ULONG nRefCount;
};
