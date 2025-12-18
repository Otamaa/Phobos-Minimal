#pragma once
#include <TemporalClass.h>

#include <Utilities/SavegameDef.h>

#include <Utilities/Container.h>

class WeaponTypeClass;
class TemporalExtData final : public AbstractExtended
{
public:
	 using base_type = TemporalClass;
	 static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
	TemporalExtData(TemporalClass* pObj) : AbstractExtended(pObj) {
		this->Name = "TemporalClass";
		this->AbsType = TemporalClass::AbsID;
	}
	TemporalExtData(TemporalClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~TemporalExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TemporalExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<TemporalExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	TemporalClass* This() const override { return reinterpret_cast<TemporalClass*>(this->AttachedToObject); }
	const TemporalClass* This_Const() const override { return reinterpret_cast<const TemporalClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TemporalExtContainer final : public Container<TemporalExtData>
{
public:
	static TemporalExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class NOVTABLE FakeTemporalClass : public TemporalClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	void CreateWarpAwayAnimation(WeaponTypeClass* pWeapon);

	void ResetTemporalState() {
		this->Target = 0;
		this->PrevTemporal = 0;
		this->NextTemporal = 0;
		this->SourceSW = 0;
		this->unknown_pointer_38 = 0;
	}

	void _Update();
	void _Detonate(TechnoClass* pTarget);

	FORCEDINLINE TemporalClass* _AsTemporal() const {
		return (TemporalClass*)this;
	}

	FORCEDINLINE TemporalExtData* _GetExtData() {
		return *reinterpret_cast<TemporalExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTemporalClass) == sizeof(TemporalClass), "Invalid Size !");
