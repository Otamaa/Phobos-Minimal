#pragma once
#include <Ext/Object/Body.h>

class MissionExtData : public ObjectExtData {
public:
	using base_type = MissionClass;
public:

	virtual MissionClass* GetAttachedObject() const override
	{
		return static_cast<MissionClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return  ObjectExtData::GetSavedOffsetSize();
	}
};