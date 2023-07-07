#include <Misc/SyncLogging.h>

#include <AircraftClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

bool SyncLogger::HooksDisabled = false;
SyncLogEventTracker<RNGCallSyncLogEvent, RNGCalls_Size> SyncLogger::RNGCalls;
SyncLogEventTracker<FacingChangeSyncLogEvent, FacingChanges_Size> SyncLogger::FacingChanges;
SyncLogEventTracker<TargetChangeSyncLogEvent, TargetChanges_Size> SyncLogger::TargetChanges;
SyncLogEventTracker<TargetChangeSyncLogEvent, DestinationChanges_Size> SyncLogger::DestinationChanges;

void SyncLogger::AddRNGCallSyncLogEvent(Randomizer* pRandomizer, int type, unsigned int callerAddress, int min, int max)
{
	// Don't log non-critical RNG calls.
	if (pRandomizer == &ScenarioClass::Instance->Random)
		SyncLogger::RNGCalls.Add(RNGCallSyncLogEvent(type, true, pRandomizer->Next1, pRandomizer->Next2, callerAddress, Unsorted::CurrentFrame, min, max));
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

void SyncLogger::WriteSyncLog(const char* logFilename)
{
	auto const pLogFile = fopen(logFilename, "at");

	if (pLogFile)
	{
		Debug::Log("Writing to sync log file '%s'.", logFilename);

		fprintf(pLogFile, "\nPhobos synchronization log:\n\n");

		int frameDigits = 0;
		int frame = Unsorted::CurrentFrame;

		while (frame)
		{
			frame /= 10;
			frameDigits++;
		}

		WriteRNGCalls(pLogFile, frameDigits);
		WriteFacingChanges(pLogFile, frameDigits);
		WriteTargetChanges(pLogFile, frameDigits);
		WriteDestinationChanges(pLogFile, frameDigits);

		fclose(pLogFile);
	}
	else
	{
		Debug::Log("Failed to open sync log file '%s'.", logFilename);
	}
}

void SyncLogger::WriteRNGCalls(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "RNG Calls:\n");

	for (size_t i = 0; i < SyncLogger::RNGCalls.Size(); i++)
	{
		auto const& rngCall = SyncLogger::RNGCalls.Get(i);

		if (!rngCall.Initialized)
			continue;

		if (rngCall.Type == 1)
		{
			fprintf(pLogFile, "#%05d: Single | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d\n",
				i, rngCall.Caller, frameDigits, rngCall.Frame, rngCall.Seed, rngCall.Index);
		}
		else if (rngCall.Type == 2)
		{
			fprintf(pLogFile, "#%05d: Ranged | Caller: %08x | Frame: %*d | Index1: %3d | Index2: %3d | Min: %d | Max: %d\n",
				i, rngCall.Caller, frameDigits, rngCall.Frame, rngCall.Seed, rngCall.Index, rngCall.Min, rngCall.Max);
		}
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteFacingChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Facing changes:\n");

	for (size_t i = 0; i < SyncLogger::FacingChanges.Size(); i++)
	{
		auto const& facingChange = SyncLogger::FacingChanges.Get(i);

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
		auto const& targetChange = SyncLogger::TargetChanges.Get(i);

		if (!targetChange.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | TargetRTTI: %02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i, targetChange.Type, targetChange.ID, targetChange.TargetType, targetChange.TargetID, targetChange.Caller, frameDigits, targetChange.Frame);
	}

	fprintf(pLogFile, "\n");
}

void SyncLogger::WriteDestinationChanges(FILE* const pLogFile, int frameDigits)
{
	fprintf(pLogFile, "Destination changes:\n");

	for (size_t i = 0; i < SyncLogger::DestinationChanges.Size(); i++)
	{
		auto const& destChange = SyncLogger::DestinationChanges.Get(i);

		if (!destChange.Initialized)
			continue;

		fprintf(pLogFile, "#%05d: RTTI: %02d | ID: %08d | TargetRTTI: %02d | TargetID: %08d | Caller: %08x | Frame: %*d\n",
			i, destChange.Type, destChange.ID, destChange.TargetType, destChange.TargetID, destChange.Caller, frameDigits, destChange.Frame);
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

DEFINE_HOOK(0x65C7D0, Random2Class_Random_SyncLog, 0x6)
{
	GET(Randomizer*, pThis, ECX);
	GET_STACK(unsigned int, callerAddress, 0x0);

	SyncLogger::AddRNGCallSyncLogEvent(pThis, 1, callerAddress);

	return 0;
}

DEFINE_HOOK(0x65C88A, Random2Class_RandomRanged_SyncLog, 0x6)
{
	GET(Randomizer*, pThis, EDX);
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

	Patch::Apply_RAW(0x65C88A, // Disable FacingClass_Set_SyncLog
	{ 0x83, 0xEC, 0x10, 0x53, 0x56 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable InfantryClass_AssignTarget_SyncLog
	{ 0x53, 0x56, 0x8B, 0xF1, 0x57 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable BuildingClass_AssignTarget_SyncLog
	{ 0x56, 0x8B, 0xF1, 0x57, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable TechnoClass_AssignTarget_SyncLog
	{ 0x83, 0xEC, 0x0C, 0x53, 0x56 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable AircraftClass_AssignDestination_SyncLog
	{ 0x53, 0x56, 0x57, 0x8B, 0x7C, 0x24, 0x10 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable BuildingClass_AssignDestination_SyncLog
	{ 0x56, 0x8B, 0xF1, 0x83, 0xBE, 0xAC, 0x0, 0x0, 0x0, 0x13 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable InfantryClass_AssignDestination_SyncLog
	{ 0x8C, 0xEC, 0x2C, 0x53, 0x55 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable UnitClass_AssignDestination_SyncLog
	{ 0x81, 0xEC, 0x80, 0x0, 0x0, 0x0 }
	);

	return 0;
}
