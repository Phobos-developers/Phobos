#include <Misc/SyncLogging.h>

#include <HouseClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

SyncLogEventTracker<RNGCallSyncLogEvent, RNGCalls_Size> SyncLogger::RNGCalls;
SyncLogEventTracker<FacingChangeSyncLogEvent, FacingChanges_Size> SyncLogger::FacingChanges;

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

		if (!rngCall.Frame)
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

		if (!facingChange.Frame)
			continue;

		fprintf(pLogFile, "#%05d: Facing: %5d | Caller: %08x | Frame: %*d\n",
			i, facingChange.Facing, facingChange.Caller, frameDigits, facingChange.Frame);
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

// Disable sync logging hooks in non-MP games
DEFINE_HOOK(0x683AB0, ScenarioClass_Start_DisableSyncLog, 0x6)
{
	if (SessionClass::Instance->IsMultiplayer())
		return 0;

	Patch::Apply_RAW(0x65C7D0, // Disable Random2Class_Random_SyncLog
	{ 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable Random2Class_RandomRanged_SyncLog
	{ 0xC2, 0x08, 0x00, 0x90, 0x90, 0x90 }
	);

	Patch::Apply_RAW(0x65C88A, // Disable FacingClass_Set_SyncLog
	{ 0x83, 0xEC, 0x10, 0x53, 0x56 }
	);

	return 0;
}
