#include "AbstractExt.h"
#include <ArrayClasses.h>
#include <SwizzleManagerClass.h>

/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: CCHyper
 */
HRESULT AbstractClassExtension::Load(IStream* pStm)
{
	LONG id;
	HRESULT hr = pStm->Read(&id, sizeof(id), nullptr);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	ULONG size = Size_Of();
	hr = pStm->Read(this, size, nullptr);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	new (this) AbstractClassExtension(noinit_t());

	/**
	 *  Announce ourself to the swizzle manager.
	 */
	SWIZZLE_HERE_I_AM(id, this);

	/**
	 *  Request the pointer to the base class be remapped.
	 */
	 //SWIZZLE_REQUEST_POINTER_REMAP(ThisPtr);

	return S_OK;
}

/**
 *  Saves the object to the stream.
 *
 *  @author: CCHyper
 */
HRESULT AbstractClassExtension::Save(IStream* pStm, BOOL fClearDirty)
{
	LONG id;
	SWIZZLE_FETCH_POINTER_ID(this, &id);
	HRESULT hr = pStm->Write(&id, sizeof(id), nullptr);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	ULONG size = Size_Of();
	hr = pStm->Write(this, size, nullptr);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	return S_OK;
}

DynamicVectorClass<AircraftClassExtension*> AircraftExtensions;

AbstractClassExtension* Fetch_Extension(ExtensionRTTIType rtti, ExtensionIndex index)
{
	AbstractClassExtension* extptr = nullptr;

	switch (rtti)
	{
	case EXT_RTTI_AIRCRAFT:
		extptr = AircraftExtensions[index];
		break;
	default:
		break;
	};

	return extptr;
}

ExtensionIndex Get_Extension_Index(AbstractClass* abs)
{
	ExtensionIndex index = (ExtensionIndex)((((uintptr_t)abs) + 0x12) & 0xFFFF);
	return index;
}

ExtensionRTTIType Get_Extension_RTTI(AbstractClass* abs)
{
	ExtensionRTTIType rtti = (ExtensionRTTIType)((((uintptr_t)abs) + 0x11) & 0xFF);
	return rtti;
}

AbstractClassExtension* Fetch_Extension(AbstractClass* abs)
{
	return Fetch_Extension(Get_Extension_RTTI(abs), Get_Extension_Index(abs));
}
