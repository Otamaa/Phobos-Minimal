#pragma once
#include <CaptureManagerClass.h>

#include <Ext/Abstract/Body.h>

class CaptureExt : public AbstractExtData
{
public:
	using base_type = CaptureManagerClass;
public:
	virtual CaptureManagerClass* GetAttachedObject() const override
	{
		return static_cast<CaptureManagerClass*>(this->AttachedToObject);
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