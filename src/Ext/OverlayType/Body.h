#pragma once

#include <OverlayTypeClass.h>

#include <Ext/ObjectType/Body.h>

class OverlayTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = OverlayTypeClass;
public:
	virtual OverlayTypeClass* GetAttachedObject() const override
	{
		return static_cast<OverlayTypeClass*>(this->AttachedToObject);
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

