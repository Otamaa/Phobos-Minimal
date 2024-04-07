#pragma once
#include <AircraftTypeClass.h>

#include <Ext/TechnoType/Body.h>

class AircraftTypeExtData : public TechnoTypeExtData {
public:
	using base_type = AircraftTypeClass;
	static constexpr DWORD Canary = 0x87654323;
public:
	virtual AircraftTypeClass* GetAttachedObject() const override
	{
		return static_cast<AircraftTypeClass*>(this->AttachedToObject);
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

struct AircraftTypeExtContainer final : public Container<AircraftTypeExtData>
{
	static AircraftTypeExtContainer Instance;
};