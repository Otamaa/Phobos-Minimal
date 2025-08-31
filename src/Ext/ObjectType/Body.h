#pragma once

#include <Ext/AbstractType/Body.h>
#include <ObjectTypeClass.h>

class ObjectTypeExtData : public AbstractTypeExtData
{
public:

	ObjectTypeExtData(ObjectTypeClass* pObj) : AbstractTypeExtData(pObj) { }
	ObjectTypeExtData(ObjectTypeClass* pObj, noinit_t& nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~ObjectTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::Internal_LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->AbstractTypeExtData::Internal_SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	virtual ObjectTypeClass* This() const override { return reinterpret_cast<ObjectTypeClass*>(this->AbstractTypeExtData::This()); }
	virtual const ObjectTypeClass* This_Const() const override { return reinterpret_cast<const ObjectTypeClass*>(this->AbstractTypeExtData::This_Const()); }
};