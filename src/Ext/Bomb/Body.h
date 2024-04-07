#pragma once
#include <BombClass.h>

#include <Ext/Abstract/Body.h>

class BombExtData final : AbstractExtData
{
public:
	using base_type = BombClass;
public:
	virtual BombClass* GetAttachedObject() const override
	{
		return static_cast<BombClass*>(this->AttachedToObject);
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
