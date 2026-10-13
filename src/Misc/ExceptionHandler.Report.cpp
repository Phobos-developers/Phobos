#include "ExceptionHandler.h"

#include <Phobos.h>
#include <Phobos.version.h>

#include <Utilities/Debug.h>
#include <Utilities/AresHelper.h>

#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <dbghelp.h>
#include <tlhelp32.h>

std::vector<ExceptionHandler::ExceptionDatabaseEntry> ExceptionHandler::ExceptionDatabase;

namespace
{
	constexpr int NumCodeBytes = 32;
	constexpr int MaxSymbolName = 256;
	constexpr int StackScanDepth = 512;
	constexpr int BacktraceDepth = 128;
	constexpr int CallStackDepth = 128;

	const char* ExceptionText[] =
	{
		"Error code: EXCEPTION_ACCESS_VIOLATION\r\nDescription: The thread tried to read from or write to a virtual address for which it does not have the appropriate access.",
		"Error code: EXCEPTION_DATATYPE_MISALIGNMENT\r\nDescription: The thread tried to read or write data that is misaligned on hardware that does not provide alignment.",
		"Error code: EXCEPTION_BREAKPOINT\r\nDescription: A breakpoint was encountered.",
		"Error code: EXCEPTION_SINGLE_STEP\r\nDescription: A trace trap or other single-instruction mechanism signaled that one instruction has been executed.",
		"Error code: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\r\nDescription: The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.",
		"Error code: EXCEPTION_FLT_DENORMAL_OPERAND\r\nDescription: One of the operands in a floating-point operation is denormal.",
		"Error code: EXCEPTION_FLT_DIVIDE_BY_ZERO\r\nDescription: The thread tried to divide a floating-point value by a floating-point divisor of zero.",
		"Error code: EXCEPTION_FLT_INEXACT_RESULT\r\nDescription: The result of a floating-point operation cannot be represented exactly as a decimal fraction.",
		"Error code: EXCEPTION_FLT_INVALID_OPERATION\r\nDescription: Some strange unknown floating point operation was attempted.",
		"Error code: EXCEPTION_FLT_OVERFLOW\r\nDescription: The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_FLT_STACK_CHECK\r\nDescription: The stack overflowed or underflowed as the result of a floating-point operation.",
		"Error code: EXCEPTION_FLT_UNDERFLOW\r\nDescription: The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_INT_DIVIDE_BY_ZERO\r\nDescription: The thread tried to divide an integer value by an integer divisor of zero.",
		"Error code: EXCEPTION_INT_OVERFLOW\r\nDescription: The result of an integer operation caused a carry out of the most significant bit of the result.",
		"Error code: EXCEPTION_PRIV_INSTRUCTION\r\nDescription: The thread tried to execute an instruction whose operation is not allowed in the current machine mode.",
		"Error code: EXCEPTION_IN_PAGE_ERROR\r\nDescription: The thread tried to access a page that was not present, and the system was unable to load the page.",
		"Error code: EXCEPTION_ILLEGAL_INSTRUCTION\r\nDescription: The thread tried to execute an invalid instruction.",
		"Error code: EXCEPTION_NONCONTINUABLE_EXCEPTION\r\nDescription: The thread tried to continue execution after a non-continuable exception occurred.",
		"Error code: EXCEPTION_STACK_OVERFLOW\r\nDescription: The thread used up its stack.",
		"Error code: EXCEPTION_INVALID_DISPOSITION\r\nDescription: An exception handler returned an invalid disposition to the exception dispatcher.",
		"Error code: EXCEPTION_GUARD_PAGE\r\nDescription: The thread accessed memory allocated with the PAGE_GUARD modifier.",
		"Error code: EXCEPTION_INVALID_HANDLE\r\nDescription: The thread used a handle to a kernel object that was invalid.",
		"Error code: 0xE06D7363\r\nDescription: A C++ exception was thrown and not caught.",
		"Error code: CONTROL_C_EXIT\r\nDescription: The application terminated as a result of a CTRL+C.",
		"Error code: ????????\r\nDescription: Unknown exception.",
	};

