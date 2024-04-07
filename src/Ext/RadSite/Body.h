#pragma once

#include <RadSiteClass.h>

#include <Ext/Abstract/Body.h>

class RadSiteExtData final : public AbstractExtData
{
public:
	using base_type = RadSiteClass;
public:
	virtual RadSiteClass* GetAttachedObject() const override
	{
		return static_cast<RadSiteClass*>(this->AttachedToObject);
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
