#pragma once
#include <CellClass.h>

#include <Ext/Abstract/Body.h>

class CellExtData final : public AbstractExtData
{
public:
	using base_type = CellClass;
public:
	virtual CellClass* GetAttachedObject() const override
	{
		return static_cast<CellClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->AbstractExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return AbstractExtData::GetSavedOffsetSize();
	}
};
