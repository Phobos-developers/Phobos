#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <Windows.h>

#define ARES_FUN(name) std::string(NAMEOF(AresFunctions::name))
#define ARES_FUN_M(name) std::string(NAMEOF_MEMBER(name))
#define IS_ARES_FUN_AVAILABLE(name) AresHelper::CanUseAres && (AresHelper::AresFunctionOffsetsFinal[ARES_FUN(name)] != 0)

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

	typedef std::unordered_map<std::string, DWORD> AresFunctionMap;
	typedef std::unordered_map<AresHelper::Version, AresFunctionMap> AresVersionFunctionMap;

	// offsets of function addresses for each version
	static const AresVersionFunctionMap AresFunctionOffsets;
	// storage for absolute addresses of functions (module base + offset)
	static AresFunctionMap AresFunctionOffsetsFinal;
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersion;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();
};
