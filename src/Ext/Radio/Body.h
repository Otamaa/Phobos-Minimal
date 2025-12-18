#pragma once

#include <Ext/Mission/Body.h>

#include <RadioClass.h>

class RadioExtData : public MissionExtData
{
public:

	RadioExtData(RadioClass* abs) : MissionExtData(abs)
	{ };

	RadioExtData(RadioClass* abs, noinit_t& noint) : MissionExtData(abs, noint) { };

	virtual ~RadioExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->MissionExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->MissionExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->MissionExtData::SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	RadioClass* This() const override { return reinterpret_cast<RadioClass*>(AttachedToObject); }
	const RadioClass* This_Const() const override { return reinterpret_cast<const RadioClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override
	{
		this->MissionExtData::CalculateCRC(crc);
	}
};