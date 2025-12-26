#pragma once
#include <TemporalClass.h>

#include <Utilities/SavegameDef.h>

#include <Utilities/Container.h>

class WeaponTypeClass;
class TemporalExtData final : public AbstractExtended
{
public:
	 using base_type = TemporalClass;
	 static COMPILETIMEEVAL const char* ClassName = "TemporalExtData";
	 static COMPILETIMEEVAL const char* BaseClassName = "TemporalClass";
	 static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	 static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

	TemporalExtData(TemporalClass* pObj) : AbstractExtended(pObj) {
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

	TemporalClass* This() const { return reinterpret_cast<TemporalClass*>(this->AttachedToObject); }
	const TemporalClass* This_Const() const { return reinterpret_cast<const TemporalClass*>(this->AttachedToObject); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TemporalExtContainer final : public Container<TemporalExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TemporalExtContainer";

public:
	static TemporalExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
};

class NOVTABLE FakeTemporalClass : public TemporalClass
{
public:

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
