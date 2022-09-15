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

	static void __cdecl Log(const char* pFormat, ...);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	static void FatalErrorAndExit(const char* pFormat, ...);
	static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);

private:
	static void __cdecl WriteLog(const char* pFormat, ...);
};

class Console
{
public:
	static HANDLE ConsoleHandle;

	static bool Create();
	static void Release();

	template<size_t LengthPlus1>
	constexpr static void Write(const char (&str)[LengthPlus1])
	{
		Write(str, LengthPlus1 - 1);
	}
	static void Write(const char* str, int len);
	static void WriteLine(const char* str, int len);
	static void __cdecl WriteFormat(const char* pFormat, ...);
};
