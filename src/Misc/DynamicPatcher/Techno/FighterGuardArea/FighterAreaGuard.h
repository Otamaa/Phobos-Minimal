#pragma once

#ifdef _aaaaaaa 
#include <GeneralStructures.h>
#include <CoordStruct.h>
#include <Utilities/Helpers.h>

#include <array>
enum class AircraftGuardState
{
	STOP, READY, GUARD, ROLLING, ATTACK, RELOAD
};

struct FighterAreaGuard
{
	AircraftClass* OwnerObject { nullptr };
	AircraftGuardState State { AircraftGuardState::STOP };
	bool Clockwise { false };
	CoordStruct destCenter {};
	std::list<CoordStruct> destList {};
	bool onStopCommand { false };
	int destIndex { 0 };

	//old
	bool isAreaProtecting { false };
	CoordStruct areaProtectTo { 0,0,0 };
	int currentAreaProtectedIndex { 0 };
	bool isAreaGuardReloading { false };
	int areaGuardTargetCheckRof { 20 };
	//

	bool IsAreaGuardRolling();
	void OnUpdate();
	void OnStopCommand();
	void StartAreaGuard();
	bool SetupDestination();
	bool SetupDestination(CoordStruct& dest);
	void CancelAreaGuard();
	void BackToAirport();
	bool FoundAndAttack(CoordStruct location);
	bool CanAttack(TechnoClass* pTarget, bool isPassiveAcquire = false);
	bool CheckTarget(TechnoClass* pTarget);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<FighterAreaGuard*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(OwnerObject, true)
			.Process(State)
			.Process(Clockwise)
			.Process(destCenter)
			.Process(destList)
			.Process(onStopCommand)
			.Process(destIndex)

			.Process(isAreaProtecting)
			.Process(areaProtectTo)
			.Process(currentAreaProtectedIndex)
			.Process(isAreaGuardReloading)
			.Process(areaGuardTargetCheckRof)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}
};

template <>
struct Savegame::ObjectFactory<FighterAreaGuard>
{
	std::unique_ptr<FighterAreaGuard> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<FighterAreaGuard>();
	}
};
#endif