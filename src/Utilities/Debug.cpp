#include "Debug.h"
#include "Macro.h"

#include <YRPPCore.h>

char Debug::StringBuffer[0x1000];

void Debug::Log(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::LogWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
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
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(nExitCode));
}

DEFINE_PATCH( // Add new line after "Init Secondary Mixfiles....."
	/* Offset */ 0x825F9B,
	/*   Data */ '\n'
);

DEFINE_PATCH( // Replace SUN.INI with RA2MD.INI in the debug.log
	/* Offset */ 0x8332F4,
	/*   Data */ "-------- Loading RA2MD.INI settings --------\n"
);

static DWORD AresLogPatchJmp1;
void __declspec(naked) AresLogPatch1()
{
	static va_list args;
	static const char* pFormat;

	__asm { mov eax, [esp + 0x4] }
	__asm { mov pFormat, eax }

	__asm { lea eax, [esp + 0x8] };
	__asm { mov args, eax }

	Console::WriteWithVArgs(pFormat, args);

	__asm { mov args, 0 }

	JMP(AresLogPatchJmp1);
}
static DWORD AresLogPatchJmp2;
void __declspec(naked) AresLogPatch2()
{
	static va_list args;
	static const char* pFormat;

	__asm { mov eax, [esp + 0x4] }
	__asm { mov pFormat, eax}

	__asm { lea eax, [esp + 0x8] };
	__asm { mov args, eax }

	Console::WriteWithVArgs(pFormat, args);

	__asm { mov args, 0 }

	JMP(AresLogPatchJmp2);
}

Console::ConsoleTextAttribute Console::TextAttribute;
HANDLE Console::ConsoleHandle;

bool Console::Create()
{
	if (FALSE == AllocConsole())
		return false;

	ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL == ConsoleHandle)
		return false;

	SetConsoleTitle("Phobos Debug Console");

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
	TextAttribute.AsWord = csbi.wAttributes;

	AresLogPatcher(0x4A4AC0, AresLogPatch1, AresLogPatchJmp1);
	AresLogPatcher(0x4068E0, AresLogPatch2, AresLogPatchJmp2);

	return true;
}

void Console::Release()
{
	if (NULL != ConsoleHandle)
		FreeConsole();
}

void Console::SetForeColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Foreground == color)
		return;

	TextAttribute.Foreground = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::SetBackColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Background == color)
		return;

	TextAttribute.Background = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::EnableUnderscore(bool enable)
{
	if (TextAttribute.Underscore == enable)
		return;

	TextAttribute.Underscore = enable;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
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

void Console::WriteWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(Debug::StringBuffer, pFormat, args);
	Write(Debug::StringBuffer, strlen(Debug::StringBuffer));
}

void __cdecl Console::WriteFormat(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	WriteWithVArgs(pFormat, args);
	va_end(args);
}

void Console::AresLogPatcher(DWORD dwAddr, void* newFunc, DWORD& newFuncJmp)
{
#pragma pack(push, 1)
	struct JMP_STRUCT
	{
		byte opcode;
		DWORD offset;
	} *pInst;
#pragma pack(pop)

	DWORD dwOldFlag;
	VirtualProtect((LPVOID)dwAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldFlag);

	pInst = (JMP_STRUCT*)dwAddr;

	DWORD dwActualAddr;
	if (pInst->opcode == 0xE9) // If this function is hooked
		dwActualAddr = pInst->offset + dwAddr + 5;
	else
		dwActualAddr = 0x4A4AF9; // From Ares

	pInst->offset = reinterpret_cast<DWORD>(newFunc) - dwAddr - 5;
	newFuncJmp = dwActualAddr;

	VirtualProtect((LPVOID)dwAddr, 5, dwOldFlag, NULL);
}
