#pragma once
#include <BulletTypeClass.h>

#include <Ext/ObjectType/Body.h>

class BulletTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = BulletTypeClass;
public:
	virtual BulletTypeClass* GetAttachedObject() const override
	{
		return static_cast<BulletTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return ObjectTypeExtData::GetSavedOffsetSize();
	}
};