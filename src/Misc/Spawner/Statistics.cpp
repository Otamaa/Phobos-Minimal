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

bool FORCEDINLINE IsStatisticsEnabled()
{
	return SpawnerMain::Configs::Active
		&& SpawnerMain::GetGameConfigs()->WriteStatistics
		&& !SessionClass::IsCampaign();
}

// Write stats.dmp
DEFINE_HOOK(0x6C856C, SendStatisticsPacket_WriteStatisticsDump, 0x5)
{
	if (IsStatisticsEnabled())
	{
		GET(void*, buf, EAX);

		CCFileClass statsFile { "stats.dmp" };
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
DEFINE_HOOK(0x6C73F8, SendStatisticsPacket_HouseFilter, 0x6)
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
DEFINE_HOOK(0x6C7053, SendStatisticsPacket_SaveGameStockKeepingUnit, 0x6)
{
	if (IsStatisticsEnabled())
		return 0x6C7030;

	return 0;
}

// Add Field HASH
// And use UIMapName instead ScenarioName for SCEN Field
DEFINE_HOOK(0x6C735E, SendStatisticsPacket_AddField_HASH, 0x5)
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
DEFINE_HOOK(0x6C7921, SendStatisticsPacket_AddField_MyId, 0x6)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A8, -0x8394));
		GET(HouseClass*, pHouse, ESI);
		GET(char, id, EBX);

		if (pHouse == HouseClass::CurrentPlayer)
		{
			pPacket->AddField<LONG>("MYID", id - '0');
			pPacket->AddField<DWORD>("NKEY", 0);
			pPacket->AddField<DWORD>("SKEY", 0);
		}
	}

	return 0;
}

// Add Player Fields
DEFINE_HOOK(0x6C7989, SendStatisticsPacket_AddField_ALY, 0x6)
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

DEFINE_HOOK(0x6C882A, RegisterGameEndTime_CorrectDuration, 0x6)
{
	if (IsStatisticsEnabled())
	{
		const int startTime = ScenarioClass::Instance->ElapsedTimer.StartTime;
		R->ECX(startTime);
		return 0x6C882A + 0x6;
	}

	return 0;
}

