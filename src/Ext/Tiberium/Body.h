#pragma once
#include <TiberiumClass.h>
#include <Ext/AbstractType/Body.h>

class TiberiumExtData final : public AbstractTypeExtData
{
public:
	using base_type = TiberiumClass;
public:
	virtual TiberiumClass* GetAttachedObject() const override
	{
		return static_cast<TiberiumClass*>(this->AttachedToObject);
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