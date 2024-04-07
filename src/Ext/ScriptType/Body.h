#pragma once

#include <ScriptTypeClass.h>

#include <Ext/AbstractType/Body.h>

class ScriptTypeExt : public AbstractTypeExtData
{
public:
	using base_type = ScriptTypeClass;
public:
	virtual ScriptTypeClass* GetAttachedObject() const override
	{
		return static_cast<ScriptTypeClass*>(this->AttachedToObject);
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