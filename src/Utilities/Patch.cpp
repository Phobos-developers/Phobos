#include "Patch.h"
#include "Macro.h"
#include <Phobos.h>

int GetSection(const char* sectionName, void** pVirtualAddress)
{
	auto hInstance = Phobos::hInstance;
	auto pHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(((PIMAGE_DOS_HEADER)hInstance)->e_lfanew + (long)hInstance);

	for (int i = 0; i < pHeader->FileHeader.NumberOfSections; i++)
	{
		auto sct_hdr = IMAGE_FIRST_SECTION(pHeader) + i;

		if (strncmp(sectionName, (char*)sct_hdr->Name, 8) == 0)
		{
			*pVirtualAddress = (void*)((DWORD)hInstance + sct_hdr->VirtualAddress);
			return sct_hdr->Misc.VirtualSize;
		}
	}
	return 0;
}

void Patch::ApplyStatic()
{
	void* buffer;
	const int len = GetSection(PATCH_SECTION_NAME, &buffer);

	for (int offset = 0; offset < len; offset += sizeof(Patch))
	{
		const auto pPatch = (Patch*)((DWORD)buffer + offset);
		if (pPatch->offset == 0)
			return;

		pPatch->Apply();
	}
}

void Patch::Apply()
{
	void* pAddress = (void*)this->offset;

	DWORD protect_flag;
	VirtualProtect(pAddress, this->size, PAGE_EXECUTE_READWRITE, &protect_flag);
	memcpy(pAddress, this->pData, this->size);
	VirtualProtect(pAddress, this->size, protect_flag, NULL);
}

void Patch::Apply_LJMP(DWORD offset, DWORD pointer)
{
	const _LJMP data(offset, pointer);
	Patch patch = { offset, sizeof(data), (byte*)&data };
	patch.Apply();
}

void Patch::Apply_CALL(DWORD offset, DWORD pointer)
{
	const _CALL data(offset, pointer);
	Patch patch = { offset, sizeof(data), (byte*)&data };
	patch.Apply();
}

void Patch::Apply_CALL6(DWORD offset, DWORD pointer)
{
	const _CALL6 data(offset, pointer);
	Patch patch = { offset, sizeof(data), (byte*)&data };
	patch.Apply();
}
