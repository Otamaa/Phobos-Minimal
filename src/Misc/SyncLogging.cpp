#include <Misc/SyncLogging.h>

#include <AircraftClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

#pragma region defines
SyncLogEventBuffer<RNGCallSyncLogEvent, RNGCalls_Size> SyncLogger::RNGCalls;
SyncLogEventBuffer<FacingChangeSyncLogEvent, FacingChanges_Size> SyncLogger::FacingChanges;
SyncLogEventBuffer<TargetChangeSyncLogEvent, TargetChanges_Size> SyncLogger::TargetChanges;
SyncLogEventBuffer<TargetChangeSyncLogEvent, DestinationChanges_Size> SyncLogger::DestinationChanges;
SyncLogEventBuffer<MissionOverrideSyncLogEvent, MissionOverrides_Size> SyncLogger::MissionOverrides;
SyncLogEventBuffer<AnimCreationSyncLogEvent, AnimCreations_Size> SyncLogger::AnimCreations;

bool SyncLogger::HooksDisabled;
int SyncLogger::AnimCreations_HighestX;
int SyncLogger::AnimCreations_HighestY;
int SyncLogger::AnimCreations_HighestZ;

#pragma endregion

static COMPILETIMEEVAL void FORCEDINLINE MakeCallerRelative(unsigned int& caller)
{
	// B for Bobos
	if (caller > Phobos::Otamaa::PhobosBaseAddress && caller < (Phobos::Otamaa::PhobosBaseAddress + 0x100000))
		caller = caller - Phobos::Otamaa::PhobosBaseAddress + 0xB0000000;
}

void SyncLogger::AddRNGCallSyncLogEvent(Random2Class* pRandomizer, int type, unsigned int callerAddress, int min, int max)
{
	MakeCallerRelative(callerAddress);
	// Don't log non-critical RNG calls.
	if (pRandomizer == &ScenarioClass::Instance->Random)
		SyncLogger::RNGCalls.Emplace(type, true, pRandomizer->Index1, pRandomizer->Index2, callerAddress, Unsorted::CurrentFrame, min, max);
}

void SyncLogger::AddFacingChangeSyncLogEvent(unsigned short facing, unsigned int callerAddress)
{
	MakeCallerRelative(callerAddress);
	SyncLogger::FacingChanges.Emplace(facing, callerAddress, Unsorted::CurrentFrame);
}

void SyncLogger::AddTargetChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress)
{
	if (!pObject)
		return;

	MakeCallerRelative(callerAddress);
	auto targetRTTI = AbstractType::None;
	unsigned int targetID = 0;

	if (pTarget)
	{
		targetRTTI = pTarget->WhatAmI();
		targetID = pTarget->UniqueID;
	}

	SyncLogger::TargetChanges.Emplace(pObject->WhatAmI(), pObject->UniqueID, targetRTTI, targetID, callerAddress, Unsorted::CurrentFrame);
}

void SyncLogger::AddDestinationChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress)
{
	if (!pObject)
		return;

	MakeCallerRelative(callerAddress);
	auto targetRTTI = AbstractType::None;
	unsigned int targetID = 0;

	if (pTarget)
	{
		targetRTTI = pTarget->WhatAmI();
		targetID = pTarget->UniqueID;
	}

	SyncLogger::DestinationChanges.Emplace(pObject->WhatAmI(), pObject->UniqueID, targetRTTI, targetID, callerAddress, Unsorted::CurrentFrame);
}

void SyncLogger::AddMissionOverrideSyncLogEvent(AbstractClass* pObject, int mission, unsigned int callerAddress)
{
	if (!pObject)
		return;

	MakeCallerRelative(callerAddress);
	SyncLogger::MissionOverrides.Emplace(pObject->WhatAmI(), pObject->UniqueID, mission, callerAddress, Unsorted::CurrentFrame);
}

void SyncLogger::AddAnimCreationSyncLogEvent(const CoordStruct& coords, unsigned int callerAddress)
{
	if (coords.X > SyncLogger::AnimCreations_HighestX)
		SyncLogger::AnimCreations_HighestX = coords.X;

	if (coords.Y > SyncLogger::AnimCreations_HighestY)
		SyncLogger::AnimCreations_HighestY = coords.Y;

	if (coords.Z > SyncLogger::AnimCreations_HighestZ)
		SyncLogger::AnimCreations_HighestZ = coords.Z;

	MakeCallerRelative(callerAddress);
	if (SyncLogger::AnimCreations.Emplace(coords, callerAddress, Unsorted::CurrentFrame))
	{
		SyncLogger::AnimCreations_HighestX = 0;
		SyncLogger::AnimCreations_HighestY = 0;
		SyncLogger::AnimCreations_HighestZ = 0;
	}
}

void SyncLogger::WriteSyncLog(const std::string& logFilename)
{
	auto const pLogFile = fopen(logFilename.c_str(), "at");

	if (pLogFile)
	{
		Debug::LogInfo("Writing to sync log file '{}'.", logFilename);

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
		Debug::LogInfo("Failed to open sync log file '{}'.", logFilename);
	}
}