	const unsigned int ExceptionCodes[] =
	{
		EXCEPTION_ACCESS_VIOLATION,
		EXCEPTION_DATATYPE_MISALIGNMENT,
		EXCEPTION_BREAKPOINT,
		EXCEPTION_SINGLE_STEP,
		EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
		EXCEPTION_FLT_DENORMAL_OPERAND,
		EXCEPTION_FLT_DIVIDE_BY_ZERO,
		EXCEPTION_FLT_INEXACT_RESULT,
		EXCEPTION_FLT_INVALID_OPERATION,
		EXCEPTION_FLT_OVERFLOW,
		EXCEPTION_FLT_STACK_CHECK,
		EXCEPTION_FLT_UNDERFLOW,
		EXCEPTION_INT_DIVIDE_BY_ZERO,
		EXCEPTION_INT_OVERFLOW,
		EXCEPTION_PRIV_INSTRUCTION,
		EXCEPTION_IN_PAGE_ERROR,
		EXCEPTION_ILLEGAL_INSTRUCTION,
		EXCEPTION_NONCONTINUABLE_EXCEPTION,
		EXCEPTION_STACK_OVERFLOW,
		EXCEPTION_INVALID_DISPOSITION,
		EXCEPTION_GUARD_PAGE,
		EXCEPTION_INVALID_HANDLE,
		0xE06D7363,
		CONTROL_C_EXIT,
		0xFFFFFFFF,
	};

	static_assert(std::size(ExceptionText) == std::size(ExceptionCodes));

	// Interpret an 80-bit x87 register (10 bytes, little-endian) as a double.
	double ReadX87Register(const unsigned char* pBytes)
	{
		const unsigned long long mantissa = *reinterpret_cast<const unsigned long long*>(pBytes);
		const unsigned short signExponent = *reinterpret_cast<const unsigned short*>(pBytes + 8);
		const double sign = (signExponent & 0x8000) ? -1.0 : 1.0;
		const int exponent = signExponent & 0x7FFF;

		if (exponent == 0 && mantissa == 0)
			return 0.0;

		if (exponent == 0x7FFF)
			return sign * HUGE_VAL;

		return sign * std::ldexp(static_cast<double>(mantissa), exponent - 16383 - 63);
	}

	// Resolve a symbol name (+displacement) and source line for an address.
	// Callers must hold DbgHelpLock and run under a __try guard.
	bool ResolveSymbol(uintptr_t address, char* pNameOut, size_t nameSize, uintptr_t* pDisplacementOut,
		char* pFileOut, size_t fileSize, unsigned* pLineOut)
	{
		if (pNameOut != nullptr)
			pNameOut[0] = '\0';
		if (pFileOut != nullptr)
			pFileOut[0] = '\0';
		if (pLineOut != nullptr)
			*pLineOut = 0;
		if (pDisplacementOut != nullptr)
			*pDisplacementOut = 0;

		if (!ExceptionHandler::SymbolsInitialized)
			return false;

		char buffer[sizeof(SYMBOL_INFO) + MaxSymbolName] = { };
		SYMBOL_INFO* pSymbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MaxSymbolName - 1;

		DWORD64 displacement = 0;
		if (!SymFromAddr(GetCurrentProcess(), address, &displacement, pSymbol))
			return false;

		if (pNameOut != nullptr)
			strncpy_s(pNameOut, nameSize, pSymbol->Name, _TRUNCATE);
		if (pDisplacementOut != nullptr)
			*pDisplacementOut = static_cast<uintptr_t>(displacement);

		IMAGEHLP_LINE64 line = { };
		line.SizeOfStruct = sizeof(line);
		DWORD lineDisplacement = 0;

		if (SymGetLineFromAddr64(GetCurrentProcess(), address, &lineDisplacement, &line) && line.FileName != nullptr)
		{
			// Strip the path off the source file name.
			const char* pName = strrchr(line.FileName, '\\');
			pName = pName != nullptr ? pName + 1 : line.FileName;

			if (pFileOut != nullptr)
				strncpy_s(pFileOut, fileSize, pName, _TRUNCATE);
			if (pLineOut != nullptr)
				*pLineOut = line.LineNumber;
		}

		return true;
	}

	// Append "module.ext+0xOFFSET Symbol()+0xDISP [file:line]" for an address.
	// Callers must hold DbgHelpLock and run under a __try guard.
	void AppendAddressAnnotation(uintptr_t address)
	{
		char module[64];
		uintptr_t base = 0;

		if (ExceptionHandler::ModuleFromAddress(address, module, sizeof(module), &base))
			ExceptionHandler::Append("  %s+0x%X", module, address - base);

		char name[MaxSymbolName];
		char file[128];
		unsigned line = 0;
		uintptr_t displacement = 0;

		if (ResolveSymbol(address, name, sizeof(name), &displacement, file, sizeof(file), &line))
		{
			ExceptionHandler::Append("  %s()+0x%X", name, displacement);

			if (file[0] != '\0')
				ExceptionHandler::Append(" [%s:%u]", file, line);
		}
	}

