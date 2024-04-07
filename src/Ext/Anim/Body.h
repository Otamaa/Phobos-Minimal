#pragma once
#include <AnimClass.h>

#include <Ext/Object/Body.h>

class AnimExtData final : public ObjectExtData
{
public:
	using base_type = AnimClass;
public:
	virtual AnimClass* GetAttachedObject() const override
	{
		return static_cast<AnimClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return ObjectExtData::GetSavedOffsetSize();
	}
};