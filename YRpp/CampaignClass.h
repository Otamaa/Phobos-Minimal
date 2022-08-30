#pragma once

#include <AbstractTypeClass.h>
#include <GeneralDefinitions.h>
#include <CCINIClass.h>

class DECLSPEC_UUID("FFDAC848-1517-11D2-8175-006008055BB5")
	NOVTABLE CampaignClass : public AbstractTypeClass {

public:
	static const AbstractType AbsID = AbstractType::Campaign;

	//Array
	ABSTRACTTYPE_ARRAY(CampaignClass, 0xA83CF8u);
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() R0;

	//Pure virtual , Only for inheritance
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) R0;

	//IRTTITypeInfo
	virtual AbstractType __stdcall What_Am_I() const RT(AbstractType);
	virtual int __stdcall Fetch_ID() const R0;
	virtual void __stdcall Create_ID() RX;

	//Destructor
	virtual ~CampaignClass() RX;


	//AbstractClass
	virtual void Init() override RX;
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override RX;
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	CampaignClass(const char *name) noexcept
		: CampaignClass(noinit_t())
	{ JMP_THIS(0x46CB60); }

protected:
	explicit __forceinline CampaignClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

public:
	static void __fastcall CreateFromINIList(CCINIClass *pINI)
		{ JMP_STD(0x46CE10); }

	static signed int __fastcall _FindIndex(const char* name)
		{ JMP_STD(0x46CC90); }

public:
	int idxCD;
	char Scenario[512];
	int FinalMovie;
	wchar_t Description[128];
};
