#include "ExceptionHandler.h"
#include "ExceptionHandler.Resource.h"

#include <Phobos.h>

#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <atomic>
#include <process.h>
#include <tlhelp32.h>

// Visual Studio thread-naming exception, must never be treated as a crash.
#define MS_VC_EXCEPTION 0x406D1388

bool ExceptionHandler::GenerateFullCrashDump = false;

DWORD ExceptionHandler::MainThreadId = 0;
char ExceptionHandler::DebugDirectory[MAX_PATH] = { '\0' };
char ExceptionHandler::SnapshotDirectory[MAX_PATH] = { '\0' };

char ExceptionHandler::ReportBuffer[ExceptionHandler::ReportBufferSize] = { '\0' };
size_t ExceptionHandler::ReportLength = 0;
bool ExceptionHandler::ReportFinished = false;

CONTEXT ExceptionHandler::SavedContext = { };
EXCEPTION_RECORD ExceptionHandler::SavedRecord = { };
EXCEPTION_POINTERS ExceptionHandler::SavedPointers = { };
unsigned int ExceptionHandler::SavedCode = 0;
DWORD ExceptionHandler::CrashedThreadId = 0;
SYSTEMTIME ExceptionHandler::CrashTime = { };

char ExceptionHandler::MinidumpPath[MAX_PATH] = { '\0' };
char ExceptionHandler::FullDumpPath[MAX_PATH] = { '\0' };
char ExceptionHandler::ReportPath[MAX_PATH] = { '\0' };

CRITICAL_SECTION ExceptionHandler::DbgHelpLock = { };
bool ExceptionHandler::DbgHelpLockReady = false;
bool ExceptionHandler::SymbolsInitialized = false;

namespace
{
	// Stack reserved by SetThreadStackGuarantee so the handler has room to run
	// after EXCEPTION_STACK_OVERFLOW (which fires with only ~1 page left). The
	// heavy dump itself runs on the dumper thread's fresh stack.
	constexpr ULONG ExceptionStackGuarantee = 64 * 1024;
	constexpr DWORD DumperTimeoutMs = 60000;
	constexpr int MaxSuspendedThreads = 256;
	constexpr int ArtifactMaxAgeDays = 5;

	std::atomic<DWORD> FirstCrashThreadId { 0 };
	std::atomic<DWORD> DumpingThreadId { 0 };
	std::atomic<int> RecursionCount { -1 };
	std::atomic<bool> InitDone { false };

	std::atomic<DWORD> DumperThreadId { 0 };
	HANDLE DumperRequestEvent = nullptr;
	HANDLE DumperDoneEvent = nullptr;

	DWORD SuspendedTids[MaxSuspendedThreads];
	int SuspendedTidCount = 0;
}

void ExceptionHandler::ReserveExceptionStack()
{
	ULONG guarantee = ExceptionStackGuarantee;
	SetThreadStackGuarantee(&guarantee); // Best effort.
}

// TerminateProcess bypasses DLL_DETACH and static destructors, neither of
// which can run safely after a crash with the game in an inconsistent state.
[[noreturn]] void ExceptionHandler::TerminateNow()
{
	TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	__assume(0);
}

namespace
{
	// Suspend every other thread so the dump and minidump observe a frozen
	// address space. Issues a GetThreadContext after each SuspendThread as a
	// kernel barrier: on x86 SuspendThread returns before the target is fully
	// parked. The dumper thread is excluded or the handoff would deadlock.
	void SuspendOtherThreads()
	{
		const DWORD selfTid = GetCurrentThreadId();
		const DWORD selfPid = GetCurrentProcessId();
		SuspendedTidCount = 0;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
			return;

		THREADENTRY32 entry { };
		entry.dwSize = sizeof(entry);

		if (Thread32First(snapshot, &entry))
		{
			do
			{
				const DWORD dumperTid = DumperThreadId.load();
				if (entry.th32OwnerProcessID != selfPid
					|| entry.th32ThreadID == selfTid
					|| (dumperTid != 0 && entry.th32ThreadID == dumperTid)
					|| SuspendedTidCount >= MaxSuspendedThreads)
				{
					continue;
				}

				HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, entry.th32ThreadID);
				if (thread == nullptr)
					continue;

				if (SuspendThread(thread) == static_cast<DWORD>(-1))
				{
					CloseHandle(thread);
					continue;
				}

				CONTEXT context { };
				context.ContextFlags = CONTEXT_CONTROL;
				GetThreadContext(thread, &context);

				SuspendedTids[SuspendedTidCount++] = entry.th32ThreadID;
				CloseHandle(thread);
			}
			while (Thread32Next(snapshot, &entry));
		}