void SyncLogger::WriteRNGCalls(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "RNG Calls:\n");

	for (size_t i = 0; i < SyncLogger::RNGCalls.Size(); i++)
	{
		auto const rngCall = SyncLogger::RNGCalls.Get();

		if (!rngCall || !rngCall->Initialized)
			continue;

		if (rngCall->Type == 1)
		{
			fprintf(pLogFile, "#%05d: Single | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d\n",
				i, rngCall->Caller, frameDigits, rngCall->Frame, rngCall->Index1, rngCall->Index2);
		}
		else if (rngCall->Type == 2)
		{
			fprintf(pLogFile, "#%05d: Ranged | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d | Min: %d | Max: %d\n",
				i, rngCall->Caller, frameDigits, rngCall->Frame, rngCall->Index1, rngCall->Index2, rngCall->Min, rngCall->Max);
		}
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteFacingChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Facing changes:\n");

	for (size_t i = 0; i < SyncLogger::FacingChanges.Size(); i++)
	{
		auto const facingChange = SyncLogger::FacingChanges.Get();

		if (!facingChange || !facingChange->Initialized)
			continue;

		fprintf(pLogFile, "#%05d: Facing: %5d | Caller: %08x | Frame: %*d\n",
			i, facingChange->Facing, facingChange->Caller, frameDigits, facingChange->Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteTargetChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Target changes:\n");

	for (size_t i = 0; i < SyncLogger::TargetChanges.Size(); i++)
	{
		auto const targetChange = SyncLogger::TargetChanges.Get();

		if (!targetChange || !targetChange->Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | TargetRTTI: %02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i, targetChange->Type, targetChange->ID, targetChange->TargetType, targetChange->TargetID, targetChange->Caller, frameDigits, targetChange->Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteDestinationChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Destination changes:\n");

	for (size_t i = 0; i < SyncLogger::DestinationChanges.Size(); i++)
	{
		auto const destChange = SyncLogger::DestinationChanges.Get();

		if (!destChange || !destChange->Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | TargetRTTI: %02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i, destChange->Type, destChange->ID, destChange->TargetType, destChange->TargetID, destChange->Caller, frameDigits, destChange->Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteMissionOverrides(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Mission overrides:\n");

	for (size_t i = 0; i < SyncLogger::MissionOverrides.Size(); i++)
	{
		auto const missionOverride = SyncLogger::MissionOverrides.Get();

		if (!missionOverride || !missionOverride->Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | Mission: %02d | Caller: %08x | Frame: %*d\n",
			i, missionOverride->Type, missionOverride->ID, missionOverride->Mission, missionOverride->Caller, frameDigits, missionOverride->Frame);
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
		auto const animCreation = SyncLogger::AnimCreations.Get();

		if (!animCreation || !animCreation->Initialized)
			continue;

		fprintf(pLogFile, "#%05d: X: %*d | Y: %*d | Z: %*d | Caller: %08x | Frame: %*d\n",
			i, xDigits, animCreation->Coords.X, yDigits, animCreation->Coords.Y, zDigits, animCreation->Coords.Z, animCreation->Caller, frameDigits, animCreation->Frame);
	}

	fprintf(pLogFile, "\n");
}

// Hooks. Anim contructor logging is in Ext/Anim/Body.cpp to reduce duplicate hooks

// RNG call logging

ASMJIT_PATCH(0x65C7D0, Random2Class_Random_SyncLog, 0x6)
{
	GET(Random2Class*, pThis, ECX);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddRNGCallSyncLogEvent(pThis, 1, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x65C88A, Random2Class_RandomRanged_SyncLog, 0x6)
{
	GET(Random2Class*, pThis, EDX);
	GET_STACK(unsigned int, callerAddress, 0x0);
	GET_STACK(int, min, 0x4);
	GET_STACK(int, max, 0x8);

	SyncLogger::AddRNGCallSyncLogEvent(pThis, 2, callerAddress, min, max);

	return 0;
}

// Facing change logging

ASMJIT_PATCH(0x4C9300, FacingClass_Set_SyncLog, 0x5)
{
	GET_STACK(DirStruct*, facing, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddFacingChangeSyncLogEvent(facing->Raw, callerAddress);

	return 0;
}

// Target change logging

ASMJIT_PATCH(0x51B1F0, InfantryClass_AssignTarget_SyncLog, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddTargetChangeSyncLogEvent(pThis, pTarget, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x443B90, BuildingClass_AssignTarget_SyncLog, 0xB)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddTargetChangeSyncLogEvent(pThis, pTarget, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x6FCDB0, TechnoClass_AssignTarget_SyncLog, 0x5)
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

ASMJIT_PATCH(0x41AA80, AircraftClass_AssignDestination_SyncLog, 0x7)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x455D50, BuildingClass_AssignDestination_SyncLog, 0xA)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}


ASMJIT_PATCH(0x741970, UnitClass_AssignDestination_SyncLog, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	return 0;
}

// Mission override logging

ASMJIT_PATCH(0x41BB30, AircraftClass_OverrideMission_SyncLog, 0x6)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x4D8F40, FootClass_OverrideMission_SyncLog, 0x5)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

ASMJIT_PATCH(0x7013A0, TechnoClass_OverrideMission_SyncLog, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, mission, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	if (pThis->WhatAmI() == AbstractType::Building)
		SyncLogger::AddMissionOverrideSyncLogEvent(pThis, mission, callerAddress);

	return 0;
}

// Disable sync logging hooks in non-MP games
ASMJIT_PATCH(0x683AB0, ScenarioClass_Start_DisableSyncLog, 0x6)
{
	if (SessionClass::IsMultiplayer() || SyncLogger::HooksDisabled) {
		return 0;
	}

	SyncLogger::HooksDisabled = true;

	Patch::Apply_RAW(0x421EA0, // Disable AnimClass_CTOR_SetContext
	{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 }
	);

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

	return 0;
}
