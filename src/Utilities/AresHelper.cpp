#include "AresHelper.h"

#include <Phobos.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <CRC.h>

#include <tlhelp32.h>

class TechnoClass;
class TechnoTypeClass;

uintptr_t AresHelper::PhobosBaseAddress = 0x0;
uintptr_t AresHelper::AresBaseAddress = 0x0;
HMODULE AresHelper::AresDllHmodule = nullptr;
AresHelper::Version AresHelper::AresVersion = AresHelper::Version::Unknown;
bool AresHelper::CanUseAres = false;
AresHelper::AresFunctionMap AresHelper::AresFunctionOffsetsFinal;

const AresHelper::AresTimestampMap AresHelper::AresTimestampBytes =
{
	{ 0x5fc37ef6, Version::Ares30 },
	{ 0x61daa114, Version::Ares30p },
};

#ifndef PHOBOS_DLL
#define PHOBOS_DLL "Phobos.dll"
#endif

void AresHelper::GetGameModulesBaseAddresses()
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
					AresBaseAddress = (uintptr_t)modEntry.modBaseAddr;
				else if(!_strcmpi(originalModuleName, PHOBOS_DLL))
					PhobosBaseAddress = (uintptr_t)modEntry.modBaseAddr;
			}
			while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
}

void AresHelper::Init()
{
	GetGameModulesBaseAddresses();

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

	try
	{
		AresVersion = AresTimestampBytes.at(TimeDateStamp);
	}
	catch(std::exception)
	{
		AresVersion = Version::Unknown;
	}
	CanUseAres = AresVersion != Version::Unknown;

	switch (AresVersion)
	{
	case Version::Ares30:
		Debug::LogDeferred("[Phobos] Detected Ares 3.0.\n");
		break;
	case Version::Ares30p:
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected a version of Ares that is not supported by Phobos. Disabling integration.\n");
		break;
	}

	constexpr const wchar_t* ARES_DLL = L"Ares.dll";
	if (CanUseAres && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, ARES_DLL, &AresDllHmodule))
	{
		for(auto x: AresFunctionOffsets.at(AresVersion))
			if (x.second > 0)
				AresFunctionOffsetsFinal[x.first] = AresBaseAddress + x.second;
			else
				AresFunctionOffsetsFinal[x.first] = 0;
	}
}
