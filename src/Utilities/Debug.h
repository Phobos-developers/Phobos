#pragma once

#include <Windows.h>

class Debug
{
public:
	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514
	};

	static char StringBuffer[0x1000];

	static void __cdecl Log(const char* pFormat, ...);
	static void LogWithVArgs(const char* pFormat, va_list args);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	static void FatalErrorAndExit(const char* pFormat, ...);
	static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
};

class Console
{
public:
	static HANDLE ConsoleHandle;

	static bool Create();
	static void Release();

	template<size_t Length>
	constexpr static void Write(const char (&str)[Length])
	{
		Write(str, Length - 1); // -1 because there is a '\0' here
	}
	static void Write(const char* str, int len);
	static void WriteLine(const char* str, int len);
	static void WriteWithVArgs(const char* pFormat, va_list args);
	static void __cdecl WriteFormat(const char* pFormat, ...);

private:
	static void AresLogPatcher(DWORD dwAddr, void* newFunc, DWORD& newFuncJmp);
};
