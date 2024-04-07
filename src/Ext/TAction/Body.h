#pragma once
#include <TActionClass.h>
#include <Ext/Abstract/Body.h>

class TActionExt : public AbstractExtData
{
public:
	using base_type = TActionClass;
public:
	virtual TActionClass* GetAttachedObject() const override
	{
		return static_cast<TActionClass*>(this->AttachedToObject);
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
