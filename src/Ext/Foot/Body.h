#pragma once

#include <Ext/Techno/Body.h>
#include <New/Entity/HijackerData.h>

#include <FootClass.h>

class FootExtData : public TechnoExtData
{
public:

	FootExtData(FootClass* abs) : TechnoExtData(abs)
	{ };

	FootExtData(FootClass* abs, noinit_t& noint) : TechnoExtData(abs, noint) { };

	virtual ~FootExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType type) override {
		this->TechnoExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->TechnoExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->TechnoExtData::SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	FootClass* This() const { return reinterpret_cast<FootClass*>(AttachedToObject); }
	const FootClass* This_Const() const { return reinterpret_cast<const FootClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override {
		this->TechnoExtData::CalculateCRC(crc);
	}

};

class FootExtContainer final //: public Container<TechnoTypeExtData>
{
public:
	static FootExtContainer Instance;


	COMPILETIMEEVAL FORCEDINLINE  FootExtData* GetExtAttribute(FootClass* key)
	{
		return (FootExtData*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	COMPILETIMEEVAL FORCEDINLINE FootExtData* Find(FootClass* key)
	{
		return this->GetExtAttribute(key);
	}

	COMPILETIMEEVAL FORCEDINLINE FootExtData* TryFind(FootClass* key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}
};

class NOVTABLE FakeFootClass final
{
public:

	static bool __fastcall _IsRecruitable(FootClass* pThis, discard_t, HouseClass* pHouse);
	static DamageState __fastcall __Take_Damage(FootClass* pThis, discard_t, int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

};