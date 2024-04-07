#pragma once

#include <AITriggerTypeClass.h>
#include <Ext/AbstractType/Body.h>

class AITriggerTypeExtData : public AbstractTypeExtData
{
public:
	using base_type = AITriggerTypeClass;
public:
	virtual AITriggerTypeClass* GetAttachedObject() const override
	{
		return static_cast<AITriggerTypeClass*>(this->AttachedToObject);
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