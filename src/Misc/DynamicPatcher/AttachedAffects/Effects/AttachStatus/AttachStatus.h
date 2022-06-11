#pragma once

#include "../Base.h"
#include "AttachStatusType.h"

class AttachStatus : public Effect<AttachStatusType>
{
public:
	bool Active;
	void* ExtData;

	AttachStatus() : Effect<AttachStatusType> { MyType::Stats }
		, Active { false }
		, ExtData { nullptr }
	{ }

	AttachStatus(const AttachStatusType& nType) : Effect<AttachStatusType> { MyType::Stats }
		, Active { false }
		, ExtData { nullptr }
	{ SetTypeData(nType); }


	bool IsAlive() override {
		return Active;
	}

	void OnEnable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker) override;

	void Disable(CoordStruct location) override;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Active)
			.Process(ExtData)
			.Success()
			;
	}
};