#pragma once
#include <VoxelAnimTypeClass.h>

#include <Ext/ObjectType/Body.h>

class VoxelAnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = VoxelAnimTypeClass;
public:
	virtual VoxelAnimTypeClass* GetAttachedObject() const override
	{
		return static_cast<VoxelAnimTypeClass*>(this->AttachedToObject);
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
