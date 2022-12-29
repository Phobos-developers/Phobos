#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>
#include <CRC.h>

#include <tlhelp32.h>

class TechnoClass;
class TechnoTypeClass;

uintptr_t AresData::PhobosBaseAddress = 0x0;
uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;
int AresData::AresVersionId = AresData::Version::Unknown;
bool AresData::CanUseAres = false;
DWORD AresData::AresFunctionOffsetsFinal[AresData::AresFunctionCount];

uintptr_t GetModuleBaseAddress(const char* modName)
{
	HANDLE hCurrentProcess = GetCurrentProcess();
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hCurrentProcess));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry { };
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_strcmpi(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			}
			while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);

	return modBaseAddr;
}

void AresData::Init()
{
	constexpr const char* ARES_DLL_S = "Ares.dll";
	AresBaseAddress = GetModuleBaseAddress(ARES_DLL_S);
	constexpr const char* PHOBOS_DLL_S = "Phobos.dll";
	PhobosBaseAddress = GetModuleBaseAddress(PHOBOS_DLL_S);
	Debug::LogDeferred("[Phobos] Phobos base address: 0x%X.\n", PhobosBaseAddress);

	if (!AresBaseAddress)
	{
		Debug::LogDeferred("[Phobos] Failed to detect Ares. Disabling integration.\n");
		return;
	}
	else
	{
		Debug::LogDeferred("[Phobos] Ares base address: 0x%X.\n", AresBaseAddress);
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
			AresData::AresFunctionOffsetsFinal[i] = AresData::AresBaseAddress + AresData::AresFunctionOffsets[i * AresData::AresVersionCount + AresVersionId];
	}
}

void AresData::UnInit()
{
	if (!AresBaseAddress)
		return;

	if (CanUseAres)
		FreeLibrary(AresDllHmodule);
}

bool AresData::ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
{
	return AresStdcall<ConvertTypeToID, bool, TechnoClass*, TechnoTypeClass*>()(pFoot, pConvertTo);
}

void AresData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool ISelect, const bool IgnoreDefenses)
{
	AresStdcall<SpawnSurvivorsID, void, FootClass*, TechnoClass*, bool, bool>()(pThis, pKiller, ISelect, IgnoreDefenses);
}