		CloseHandle(snapshot);
	}

	// Workers must be resumed before the dialog runs, or it deadlocks on OS
	// locks (heap, USER32) a worker may hold at its suspended instruction.
	void ResumeOtherThreads()
	{
		for (int i = 0; i < SuspendedTidCount; ++i)
		{
			HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, SuspendedTids[i]);
			if (thread == nullptr)
				continue;

			ResumeThread(thread);
			CloseHandle(thread);
		}

		SuspendedTidCount = 0;
	}

	// Runs the dump off-filter: a fault inside the dump is caught by the
	// per-section __try guards here, while on the crashing thread the dump
	// runs inside an exception filter where nested faults are uncatchable
	// (a nested exception's search skips frames newer than the active filter).
	// A fresh stack also survives EXCEPTION_STACK_OVERFLOW.
	unsigned __stdcall DumperThreadProc(void*)
	{
		DumperThreadId.store(GetCurrentThreadId());
		ExceptionHandler::ReserveExceptionStack();

		for (;;)
		{
			if (WaitForSingleObject(DumperRequestEvent, INFINITE) != WAIT_OBJECT_0)
				continue;

			__try
			{
				ExceptionHandler::WriteCrashArtifacts(&ExceptionHandler::SavedPointers,
					ExceptionHandler::SavedCode, ExceptionHandler::CrashedThreadId);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				// Minidump-first ordering means the key artifact is already on disk.
				Debug::Log("Dumper thread faulted while writing crash artifacts; the report may be partial.\n");
			}

			SetEvent(DumperDoneEvent);
		}

		__assume(0);
	}

	void StartDumperThread()
	{
		// Create the events before spawning, so DumperThreadId != 0 (the
		// availability signal) implies both are valid. Auto-reset = one-shot.
		DumperRequestEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
		DumperDoneEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
		if (DumperRequestEvent == nullptr || DumperDoneEvent == nullptr)
		{
			Debug::Log("ExceptionHandler: could not create dumper events; crash dumps will use the inline path.\n");
			return;
		}

		const uintptr_t thread = _beginthreadex(nullptr, 0, DumperThreadProc, nullptr, 0, nullptr);
		if (thread != 0)
			CloseHandle(reinterpret_cast<HANDLE>(thread));
	}

	void RemoveDirectoryRecursive(const char* pDir)
	{
		char search[MAX_PATH];
		_snprintf_s(search, _TRUNCATE, "%s\\*", pDir);

		WIN32_FIND_DATAA data;
		HANDLE find = FindFirstFileA(search, &data);
		if (find != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
					continue;

				char child[MAX_PATH];
				_snprintf_s(child, _TRUNCATE, "%s\\%s", pDir, data.cFileName);

				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					RemoveDirectoryRecursive(child);
				else
					DeleteFileA(child);
			}
			while (FindNextFileA(find, &data));

			FindClose(find);
		}

		RemoveDirectoryA(pDir);
	}

	// Remove crash snapshot folders (debug\snapshot-*) older than `days`. The
	// CnCNet client also prunes debug\ (at 7 days), so this is a shorter-horizon
	// cleanup for standalone runs.
	void DeleteOldSnapshots(int days)
	{
		char search[MAX_PATH];
		_snprintf_s(search, _TRUNCATE, "%s\\snapshot-*", ExceptionHandler::DebugDirectory);

		FILETIME nowFt;
		GetSystemTimeAsFileTime(&nowFt);
		ULARGE_INTEGER now;
		now.LowPart = nowFt.dwLowDateTime;
		now.HighPart = nowFt.dwHighDateTime;
		const unsigned long long maxAge = 10000000ULL * 60 * 60 * 24 * days;

		WIN32_FIND_DATAA data;
		HANDLE find = FindFirstFileA(search, &data);
		if (find == INVALID_HANDLE_VALUE)
			return;

		do
		{
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			ULARGE_INTEGER written;
			written.LowPart = data.ftLastWriteTime.dwLowDateTime;
			written.HighPart = data.ftLastWriteTime.dwHighDateTime;

			if (now.QuadPart > written.QuadPart && now.QuadPart - written.QuadPart > maxAge)
			{
				char path[MAX_PATH];
				_snprintf_s(path, _TRUNCATE, "%s\\%s", ExceptionHandler::DebugDirectory, data.cFileName);
				RemoveDirectoryRecursive(path);
			}
		}
		while (FindNextFileA(find, &data));

		FindClose(find);
	}

	// Route pure virtual calls from Phobos' (static) CRT into the handler.
	void __cdecl PurecallHandler()
	{
		Debug::Log("Pure virtual function call!\n");
		RaiseException(EXCEPTION_NONCONTINUABLE_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
		ExitProcess(EXIT_FAILURE);
	}

	// A raw __debugbreak here would be fatal under an attached Syringe: it
	// forwards unknown breakpoints back unhandled and the process dies. The
	// guard lets an attached debugger (e.g. VS with Syringe -detach) take the
	// break while falling through to termination everywhere else.
	void GuardedDebugBreak()
	{
		__try
		{
			__debugbreak();
		}
		__except (GetExceptionCode() == EXCEPTION_BREAKPOINT
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
		}
	}
}

