#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <CRC.h>

#include <vector>
#include <tlhelp32.h>

class TechnoClass;
class TechnoTypeClass;

uintptr_t AresData::PhobosBaseAddress = 0x0;
uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;
int AresData::AresVersionId = AresData::Version::Unknown;
bool AresData::CanUseAres = false;
DWORD AresData::AresFunctionOffsetsFinal[AresData::AresFunctionCount];

#ifndef PHOBOS_DLL
#define PHOBOS_DLL "Phobos.dll"
#endif

void AresData::GetGameModulesBaseAddresses()
{
	HANDLE hCurrentProcess = GetCurrentProcess();

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hCurrentProcess));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry{ };
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				DWORD _useless;
				DWORD infoBufferSize = GetFileVersionInfoSize(modEntry.szModule, &_useless);
				if (infoBufferSize == 0)
					continue;
				std::vector<char> infoBuffer(infoBufferSize);
				if (!GetFileVersionInfo(modEntry.szModule, NULL, infoBufferSize, infoBuffer.data()))
					continue;
				LPVOID nameBuffer;
				unsigned int nameBufferSize;
				// 0409 04b0 is language: English (American), codepage: UTF-16
				if (!VerQueryValue(infoBuffer.data(), "\\StringFileInfo\\040904b0\\OriginalFilename", &nameBuffer, &nameBufferSize))
					continue;
				const char* originalModuleName = (const char*)nameBuffer;

				Debug::LogDeferred("Module %s base address : 0x%p.\n", originalModuleName, modEntry.modBaseAddr);
				if (!_strcmpi(originalModuleName, "Ares.dll"))
					AresData::AresBaseAddress = (uintptr_t)modEntry.modBaseAddr;
				else if(!_strcmpi(originalModuleName, PHOBOS_DLL))
					AresData::PhobosBaseAddress = (uintptr_t)modEntry.modBaseAddr;
			}
			while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
}

void AresData::Init()
{
	AresData::GetGameModulesBaseAddresses();

	if (!AresBaseAddress)
	{
		Debug::LogDeferred("[Phobos] Failed to detect Ares. Disabling integration.\n");
		return;
	}
	// find offset of PE header
	const int PEHeaderOffset = *(DWORD*)(AresBaseAddress + 0x3c);
	// find absolute address of PE header
	const DWORD* PEHeaderPtr = (DWORD*)(AresBaseAddress + PEHeaderOffset);
	// read the timedatestamp at 0x8 offset
	const DWORD TimeDateStamp = *(PEHeaderPtr + 2);
	switch (TimeDateStamp)
	{
	case AresTimestampBytes[Version::Ares30]:
		AresVersionId = Version::Ares30;
		CanUseAres = true;
		Debug::LogDeferred("[Phobos] Detected Ares 3.0.\n");
		break;
	case AresTimestampBytes[Version::Ares30p]:
		AresVersionId = Version::Ares30p;
		CanUseAres = true;
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected a version of Ares that is not supported by Phobos. Disabling integration.\n");
		break;
	}

	constexpr const wchar_t* ARES_DLL = L"Ares.dll";
	if (CanUseAres && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, ARES_DLL, &AresDllHmodule))
	{
		for (int i = 0; i < AresData::AresFunctionCount; i++)
			AresData::AresFunctionOffsetsFinal[i] = AresData::AresBaseAddress + AresData::AresFunctionOffsets[i][AresVersionId];
	}
}

void AresData::UnInit()
{
	if (!AresBaseAddress)
		return;

	if (CanUseAres)
		FreeLibrary(AresDllHmodule);
}

template<int idx, typename Tret, typename... TArgs>
struct AresStdcall
{
	using fp_type = Tret(__stdcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresStdcall<idx, void, TArgs...>
{
	using fp_type = void(__stdcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename... TArgs>
struct AresCdecl
{
	using fp_type = Tret(__cdecl*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresCdecl<idx, void, TArgs...>
{
	using fp_type = void(__cdecl*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename... TArgs>
struct AresFastcall
{
	using fp_type = Tret(__fastcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresFastcall<idx, void, TArgs...>
{
	using fp_type = void(__fastcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename TThis, typename... TArgs>
struct AresThiscall
{
	using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);
	decltype(auto) operator()(TThis pThis, TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
	}
};

template<int idx, typename TThis, typename... TArgs>
struct AresThiscall<idx, void, TThis, TArgs...>
{
	using fp_type = void(__fastcall*)(TThis, void*, TArgs...);
	void operator()(TThis pThis, TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
	}
};

bool AresData::ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
{
	return AresStdcall<ConvertTypeToID, bool, TechnoClass*, TechnoTypeClass*>()(pFoot, pConvertTo);
}

void AresData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool ISelect, const bool IgnoreDefenses)
{
	AresStdcall<SpawnSurvivorsID, void, FootClass*, TechnoClass*, bool, bool>()(pThis, pKiller, ISelect, IgnoreDefenses);
}

int AresData::HasFactory(int buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown)
{
	return AresStdcall<HasFactoryID, int, int, HouseClass*, TechnoTypeClass*, bool, bool, bool, bool>()(buffer, pOwner, pType, skipAircraft, requirePower, checkCanBuild, unknown);
}

bool AresData::CanBeBuiltAt(DWORD pTechnoTypeExt, BuildingTypeClass* pBuildingType)
{
	return AresThiscall<CanBeBuiltAtID, bool, DWORD, BuildingTypeClass*>()(pTechnoTypeExt, pBuildingType);
}
