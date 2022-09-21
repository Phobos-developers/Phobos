#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>

//#include <filesystem>
#include <tlhelp32.h>

class TechnoClass;
class TechnoTypeClass;

constexpr const wchar_t* ARES_DLL = L"Ares.dll";
constexpr const char* ARES_DLL_S = "Ares.dll";

uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;

int AresData::AresVersionId = -1;
bool AresData::CanUseAres = false;

// first is 3.0, second 3.0p1
const DWORD AresData::AresFunctionOffsets[2] = {
	0x43650, 0x44130,	// HandleConvert
};

// the absolute address calculated on runtime
DWORD AresData::AresFunctionOffestsFinal[1];

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
	AresData::AresBaseAddress = GetModuleBaseAddress(ARES_DLL_S);

	if (!AresData::AresBaseAddress)
		return;

	DWORD newCRC = 0;

	switch (newCRC)
	{
		case AresData::Ares30Hash:
			AresVersionId = 0;
			CanUseAres = true;
			break;
		case AresData::Ares30p1Hash:
			AresVersionId = 1;
			CanUseAres = true;
			break;
		default:
			Debug::Log("Detected a version of Ares that is not supported by Phobos. Disabling integration.");
			break;
	}

	if (CanUseAres && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, ARES_DLL, &AresDllHmodule))
	{
		AresData::AresFunctionOffestsFinal[0] = AresData::AresBaseAddress + AresData::AresFunctionOffsets[0 + AresVersionId];
	}
}

void AresData::UnInit()
{
	if (!AresData::AresBaseAddress)
		return;

	if (CanUseAres)
		FreeLibrary(AresDllHmodule);
}

void __stdcall AresData::CallHandleConvert(TechnoClass* pTechno, TechnoTypeClass* pConvertTo)
{
	if (!CanUseAres)
		return;
	JMP_STD(AresData::AresFunctionOffestsFinal[0]);
}