void ExceptionHandler::EnsureDebugDirectory()
{
	if (DebugDirectory[0] == '\0')
	{
		char cwd[MAX_PATH];
		if (GetCurrentDirectoryA(sizeof(cwd), cwd) == 0)
			strcpy_s(cwd, ".");

		_snprintf_s(DebugDirectory, _TRUNCATE, "%s\\debug", cwd);
	}

	CreateDirectoryA(DebugDirectory, nullptr);
}

void ExceptionHandler::EnsureSnapshotDirectory()
{
	EnsureDebugDirectory();

	// One snapshot-<timestamp> folder per crash, named like Ares' so the
	// CnCNet client picks it up. Built once from the crash time and reused
	// for every artifact of the same crash (report, dumps, game log).
	if (SnapshotDirectory[0] == '\0')
	{
		_snprintf_s(SnapshotDirectory, _TRUNCATE, "%s\\snapshot-%04u%02u%02u-%02u%02u%02u",
			DebugDirectory, CrashTime.wYear, CrashTime.wMonth, CrashTime.wDay,
			CrashTime.wHour, CrashTime.wMinute, CrashTime.wSecond);
	}

	CreateDirectoryA(SnapshotDirectory, nullptr);
}

void ExceptionHandler::Init()
{
	if (InitDone.exchange(true))
		return;

	MainThreadId = GetCurrentThreadId();
	ReserveExceptionStack();
	_set_purecall_handler(PurecallHandler);

	InitializeCriticalSection(&DbgHelpLock);
	DbgHelpLockReady = true;

	EnsureDebugDirectory();
	DeleteOldSnapshots(ArtifactMaxAgeDays);

	// Load dbghelp and the symbol tables now - during a crash, threads are
	// suspended and one of them might hold the loader lock.
	InitSymbols();

	LoadExceptionDatabase();

	StartDumperThread();

	Debug::Log("ExceptionHandler initialized.\n");
}

