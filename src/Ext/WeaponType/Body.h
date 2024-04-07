#pragma once
#include <WeaponTypeClass.h>
#include <Ext/AbstractType/Body.h>

class WeaponTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = WeaponTypeClass;
public:
	virtual WeaponTypeClass* GetAttachedObject() const override
	{
		return static_cast<WeaponTypeClass*>(this->AttachedToObject);
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
		return AbstractTypeExtData::GetSavedOffsetSize();
	}
};