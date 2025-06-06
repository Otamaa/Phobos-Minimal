#pragma once

#include "Base.h"
#include <FootClass.h>
#include <InfantryClass.h>
#include <VocClass.h>
#include <Utilities/Debug.h>
#include <MapClass.h>
#include <CellClass.h>

#include <comip.h>
#include <comdef.h>
#include <combaseapi.h>

DEFINE_PIGGYLOCO(AdvancedWalk, 5B8A5F12-E7C3-4A2D-8F1E-3B4C2A9D7E6F)
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject)
	{
		HRESULT hr = LocomotionClass::QueryInterface(iid, ppvObject);
		if (hr != E_NOINTERFACE)
			return hr;

		if (iid == __uuidof(IPiggyback))
		{
			*ppvObject = static_cast<IPiggyback*>(this);
			this->AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG __stdcall AddRef()
	{
		return LocomotionClass::AddRef();
	}

	virtual ULONG __stdcall Release()
	{
		return LocomotionClass::Release();
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override
	{
		if (pClassID == nullptr)
			return E_POINTER;

		*pClassID = __uuidof(this);
		return S_OK;
	}

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override
	{
		HRESULT hr = this->LocomotionClass::Internal_Load(this, pStm);

		if (FAILED(hr))
			return hr;

		bool piggybackerPresent;
		hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent || FAILED(hr))
			return hr;

		CLSID classid;
		hr = pStm->Read(&classid, sizeof(classid), nullptr);

		if (FAILED(hr))
			return hr;

		ILocomotionPtr piggyback;
		hr = OleLoadFromStream(pStm, __uuidof(ILocomotion), reinterpret_cast<LPVOID*>(&piggyback));

		if (SUCCEEDED(hr))
			this->Piggybacker = piggyback;

		return hr;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)
	{
		HRESULT hr = this->LocomotionClass::Internal_Save(this, pStm, fClearDirty);

		if (FAILED(hr))
			return hr;

		bool piggybackerPresent = this->Piggybacker != nullptr;
		hr = pStm->Write(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent)
			return hr;

		IPersistStreamPtr piggyPersist(this->Piggybacker);
		hr = OleSaveToStream(piggyPersist, pStm);

		return hr;
	}
	virtual int Size() override { return sizeof(*this); }

	// ILocomotion - minimal but complete implementation 
	virtual bool __stdcall Is_Moving() override
	{
		return this->IsMoving;
	}
	virtual CoordStruct __stdcall Destination() override { return this->MovingDestination; }
	virtual CoordStruct __stdcall Head_To_Coord() override 
	{ 
		if (this->CoordHeadTo != CoordStruct::Empty)
			return this->CoordHeadTo;
		return this->LinkedTo ? this->LinkedTo->Location : CoordStruct::Empty;
	}
	
	// Cell occupancy check for infantry (max 3 per cell)
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override;
	
	// Essential virtual methods that MUST be implemented
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override;
	virtual void __stdcall Stop_Moving() override;
	
	// Occupation marking for proper cell management
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override;
	
	// Additional required methods
	virtual void __stdcall Do_Turn(DirStruct dir) override { this->LinkedTo->PrimaryFacing.Set_Desired(dir); }
	virtual Layer __stdcall In_Which_Layer() override { return Layer::Ground; }
	virtual bool __stdcall Is_Moving_Now() override { return this->IsMoving && this->IsReallyMoving; }
	virtual bool __stdcall Is_Really_Moving_Now() override { return this->IsReallyMoving; }
	virtual void __stdcall Stop_Movement_Animation() override;
	virtual void __stdcall Clear_Coords() override { this->Stop_Moving(); }

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override
	{
		if (!pointer)
			return E_POINTER;

		if (this->Piggybacker)
			return E_FAIL;

		this->Piggybacker = pointer;
		pointer->AddRef();

		return S_OK;
	}

	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override
	{
		if (!pointer)
			return E_POINTER;

		if (!this->Piggybacker)
			return S_FALSE;

		*pointer = this->Piggybacker.Detach();

		return S_OK;
	}

	virtual bool __stdcall Is_Ok_To_End() override
	{
		return false;
	}

	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override
	{
		if (!classid)
			return E_POINTER;

		if (!this->Piggybacker)
			return S_FALSE;

		IPersistPtr piggyPersist(this->Piggybacker);
		return piggyPersist->GetClassID(classid);
	}

	virtual bool __stdcall Is_Piggybacking() override
	{
		return this->Piggybacker != nullptr;
	}

public:
	AdvancedWalkLocomotionClass() : LocomotionClass { }
		, MovingDestination { CoordStruct::Empty }
		, CoordHeadTo { CoordStruct::Empty }
		, IsMoving { false }
		, _bool35 { false }
		, IsReallyMoving { false }
		, Piggybacker { nullptr }
	{
	}

	// Destructor
	virtual ~AdvancedWalkLocomotionClass() override = default;

public:
	AdvancedWalkLocomotionClass(const AdvancedWalkLocomotionClass&) = delete;
	AdvancedWalkLocomotionClass(noinit_t) : LocomotionClass { noinit_t() } { }
	AdvancedWalkLocomotionClass& operator=(const AdvancedWalkLocomotionClass&) = delete;

public:

	// Properties - exact clone of WalkLocomotionClass
	CoordStruct MovingDestination;
	CoordStruct CoordHeadTo;
	bool IsMoving;
	bool _bool35;
	bool IsReallyMoving;
	ILocomotionPtr Piggybacker; // Instead of LocomotionClass* PiggyBackee

private:
	// Helper methods for complete implementation
	bool WalkingProcess();
	bool CanEnterCell(CellClass* pCell);
	int CountInfantryInCell(CellClass* pCell);
	bool HasAvailableSubposition(CellClass* pCell); // Check if cell has free infantry subposition
	void UpdateFacingDirection(const CoordStruct& direction);
}; 