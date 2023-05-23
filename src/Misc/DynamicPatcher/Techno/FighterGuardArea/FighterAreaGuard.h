#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
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
	std::vector<CoordStruct> destList {};
	bool onStopCommand { false };
	int destIndex { 0 };

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

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(isAreaProtecting)
			.Process(OwnerObject)
			.Process(State)
			.Process(Clockwise)
			.Process(destCenter)
			.Process(destList)
			.Process(onStopCommand)
			.Process(destIndex)
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