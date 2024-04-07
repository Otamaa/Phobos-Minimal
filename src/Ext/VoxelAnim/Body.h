#pragma once

#include <VoxelAnimClass.h>

#include <Ext/Object/Body.h>

class VoxelAnimExtData final : public ObjectExtData
{
public:
	using base_type = VoxelAnimClass;
public:
	virtual VoxelAnimClass* GetAttachedObject() const override
	{
		return static_cast<VoxelAnimClass*>(this->AttachedToObject);
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
