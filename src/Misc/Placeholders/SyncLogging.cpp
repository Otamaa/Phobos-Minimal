#include "SyncLogging.h"

#include <AircraftClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

bool SyncLogger::HooksDisabled = false;
int SyncLogger::AnimCreations_HighestX = 0;
int SyncLogger::AnimCreations_HighestY = 0;
int SyncLogger::AnimCreations_HighestZ = 0;

SyncLogEventBuffer<RNGCallSyncLogEvent, RNGCalls_Size> SyncLogger::RNGCalls;
SyncLogEventBuffer<FacingChangeSyncLogEvent, FacingChanges_Size> SyncLogger::FacingChanges;
SyncLogEventBuffer<TargetChangeSyncLogEvent, TargetChanges_Size> SyncLogger::TargetChanges;
SyncLogEventBuffer<TargetChangeSyncLogEvent, DestinationChanges_Size> SyncLogger::DestinationChanges;
SyncLogEventBuffer<MissionOverrideSyncLogEvent, MissionOverrides_Size> SyncLogger::MissionOverrides;
SyncLogEventBuffer<AnimCreationSyncLogEvent, AnimCreations_Size> SyncLogger::AnimCreations;

void SyncLogger::AddRNGCallSyncLogEvent(Random2Class* pRandomizer, int type, unsigned int callerAddress, int min, int max)
{
	// Don't log non-critical RNG calls.
	if (pRandomizer == &ScenarioClass::Instance->Random)
		SyncLogger::RNGCalls.Add(RNGCallSyncLogEvent(type, true, pRandomizer->Index1, pRandomizer->Index2, callerAddress, Unsorted::CurrentFrame, min, max));
}

void SyncLogger::AddFacingChangeSyncLogEvent(unsigned short facing, unsigned int callerAddress)
{
	SyncLogger::FacingChanges.Add(FacingChangeSyncLogEvent(facing, callerAddress, Unsorted::CurrentFrame));
}

void SyncLogger::AddTargetChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress)
{
	if (!pObject)
		return;

	auto targetRTTI = AbstractType::None;
	unsigned int targetID = 0;

	if (pTarget)
	{
		targetRTTI = pTarget->WhatAmI();
		targetID = pTarget->UniqueID;
	}

	SyncLogger::TargetChanges.Add(TargetChangeSyncLogEvent(pObject->WhatAmI(), pObject->UniqueID, targetRTTI, targetID, callerAddress, Unsorted::CurrentFrame));
}

void SyncLogger::AddDestinationChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress)
{
	if (!pObject)
		return;

	auto targetRTTI = AbstractType::None;
	unsigned int targetID = 0;

	if (pTarget)
	{
		targetRTTI = pTarget->WhatAmI();
		targetID = pTarget->UniqueID;
	}

	SyncLogger::DestinationChanges.Add(TargetChangeSyncLogEvent(pObject->WhatAmI(), pObject->UniqueID, targetRTTI, targetID, callerAddress, Unsorted::CurrentFrame));
}

void SyncLogger::AddMissionOverrideSyncLogEvent(AbstractClass* pObject, int mission, unsigned int callerAddress)
{
	if (!pObject)
		return;

	SyncLogger::MissionOverrides.Add(MissionOverrideSyncLogEvent(pObject->WhatAmI(), pObject->UniqueID, mission, callerAddress, Unsorted::CurrentFrame));
}

void SyncLogger::AddAnimCreationSyncLogEvent(const CoordStruct& coords, unsigned int callerAddress)
{
	if (coords.X > SyncLogger::AnimCreations_HighestX)
		SyncLogger::AnimCreations_HighestX = coords.X;

	if (coords.Y > SyncLogger::AnimCreations_HighestY)
		SyncLogger::AnimCreations_HighestY = coords.Y;

	if (coords.Z > SyncLogger::AnimCreations_HighestZ)
		SyncLogger::AnimCreations_HighestZ = coords.Z;

	if (SyncLogger::AnimCreations.Add(AnimCreationSyncLogEvent(coords, callerAddress, Unsorted::CurrentFrame)))
	{
		SyncLogger::AnimCreations_HighestX = 0;
		SyncLogger::AnimCreations_HighestY = 0;
		SyncLogger::AnimCreations_HighestZ = 0;
	}
}

