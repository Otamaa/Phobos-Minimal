#pragma once

#include <AnimTypeClass.h>
#include <Ext/ObjectType/Body.h>

class AnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = AnimTypeClass;
public:
	virtual AnimTypeClass* GetAttachedObject() const override
	{
		return static_cast<AnimTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return ObjectTypeExtData::GetSavedOffsetSize();
	}
};
