#pragma once

#include <ObjectClass.h>

#include <Utilities/Container.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>

class ObjectExtData : public AbstractExtData {
public:
	using base_type = ObjectClass;
public:

	virtual ObjectClass* GetAttachedObject() const override
	{
		return static_cast<ObjectClass*>(this->AttachedToObject);
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
		return  AbstractExtData::GetSavedOffsetSize();
	}
};