void SyncLogger::WriteSyncLog(const char* logFilename)
{
	auto const pLogFile = fopen(logFilename, "at");

	if (pLogFile)
	{
		Debug::Log("Writing to sync log file '%s'.\n", logFilename);

		fprintf(pLogFile, "\nPhobos synchronization log:\n\n");

		int frameDigits = GeneralUtils::CountDigitsInNumber(Unsorted::CurrentFrame);

		WriteRNGCalls(pLogFile, frameDigits);
		WriteFacingChanges(pLogFile, frameDigits);
		WriteTargetChanges(pLogFile, frameDigits);
		WriteDestinationChanges(pLogFile, frameDigits);
		WriteAnimCreations(pLogFile, frameDigits);

		fclose(pLogFile);
	}
	else
	{
		Debug::Log("Failed to open sync log file '%s'.\n", logFilename);
	}
}
template<typename T>
void WriteLog(const T* it, int idx, DWORD checksum, FILE* F)
{
	fprintf(F, "#%05d:\t%08X", idx, checksum);
}

template<>
void WriteLog(const AbstractClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);
	auto abs = it->WhatAmI();
	fprintf(F, "; Abs: %u (%s)", abs, AbstractClass::GetAbstractClassName(abs));
}

template<>
void WriteLog(const ObjectClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);

	const char* typeID = GameStrings::NoneStr();
	int typeIndex = -1;
	if (auto pType = it->GetType())
	{
		typeID = pType->ID;
		typeIndex = pType->GetArrayIndex();
	}

	CoordStruct crd = it->GetCoords();
	CellStruct cell = CellClass::Coord2Cell(crd);

	fprintf(F, "; Type: %d (%s); Coords: %d,%d,%d (%d,%d); Health: %d; InLimbo: %u",
		typeIndex, typeID, crd.X, crd.Y, crd.Z, cell.X, cell.Y, it->Health, it->InLimbo);
}

template<>
void WriteLog(const MissionClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<ObjectClass>(it, idx, checksum, F);
	fprintf(F, "; Mission: %d; StartTime: %d",
		it->GetCurrentMission(), it->CurrentMissionStartTime);
}

template<>
void WriteLog(const RadioClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<MissionClass>(it, idx, checksum, F);
	//fprintf(F, "; LastCommand: %d", it->LastCommands[0]);
}

template<>
void WriteLog(const TechnoClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<RadioClass>(it, idx, checksum, F);

	const char* targetID = GameStrings::NoneStr();
	int targetIndex = -1;
	CoordStruct targetCrd = { -1, -1, -1 };
	if (auto pTarget = it->Target)
	{
		targetID = pTarget->GetThisClassName();
		targetIndex = pTarget->GetArrayIndex();
		targetCrd = pTarget->GetCoords();
	}

	fprintf(F, "; Facing: %d; Facing2: %d; Target: %s (%d; %d,%d)",
		it->PrimaryFacing.Current().Getvalue8(), it->SecondaryFacing.Current().Getvalue8(), targetID, targetIndex, targetCrd.X, targetCrd.Y);
}

template<>
void WriteLog(const FootClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);

	const char* destID = GameStrings::NoneStr();
	int destIndex = -1;
	CoordStruct destCrd = { -1, -1, -1 };
	if (auto pDest = it->Destination)
	{
		destID = pDest->GetThisClassName();
		destIndex = pDest->GetArrayIndex();
		destCrd = pDest->GetCoords();
	}

	fprintf(F, "; Destination: %s (%d; %d,%d)",
		destID, destIndex, destCrd.X, destCrd.Y);
}

