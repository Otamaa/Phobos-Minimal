#pragma once
#include <Ext/Mission/Body.h>

class RadioExtData : public MissionExtData {
public:
	using base_type = RadioClass;
public:
	virtual RadioClass* GetAttachedObject() const override
	{
		return static_cast<RadioClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->MissionExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->MissionExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return MissionExtData::GetSavedOffsetSize();
	}
};