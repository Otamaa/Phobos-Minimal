#pragma once

#include <Ext/TechnoType/Body.h>

class FootTypeExtData : public TechnoTypeExtData
{
public:

	FootTypeExtData(TechnoTypeClass* abs) : TechnoTypeExtData(abs)
	{ };

	FootTypeExtData(TechnoTypeClass* abs, noinit_t& noint) : TechnoTypeExtData(abs, noint) { };

	virtual ~FootTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {
		this->TechnoTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->TechnoTypeExtData::LoadFromStream(Stm);
		const_cast<FootTypeExtData*>(this)->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->TechnoTypeExtData::SaveToStream(Stm);
		const_cast<FootTypeExtData*>(this)->Serialize(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const override {
		this->TechnoTypeExtData::CalculateCRC(crc);
	}

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm)
	{

	}

};

class FootTypeExtContainer final //: public Container<TechnoTypeExtData>
{
public:
	static FootTypeExtContainer Instance;

	COMPILETIMEEVAL FORCEDINLINE  FootTypeExtData* GetExtAttribute(TechnoTypeClass* key)
	{
		return (FootTypeExtData*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	COMPILETIMEEVAL FORCEDINLINE FootTypeExtData* Find(TechnoTypeClass* key)
	{
		return this->GetExtAttribute(key);
	}

	COMPILETIMEEVAL FORCEDINLINE FootTypeExtData* TryFind(TechnoTypeClass* key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}
};