template<>
void WriteLog(const InfantryClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
	fprintf(F, "; Speed %d", Game::F2I(it->SpeedPercentage * 256));
}

template<>
void WriteLog(const UnitClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);

	const auto& Loco = it->Locomotor;
	auto accum = Loco->Get_Speed_Accum();
	auto index = Loco->Get_Track_Index();
	auto number = Loco->Get_Track_Number();

	fprintf(F, "; Speed %d; TrackNumber: %d; TrackIndex: %d", accum, number, index);
}

template<>
void WriteLog(const AircraftClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
	fprintf(F, "; Speed %d; Height: %d", Game::F2I(it->SpeedPercentage * 256), it->GetHeight());
}

template<>
void WriteLog(const BuildingClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const AbstractTypeClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);
	fprintf(F, "; ID: %s; Name: %s", it->ID, it->Name);
}

template<>
void WriteLog(const HouseClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);

	fprintf(F, "; CurrentPlayer: %u; ColorScheme: %s; ID: %d; HouseType: %s; Edge: %d; StartingAllies: %u; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d",
		it->IsHumanPlayer, ColorScheme::Array->GetItem(it->ColorSchemeIndex)->ID,
		it->ArrayIndex, HouseTypeClass::Array->GetItem(it->Type->ArrayIndex)->Name,
		it->Edge, it->StartingAllies.data, it->StartingCell.X, it->StartingCell.Y, it->Visionary,
		it->MapIsClear, it->Available_Money());
}

// calls WriteLog and appends a newline
template<typename T>
void WriteLogLine(const T* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog(it, idx, checksum, F);
	fprintf(F, "\n");
}

template<typename T>
void LogItem(const T* it, int idx, FILE* F)
{
	if (it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2)
	{
		DWORD Checksum(0);
#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
		if (auto ExtData = AbstractExt::ExtMap.Find(it))
		{
			Checksum = ExtData->LastChecksum;
		}
#else
		SafeChecksummer Ch;
		it->CalculateChecksum(Ch);
		Checksum = Ch.Intermediate();
#endif
		WriteLogLine(it, idx, Checksum, F);
	}
}

