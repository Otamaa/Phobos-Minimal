#pragma once
#include <SuperClass.h>

#include <Ext/Abstract/Body.h>

class SuperExtData final : public AbstractExtData
{
public:
	using base_type = SuperClass;
public:
	virtual SuperClass* GetAttachedObject() const override
	{
		return static_cast<SuperClass*>(this->AttachedToObject);
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
