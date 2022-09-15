#include "Debug.h"
#include "Macro.h"

#include <YRPPCore.h>

void Debug::Log(const char* pFormat, ...)
{
#ifdef DEBUG
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Console::Write(buffer, strlen(buffer));
	Debug::WriteLog("%s", buffer);
#else
	JMP_STD(0x4068E0);
#endif
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	const char* LogMessage = (Message == nullptr)
		? "Failed to parse INI file content: [%s]%s=%s\n"
		: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

	Debug::Log(LogMessage, section, flag, value, Message);
}

void Debug::FatalErrorAndExit(const char* pFormat, ...)
{
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Debug::Log(buffer);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Debug::Log(buffer);
	FatalExit(static_cast<int>(nExitCode));
}

void __cdecl Debug::WriteLog(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

DEFINE_PATCH( // Add new line after "Init Secondary Mixfiles....."
	/* Offset */ 0x825F9B,
	/*   Data */ '\n'
);

DEFINE_PATCH( // Replace SUN.INI with RA2MD.INI in the debug.log
	/* Offset */ 0x8332F4,
	/*   Data */ "-------- Loading RA2MD.INI settings --------\n"
);

HANDLE Console::ConsoleHandle;

bool Console::Create()
{
	if (FALSE == AllocConsole())
		return false;

	ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL == ConsoleHandle)
		return false;

	SetConsoleTitle("Phobos Debug Console");

	return true;
}

void Console::Release()
{
	if (NULL != ConsoleHandle)
		FreeConsole();
}

void Console::Write(const char* str, int len)
{
	if (NULL != ConsoleHandle)
		WriteConsole(ConsoleHandle, str, len, nullptr, nullptr);
}

void Console::WriteLine(const char* str, int len)
{
	Write(str, len);
	Write("\n");
}

void __cdecl Console::WriteFormat(const char* pFormat, ...)
{
	char buffer[0x400];
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(buffer, pFormat, args);
	va_end(args);
	Write(buffer, strlen(buffer));
}