LONG __fastcall ExceptionHandler::Handle(int code, EXCEPTION_POINTERS* pExs)
{
	// First-thread-wins entry gate. A second thread that crashes while the
	// winner is dumping parks forever; the winner owns the exit.
	const DWORD selfTid = GetCurrentThreadId();
	DWORD expected = 0;
	if (!FirstCrashThreadId.compare_exchange_strong(expected, selfTid)
		&& expected != selfTid)
	{
		SuspendThread(GetCurrentThread());
		ExitProcess(EXIT_FAILURE);
	}

	// Same-thread re-entry mid-(inline-)dump means the dump itself faulted,
	// which is uncatchable in filter context. Re-running it would recurse
	// forever; the minidump is already on disk, so just die.
	if (DumpingThreadId.load() == selfTid)
	{
		Debug::Log("Nested fault inside the exception dump - terminating (minidump already written).\n");
		TerminateNow();
	}

	if (RecursionCount.load() >= 3)
		ExitProcess(EXIT_FAILURE);

	if (RecursionCount.fetch_add(1) + 1 > 2)
		return EXCEPTION_CONTINUE_SEARCH;

	const unsigned int eCode = static_cast<unsigned int>(code);

	// Debugger-related exceptions are not crashes - release the gate and
	// let the system (or the debugger) continue the search.
	if (eCode == EXCEPTION_BREAKPOINT || eCode == MS_VC_EXCEPTION)
	{
		RecursionCount.fetch_sub(1);
		FirstCrashThreadId.store(0);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	Debug::Log("Exception! Code 0x%08X at 0x%p\n", eCode,
		pExs ? pExs->ExceptionRecord->ExceptionAddress : nullptr);

	// Copy the crashing thread's state into stable storage - the dumper
	// thread and the dialog's full-dump button must not read this thread's
	// live stack. Only the first entry gets to do this.
	if (RecursionCount.load() == 0)
	{
		GetLocalTime(&CrashTime);
		CrashedThreadId = selfTid;
		SavedCode = eCode;

		if (pExs != nullptr && pExs->ContextRecord != nullptr && pExs->ExceptionRecord != nullptr)
		{
			SavedContext = *pExs->ContextRecord;
			SavedRecord = *pExs->ExceptionRecord;
			SavedPointers.ContextRecord = &SavedContext;
			SavedPointers.ExceptionRecord = &SavedRecord;
		}
	}

	EXCEPTION_POINTERS* stablePtrs = (SavedPointers.ContextRecord != nullptr) ? &SavedPointers : nullptr;
	const bool onMainThread = (MainThreadId != 0 && selfTid == MainThreadId);

	// Phase A - snapshot. Freeze the other threads for the dump + minidump,
	// resume them before any UI runs in phase B.
	SuspendOtherThreads();

	if (RecursionCount.load() < 2)
	{
		const DWORD dumperTid = DumperThreadId.load();
		const bool dumperAvailable = dumperTid != 0
			&& selfTid != dumperTid
			&& RecursionCount.load() == 0
			&& stablePtrs != nullptr;

		bool delegated = false;

		if (dumperAvailable)
		{
			SetEvent(DumperRequestEvent);
			delegated = (WaitForSingleObject(DumperDoneEvent, DumperTimeoutMs) == WAIT_OBJECT_0);

			if (!delegated)
				Debug::Log("Dumper thread timed out; continuing with whatever artifacts exist.\n");
		}

		if (!delegated)
		{
			// Inline fallback: dumper not up yet, crash on the dumper itself,
			// or a recursive entry. The DumpingThreadId latch is what stops a
			// nested fault from recursing.
			DumpingThreadId.store(selfTid);
			WriteCrashArtifacts(stablePtrs, eCode, selfTid);
			DumpingThreadId.store(0);
		}
	}

	ResumeOtherThreads();

	if (RecursionCount.load() < 2)
	{
		// Phase B - UI. Surfaces are owned by the main thread, so only free
		// the mouse and black them out when crashing there.
		if (onMainThread)
			FreeMouse();
		else
			ShowCursor(TRUE);

		const INT_PTR result = ShowDialog(onMainThread ? Game::hWnd : nullptr, RecursionCount.load());

		// Phase C - terminate. Never return through the game's __except body
		// or the OS default handler; both run cleanup that depends on
		// subsystems whose state is now suspect.
		switch (result)
		{
		case IDC_EXCEPTION_DEBUG:
			Debug::Log("Debug button pressed.\n");
			GuardedDebugBreak();
			TerminateNow();

		default:
		case IDC_EXCEPTION_QUIT:
			Debug::Log("Quit button pressed.\n");
			TerminateNow();
		}
	}

	if (RecursionCount.load() == 2)
		return EXCEPTION_CONTINUE_SEARCH;

	TerminateNow();
}

// The game's Exception_Handler (0x4C8FE0) is reached from exactly two places,
// both ending in "mov ecx, code; mov edx, e_info; jmp 0x4C8FE0" - matching
// this handler's __fastcall signature. Patching the callsites instead of
// 0x4C8FE0 itself leaves Syringe's hook bookkeeping alone and takes
// precedence over Ares, whose noreturn hook at 0x4C8FE0 would otherwise
// starve a Phobos hook chained after it.
//
// yrpp-spawner (CnCNet-Spawner.dll) pre-filters at the funclet ENTRY with
// DEFINE_HOOK(0x6BE068, ..., 0x7): it recovers a few known crash EIPs by
// returning through the funclet's retn at 0x6BE074 and falls through to
// 0x6BE06F for everything else - so it keeps working, and unrecovered
// crashes land here. This patch must stay exactly 5 bytes (0x6BE06F-0x6BE073)
// so the retn at 0x6BE074 survives.

// WinMain's __except filter funclet around Main_Game.
DEFINE_FUNCTION_JUMP(LJMP, 0x6BE06F, ExceptionHandler::Handle);
// Top_Level_Exception_Filter, registered via SetUnhandledExceptionFilter
// for the shutdown phase after Main_Game returns.
DEFINE_FUNCTION_JUMP(LJMP, 0x6BB996, ExceptionHandler::Handle);
