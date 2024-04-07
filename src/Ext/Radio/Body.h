#pragma once
#include <Ext/Mission/Body.h>

class RadioExtData : public MissionExtData {
public:
	virtual RadioClass* GetAttachedObject() const override
	{
		return static_cast<RadioClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->MissionExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override const
	{
		this->MissionExtData::SaveToStream(Stm);
	}
};