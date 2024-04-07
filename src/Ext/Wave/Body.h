#pragma once
#include <WaveClass.h>

#include <Ext/Object/Body.h>

class WaveExtData final : public ObjectExtData
{
public:
	using base_type = WaveClass;
public:
	virtual WaveClass* GetAttachedObject() const override
	{
		return static_cast<WaveClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->ObjectExtData::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		//AttachedToObject
		return ObjectExtData::GetSavedOffsetSize();
	}

};
