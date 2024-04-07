#pragma once

#include <AbstractTypeClass.h>
#include <Ext/Abstract/Body.h>

class AbstractTypeExtData : public AbstractExtData {
public:
	virtual AbstractTypeClass* GetAttachedObject() const override {
		return static_cast<AbstractTypeClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override const
	{
		this->AbstractExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		return AbstractExtData::GetSavedOffsetSize();
	}
};