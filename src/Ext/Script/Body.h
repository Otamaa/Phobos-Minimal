#pragma once

#include <ScriptClass.h>

#include <Ext/Abstract/Body.h>

class ScriptExtData final : public AbstractExtData
{
public:
	using base_type = ScriptClass;
public:
	virtual ScriptClass* GetAttachedObject() const override
	{
		return static_cast<ScriptClass*>(this->AttachedToObject);
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
