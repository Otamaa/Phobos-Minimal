#pragma once
#include <TechnoTypeClass.h>

#include <Ext/ObjectType/Body.h>

class TechnoTypeExtData : public ObjectTypeExtData
{
public:
	using base_type = TechnoTypeClass;
public:
	virtual TechnoTypeClass* GetAttachedObject() const override
	{
		return static_cast<TechnoTypeClass*>(this->AttachedToObject);
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
