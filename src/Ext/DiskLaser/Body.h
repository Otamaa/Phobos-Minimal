#pragma once
#include <DiskLaserClass.h>

#include <Ext/Abstract/Body.h>

class DiskLaserExt : public AbstractExtData
{
public:
	using base_type = DiskLaserClass;
public:
	virtual DiskLaserClass* GetAttachedObject() const override
	{
		return static_cast<DiskLaserClass*>(this->AttachedToObject);
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