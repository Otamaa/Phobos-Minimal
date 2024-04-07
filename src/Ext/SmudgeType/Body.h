#pragma once
#include <SmudgeTypeClass.h>

#include <Ext/ObjectType/Body.h>

class SmudgeTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = SmudgeTypeClass;
public:
	virtual SmudgeTypeClass* GetAttachedObject() const override
	{
		return static_cast<SmudgeTypeClass*>(this->AttachedToObject);
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
