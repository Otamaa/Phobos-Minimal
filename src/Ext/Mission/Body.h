#pragma once

#include <Ext/Object/Body.h>

#include <MissionClass.h>

class MissionExtData : public ObjectExtData
{
public:

	MissionExtData(MissionClass* abs) : ObjectExtData(abs)
	{ };

	MissionExtData(MissionClass* abs, noinit_t& noint) : ObjectExtData(abs, noint) { };

	virtual ~MissionExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override {
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override {
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override {
		this->ObjectExtData::SaveToStream(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	MissionClass* This() const { return reinterpret_cast<MissionClass*>(AttachedToObject); }
	const MissionClass* This_Const() const { return reinterpret_cast<const MissionClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override {
		this->ObjectExtData::CalculateCRC(crc);
	}
};