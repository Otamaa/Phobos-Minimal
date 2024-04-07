#pragma once

#include <ObjectTypeClass.h>

#include <Ext/AbstractType/Body.h>

class ObjectTypeExtData : public AbstractTypeExtData {
public:
	using base_type = ObjectTypeClass;
public:
	virtual ObjectTypeClass* GetAttachedObject() const override
	{
		return static_cast<ObjectTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return  AbstractTypeExtData::GetSavedOffsetSize();
	}
};