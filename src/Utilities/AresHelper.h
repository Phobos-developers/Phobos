#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <Windows.h>

class AresHelper
{
public:
	enum class Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

private:
	typedef std::unordered_map<DWORD, Version> AresTimestampMap;

	// timestamp bytes for each version
	static const AresTimestampMap AresTimestampBytes;

	static void GetGameModulesBaseAddresses();

public:
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersion;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();
};
