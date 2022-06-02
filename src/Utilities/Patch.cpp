#include "Patch.h"
#include "Macro.h"
#include <Phobos.h>

int GetSection(char* sectionName, void** pVirtualAddress)
{
	char buf[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, buf, sizeof(buf));

	auto hInstance = Phobos::hInstance;

	auto pHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(((PIMAGE_DOS_HEADER)hInstance)->e_lfanew + (long)hInstance);

	for (int i = 0; i < pHeader->FileHeader.NumberOfSections; i++)	{
		auto sct_hdr = IMAGE_FIRST_SECTION(pHeader) + i;

		if (strncmp(sectionName, (char*)sct_hdr->Name, 8) == 0) {
			*pVirtualAddress = (void*)((DWORD)hInstance + sct_hdr->VirtualAddress);
			return sct_hdr->Misc.VirtualSize;
		}
	}
	return 0;
}

void Patch::Apply()
{
	void* buffer;
	const int len = GetSection(PATCH_SECTION_NAME, &buffer);
	int offset = 0;

	while(offset < len)
	{
		const auto pItem = (patch_decl*)((DWORD)buffer + offset);
		if (pItem->offset == 0)
			return;

		Apply(pItem);
		offset += sizeof(patch_decl);
	}
}

void Patch::Apply(const patch_decl* pItem)
{
	void* pAddress = (void*)pItem->offset;

	DWORD protect_flag;
	VirtualProtect(pAddress, pItem->size, PAGE_EXECUTE_READWRITE, &protect_flag);
	memcpy(pAddress, pItem->pData, pItem->size);
	VirtualProtect(pAddress, pItem->size, protect_flag, NULL);
}
