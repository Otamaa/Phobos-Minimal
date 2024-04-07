#pragma once
#include <SuperWeaponTypeClass.h>

#include <Ext/AbstractType/Body.h>

class SWTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = SuperWeaponTypeClass;
public:
	virtual SuperWeaponTypeClass* GetAttachedObject() const override
	{
		return static_cast<SuperWeaponTypeClass*>(this->AttachedToObject);
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
