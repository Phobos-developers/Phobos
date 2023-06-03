#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

class AresData
{
public:
	enum class Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

private:
	// timestamp bytes for each version
	static const std::unordered_map<DWORD, Version> AresTimestampBytes;

	static void GetGameModulesBaseAddresses();

public:
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// offsets of function addresses for each version
	static const std::unordered_map<std::string, std::vector<DWORD>> AresFunctionOffsets;
	// storage for absolute addresses of functions (module base + offset)
	static std::unordered_map<std::string, DWORD> AresFunctionOffsetsFinal;
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersion;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();
};