	void GuardedCrashSite(CONTEXT* pContext)
	{
		if (ExceptionHandler::DbgHelpLockReady)
			EnterCriticalSection(&ExceptionHandler::DbgHelpLock);

		__try
		{
			ExceptionHandler::Append("Exception occurred at 0x%08X", pContext->Eip);
			AppendAddressAnnotation(pContext->Eip);
			ExceptionHandler::Append("\r\n");
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ExceptionHandler::Append("Exception occurred at 0x%08X <symbol lookup faulted>\r\n", pContext->Eip);
		}

		if (ExceptionHandler::DbgHelpLockReady)
			LeaveCriticalSection(&ExceptionHandler::DbgHelpLock);
	}

	// Symbol-free backtrace by walking the EBP frame chain, each frame
	// validated before deref. Needs no dbghelp, so it yields a usable
	// return-address list even when the StackWalk64 walk dies entirely.
	void GuardedManualBacktrace(CONTEXT* pContext)
	{
		if (ExceptionHandler::DbgHelpLockReady)
			EnterCriticalSection(&ExceptionHandler::DbgHelpLock);

		__try
		{
			ExceptionHandler::Append("Raw EBP-chain backtrace:\r\n");

			uintptr_t* pFrame = reinterpret_cast<uintptr_t*>(pContext->Ebp);
			uintptr_t previous = 0;

			for (int i = 0; i < BacktraceDepth; ++i)
			{
				// A valid frame must be readable as a [saved-EBP, return-address]
				// pair and strictly increasing (the x86 stack grows downward).
				if (pFrame == nullptr
					|| reinterpret_cast<uintptr_t>(pFrame) <= previous
					|| IsBadReadPtr(pFrame, 2 * sizeof(uintptr_t)))
				{
					break;
				}

				ExceptionHandler::Append("  0x%08X", pFrame[1]);
				AppendAddressAnnotation(pFrame[1]);
				ExceptionHandler::Append("\r\n");

				previous = reinterpret_cast<uintptr_t>(pFrame);
				pFrame = reinterpret_cast<uintptr_t*>(pFrame[0]);
			}

			ExceptionHandler::Append("\r\n");
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ExceptionHandler::Append("  <manual backtrace faulted>\r\n");
		}

		if (ExceptionHandler::DbgHelpLockReady)
			LeaveCriticalSection(&ExceptionHandler::DbgHelpLock);
	}

	// Full StackWalk64 walk with symbol resolution - the #1 faulter on a
	// corrupt stack, hence its own guard.
	void GuardedCallStack(CONTEXT* pContext)
	{
		if (ExceptionHandler::DbgHelpLockReady)
			EnterCriticalSection(&ExceptionHandler::DbgHelpLock);

		__try
		{
			ExceptionHandler::Append("Call stack:\r\n");

			STACKFRAME64 frame = { };
			frame.AddrPC.Mode = AddrModeFlat;
			frame.AddrPC.Offset = pContext->Eip;
			frame.AddrStack.Mode = AddrModeFlat;
			frame.AddrStack.Offset = pContext->Esp;
			frame.AddrFrame.Mode = AddrModeFlat;
			frame.AddrFrame.Offset = pContext->Ebp;

			for (int i = 0; i < CallStackDepth; ++i)
			{
				// The context record parameter is only required for machine
				// types other than IMAGE_FILE_MACHINE_I386.
				if (!StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(),
					&frame, nullptr, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)
					|| frame.AddrFrame.Offset == 0)
				{
					break;
				}

				const uintptr_t address = static_cast<uintptr_t>(frame.AddrPC.Offset);
				ExceptionHandler::Append("  0x%08X", address);
				AppendAddressAnnotation(address);
				ExceptionHandler::Append("\r\n");
			}

			ExceptionHandler::Append("\r\n");
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ExceptionHandler::Append("  <call-stack walk faulted (corrupt stack or symbols); skipped>\r\n");
		}

