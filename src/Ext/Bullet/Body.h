#pragma once
#include <BulletClass.h>
#include <Ext/Object/Body.h>

class BulletExtData : public ObjectExtData
{
public:
	using base_type = BulletClass;
public:
	virtual BulletClass* GetAttachedObject() const override
	{
		return static_cast<BulletClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return ObjectExtData::GetSavedOffsetSize();
	}
};
