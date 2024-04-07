#pragma once
#include <WarheadTypeClass.h>
#include <Ext/AbstractType/Body.h>

class WarheadTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = WarheadTypeClass;
public:
	virtual WarheadTypeClass* GetAttachedObject() const override
	{
		return static_cast<WarheadTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractTypeExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return AbstractTypeExtData::GetSavedOffsetSize();
	}

};
