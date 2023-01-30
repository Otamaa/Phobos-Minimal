#pragma once

#include <AbstractTypeClass.h>
#include <GeneralDefinitions.h>
#include <CCINIClass.h>

class DECLSPEC_UUID("FFDAC848-1517-11D2-8175-006008055BB5")
	NOVTABLE CampaignClass : public AbstractTypeClass {

public:
	static const AbstractType AbsID = AbstractType::Campaign;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<CampaignClass*>, 0xA83CF8u> const Array {};

	static NOINLINE CampaignClass* Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	//Was inlined
	static NOINLINE CampaignClass* FindOrAllocate(const char* pID)
	{
		if (!CRT::strcmpi(pID, GameStrings::NoneStr()) || !CRT::strcmpi(pID, GameStrings::NoneStrb())) {
			return nullptr;
		}

		if(auto pRet = Find(pID)) {
			return pRet;
		}

		return GameCreate<CampaignClass>(pID);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x46CC90);
	}

	static void __fastcall CreateFromINIList(CCINIClass *pINI) {
		JMP_STD(0x46CE10);
	}

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

	bool ReadDescription(CCINIClass *pINI) const {
		JMP_THIS(0x46CCD0);
	}

	CampaignClass(const char *name) noexcept
		: CampaignClass(noinit_t())
	{ JMP_THIS(0x46CB60); }

protected:
	explicit __forceinline CampaignClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

public:
	int idxCD;
	char Scenario[512];
	int FinalMovie;
	wchar_t Description[128];
};

static_assert(sizeof(CampaignClass) == 0x3A0, "Invalid Size !");