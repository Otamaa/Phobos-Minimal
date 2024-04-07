#pragma once
#include <ParticleTypeClass.h>

#include <Ext/ObjectType/Body.h>

class ParticleTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleTypeClass;
public:
	virtual ParticleTypeClass* GetAttachedObject() const override
	{
		return static_cast<ParticleTypeClass*>(this->AttachedToObject);
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
