#pragma once

#include <ParticleSystemClass.h>

#include <Ext/Object/Body.h>

class ParticleSystemExtData final : public ObjectExtData
{
public:
	using base_type = ParticleSystemClass;
public:
	virtual ParticleSystemClass* GetAttachedObject() const override
	{
		return static_cast<ParticleSystemClass*>(this->AttachedToObject);
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
