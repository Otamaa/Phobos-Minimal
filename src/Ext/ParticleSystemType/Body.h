#pragma once
#include <ParticleSystemTypeClass.h>


#include <Ext/ObjectType/Body.h>

class ParticleSystemTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleSystemTypeClass;
public:
	virtual ParticleSystemTypeClass* GetAttachedObject() const override
	{
		return static_cast<ParticleSystemTypeClass*>(this->AttachedToObject);
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
