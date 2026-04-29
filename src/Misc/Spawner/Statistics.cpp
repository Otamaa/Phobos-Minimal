/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"

#include <CCFileClass.h>
#include <HouseClass.h>
#include <PacketClass.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <Unsorted.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <Ext/House/Body.h>
#include <New/Type/CrateTypeClass.h>

#include <Phobos.Lua.h>

#include <InfantryClass.h>
#include <AircraftClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>

bool FORCEDINLINE IsStatisticsEnabled()
{
	return SpawnerMain::Configs::Active
		&& SpawnerMain::GetGameConfigs()->WriteStatistics
		&& !SessionClass::IsCampaign();
}

// Write stats.dmp
ASMJIT_PATCH(0x6C856C, SendStatisticsPacket_WriteStatisticsDump, 0x5)
{
	if (IsStatisticsEnabled())
	{
		GET(void*, buf, EAX);

		CCFileClass statsFile { LuaData::StatisticPacketName.c_str() };
		if (statsFile.Open(FileAccessMode::Write))
		{
			statsFile.WriteBytes(buf, Game::SendStatistic_PacketSize());
			statsFile.Close();
		}

		Game::SendStatistic_Sended = true;

		return 0x6C87B8;
	}

	return 0;
}

// Send AI player
// Dont send observer
ASMJIT_PATCH(0x6C73F8, SendStatisticsPacket_HouseFilter, 0x6)
{
	enum { Send = 0x6C7406, DontSend = 0x6C7414 };

	if (IsStatisticsEnabled())
	{
		GET(HouseClass*, pHouse, EAX);

		const bool isMultiplayPassive = (pHouse && pHouse->Type && pHouse->Type->MultiplayPassive);
		const bool isObserver = (pHouse && pHouse->IsInitiallyObserver());

		return (isMultiplayPassive || isObserver)
			? DontSend
			: Send;
	}

	return 0;
}

// Use GameStockKeepingUnit instead IsWordDominationTour for GSKU Field
ASMJIT_PATCH(0x6C7053, SendStatisticsPacket_SaveGameStockKeepingUnit, 0x6)
{
	if (IsStatisticsEnabled())
		return 0x6C7030;

	return 0;
}

// Add Field HASH
// And use UIMapName instead ScenarioName for SCEN Field
ASMJIT_PATCH(0x6C735E, SendStatisticsPacket_AddField_HASH, 0x5)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A4, -0x8394));
		pPacket->AddField<wchar_t*>("SCEN", SpawnerMain::GetGameConfigs()->UIMapName, sizeof(SpawnerMain::GetGameConfigs()->UIMapName));
		pPacket->AddField<char*>("HASH", SpawnerMain::GetGameConfigs()->MapHash);
		return 0x6C737D;
	}

	return 0;
}

// Add Field MYID
ASMJIT_PATCH(0x6C7921, SendStatisticsPacket_AddField_MyId, 0x6)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A8, -0x8394));
		GET(HouseClass*, pHouse, ESI);
		GET(char, id, EBX);

		if (pHouse == HouseClass::CurrentPlayer())
		{
			pPacket->AddField<LONG>("MYID", id - '0');
			pPacket->AddField<DWORD>("NKEY", 0);
			pPacket->AddField<DWORD>("SKEY", 0);
		}
	}

	return 0;
}

// Add Player Fields
ASMJIT_PATCH(0x6C7989, SendStatisticsPacket_AddField_ALY, 0x6)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A4, -0x8394));
		GET(HouseClass*, pHouse, ESI);
		const char id = *reinterpret_cast<char*>(0x841F43);

		char fieldALY[] = "ALY*";
		fieldALY[3] = id;
		pPacket->AddField<DWORD>(fieldALY, pHouse->Allies.data);

		char fieldBSP[] = "BSP*";
		fieldBSP[3] = id;
		pPacket->AddField<DWORD>(fieldBSP, pHouse->GetSpawnPosition());
	}

	return 0;
}

ASMJIT_PATCH(0x6C882A, RegisterGameEndTime_CorrectDuration, 0x6)
{
	if (IsStatisticsEnabled())
	{
		const int startTime = ScenarioClass::Instance->ElapsedTimer.StartTime;
		R->ECX(startTime);
		return 0x6C882A + 0x6;
	}

	return 0;
}

