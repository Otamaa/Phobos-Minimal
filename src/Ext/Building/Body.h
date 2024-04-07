#pragma once
#include <BuildingClass.h>
#include <Ext/Techno/Body.h>

class BuildingExtData final : TechnoExtData
{
public:
	using base_type = BuildingClass;
public:
	virtual BuildingClass* GetAttachedObject() const override
	{
		return static_cast<BuildingClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->TechnoExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return TechnoExtData::GetSavedOffsetSize();
	}
};
