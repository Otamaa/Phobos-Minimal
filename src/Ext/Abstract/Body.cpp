#include "Body.h"
//#ifndef DISABLE_DIRECT_Ext
#include <Base/Always.h>
#include <SwizzleManagerClass.h>
#include <Checksummer.h>

DEFINE_HOOK(0x410450, AbstractClass_IsDirty, 0x6)
{
	GET_STACK(AbstractClass*, pThis, 0x4);

	R->EAX(ExtensionWrapper::GetWrapper(pThis)->IsDirty());

	return 0x41045E;
}

DEFINE_HOOK(0x4103E0, AbstractClass_GetSizeMax, 0x5)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(ULARGE_INTEGER*, pcbSize, 0x4);

	pcbSize->HighPart = 0;
	pcbSize->LowPart =
		pThis->Size() + sizeof(pThis) +
		sizeof(IExtension*) +
		ExtensionWrapper::GetWrapper(pThis)->Size();

	R->EAX(S_OK);

	return 0x4103EF;
}

DEFINE_HOOK(0x512570, HouseTypeClass_GetSizeMax, 0x5)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(ULARGE_INTEGER*, pcbSize, 0x4);

	pcbSize->HighPart = 0;
	pcbSize->LowPart =
		pThis->Size() + sizeof(pThis) +
		sizeof(IExtension*) +
		ExtensionWrapper::GetWrapper(pThis)->Size();

	R->EAX(S_OK);

	return 0x51257D;
}

DEFINE_HOOK(0x4101B6, AbstractClass_CTOR, 0x5)
{
	GET(AbstractClass*, pThis, EAX);

	ExtensionWrapper::GetWrapper(pThis) = GameCreate<ExtensionWrapper>();

	return 0;
}

DEFINE_HOOK(0x4105BD, AbsractClass_SDTOR, 0x7)
{
	GET(AbstractClass*, pThis, ESI);

	if (auto pWrapper = ExtensionWrapper::GetWrapper(pThis)){
		GameDelete<true>(pWrapper);
	}

	return 0;
}

DEFINE_HOOK(0x41020B, AbsractClass_DTOR, 0x5)
{
	GET(AbstractClass*, pThis, ECX);

	if (auto pWrapper = ExtensionWrapper::GetWrapper(pThis)) {
		GameDelete<true>(pWrapper);
	}

	return 0;
}

DEFINE_HOOK(0x410423, AbstractClass_ComputeCRC, 0x4)
{
	class WWCRCEngine {
	public:
		void Add(bool bIn) {
			JMP_THIS(0x4A1CA0);
		}

	protected:
		long CRC;
		int Index;
		union
		{
			long Composite;
			char Buffer[sizeof(long)];
		} StagingBuffer;
	};

	GET(AbstractClass*, pThis, ESI);
	GET(WWCRCEngine*, pCheck, EDI);

	pCheck->Add(ExtensionWrapper::GetWrapper(pThis)->IsDirty());

	return 0x41042E;
}

DEFINE_HOOK(0x410320, AbstractClass_Save, 0x5)
{
	GET_STACK(AbstractClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	GET_STACK(BOOL, fClearDirty, 0xC);

	if (!pStm)
	{
		R->EAX(E_POINTER);
		return 0x410331;
	}

	HRESULT hr = pStm->Write(&pThis, sizeof(pThis), nullptr);
	if (SUCCEEDED(hr))
	{
		hr = pStm->Write(pThis, pThis->Size(), nullptr);
		if (SUCCEEDED(hr))
		{
			// Write ext wrapper
			auto pWrapper = ExtensionWrapper::GetWrapper(pThis);
			if (SUCCEEDED(hr))
			{
				hr = pWrapper->Save(pStm);
				if (SUCCEEDED(hr))
				{
					if (fClearDirty)
						pWrapper->SetDirtyFlag(FALSE);
				}
			}
		}
	}

	R->EAX(hr);

	return 0x410331;
}

DEFINE_HOOK(0x410380, AbstractClass_Load, 0x5)
{
	GET_STACK(AbstractClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	if (!pStm)
	{
		R->EAX(E_POINTER);
		return 0x41038F;
	}

	AbstractClass* pOldThis;
	HRESULT hr = pStm->Read(&pOldThis, sizeof(pThis), nullptr);
	if (SUCCEEDED(hr))
	{
		SwizzleManagerClass::Instance->Here_I_Am((long)pOldThis, pThis);
		int nRefCount = pThis->RefCount;
		auto const pWrapper = ExtensionWrapper::GetWrapper(pThis);

		hr = pStm->Read(pThis, pThis->Size(), nullptr);
		if (SUCCEEDED(hr))
			hr = pWrapper->Load(pStm, pThis);

		pThis->RefCount = nRefCount;
		ExtensionWrapper::GetWrapper(pThis) = pWrapper;
	}

	R->EAX(hr);

	return 0x41038F;
}
//#endif