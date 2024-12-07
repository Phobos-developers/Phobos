#include "AresHelper.h"
#include "AresFunctions.h"
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

const AresHelper::AresTimestampMap AresHelper::AresTimestampBytes =
{
	{ 0x5fc37ef6, Version::Ares30 },
	{ 0x61daa114, Version::Ares30p },
};

#ifndef PHOBOS_DLL
#define PHOBOS_DLL "Phobos.dll"
#endif

bool module_has_syhks00(HMODULE hModule)
{
	auto* baseAddress = reinterpret_cast<unsigned char*>(hModule);

	auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(baseAddress);
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	auto* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(baseAddress + dosHeader->e_lfanew);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	auto* sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++sectionHeader)
	{
		if (strncmp(reinterpret_cast<const char*>(sectionHeader->Name), ".syhks00", IMAGE_SIZEOF_SHORT_NAME) == 0)
			return true;
	}

	return false;
}

void AresHelper::GetGameModulesBaseAddresses()
{
	HANDLE hCurrentProcess = GetCurrentProcess();
	std::vector<std::pair<std::string, BYTE*>> syringables;
	std::vector<std::pair<std::string, BYTE*>> others;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hCurrentProcess));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry { };
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
				if (!_strcmpi(originalModuleName, "Sun.exe"))
					continue;
				if (module_has_syhks00(modEntry.hModule))
				{
					if (!_strcmpi(originalModuleName, "Ares.dll"))
						AresBaseAddress = (uintptr_t)modEntry.modBaseAddr;
					else if (!_strcmpi(originalModuleName, PHOBOS_DLL))
						PhobosBaseAddress = (uintptr_t)modEntry.modBaseAddr;
					syringables.emplace_back(originalModuleName, modEntry.modBaseAddr);
				}
				else
				{
					others.emplace_back(originalModuleName, modEntry.modBaseAddr);
				}
			}
			while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
#ifndef USING_MULTIFINITE_SYRINGE
	Debug::LogDeferred("Modules presumably injected by Syringe:\n");
	for (auto const& [modName, modBaseAddr] : syringables)
		Debug::LogDeferred("Module %s  base address : 0x%p.\n", modName.c_str(), modBaseAddr);
	Debug::LogDeferred("Other modules:\n");
	for (auto const& [modName, modBaseAddr] : others)
		Debug::LogDeferred("Module %s  base address : 0x%p.\n", modName.c_str(), modBaseAddr);
#endif
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
	catch (std::exception)
	{
		AresVersion = Version::Unknown;
	}
	CanUseAres = AresVersion != Version::Unknown;

	switch (AresVersion)
	{
	case Version::Ares30:
		Debug::LogDeferred("[Phobos] Detected Ares 3.0.\n");
		AresFunctions::InitAres3_0();
		break;
	case Version::Ares30p:
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		AresFunctions::InitAres3_0p1();
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected a version of Ares that is not supported by Phobos. Disabling integration.\n");
		break;
	}
}
