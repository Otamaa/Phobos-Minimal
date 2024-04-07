#pragma once

#include <TEventClass.h>
#include <Ext/Abstract/Body.h>

class TEventExtData final : public AbstractExtData
{
public:
	using base_type = TEventClass;
public:
	virtual TEventClass* GetAttachedObject() const override
	{
		return static_cast<TEventClass*>(this->AttachedToObject);
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
