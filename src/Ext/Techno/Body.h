#pragma once
#include <TechnoClass.h>

#include <Ext/Radio/Body.h>

class TechnoExtData : public RadioExtData
{
public:
	using base_type = TechnoClass;
public:
	virtual TechnoClass* GetAttachedObject() const override
	{
		return static_cast<TechnoClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->RadioExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->RadioExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return RadioExtData::GetSavedOffsetSize();
	}

};