DEFINE_HOOK(0x55D0FB, AuxLoop_SendStatistics_1, 0x5)
{
	enum { Send = 0x55D100, DontSend = 0x55D123 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x55D189, AuxLoop_SendStatistics_2, 0x5)
{
	enum { Send = 0x55D18E, DontSend = 0x55D1B1 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64C7FA, ExecuteDoList_SendStatistics_1, 0x6)
{
	enum { Send = 0x64C802, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64C81E, ExecuteDoList_SendStatistics_2, 0x6)
{
	enum { Send = 0x64C826, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x647AE8, QueueAIMultiplayer_SendStatistics_1, 0x7)
{
	enum { Send = 0x647AF5, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64823C, QueueAIMultiplayer_SendStatistics_2, 0x5)
{
	Debug::Log(reinterpret_cast<char*>(0x8373BC) /* "Failure executing DoList\n" */);

	enum { Send = 0x648257, DontSend = 0x64825C };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64827D, QueueAIMultiplayer_SendStatistics_3, 0x6)
{
	enum { Send = 0x648285, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x648089, QueueAIMultiplayer_SendStatistics_4, 0x5)
{
	enum { Send = 0x64808E, DontSend = 0x648093 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64B2E4, KickPlayerNow_SendStatistics, 0x7)
{
	enum { Send = 0x64B2ED, DontSend = 0x64B352 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

#ifdef TRACKER_REPLACE
#include <PacketClass.h>

DEFINE_JUMP(LJMP, 0x4F638F, 0x4F643B)

DEFINE_HOOK(0x6C92CB, StandaloneScore_SinglePlayerScoreDialog_Trackers, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	int sum = 0;
	const auto pExt = HouseExtContainer::Instance.Find(pHouse);
	sum += pExt->KilledAircraftTypes.GetAll();
	sum += pExt->KilledInfantryTypes.GetAll();
	sum += pExt->KilledUnitTypes.GetAll();
	sum += pExt->KilledBuildingTypes.GetAll();
	R->ESI(sum);
	return 0x6C9303;
}

template <typename T>
void FillTracker(HouseClass* pHouse, TrackerClass& tracker) {
	for (int i = 0; i < T::Array->Count; ++i) {

		if (T::Array->Items[i] && T::Array->Items[i]->Owner == pHouse) {
			tracker.Increment(T::Array->Items[i]->Type->ArrayIndex);
		}
	}
}

DEFINE_HOOK(0x6C7B68, SendStatistic_Trackers, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	LEA_STACK(PacketClass*, pPacket, 0x83A4 - 0x8394);
	char last = R->BL();

	const auto pExt = HouseExtContainer::Instance.Find(pHouse);


	auto FillPacket = [pPacket ,last](TrackerClass& tracker , PacketFieldRep field)
	{
		int size = 0;
		for (int i = 0; i < tracker.GetCounts(); ++i) {
			if (tracker.GetCount(i)){
				size = i + 1;
			}
		}

		Game::PacketFields[(int)field].str[3] = last;
		pPacket->AddField<void*>(Game::PacketFields[(int)field].str, (void*)tracker.GetData(), size);
	};

	//Send Previous Built Data
	pExt->BuiltAircraftTypes.ToNetwork();
	pExt->BuiltInfantryTypes.ToNetwork();
	pExt->BuiltUnitTypes.ToNetwork();
	pExt->BuiltBuildingTypes.ToNetwork();

	FillPacket(pExt->BuiltInfantryTypes, PacketFieldRep::INB);
	FillPacket(pExt->BuiltUnitTypes, PacketFieldRep::UNB);
	FillPacket(pExt->BuiltAircraftTypes, PacketFieldRep::PLB);
	FillPacket(pExt->BuiltBuildingTypes, PacketFieldRep::BLB);

	pExt->BuiltAircraftTypes.ToPC();
	pExt->BuiltInfantryTypes.ToPC();
	pExt->BuiltUnitTypes.ToPC();
	pExt->BuiltBuildingTypes.ToPC();
	//

	//Send Current Built Data
	pExt->BuiltAircraftTypes.ClearCount();
	pExt->BuiltInfantryTypes.ClearCount();
	pExt->BuiltUnitTypes.ClearCount();
	pExt->BuiltBuildingTypes.ClearCount();

	FillTracker<UnitClass>(pHouse, pExt->BuiltUnitTypes);
	FillTracker<InfantryClass>(pHouse, pExt->BuiltInfantryTypes);
	FillTracker<AircraftClass>(pHouse, pExt->BuiltAircraftTypes);
	FillTracker<BuildingClass>(pHouse, pExt->BuiltBuildingTypes);

	pExt->BuiltAircraftTypes.ToNetwork();
	pExt->BuiltInfantryTypes.ToNetwork();
	pExt->BuiltUnitTypes.ToNetwork();
	pExt->BuiltBuildingTypes.ToNetwork();

	FillPacket(pExt->BuiltInfantryTypes, PacketFieldRep::INL);
	FillPacket(pExt->BuiltUnitTypes, PacketFieldRep::UNL);
	FillPacket(pExt->BuiltAircraftTypes, PacketFieldRep::PLL);
	FillPacket(pExt->BuiltBuildingTypes, PacketFieldRep::BLL);
	//

	//Send Kill Data
	pExt->KilledInfantryTypes.ToNetwork();
	pExt->KilledUnitTypes.ToNetwork();
	pExt->KilledAircraftTypes.ToNetwork();
	pExt->KilledBuildingTypes.ToNetwork();

	FillPacket(pExt->KilledInfantryTypes, PacketFieldRep::INK);
	FillPacket(pExt->KilledUnitTypes, PacketFieldRep::UNK);
	FillPacket(pExt->KilledAircraftTypes, PacketFieldRep::PLK);
	FillPacket(pExt->KilledBuildingTypes, PacketFieldRep::BLK);
	//

	//Send Captured Data
	pExt->CapturedBuildings.ToNetwork();
	FillPacket(pExt->CapturedBuildings, PacketFieldRep::BLC);
	//

	//Semd ColledtedCrates Data
	pExt->CollectedCrates.ToNetwork();
	FillPacket(pExt->CollectedCrates, PacketFieldRep::CRA);
	//

	return 0x6C8369;
}

template<uintptr_t offset>
FORCEDINLINE HouseClass* GetHouseClassptr(UnitTrackerClass* pTrack)
{
	uintptr_t ptr = (uintptr_t)pTrack;
	return reinterpret_cast<HouseClass*>(ptr - offset);
}

void HouseExtData::IncremetCrateTracking(HouseClass* pHouse, Powerup type) {
	if (IsStatisticsEnabled() || SessionClass::Instance->GameMode == GameMode::Internet) {
		HouseExtContainer::Instance.Find(pHouse)->CollectedCrates.Increment((int)type);
	}
}

DEFINE_HOOK(0x448524, BuildingClass_Captured_SendStatistics, 0x7)
{
	enum { Send = 0x44852D, DontSend = 0x448559 };
	GET(HouseClass*, pNewOwner, EBX);
	GET(BuildingClass*, pThis, ESI);

	if ((IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet))
		&& !pThis->Type->DontScore ) {
		HouseExtContainer::Instance.Find(pNewOwner)->BuiltInfantryTypes.Increment(pThis->Type->ArrayIndex);
	}

	return DontSend;
}

void __fastcall increment_tracker_inf(UnitTrackerClass* pTracker, DWORD, int idx) {
	HouseExtContainer::Instance.Find(GetHouseClassptr<0xB30>(pTracker))->BuiltInfantryTypes.Increment(idx);
}
DEFINE_JUMP(CALL, 0x4FF854, MiscTools::to_DWORD(&increment_tracker_inf));
void __fastcall increment_tracker_destroyedinf(UnitTrackerClass* pTracker, DWORD, int idx) {
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x2B50>(pTracker))->KilledInfantryTypes.Increment(idx);
}
//Destroyed
DEFINE_JUMP(CALL, 0x703152, MiscTools::to_DWORD(&increment_tracker_destroyedinf));
DEFINE_JUMP(CALL, 0x7034B4, MiscTools::to_DWORD(&increment_tracker_destroyedinf));

void __fastcall increment_tracker_Unit(UnitTrackerClass* pTracker, DWORD, int idx) {
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x1338>(pTracker))->BuiltInfantryTypes.Increment(idx);
}
DEFINE_JUMP(CALL,0x4FF893, MiscTools::to_DWORD(&increment_tracker_Unit));
void __fastcall increment_tracker_destroyedunit(UnitTrackerClass* pTracker, DWORD, int idx)
{
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x3358>(pTracker))->KilledUnitTypes.Increment(idx);
}
//Destroyed
DEFINE_JUMP(CALL, 0x703198, MiscTools::to_DWORD(&increment_tracker_destroyedunit));
DEFINE_JUMP(CALL, 0x7034F4, MiscTools::to_DWORD(&increment_tracker_destroyedunit));

void __fastcall increment_tracker_Aircraft(UnitTrackerClass* pTracker, DWORD, int idx) {
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x328>(pTracker))->BuiltAircraftTypes.Increment(idx);
}
DEFINE_JUMP(CALL,0x4FF7FB, MiscTools::to_DWORD(&increment_tracker_Aircraft));
//destroyed
void __fastcall increment_tracker_destroyedaircraft(UnitTrackerClass* pTracker, DWORD, int idx)
{
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x2348>(pTracker))->KilledAircraftTypes.Increment(idx);
}
DEFINE_JUMP(CALL, 0x703108, MiscTools::to_DWORD(&increment_tracker_destroyedaircraft));
DEFINE_JUMP(CALL, 0x703474, MiscTools::to_DWORD(&increment_tracker_destroyedaircraft));

void __fastcall increment_tracker_Building(UnitTrackerClass* pTracker, DWORD, int idx) {
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x1B40>(pTracker))->BuiltBuildingTypes.Increment(idx);
}
DEFINE_JUMP(CALL,0x4FF7BD, MiscTools::to_DWORD(&increment_tracker_Building));
//destroyed
void __fastcall increment_tracker_destroyedbuilding (UnitTrackerClass* pTracker, DWORD, int idx)
{
	HouseExtContainer::Instance.Find(GetHouseClassptr<0x3B60>(pTracker))->KilledBuildingTypes.Increment(idx);
}
DEFINE_JUMP(CALL, 0x703093, MiscTools::to_DWORD(&increment_tracker_destroyedbuilding));
DEFINE_JUMP(CALL, 0x703403, MiscTools::to_DWORD(&increment_tracker_destroyedbuilding));
#else

DEFINE_HOOK(0x448524, BuildingClass_Captured_SendStatistics, 0x7)
{
	enum { Send = 0x44852D, DontSend = 0x448559 };

	return IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

void HouseExtData::IncremetCrateTracking(HouseClass* pHouse, Powerup type)
{
	if ((IsStatisticsEnabled() || (SessionClass::Instance->GameMode == GameMode::Internet)) && (int)type < 19u) {
		pHouse->CollectedCrates.IncrementUnitCount((int)type);
	}
}


#endif