ASMJIT_PATCH(0x55D0FB, AuxLoop_SendStatistics_1, 0x5)
{
	enum { Send = 0x55D100, DontSend = 0x55D123 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x55D189, AuxLoop_SendStatistics_2, 0x5)
{
	enum { Send = 0x55D18E, DontSend = 0x55D1B1 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x64C7FA, ExecuteDoList_SendStatistics_1, 0x6)
{
	enum { Send = 0x64C802, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x64C81E, ExecuteDoList_SendStatistics_2, 0x6)
{
	enum { Send = 0x64C826, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}


// add a call to RegisterGameEndTime here to set the correct GameEndTime
ASMJIT_PATCH(0x64C84B, ExecuteDoList_SendStatistics_3, 0x5)
{
	Game::RegisterGameEndTime();
	Game::SendStatisticsPacket();
	return 0x64C84B + 0x5;
}

ASMJIT_PATCH(0x647AE8, QueueAIMultiplayer_SendStatistics_1, 0x7)
{
	enum { Send = 0x647AF5, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x64823C, QueueAIMultiplayer_SendStatistics_2, 0x5)
{
	Debug::LogInfo("Failure executing DoList");

	enum { Send = 0x648257, DontSend = 0x64825C };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x64827D, QueueAIMultiplayer_SendStatistics_3, 0x6)
{
	enum { Send = 0x648285, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x648089, QueueAIMultiplayer_SendStatistics_4, 0x5)
{
	enum { Send = 0x64808E, DontSend = 0x648093 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

ASMJIT_PATCH(0x64B2E4, KickPlayerNow_SendStatistics, 0x7)
{
	enum { Send = 0x64B2ED, DontSend = 0x64B352 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}


#ifndef TRACKER_REPLACE

#include <PacketClass.h>

#define GET_Tracker(a) (a.GetTrackerptr<PhobosUnitTrackerClass>())

class NOVTABLE UnitTrackerPadWrapClass : public UnitTrackerPadClass
{
public:

	void IncrementUnitCount(int nUnit) {
		auto pTracker = this->GetTrackerptr<PhobosUnitTrackerClass>();
		auto count = pTracker->GetCounts();

		if (nUnit >= count)
			Debug::FatalError("IncrementTracking Index %d of avaible %d !", nUnit, count);

		pTracker->Increment(nUnit);
	}

	void DecrementUnitCount(int nUnit) {
		auto pTracker = this->GetTrackerptr<PhobosUnitTrackerClass>();
		auto count = pTracker->GetCounts();

		if (nUnit >= count)
			Debug::FatalError("DecrementTracking Index %d of avaible %d !", nUnit, count);

		pTracker->Decrement(nUnit);
	}

	int GetCounts() {
		return this->GetTrackerptr<PhobosUnitTrackerClass>()->GetAll();
	}

	void ClearCount() {
		this->GetTrackerptr<PhobosUnitTrackerClass>()->ClearCount();
	}
};
static_assert(sizeof(UnitTrackerPadWrapClass) == sizeof(UnitTrackerClass), "size missmatch !");
static_assert(!std::is_polymorphic_v<UnitTrackerPadWrapClass>, "UnitTrackerPadWrapClass has vtable!");
static_assert(alignof(UnitTrackerPadWrapClass) == alignof(UnitTrackerClass), "Alignment mismatch!");
static_assert(!std::is_polymorphic_v<PhobosUnitTrackerClass>, "PhobosUnitTrackerClass has vtable!");

ASMJIT_PATCH(0x4F58E9, HouseClass_CTOR_Trackers, 0x6){
	GET(HouseClass*, pThis ,EBP);

	pThis->TrackedBuiltAircraftTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltInfantryTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltUnitTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltBuildingTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledAircraftTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledInfantryTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledUnitTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledBuildingTypes.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedCapturedBuildings.AllocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedCollectedCrates.AllocateTrackerptr<PhobosUnitTrackerClass>();

	GET_Tracker(pThis->TrackedBuiltAircraftTypes)->Populate(AircraftTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedBuiltInfantryTypes)->Populate(InfantryTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedBuiltUnitTypes)->Populate(UnitTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedBuiltBuildingTypes)->Populate(BuildingTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedKilledAircraftTypes)->Populate(AircraftTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedKilledInfantryTypes)->Populate(InfantryTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedKilledUnitTypes)->Populate(UnitTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedKilledBuildingTypes)->Populate(BuildingTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedCapturedBuildings)->Populate(BuildingTypeClass::Array->Count);
	GET_Tracker(pThis->TrackedCollectedCrates)->Populate(CrateTypeClass::Array.size());

	return 0x4F5957;
}

DEFINE_JUMP(LJMP, 0x4F638F, 0x4F643B)

ASMJIT_PATCH(0x4F7527, HouseClass_DTOR_Trackers, 0x6){
	GET(HouseClass*, pThis ,ESI);

	pThis->TrackedBuiltAircraftTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltInfantryTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltUnitTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedBuiltBuildingTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledAircraftTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledInfantryTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledUnitTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedKilledBuildingTypes.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedCapturedBuildings.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	pThis->TrackedCollectedCrates.DeallocateTrackerptr<PhobosUnitTrackerClass>();
	return 0x4F7595;
}

ASMJIT_PATCH(0x6C92CB, StandaloneScore_SinglePlayerScoreDialog_Trackers, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	int sum = 0;

	sum += GET_Tracker(pHouse->TrackedKilledAircraftTypes)->GetCounts();
	sum += GET_Tracker(pHouse->TrackedKilledInfantryTypes)->GetCounts();
	sum += GET_Tracker(pHouse->TrackedKilledUnitTypes)->GetCounts();
	sum += GET_Tracker(pHouse->TrackedKilledBuildingTypes)->GetCounts();
	R->ESI(sum);
	return 0x6C9303;
}

template <typename T>
void FillTracker(HouseClass* pHouse, PhobosUnitTrackerClass* tracker) {
	for (int i = 0; i < T::Array->Count; ++i) {

		if (T::Array->Items[i] && T::Array->Items[i]->Owner == pHouse) {
			tracker->Increment(T::Array->Items[i]->Type->ArrayIndex);
		}
	}
}

ASMJIT_PATCH(0x6C7B68, SendStatistic_Trackers, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	LEA_STACK(PacketClass*, pPacket, 0x83A4 - 0x8394);
	char last = R->BL();

	auto FillPacket = [pPacket ,last](PhobosUnitTrackerClass* tracker , PacketFieldRep field, bool clear)
	{
		Game::PacketFields[(int)field].str[3] = last;
		auto[data , size] = tracker->SendNetworkFlatData();
		pPacket->AddField<void*>(Game::PacketFields[(int)field].str, (void*)data, size);

		if(clear)
			tracker->Clear();
	};

	//Send Previous Built Data
	FillPacket(GET_Tracker(pHouse->TrackedBuiltInfantryTypes), PacketFieldRep::INB, true);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltUnitTypes), PacketFieldRep::UNB, true);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltAircraftTypes), PacketFieldRep::PLB, true);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltBuildingTypes), PacketFieldRep::BLB, true);

	//Re-Count Built Data
	FillTracker<InfantryClass>(pHouse, pHouse->TrackedBuiltInfantryTypes.GetTrackerptr<PhobosUnitTrackerClass>());
	FillTracker<UnitClass>(pHouse, pHouse->TrackedBuiltUnitTypes.GetTrackerptr<PhobosUnitTrackerClass>());
	FillTracker<AircraftClass>(pHouse, pHouse->TrackedBuiltAircraftTypes.GetTrackerptr<PhobosUnitTrackerClass>());
	FillTracker<BuildingClass>(pHouse, pHouse->TrackedBuiltBuildingTypes.GetTrackerptr<PhobosUnitTrackerClass>());
	//Send Current Built Data
	FillPacket(GET_Tracker(pHouse->TrackedBuiltInfantryTypes), PacketFieldRep::INL, false);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltUnitTypes), PacketFieldRep::UNL, false);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltAircraftTypes), PacketFieldRep::PLL, false);
	FillPacket(GET_Tracker(pHouse->TrackedBuiltBuildingTypes), PacketFieldRep::BLL, false);

	//Send Current Kill Data
	FillPacket(GET_Tracker(pHouse->TrackedKilledInfantryTypes), PacketFieldRep::INK, false);
	FillPacket(GET_Tracker(pHouse->TrackedKilledUnitTypes), PacketFieldRep::UNK, false);
	FillPacket(GET_Tracker(pHouse->TrackedKilledAircraftTypes), PacketFieldRep::PLK, false);
	FillPacket(GET_Tracker(pHouse->TrackedKilledBuildingTypes), PacketFieldRep::BLK, false);

	//Send Current Capture Data
	FillPacket(GET_Tracker(pHouse->TrackedCapturedBuildings), PacketFieldRep::BLC, false);

	//Send Current Crate Data
	FillPacket(GET_Tracker(pHouse->TrackedCollectedCrates), PacketFieldRep::CRA, false);


	return 0x6C8369;
}

