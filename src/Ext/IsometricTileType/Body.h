#pragma once
#include <IsometricTileTypeClass.h>

#include <Ext/ObjectType/Body.h>

class IsometricTileTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = IsometricTileTypeClass;
public:
	virtual IsometricTileTypeClass* GetAttachedObject() const override
	{
		return static_cast<IsometricTileTypeClass*>(this->AttachedToObject);
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
