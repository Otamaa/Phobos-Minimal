#pragma once
#include <ParasiteClass.h>

#include <Ext/Abstract/Body.h>

class ParasiteExt : public AbstractExtData
{
public:
	using base_type = ParasiteClass;
public:
	virtual ParasiteClass* GetAttachedObject() const override
	{
		return static_cast<ParasiteClass*>(this->AttachedToObject);
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