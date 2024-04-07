#pragma once

#include <AircraftClass.h>
#include <Ext/Foot/Body.h>

class AbstractClass;
class AircraftClass;
class WeaponTypeClass;
class AircraftExtData : public FootExtData
{
public:
	using base_type = AircraftClass;
	static constexpr DWORD Canary = 0x87654322;

public:

	virtual AircraftClass* GetAttachedObject() const override
	{
		return static_cast<AircraftClass*>(this->AttachedToObject);
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
		return FootExtData::GetSavedOffsetSize();
	}
};

struct AircraftExtContainer final : public Container<AircraftExtData>
{
	static AircraftExtContainer Instance;
};


