#pragma once
#include <HouseTypeClass.h>

#include <Ext/AbstractType/Body.h>

class HouseTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = HouseTypeClass;
public:
	virtual HouseTypeClass* GetAttachedObject() const override
	{
		return static_cast<HouseTypeClass*>(this->AttachedToObject);
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
