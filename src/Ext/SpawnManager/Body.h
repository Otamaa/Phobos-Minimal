#pragma once
#include <SpawnManagerClass.h>

#include <Ext/Abstract/Body.h>

class SpawnManagerExt : public AbstractExtData
{
public:
	using base_type = SpawnManagerClass;
public:
	virtual SpawnManagerClass* GetAttachedObject() const override
	{
		return static_cast<SpawnManagerClass*>(this->AttachedToObject);
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