		if (ExceptionHandler::DbgHelpLockReady)
			LeaveCriticalSection(&ExceptionHandler::DbgHelpLock);
	}

	// Raw scan of stack memory from ESP: print each slot and annotate the
	// ones that look like code pointers.
	void GuardedRawStackScan(CONTEXT* pContext)
	{
		if (ExceptionHandler::DbgHelpLockReady)
			EnterCriticalSection(&ExceptionHandler::DbgHelpLock);

		__try
		{
			uintptr_t* pAddress = reinterpret_cast<uintptr_t*>(pContext->Esp);

			for (int i = 0; i < StackScanDepth; ++i, ++pAddress)
			{
				if (IsBadReadPtr(pAddress, sizeof(uintptr_t)))
				{
					ExceptionHandler::Append("%08X: ????????\r\n", reinterpret_cast<uintptr_t>(pAddress));
					continue;
				}

				if (IsBadCodePtr(reinterpret_cast<FARPROC>(*pAddress)))
				{
					ExceptionHandler::Append("%08X: %08X\r\n", reinterpret_cast<uintptr_t>(pAddress), *pAddress);
					continue;
				}

				ExceptionHandler::Append("%08X: %08X *", reinterpret_cast<uintptr_t>(pAddress), *pAddress);
				AppendAddressAnnotation(*pAddress);
				ExceptionHandler::Append("\r\n");
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ExceptionHandler::Append("  <raw stack scan faulted; truncated>\r\n");
		}

		if (ExceptionHandler::DbgHelpLockReady)
			LeaveCriticalSection(&ExceptionHandler::DbgHelpLock);
	}

	void GuardedModuleList()
	{
		__try
		{
			ExceptionHandler::Append("Loaded modules:\r\n");

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
			if (snapshot == INVALID_HANDLE_VALUE)
			{
				ExceptionHandler::Append("  <module snapshot failed>\r\n");
				return;
			}

			MODULEENTRY32 entry = { };
			entry.dwSize = sizeof(entry);

			if (Module32First(snapshot, &entry))
			{
				do
				{
					ExceptionHandler::Append("  0x%08X - 0x%08X  %s\r\n",
						reinterpret_cast<uintptr_t>(entry.modBaseAddr),
						reinterpret_cast<uintptr_t>(entry.modBaseAddr) + entry.modBaseSize,
						entry.szModule);
				}
				while (Module32Next(snapshot, &entry));
			}

			CloseHandle(snapshot);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			ExceptionHandler::Append("  <module list faulted>\r\n");
		}
	}

	void AppendRegisters(CONTEXT* pContext)
	{
		ExceptionHandler::Append("Details:\r\n");

		ExceptionHandler::Append("Eip:%08X\tEsp:%08X\tEbp:%08X\r\n", pContext->Eip, pContext->Esp, pContext->Ebp);
		ExceptionHandler::Append("Eax:%08X\tEbx:%08X\tEcx:%08X\r\n", pContext->Eax, pContext->Ebx, pContext->Ecx);
		ExceptionHandler::Append("Edx:%08X\tEsi:%08X\tEdi:%08X\r\n", pContext->Edx, pContext->Esi, pContext->Edi);
		ExceptionHandler::Append("EFlags:%08X\r\n", pContext->EFlags);
		ExceptionHandler::Append("CS:%04X  SS:%04X  DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n",
			pContext->SegCs, pContext->SegSs, pContext->SegDs, pContext->SegEs, pContext->SegFs, pContext->SegGs);

		if (pContext->ContextFlags & CONTEXT_FLOATING_POINT)
		{
			ExceptionHandler::Append("\r\nFloating point status:\r\n");
			ExceptionHandler::Append("     Control word: %08X\r\n", pContext->FloatSave.ControlWord);
			ExceptionHandler::Append("      Status word: %08X\r\n", pContext->FloatSave.StatusWord);
			ExceptionHandler::Append("         Tag word: %08X\r\n", pContext->FloatSave.TagWord);
			ExceptionHandler::Append("     Error offset: %08X\r\n", pContext->FloatSave.ErrorOffset);
			ExceptionHandler::Append("   Error selector: %08X\r\n", pContext->FloatSave.ErrorSelector);
			ExceptionHandler::Append("      Data offset: %08X\r\n", pContext->FloatSave.DataOffset);
			ExceptionHandler::Append("    Data selector: %08X\r\n", pContext->FloatSave.DataSelector);

			for (int i = 0; i < 8; ++i)
			{
				ExceptionHandler::Append("ST%d : ", i);

				for (int j = 0; j < 10; ++j)
					ExceptionHandler::Append("%02X", pContext->FloatSave.RegisterArea[i * 10 + j]);

				ExceptionHandler::Append("   %+#.17e\r\n", ReadX87Register(&pContext->FloatSave.RegisterArea[i * 10]));
			}
		}

		// The FXSAVE area: MM registers live at offset 32, 16 bytes apart,
		// aliasing the low 8 bytes of the ST registers.
		if ((pContext->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS
			&& IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
		{
			ExceptionHandler::Append("\r\n");

			for (int i = 0; i < 8; ++i)
			{
				const auto value = *reinterpret_cast<const unsigned long long*>(&pContext->ExtendedRegisters[32 + 16 * i]);
				ExceptionHandler::Append("MMX%d:%016llX%s", i, value, (i % 4 == 3) ? "\r\n" : "\t");
			}
		}

		if ((pContext->ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS)
		{
			ExceptionHandler::Append("\r\n");
			ExceptionHandler::Append("Dr0:%08X\tDr1:%08X\tDr2:%08X\tDr3:%08X\r\n",
				pContext->Dr0, pContext->Dr1, pContext->Dr2, pContext->Dr3);
			ExceptionHandler::Append("Dr6:%08X\tDr7:%08X\r\n", pContext->Dr6, pContext->Dr7);
		}
	}

	void AppendCodeBytes(CONTEXT* pContext)
	{
		ExceptionHandler::Append("\r\nBytes at CS:EIP (%08X): ", pContext->Eip);

		// The crash may have been caused by a bad instruction pointer, so
		// every byte is validated before the read; unreadable ones print '??'.
		const unsigned char* pByte = reinterpret_cast<const unsigned char*>(pContext->Eip);

		for (int i = 0; i < NumCodeBytes; ++i, ++pByte)
		{
			if (IsBadReadPtr(pByte, sizeof(unsigned char)))
				ExceptionHandler::Append("?? ");
			else
				ExceptionHandler::Append("%02X ", *pByte);
		}

		ExceptionHandler::Append("\r\n");
	}

	void AppendMemoryStatus()
	{
		MEMORYSTATUSEX status = { };
		status.dwLength = sizeof(status);

		if (GlobalMemoryStatusEx(&status))
		{
			ExceptionHandler::Append("Memory status:\r\n");
			ExceptionHandler::Append("  Memory load: %u%%\r\n", status.dwMemoryLoad);
			ExceptionHandler::Append("  Physical: %llu MB free of %llu MB\r\n",
				status.ullAvailPhys / (1024 * 1024), status.ullTotalPhys / (1024 * 1024));
			ExceptionHandler::Append("  Virtual (process): %llu MB free of %llu MB\r\n",
				status.ullAvailVirtual / (1024 * 1024), status.ullTotalVirtual / (1024 * 1024));
			ExceptionHandler::Append("\r\n");
		}
	}

	void AppendHeader(const char* pKnownCrash)
	{
		ExceptionHandler::Append("Yuri's Revenge has encountered a fatal error - Phobos exception report\r\n");
		ExceptionHandler::Append("=======================================================================\r\n\r\n");

		if (pKnownCrash != nullptr)
		{
			ExceptionHandler::Append("Known information:\r\n  %s\r\n", pKnownCrash);
			ExceptionHandler::Append("-----------------------------------------------------------------------\r\n\r\n");
		}

		ExceptionHandler::Append("Phobos version: " PRODUCT_VERSION "\r\n");
#ifdef STR_GIT_COMMIT
		ExceptionHandler::Append("Git commit: " STR_GIT_COMMIT "\r\n");
		ExceptionHandler::Append("Git dirty: " GIT_DIRTY_FLAG "\r\n");
#endif
#ifdef STR_GIT_REF
		ExceptionHandler::Append("Git ref: " STR_GIT_REF "\r\n");
#endif
		ExceptionHandler::Append("Phobos base address: 0x%08X\r\n", AresHelper::PhobosBaseAddress);

		if (AresHelper::AresBaseAddress != 0)
		{
			const char* pAresVersion = "unknown version";
			switch (AresHelper::AresVersion)
			{
			case AresHelper::Version::Ares30: pAresVersion = "3.0"; break;
			case AresHelper::Version::Ares30p: pAresVersion = "3.0p1"; break;
			default: break;
			}

			ExceptionHandler::Append("Ares: %s at base address 0x%08X\r\n", pAresVersion, AresHelper::AresBaseAddress);
		}
		else
		{
			ExceptionHandler::Append("Ares: not detected\r\n");
		}

		ExceptionHandler::Append("Time stamp: %02u-%02u-%04u %02u:%02u:%02u\r\n",
			ExceptionHandler::CrashTime.wDay, ExceptionHandler::CrashTime.wMonth, ExceptionHandler::CrashTime.wYear,
			ExceptionHandler::CrashTime.wHour, ExceptionHandler::CrashTime.wMinute, ExceptionHandler::CrashTime.wSecond);
		ExceptionHandler::Append("Command line: %s\r\n", GetCommandLineA());
		ExceptionHandler::Append("Crashed thread: 0x%X%s\r\n\r\n", ExceptionHandler::CrashedThreadId,
			ExceptionHandler::CrashedThreadId == ExceptionHandler::MainThreadId ? " (main thread)" : "");
	}

	void CopyGameLog()
	{
		// Best effort - the log location depends on the environment: Ares
		// keeps the live log at debug\debug.log.tmp, others use the game dir.
		const char* candidates[] = { "debug\\debug.log.tmp", "debug\\debug.log", "debug.log" };

		for (const char* pCandidate : candidates)
		{
			if (GetFileAttributesA(pCandidate) == INVALID_FILE_ATTRIBUTES)
				continue;

			char destination[MAX_PATH];
			_snprintf_s(destination, _TRUNCATE, "%s\\debug.log", ExceptionHandler::SnapshotDirectory);

			CopyFileA(pCandidate, destination, FALSE);
			break;
		}
	}

	// The dump call gets its own guard so a fault inside dbghelp cannot
	// unwind past the DbgHelpLock release in WriteMinidump - the lock would
	// stay owned forever and every later dump attempt would deadlock on it.
	BOOL GuardedMiniDumpWrite(HANDLE file, MINIDUMP_TYPE flags, MINIDUMP_EXCEPTION_INFORMATION* pInfo)
	{
		__try
		{
			return MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, flags, pInfo, nullptr, nullptr);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return FALSE;
		}
	}
}

void ExceptionHandler::Append(const char* pFormat, ...)
{
	char scratch[2048];

	va_list args;
	va_start(args, pFormat);
	const int written = vsnprintf(scratch, sizeof(scratch), pFormat, args);
	va_end(args);

	if (written <= 0)
		return;

	size_t length = static_cast<size_t>(written);
	if (length > sizeof(scratch) - 1)
		length = sizeof(scratch) - 1;
	if (ReportLength + length >= ReportBufferSize)
		return;

	memcpy(&ReportBuffer[ReportLength], scratch, length);
	ReportLength += length;
	ReportBuffer[ReportLength] = '\0';
}

bool ExceptionHandler::InitSymbols()
{
	if (DbgHelpLockReady)
		EnterCriticalSection(&DbgHelpLock);

	// The dbghelp calls run under a guard so a fault in them cannot unwind
	// past the lock release below and leave DbgHelpLock owned forever.
	__try
	{
		if (!SymbolsInitialized)
		{
			// SYMOPT_FAIL_CRITICAL_ERRORS keeps dbghelp from raising hard-error
			// dialogs of its own mid-crash.
			SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME
				| SYMOPT_OMAP_FIND_NEAREST | SYMOPT_LOAD_ANYTHING | SYMOPT_FAIL_CRITICAL_ERRORS);

			// The default search path already covers each module's own directory,
			// which is where Phobos.pdb lives; gamemd addresses simply resolve to
			// module+offset.
			SymbolsInitialized = SymInitialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;

			// gamemd.exe carries no CodeView debug record, so dbghelp will never
			// find a pdb for it on its own. If the user dropped one into the game
			// directory, force-load it over the module - SYMOPT_LOAD_ANYTHING
			// skips the signature checks.
			if (SymbolsInitialized && GetFileAttributesA("gamemd.pdb") != INVALID_FILE_ATTRIBUTES)
			{
				const HMODULE hGame = GetModuleHandleA(nullptr);
				const uintptr_t base = reinterpret_cast<uintptr_t>(hGame);
				const auto pDosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(hGame);
				const auto pNtHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + pDosHeader->e_lfanew);

				SymUnloadModule64(GetCurrentProcess(), base);

				if (SymLoadModuleEx(GetCurrentProcess(), nullptr, "gamemd.pdb", nullptr,
					base, pNtHeaders->OptionalHeader.SizeOfImage, nullptr, 0))
				{
					Debug::Log("ExceptionHandler: loaded gamemd.pdb for symbol resolution.\n");
				}
				else
				{
					Debug::Log("ExceptionHandler: failed to load gamemd.pdb (error %u).\n", GetLastError());
				}
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	const bool result = SymbolsInitialized;

	if (DbgHelpLockReady)
		LeaveCriticalSection(&DbgHelpLock);

	return result;
}

void ExceptionHandler::LoadExceptionDatabase()
{
	FILE* pFile = nullptr;
	if (fopen_s(&pFile, "gamemd.edb", "r") != 0 || pFile == nullptr)
		return;

	char line[1200];

	while (fgets(line, sizeof(line), pFile) != nullptr)
	{
		char* pCursor = line;
		while (std::isspace(static_cast<unsigned char>(*pCursor)))
			++pCursor;

		if (*pCursor == '\0' || *pCursor == ';')
			continue;

		// Format (shared with Vinifera): 0xADDRESS,canContinue,ignore,description
		char* pContext = nullptr;
		char* pToken = strtok_s(pCursor, ",", &pContext);
		if (pToken == nullptr || pToken[0] != '0' || (pToken[1] != 'x' && pToken[1] != 'X'))
			continue;

		const unsigned int address = std::strtoul(pToken + 2, nullptr, 16);

		// CanContinue and Ignore are part of the format but not consulted.
		if (strtok_s(nullptr, ",", &pContext) == nullptr || strtok_s(nullptr, ",", &pContext) == nullptr)
			continue;

		char* pDescription = strtok_s(nullptr, "\r\n", &pContext);
		if (pDescription == nullptr)
			continue;

		ExceptionDatabase.push_back({ address, pDescription });
	}

	fclose(pFile);

	Debug::Log("ExceptionHandler: loaded %u entries from gamemd.edb.\n",
		static_cast<unsigned int>(ExceptionDatabase.size()));
}

bool ExceptionHandler::ModuleFromAddress(uintptr_t address, char* pNameOut, size_t nameSize, uintptr_t* pBaseOut)
{
	if (pNameOut != nullptr)
		pNameOut[0] = '\0';
	if (pBaseOut != nullptr)
		*pBaseOut = 0;

	MEMORY_BASIC_INFORMATION info = { };
	if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0 || info.AllocationBase == nullptr)
		return false;

	char path[MAX_PATH];
	if (GetModuleFileNameA(reinterpret_cast<HMODULE>(info.AllocationBase), path, sizeof(path)) == 0)
		return false;

	const char* pName = strrchr(path, '\\');
	pName = pName != nullptr ? pName + 1 : path;

	if (pNameOut != nullptr)
		strncpy_s(pNameOut, nameSize, pName, _TRUNCATE);
	if (pBaseOut != nullptr)
		*pBaseOut = reinterpret_cast<uintptr_t>(info.AllocationBase);

	return true;
}

bool ExceptionHandler::WriteMinidump(EXCEPTION_POINTERS* pExs, DWORD crashedTid, bool fullMemory, char* pPathOut, size_t pathOutSize)
{
	EnsureSnapshotDirectory();

	_snprintf_s(pPathOut, pathOutSize, _TRUNCATE, "%s\\%s",
		SnapshotDirectory, fullMemory ? "fulldump.dmp" : "crashdump.dmp");

	// Cached I/O: write-through would force each of MiniDumpWriteDump's many
	// small writes synchronously to disk, stretching a full-memory dump from
	// seconds into minutes of apparent hang.
	HANDLE file = CreateFileA(pPathOut, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, nullptr);

	if (file == INVALID_HANDLE_VALUE)
	{
		Debug::Log("Failed to create minidump file \"%s\" (error %u).\n", pPathOut, GetLastError());
		pPathOut[0] = '\0';
		return false;
	}

	// A full-memory dump already contains everything indirectly-referenced
	// chasing would add - keeping that flag only buys an extra pointer-chasing
	// pass over every thread stack.
	const MINIDUMP_TYPE flags = static_cast<MINIDUMP_TYPE>(fullMemory
		? MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithFullMemory
		: MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory);

	MINIDUMP_EXCEPTION_INFORMATION info = { };
	// ThreadId must be the CRASHING thread - when the dump runs on the dumper
	// thread that is not the current thread.
	info.ThreadId = (crashedTid != 0) ? crashedTid : GetCurrentThreadId();
	info.ExceptionPointers = pExs;
	info.ClientPointers = FALSE;

	if (DbgHelpLockReady)
		EnterCriticalSection(&DbgHelpLock);

	const BOOL result = GuardedMiniDumpWrite(file, flags, pExs != nullptr ? &info : nullptr);

	if (DbgHelpLockReady)
		LeaveCriticalSection(&DbgHelpLock);

	CloseHandle(file);

	Debug::Log("Minidump %s: \"%s\".\n", result ? "generated" : "FAILED", pPathOut);
	return result != FALSE;
}

void ExceptionHandler::BuildReport(unsigned int code, EXCEPTION_POINTERS* pExs)
{
	ReportLength = 0;
	ReportBuffer[0] = '\0';
	ReportFinished = false;

	InitSymbols();

	// Surface a known-crash description from the EDB right under the title,
	// where a user pasting just the top of the report still includes it.
	const char* pKnownCrash = nullptr;
	if (pExs != nullptr && pExs->ContextRecord != nullptr)
	{
		for (const auto& entry : ExceptionDatabase)
		{
			if (entry.Address == pExs->ContextRecord->Eip)
			{
				pKnownCrash = entry.Description.c_str();
				break;
			}
		}
	}

	AppendHeader(pKnownCrash);

	const char* pDescription = ExceptionText[std::size(ExceptionText) - 1];
	for (size_t i = 0; i < std::size(ExceptionCodes); ++i)
	{
		if (code == ExceptionCodes[i])
		{
			pDescription = ExceptionText[i];
			break;
		}
	}

	Append("%s\r\n", pDescription);

	if (pExs == nullptr || pExs->ContextRecord == nullptr || pExs->ExceptionRecord == nullptr)
	{
		Append("\r\nNo exception context is available; the report ends here.\r\n");
		ReportFinished = true;
		return;
	}

	EXCEPTION_RECORD* pRecord = pExs->ExceptionRecord;
	CONTEXT* pContext = pExs->ContextRecord;

	if (code == EXCEPTION_ACCESS_VIOLATION)
	{
		switch (pRecord->ExceptionInformation[0])
		{
		case 0:
			Append("Access address: 0x%08X was read from.\r\n", pRecord->ExceptionInformation[1]);
			break;
		case 1:
			Append("Access address: 0x%08X was written to.\r\n", pRecord->ExceptionInformation[1]);
			break;
		case 8:
			Append("Access address: 0x%08X DEP violation.\r\n", pRecord->ExceptionInformation[1]);
			break;
		default:
			Append("Access address: 0x%08X unknown violation.\r\n", pRecord->ExceptionInformation[1]);
			break;
		}
	}

	GuardedCrashSite(pContext);

	Append("\r\n");

	// A symbol-free EBP-chain backtrace first (always works, even if dbghelp
	// dies), then the full StackWalk64 walk. Individually guarded so a fault
	// in either doesn't cost the rest of the dump.
	GuardedManualBacktrace(pContext);
	GuardedCallStack(pContext);

	AppendRegisters(pContext);
	AppendCodeBytes(pContext);
	Append("\r\n");

	GuardedModuleList();
	Append("\r\n");

	AppendMemoryStatus();

	Append("Stack dump (* indicates possible code address):\r\n");
	GuardedRawStackScan(pContext);

	Append("\r\nCrash artifacts have been saved to:\r\n  %s\r\n", SnapshotDirectory);
	Append("When reporting this crash, please include that whole folder (report, minidump and logs).\r\n");

	ReportFinished = true;
}

void ExceptionHandler::WriteCrashArtifacts(EXCEPTION_POINTERS* pExs, unsigned int code, DWORD crashedTid)
{
	EnsureSnapshotDirectory();

	// Minidump first - anything after this can fault, but the key artifact
	// is already on disk.
	WriteMinidump(pExs, crashedTid, GenerateFullCrashDump, MinidumpPath, sizeof(MinidumpPath));

	BuildReport(code, pExs);

	_snprintf_s(ReportPath, _TRUNCATE, "%s\\except.txt", SnapshotDirectory);

	HANDLE file = CreateFileA(ReportPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, nullptr);

	if (file != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		WriteFile(file, ReportBuffer, static_cast<DWORD>(ReportLength), &written, nullptr);
		CloseHandle(file);
		Debug::Log("Exception report written to \"%s\".\n", ReportPath);
	}
	else
	{
		Debug::Log("Failed to create exception report file \"%s\" (error %u).\n", ReportPath, GetLastError());
	}

	CopyGameLog();
}
