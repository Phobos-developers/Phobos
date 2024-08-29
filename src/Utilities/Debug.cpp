#include "Debug.h"
#include "Macro.h"

#include <YRPPCore.h>
#include <MessageListClass.h>
#include <CRT.h>

char Debug::StringBuffer[0x1000];
char Debug::FinalStringBuffer[0x1000];
char Debug::DeferredStringBuffer[0x1000];
int Debug::CurrentBufferSize = 0;

void Debug::Log(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(FinalStringBuffer, pFormat, args);
	LogGame("%s %s", "[Phobos]", FinalStringBuffer);
	va_end(args);
}

void Debug::LogGame(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::LogDeferred(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	CurrentBufferSize += vsprintf_s(DeferredStringBuffer + CurrentBufferSize, 4096 - CurrentBufferSize, pFormat, args);
	va_end(args);
}

void Debug::LogDeferredFinalize()
{
	Log("%s", DeferredStringBuffer);
	CurrentBufferSize = 0;
}

void Debug::LogAndMessage(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
	va_end(args);
	wchar_t buffer[0x1000];
	CRT::mbstowcs(buffer, StringBuffer, 0x1000);
	MessageListClass::Instance->PrintMessage(buffer);
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
	MessageBox(0, StringBuffer, "Fatal error ", MB_ICONERROR);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	MessageBox(0, StringBuffer, "Fatal error ", MB_ICONERROR);
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

static DWORD _Real_Debug_Log = 0x4A4AF9;
void __declspec(naked) _Fake_Debug_Log()
{
	// va_start(args, pFormat);
	// Console::WriteWithVArgs(pFormat, args);
	// // va_end(args);
	// No need to use va_end here.
	//
	// As Console::WriteWithVArgs uses __fastcall,
	// ECX: pFormat, EDX: args
	__asm { mov ecx, [esp + 0x4] }
	__asm { lea edx, [esp + 0x8] }
	__asm { call Console::WriteWithVArgs }
	// __asm { mov edx, 0}

	// goto original bytes
	__asm { mov eax, _Real_Debug_Log }
	__asm { jmp eax }
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

	PatchLog(0x4A4AC0, _Fake_Debug_Log, &_Real_Debug_Log);
	PatchLog(0x4068E0, _Fake_Debug_Log, nullptr);

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
	if (NULL == ConsoleHandle)
		return;

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

void __fastcall Console::WriteWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(Debug::StringBuffer, pFormat, args);
	Write(Debug::StringBuffer, strlen(Debug::StringBuffer));
}

void Console::WriteFormat(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	WriteWithVArgs(pFormat, args);
	va_end(args);
}

void Console::PatchLog(DWORD dwAddr, void* fakeFunc, DWORD* pdwRealFunc)
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

	if (pdwRealFunc && pInst->opcode == 0xE9) // If this function is hooked by Ares
		*pdwRealFunc = pInst->offset + dwAddr + 5;

	pInst->offset = reinterpret_cast<DWORD>(fakeFunc) - dwAddr - 5;

	VirtualProtect((LPVOID)dwAddr, 5, dwOldFlag, NULL);
}
