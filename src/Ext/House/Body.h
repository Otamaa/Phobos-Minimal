#pragma once
#include <HouseClass.h>

#include <Ext/Abstract/Body.h>

class HouseExtData final : public AbstractExtData
{
public:
	using base_type = HouseClass;

public:
	virtual HouseClass* GetAttachedObject() const override
	{
		return static_cast<HouseClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return AbstractExtData::GetSavedOffsetSize();
	}
};
