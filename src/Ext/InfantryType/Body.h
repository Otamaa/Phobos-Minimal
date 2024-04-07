#pragma once
#include <InfantryTypeClass.h>

#include <Ext/TechnoType/Body.h>

class InfantryTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = InfantryTypeClass;
public:
	virtual InfantryTypeClass* GetAttachedObject() const override
	{
		return static_cast<InfantryTypeClass*>(this->AttachedToObject);
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
