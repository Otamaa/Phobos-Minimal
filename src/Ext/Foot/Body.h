#pragma once

#include <Ext/Techno/Body.h>

class FootExtData : public TechnoExtData {
public:
	using base_type = FootClass;

public:

	virtual FootClass* GetAttachedObject() const override
	{
		return static_cast<FootClass*>(this->AttachedToObject);
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
		return  TechnoExtData::GetSavedOffsetSize();
	}
};