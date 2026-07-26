#pragma once
#include <unordered_map>
#include <Windows.h>

#include "AntaresAPI.h"

class AresHelper
{
public:
	enum class Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,

		//! Antares, an open-source reimplementation of Ares. Detected by export
		//! rather than by filename and timestamp, and driven entirely through its
		//! interop table -- none of the Ares RVAs or byte patches apply to it.
		Antares,
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

	static HMODULE AntaresDllHmodule;
	static uintptr_t AntaresBaseAddress;
	//! Antares' interop table, or null. Owned by Antares.
	static AntaresAPI_v1* Antares;

	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersion;

	//! Genuine Ares, with the exact build we know the addresses for. This gates
	//! everything that reads an Ares RVA, patches Ares' code, or reaches into its
	//! extension data by offset. It is FALSE for Antares -- see CanUseExtension.
	static bool CanUseAres;

	//! Antares detected and its interop table obtained.
	static bool CanUseAntares;

	//! An Ares-compatible extension of either kind is present. Use this to gate
	//! functionality that goes through AresFunctions, which is filled from whichever
	//! one was found.
	static bool CanUseExtension;

	static void Init();
};
