#pragma once

#include <Utilities/Container.h>

#include <ObjectClass.h>

class ObjectExtData : public AbstractExtended
{
public:

	ObjectExtData(ObjectClass* abs) : AbstractExtended(abs)
	{ };

	ObjectExtData(ObjectClass* abs, noinit_t& noint) : AbstractExtended(abs, noint) { };

	virtual ~ObjectExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->Internal_LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->Internal_SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual ObjectClass* This() const override { return reinterpret_cast<ObjectClass*>(AbstractExtended::This()); }
	virtual const ObjectClass* This_Const() const override { return reinterpret_cast<const ObjectClass*>(AbstractExtended::This_Const()); }

	virtual void CalculateCRC(CRCEngine& crc) const override { }
};