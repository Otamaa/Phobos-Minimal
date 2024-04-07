#pragma once
#include <TerrainTypeClass.h>

#include <Ext/ObjectType/Body.h>

class TerrainTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = TerrainTypeClass;
public:
	virtual TerrainTypeClass* GetAttachedObject() const override
	{
		return static_cast<TerrainTypeClass*>(this->AttachedToObject);
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
