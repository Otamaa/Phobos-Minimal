#pragma once
#include <TeamClass.h>
#include <Ext/Abstract/Body.h>

class TeamExtData final : public AbstractExtData
{
public:
	using base_type = TeamClass;
public:
	virtual TeamClass* GetAttachedObject() const override
	{
		return static_cast<TeamClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return AbstractExtData::GetSavedOffsetSize();
	}
};
