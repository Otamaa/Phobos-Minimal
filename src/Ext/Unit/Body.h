#pragma once
#include <UnitClass.h>
#include <Ext/Foot/Body.h>

class UnitExtData : public FootExtData
{
public:
	using base_type = UnitClass;
public:
	virtual UnitClass* GetAttachedObject() const override
	{
		return static_cast<UnitClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->FootExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return FootExtData::GetSavedOffsetSize();
	}
};