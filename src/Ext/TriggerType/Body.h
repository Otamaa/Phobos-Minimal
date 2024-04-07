#pragma once

#include <TriggerTypeClass.h>
#include <Ext/AbstractType/Body.h>

class TriggerTypeExt : public AbstractTypeExtData
{
public:
	using base_type = TriggerTypeClass;
public:
	virtual TriggerTypeClass* GetAttachedObject() const override
	{
		return static_cast<TriggerTypeClass*>(this->AttachedToObject);
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