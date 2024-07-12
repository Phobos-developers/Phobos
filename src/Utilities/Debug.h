#pragma once

#include <Windows.h>

class Debug
{
public:
	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514,
		BadINIUsage = 1919810,
	};

	static char StringBuffer[0x1000];
	static char FinalStringBuffer[0x1000];
	static char DeferredStringBuffer[0x1000];
	static int CurrentBufferSize;

	static void Log(const char* pFormat, ...);
	static void LogGame(const char* pFormat, ...);
	static void LogDeferred(const char* pFormat, ...);
	static void LogDeferredFinalize();
	static void LogAndMessage(const char* pFormat, ...);
	static void LogWithVArgs(const char* pFormat, va_list args);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	static void FatalErrorAndExit(const char* pFormat, ...);
	static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
};

class Console
{
public:
	enum class ConsoleColor
	{
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkRed = 4,
		Intensity = 8,

		DarkCyan = DarkBlue | DarkGreen,
		DarkMagenta = DarkBlue | DarkRed,
		DarkYellow = DarkGreen | DarkRed,
		Gray = DarkBlue | DarkGreen | DarkRed,
		DarkGray = Black | Intensity,

		Blue = DarkBlue | Intensity,
		Green = DarkGreen | Intensity,
		Red = DarkRed | Intensity,
		Cyan = Blue | Green,
		Magenta = Blue | Red,
		Yellow = Green | Red,
		White = Red | Green | Blue,
	};

	union ConsoleTextAttribute
	{
		WORD AsWord;
		struct
		{
			ConsoleColor Foreground : 4;
			ConsoleColor Background : 4;
			bool LeadingByte : 1;
			bool TrailingByte : 1;
			bool GridTopHorizontal : 1;
			bool GridLeftVertical : 1;
			bool GridRightVerticle : 1;
			bool ReverseVideo : 1; // Reverse fore/back ground attribute
			bool Underscore : 1;
			bool Unused : 1;
		};
	};
	static ConsoleTextAttribute TextAttribute;
	static HANDLE ConsoleHandle;

	static bool Create();
	static void Release();

	template<size_t Length>
	constexpr static void Write(const char (&str)[Length])
	{
		Write(str, Length - 1); // -1 because there is a '\0' here
	}
	static void SetForeColor(ConsoleColor color);
	static void SetBackColor(ConsoleColor color);
	static void EnableUnderscore(bool enable);
	static void Write(const char* str, int len);
	static void WriteLine(const char* str, int len);
	static void __fastcall WriteWithVArgs(const char* pFormat, va_list args);
	static void WriteFormat(const char* pFormat, ...);

private:
	static void PatchLog(DWORD dwAddr, void* realFunc, DWORD* pdwRealFunc);
};
