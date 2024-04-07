#pragma once
#include <AircraftTypeClass.h>

#include <Ext/TechnoType/Body.h>

class AircraftTypeExtData : public TechnoTypeExtData {

	virtual AircraftTypeClass* GetAttachedObject() const override
	{
		return static_cast<AircraftTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override const
	{
		this->TechnoTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return TechnoTypeExtData::GetSavedOffsetSize();
	}
};

struct AircraftTypeExtContainer final : public Container<AircraftTypeExtData, 0x87654323>
{
	static AircraftTypeExtContainer Instance;
};