void HouseExtData::IncremetCrateTracking(HouseClass* pHouse, Powerup type) {
	if (IsStatisticsEnabled() || SessionClass::Instance->GameMode == GameMode::Internet) {
		GET_Tracker(pHouse->TrackedCollectedCrates)->Increment((int)type);
	}
}

ASMJIT_PATCH(0x448524, BuildingClass_Captured_SendStatistics, 0x7)
{
	enum { Send = 0x44852D, DontSend = 0x448559 };
	GET(HouseClass*, pNewOwner, EBX);
	GET(BuildingClass*, pThis, ESI);

	if ((IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet))
		&& !pThis->Type->DontScore ) {
		GET_Tracker(pNewOwner->TrackedCapturedBuildings)->Increment(pThis->Type->ArrayIndex);
	}

	return DontSend;
}

DEFINE_FUNCTION_JUMP(CALL, 0x4FF854, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703152, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x7034B4, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x4FF893, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703198, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x7034F4, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x4FF7FB, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703108, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703474, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x4FF7BD, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703093, UnitTrackerPadWrapClass::IncrementUnitCount);
DEFINE_FUNCTION_JUMP(CALL, 0x703403, UnitTrackerPadWrapClass::IncrementUnitCount);
#else

ASMJIT_PATCH(0x448524, BuildingClass_Captured_SendStatistics, 0x7)
{
	enum { Send = 0x44852D, DontSend = 0x448559 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

void HouseExtData::IncremetCrateTracking(HouseClass* pHouse, Powerup type)
{
	if ((IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)) && (int)type < 19u) {
		pHouse->TrackedCollectedCrates.IncrementUnitCount((int)type);
	}
}


#endif