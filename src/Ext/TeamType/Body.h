#pragma once
#include <TeamTypeClass.h>

#include <Ext/AbstractType/Body.h>

class TeamTypeExtData : public AbstractTypeExtData
{
public:
	using base_type = TeamTypeClass;
public:
	virtual TeamTypeClass* GetAttachedObject() const override
	{
		return static_cast<TeamTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return AbstractTypeExtData::GetSavedOffsetSize();
	}
};