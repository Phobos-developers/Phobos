#pragma once

#include <Windows.h>

#include <string>
#include <vector>

// Custom exception handler with a crash dialog, ported from Vinifera.
// Replaces the game's Exception_Handler (0x4C8FE0) by patching its two
// callsites, so it also takes precedence over Ares' hook at that address.
// Active only when the game's main loop exception handler is armed
// (-ExceptionHandler, Game::DontSetExceptionHandler).
class ExceptionHandler
{
public:
	static void Init();
	static LONG __fastcall Handle(int code, EXCEPTION_POINTERS* pExs);

	// Make the automatic crash dump a full memory dump (-FullCrashDump).
	static bool GenerateFullCrashDump;

	// Internals shared between the ExceptionHandler translation units.
	static constexpr size_t ReportBufferSize = 0x20000;

	static DWORD MainThreadId;
	static char DebugDirectory[MAX_PATH];
	// Per-crash Ares-style folder: debug\snapshot-<timestamp>\. The CnCNet
	// client recognizes these and folds debug.log / syringe.log into them.
	static char SnapshotDirectory[MAX_PATH];

	static char ReportBuffer[ReportBufferSize];
	static size_t ReportLength;
	static bool ReportFinished;

	// The crashing thread's state, copied into stable storage so the dumper
	// thread and the dialog's full-dump button can use it safely.
	static CONTEXT SavedContext;
	static EXCEPTION_RECORD SavedRecord;
	static EXCEPTION_POINTERS SavedPointers;
	static unsigned int SavedCode;
	static DWORD CrashedThreadId;
	static SYSTEMTIME CrashTime;

	static char MinidumpPath[MAX_PATH];
	static char FullDumpPath[MAX_PATH];
	static char ReportPath[MAX_PATH];

	// DbgHelp is not thread-safe; every Sym*/StackWalk64/MiniDumpWriteDump
	// call is serialized through this lock.
	static CRITICAL_SECTION DbgHelpLock;
	static bool DbgHelpLockReady;
	static bool SymbolsInitialized;

	// Known-crash-address database (gamemd.edb, Vinifera-compatible format):
	// matched against the faulting EIP to annotate the report.
	struct ExceptionDatabaseEntry
	{
		unsigned int Address;
		std::string Description;
	};
	static std::vector<ExceptionDatabaseEntry> ExceptionDatabase;

	static void Append(const char* pFormat, ...);
	static void LoadExceptionDatabase();
	static bool InitSymbols();
	static void EnsureDebugDirectory();
	static void EnsureSnapshotDirectory();
	static void BuildReport(unsigned int code, EXCEPTION_POINTERS* pExs);
	static void WriteCrashArtifacts(EXCEPTION_POINTERS* pExs, unsigned int code, DWORD crashedTid);
	static bool WriteMinidump(EXCEPTION_POINTERS* pExs, DWORD crashedTid, bool fullMemory, char* pPathOut, size_t pathOutSize);
	static bool ModuleFromAddress(uintptr_t address, char* pNameOut, size_t nameSize, uintptr_t* pBaseOut);
	static void FreeMouse();
	static INT_PTR ShowDialog(HWND parent, int recursionCount);
	static void ReserveExceptionStack();
	[[noreturn]] static void TerminateNow();
};
