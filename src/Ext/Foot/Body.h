#pragma once

#include <Ext/Techno/Body.h>

#include <FootClass.h>

class FootExtData : public TechnoExtData
{
public:

	FootExtData(FootClass* abs) : TechnoExtData(abs)
	{ };

	FootExtData(FootClass* abs, noinit_t& noint) : TechnoExtData(abs, noint) { };

	virtual ~FootExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {
		this->TechnoExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->TechnoExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->TechnoExtData::SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	FootClass* This() const override { return reinterpret_cast<FootClass*>(AttachedToObject); }
	const FootClass* This_Const() const override { return reinterpret_cast<const FootClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override {
		this->TechnoExtData::CalculateCRC(crc);
	}
};