#pragma once
#include <ParticleClass.h>

#include <Ext/Object/Body.h>

class ParticleExtData final : public ObjectExtData
{
public:
	using base_type = ParticleClass;
public:
	virtual ParticleClass* GetAttachedObject() const override
	{
		return static_cast <ParticleClass*>(this->AttachedToObject);
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
