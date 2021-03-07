#include "Debug.h"

void Debug::Log(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message) {
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