template<typename T>
void VectorLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Label)
	{
		fprintf(F, "Checksums for [%s] (%d)\n", Label, Array ? Array->Count : -1);
	}
	if (Array)
	{
		for (auto i = 0; i < Array->Count; ++i)
		{
			auto it = Array->Items[i];
			LogItem(it, i, F);
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

template<typename T>
void HouseLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Array)
	{
		for (auto j = 0; j < HouseClass::Array->Count; ++j)
		{
			auto pHouse = HouseClass::Array->GetItem(j);
			fprintf(F, "-------------------- %s (%d) %s -------------------\n", pHouse->Type->Name, j, Label ? Label : "");

			for (auto i = 0; i < Array->Count; ++i)
			{
				auto it = Array->Items[i];

				if (it->Owner == pHouse)
				{
					LogItem(it, i, F);
				}
			}
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

#include <Networking.h>
#include <FPSCounter.h>
#include <Phobos.version.h>

//bool LogFrame(const char* LogFilename, NetworkEvent* OffendingEvent = nullptr)
//{
//	FILE* LogFile = nullptr;
//	if (!fopen_s(&LogFile, LogFilename, "wt") && LogFile)
//	{
//		std::setvbuf(LogFile, nullptr, _IOFBF, 1024 * 1024); // 1024 kb buffer - should be sufficient for whole log
//
//		fprintf(LogFile, "YR synchronization log\n");
//		fprintf(LogFile, "With Ares [21.352.1218] and Phobos [%s]\n", _STR(BUILD_NUMBER));
//
//		for (auto ixF = 0; ixF < Networking::LatestFramesCRC.size(); ++ixF) {
//			fprintf(LogFile, "LastFrame CRC[%02X] = %08X\n", ixF, Networking::LatestFramesCRC[ixF]);
//		}
//
//		fprintf(LogFile, "My Random Number: %08X\n", ScenarioClass::Instance->Random.Random());
//		fprintf(LogFile, "My Frame: %08X\n", Unsorted::CurrentFrame);
//		fprintf(LogFile, "Average FPS: %d\n", Game::F2I(FPSCounter::GetAverageFrameRate()));
//		fprintf(LogFile, "Max MaxAhead: %d\n",  Unsorted::MaxAhead());
//		fprintf(LogFile, "Latency setting: %d\n", Network::LatencyFudge());
//		fprintf(LogFile, "Game speed setting: %d\n", GameOptionsClass::Instance->GameSpeed);
//		fprintf(LogFile, "FrameSendRate: %d\n", Network::FrameSendRate());
//		fprintf(LogFile, "Mod is %s with %d\n", );
//		fprintf(LogFile, "Player Name %s\n", HouseClass::CurrentPlayer->PlainName);
//		fprintf(LogFile, "Rules checksum: %08X\n", );
//		fprintf(LogFile, "Art checksum: %08X\n", );
//		fprintf(LogFile, "AI checksum: %08X\n", );
//
//		if (OffendingEvent)
//		{
//			fprintf(LogFile, "\nOffending event:\n");
//			fprintf(LogFile, "Type:         %X\n", OffendingEvent->Kind);
//			fprintf(LogFile, "Frame:        %X\n", OffendingEvent->Timestamp);
//			fprintf(LogFile, "ID:           %X\n", OffendingEvent->HouseIndex);
//			fprintf(LogFile, "CRC:          %X\n", OffendingEvent->Checksum);
//			fprintf(LogFile, "CommandCount: %hu\n", OffendingEvent->CommandCount);
//			fprintf(LogFile, "Delay:        %hhu\n", OffendingEvent->Delay);
//			fprintf(LogFile, "\n\n");
//		}
//
//		fprintf(LogFile, "\nTypes\n");
//		HouseLogger(InfantryClass::Array(), LogFile, "Infantry");
//		HouseLogger(UnitClass::Array(), LogFile, "Units");
//		HouseLogger(AircraftClass::Array(), LogFile, "Aircraft");
//		HouseLogger(BuildingClass::Array(), LogFile, "Buildings");
//
//		fprintf(LogFile, "\nChecksums\n");
//		VectorLogger(HouseClass::Array(), LogFile, "Houses");
//		VectorLogger(InfantryClass::Array(), LogFile, "Infantry");
//		VectorLogger(UnitClass::Array(), LogFile, "Units");
//		VectorLogger(AircraftClass::Array(), LogFile, "Aircraft");
//		VectorLogger(BuildingClass::Array(), LogFile, "Buildings");
//
//		fprintf(LogFile, "\n");
//		VectorLogger(&ObjectClass::CurrentObjects(), LogFile, "Current Objects");
//		VectorLogger(ObjectClass::Logics(), LogFile, "Logics");
//
//		fprintf(LogFile, "\nChecksums for Map Layers\n");
//		for (auto ixL = 0; ixL < 5; ++ixL)
//		{
//			fprintf(LogFile, "Checksums for Layer %d\n", ixL);
//			VectorLogger(&(ObjectClass::ObjectsInLayers[ixL]), LogFile);
//		}
//
//		fprintf(LogFile, "\nChecksums for Logics\n");
//		VectorLogger(ObjectClass::Logics(), LogFile);
//
//		fprintf(LogFile, "\nChecksums for Abstracts\n");
//		VectorLogger(AbstractClass::Array(), LogFile, "Abstracts");
//		VectorLogger(AbstractTypeClass::Array(), LogFile, "AbstractTypes");
//
// 		int frameDigits = GeneralUtils::CountDigitsInNumber(Unsorted::CurrentFrame);
//
//		WriteRNGCalls(pLogFile, frameDigits);
//		WriteFacingChanges(pLogFile, frameDigits);
//		WriteTargetChanges(pLogFile, frameDigits);
//		WriteDestinationChanges(pLogFile, frameDigits);
//		WriteAnimCreations(pLogFile, frameDigits);
//
//		fclose(LogFile);
//		return true;
//	}
//	else
//	{
//		Debug::Log("Failed to open file for sync log. Error code %X.\n", errno);
//		return false;
//	}
//}

void SyncLogger::WriteRNGCalls(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "RNG Calls:\n");

	for (size_t i = 0; i < SyncLogger::RNGCalls.Size(); i++)
	{
		auto const& rngCall = SyncLogger::RNGCalls.Get();

		if (!rngCall.Initialized)
			continue;

		if (rngCall.Type == 1)
		{
			fprintf(pLogFile, "#%05d: Single | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d\n",
				i, rngCall.Caller, frameDigits, rngCall.Frame, rngCall.Index1, rngCall.Index2);
		}
		else if (rngCall.Type == 2)
		{
			fprintf(pLogFile, "#%05d: Ranged | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d | Min: %d | Max: %d\n",
				i, rngCall.Caller, frameDigits, rngCall.Frame, rngCall.Index1, rngCall.Index2, rngCall.Min, rngCall.Max);
		}
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteFacingChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Facing changes:\n");

	for (size_t i = 0; i < SyncLogger::FacingChanges.Size(); i++)
	{
		auto const& facingChange = SyncLogger::FacingChanges.Get();

		if (!facingChange.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: Facing: %5d | Caller: %08x | Frame: %*d\n",
			i, facingChange.Facing, facingChange.Caller, frameDigits, facingChange.Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteTargetChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Target changes:\n");

	for (size_t i = 0; i < SyncLogger::TargetChanges.Size(); i++)
	{
		auto const& targetChange = SyncLogger::TargetChanges.Get();

		if (!targetChange.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: (%s)%02d | ID: %08d | TargetRTTI: (%s)%02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i, AbstractClass::GetAbstractClassName(targetChange.Type), targetChange.Type,
			targetChange.ID, AbstractClass::GetAbstractClassName(targetChange.TargetType), targetChange.TargetType, targetChange.TargetID, targetChange.Caller, frameDigits, targetChange.Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteDestinationChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Destination changes:\n");

	for (size_t i = 0; i < SyncLogger::DestinationChanges.Size(); i++)
	{
		auto const& destChange = SyncLogger::DestinationChanges.Get();

		if (!destChange.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: (%s)%02d | ID: %08d | TargetRTTI: (%s)%02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i,
			AbstractClass::GetAbstractClassName(destChange.Type), destChange.Type,
			destChange.ID, AbstractClass::GetAbstractClassName(destChange.TargetType) , destChange.TargetType, destChange.TargetID, destChange.Caller, frameDigits, destChange.Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteMissionOverrides(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Mission overrides:\n");

	for (size_t i = 0; i < SyncLogger::MissionOverrides.Size(); i++)
	{
		auto const& missionOverride = SyncLogger::MissionOverrides.Get();

		if (!missionOverride.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | Mission: %02d | Caller: %08x | Frame: %*d\n",
			i, missionOverride.Type, missionOverride.ID, missionOverride.Mission, missionOverride.Caller, frameDigits, missionOverride.Frame);
	}

	fprintf(pLogFile, "\n");
}


void SyncLogger::WriteAnimCreations(FILE* const pLogFile, int frameDigits)
{
	int xDigits = GeneralUtils::CountDigitsInNumber(SyncLogger::AnimCreations_HighestX);
	int yDigits = GeneralUtils::CountDigitsInNumber(SyncLogger::AnimCreations_HighestY);
	int zDigits = GeneralUtils::CountDigitsInNumber(SyncLogger::AnimCreations_HighestZ);

	fprintf(pLogFile, "Animation creations:\n");

	for (size_t i = 0; i < SyncLogger::AnimCreations.Size(); i++)
	{
		auto const& animCreation = SyncLogger::AnimCreations.Get();

		if (!animCreation.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: X: %*d | Y: %*d | Z: %*d | Caller: %08x | Frame: %*d\n",
			i, xDigits, animCreation.Coords.X, yDigits, animCreation.Coords.Y, zDigits, animCreation.Coords.Z, animCreation.Caller, frameDigits, animCreation.Frame);
	}

	fprintf(pLogFile, "\n");
}

// Hooks

// Sync file writing

DEFINE_HOOK(0x64736D, Queue_AI_WriteDesyncLog, 0x5)
{
	GET(int, frame, ECX);

	char logFilename[0x40];

	if (Game::EnableMPSyncDebug)
		_snprintf_s(logFilename, _TRUNCATE, "SYNC%01d_%03d.TXT", HouseClass::CurrentPlayer->ArrayIndex, frame % 256);
	else
		_snprintf_s(logFilename, _TRUNCATE, "SYNC%01d.TXT", HouseClass::CurrentPlayer->ArrayIndex);

	SyncLogger::WriteSyncLog(logFilename);

	// Replace overridden instructions.
	JMP_STD(0x6BEC60);

	return 0x647374;
}

DEFINE_HOOK(0x64CD11, ExecuteDoList_WriteDesyncLog, 0x8)
{
	char logFilename[0x40];

	if (Game::EnableMPSyncDebug)
	{
		for (int i = 0; i < 256; i++)
		{
			_snprintf_s(logFilename, _TRUNCATE, "SYNC%01d_%03d.TXT", HouseClass::CurrentPlayer->ArrayIndex, i);
			SyncLogger::WriteSyncLog(logFilename);
		}
	}
	else
	{
		_snprintf_s(logFilename, _TRUNCATE, "SYNC%01d.TXT", HouseClass::CurrentPlayer->ArrayIndex);
		SyncLogger::WriteSyncLog(logFilename);
	}

	return 0;
}

// RNG call logging
#pragma optimize("", off )
DEFINE_HOOK(0x65C7D0, Random2Class_Random_SyncLog, 0x6)
{
	GET(Random2Class*, pThis, ECX);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddRNGCallSyncLogEvent(pThis, 1, callerAddress);

	return 0;
}

DEFINE_HOOK(0x65C88A, Random2Class_RandomRanged_SyncLog, 0x6)
{
	GET(Random2Class*, pThis, EDX);
	GET_STACK(unsigned int, callerAddress, 0x0);
	GET_STACK(int, min, 0x4);
	GET_STACK(int, max, 0x8);

	SyncLogger::AddRNGCallSyncLogEvent(pThis, 2, callerAddress, min, max);

	return 0;
}

// Facing change logging

DEFINE_HOOK(0x4C9300, FacingClass_Set_SyncLog, 0x5)
{
	GET_STACK(DirStruct*, facing, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddFacingChangeSyncLogEvent(facing->Raw, callerAddress);

	return 0;
}

// Target change logging

DEFINE_HOOK(0x51B1F0, InfantryClass_AssignTarget_SyncLog, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddTargetChangeSyncLogEvent(pThis, pTarget, callerAddress);

	return 0;
}

DEFINE_HOOK(0x443B90, BuildingClass_AssignTarget_SyncLog, 0xB)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddTargetChangeSyncLogEvent(pThis, pTarget, callerAddress);

	return 0;
}

DEFINE_HOOK(0x6FCDB0, TechnoClass_AssignTarget_SyncLog, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	auto const RTTI = pThis->WhatAmI();

	if (RTTI != AbstractType::Building && RTTI != AbstractType::Infantry)
		SyncLogger::AddTargetChangeSyncLogEvent(pThis, pTarget, callerAddress);

	return 0;
}

// Destination change logging

DEFINE_HOOK(0x41AA80, AircraftClass_AssignDestination_SyncLog, 0x7)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

DEFINE_HOOK(0x455D50, BuildingClass_AssignDestination_SyncLog, 0xA)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

DEFINE_HOOK(0x51AA40, InfantryClass_AssignDestination_SyncLog, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

DEFINE_HOOK(0x741970, UnitClass_AssignDestination_SyncLog, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

// Mission override logging

DEFINE_HOOK(0x41BB30, AircraftClass_OverrideMission_SyncLog, 0x6)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

DEFINE_HOOK(0x4D8F40, FootClass_OverrideMission_SyncLog, 0x5)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

DEFINE_HOOK(0x7013A0, TechnoClass_OverrideMission_SyncLog, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	if (pThis->WhatAmI() == AbstractType::Building)
		SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

// Anim creation logging
DEFINE_HOOK(0x421EA0, AnimClass_CTOR_CallerAddress, 0x6)
{
	GET_STACK(CoordStruct*, coords, 0x8);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddAnimCreationSyncLogEvent(*coords, callerAddress);

	return 0;
}
#pragma optimize("", on )

// Disable sync logging hooks in non-MP games
DEFINE_HOOK(0x683AB0, ScenarioClass_Start_DisableSyncLog, 0x6)
{
	if (SessionClass::Instance->IsMultiplayer() || SyncLogger::HooksDisabled)
		return 0;

	SyncLogger::HooksDisabled = true;

	Patch::Apply_RAW(0x65C7D0, // Disable Random2Class_Random_SyncLog
	{ 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable Random2Class_RandomRanged_SyncLog
	{ 0xC2, 0x08, 0x00, 0x90, 0x90, 0x90 }
	);

	Patch::Apply_RAW(0x4C9300, // Disable FacingClass_Set_SyncLog
	{ 0x83, 0xEC, 0x10, 0x53, 0x56 }
	);

	Patch::Apply_RAW(0x51B1F0, // Disable InfantryClass_AssignTarget_SyncLog
	{ 0x53, 0x56, 0x8B, 0xF1, 0x57 }
	);

	Patch::Apply_RAW(0x443B90, // Disable BuildingClass_AssignTarget_SyncLog
	{ 0x56, 0x8B, 0xF1, 0x57, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 }
	);

	Patch::Apply_RAW(0x6FCDB0, // Disable TechnoClass_AssignTarget_SyncLog
	{ 0x83, 0xEC, 0x0C, 0x53, 0x56 }
	);

	Patch::Apply_RAW(0x41AA80, // Disable AircraftClass_AssignDestination_SyncLog
	{ 0x53, 0x56, 0x57, 0x8B, 0x7C, 0x24, 0x10 }
	);

	Patch::Apply_RAW(0x455D50, // Disable BuildingClass_AssignDestination_SyncLog
	{ 0x56, 0x8B, 0xF1, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 }
	);

	Patch::Apply_RAW(0x51AA40, // Disable InfantryClass_AssignDestination_SyncLog
	{ 0x83, 0xEC, 0x2C, 0x53, 0x55 }
	);

	Patch::Apply_RAW(0x741970, // Disable UnitClass_AssignDestination_SyncLog
	{ 0x81, 0xEC, 0x80, 0x0, 0x0, 0x0 }
	);

	Patch::Apply_RAW(0x41BB30, // Disable AircraftClass_OverrideMission_SyncLog
	{ 0x8B, 0x81, 0xAC, 0x0, 0x0, 0x0 }
	);

	Patch::Apply_RAW(0x4D8F40, // Disable FootClass_OverrideMission_SyncLog
	{ 0x8B, 0x54, 0x24, 0x4, 0x56 }
	);

	Patch::Apply_RAW(0x7013A0, // Disable TechnoClass_OverrideMission_SyncLog
	{ 0x8B, 0x54, 0x24, 0x4, 0x56 }
	);

	Patch::Apply_RAW(0x421EA0, // Disable AnimClass_CTOR_SyncLog
	{ 0x53, 0x56, 0x57, 0x8B, 0xF1 }
	);

	return 0;
}
