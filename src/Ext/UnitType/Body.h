#pragma once
#include <UnitTypeClass.h>
#include <Ext/TechnoType/Body.h>

class UnitTypeExtData : public TechnoTypeExtData
{
public:
	using base_type = UnitTypeClass;
public:
	virtual UnitTypeClass* GetAttachedObject() const override
	{
		return static_cast<UnitTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->TechnoTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return TechnoTypeExtData::GetSavedOffsetSize();
	}

};