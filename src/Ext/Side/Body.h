#pragma once
#include <SideClass.h>

#include <Ext/AbstractType/Body.h>

class SideExtData final : public AbstractTypeExtData
{
public:
	using base_type = SideClass;
public:
	virtual SideClass* GetAttachedObject() const override
	{
		return static_cast<SideClass*>(this->AttachedToObject);
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
