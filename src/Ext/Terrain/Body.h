#pragma once
#include <TerrainClass.h>
#include <Ext/Object/Body.h>

class TerrainExtData final : public ObjectExtData
{
public:
	using base_type = TerrainClass;
public:
	virtual TerrainClass* GetAttachedObject() const override
	{
		return static_cast<TerrainClass*>(this->AttachedToObject);
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
