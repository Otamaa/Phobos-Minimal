#pragma once
#include <TemporalClass.h>

#include <Ext/Abstract/Body.h>

class TemporalExt : public AbstractExtData
{
public:
	using base_type = TemporalClass;
public:
	virtual TemporalClass* GetAttachedObject() const override
	{
		return static_cast<TemporalClass*>(this->AttachedToObject);
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