#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>
#include <Utilities/TemplateDef.h>

#include <vector>
#include <set>
#include <string>

namespace INIInheritance
{
	int ReadString(REGISTERS* R, int address);
	void PushEntry(REGISTERS* R, int stackOffset);
	void PopEntry();

	template<typename T>
	T ReadTemplate(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(char*, section, 0x4);
		GET_STACK(char*, entry, 0x8);
		GET_STACK(T, defaultValue, 0xC);
		INI_EX exINI(ini);
		T result;

		if (!detail::read<T>(result, exINI, section, entry, false))
			result = defaultValue;
		return result;
	}

	template<typename T>
	T* ReadTemplatePtr(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(T*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(T*, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<T>(*result, exINI, section, entry, false))
			*result = *defaultValue;
		return result;
	}

	// for some reason, WW passes the default locomotor by value
	template<>
	CLSID* ReadTemplatePtr<CLSID>(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(CLSID*, result, 0x4);
		GET_STACK(char*, section, 0x8);
		GET_STACK(char*, entry, 0xC);
		GET_STACK(CLSID, defaultValue, 0x10);
		INI_EX exINI(ini);

		if (!detail::read<CLSID>(*result, exINI, section, entry, false))
			*result = defaultValue;
		return result;
	}

	CCINIClass* LastINIFile = nullptr;
	std::vector<char*> SavedEntries;
	std::vector<char*> SavedSections;
	std::set<std::string> SavedIncludes;
}

int INIInheritance::ReadString(REGISTERS* R, int address)
{
	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));

	char* entryName = INIInheritance::SavedEntries.back();
	char* sectionName = INIInheritance::SavedSections.back();

	auto finalize = [R, buffer, address](const char* value)
	{
		R->EDI(buffer);
		R->EAX(0);
		R->ECX(value);
		return address;
	};

	// if we were looking for $Inherits and failed, no recursion
	if (strncmp(entryName, "$Inherits", 10) == 0)
		return finalize(defaultValue);

	// search for $Inherits entry
	char inheritSectionsString[0x100];
	if (ini->ReadString(sectionName, "$Inherits", NULL, inheritSectionsString, 0x100) == 0)
		return finalize(defaultValue);

	// for each section in csv, search for entry
	char* state = NULL;
	char* split = strtok_s(inheritSectionsString, ",", &state);
	do
	{
		// if we found anything new (not default), we're done
		if (ini->ReadString(split, entryName, NULL, buffer, length) != 0)
			break;
		split = strtok_s(NULL, ",", &state);
	}
	while (split);

	return finalize(buffer[0] ? buffer : defaultValue);
}

void INIInheritance::PushEntry(REGISTERS* R, int stackOffset)
{
	GET_STACK(char*, entryName, STACK_OFFSET(stackOffset, 0x8));
	GET_STACK(char*, sectionName, STACK_OFFSET(stackOffset, 0x4));
	INIInheritance::SavedEntries.push_back(_strdup(entryName));
	INIInheritance::SavedSections.push_back(_strdup(sectionName));
}

void INIInheritance::PopEntry()
{
	char* entry = INIInheritance::SavedEntries.back();
	if (entry)
		free(entry);
	INIInheritance::SavedEntries.pop_back();

	char* section = INIInheritance::SavedSections.back();
	if (section)
		free(section);
	INIInheritance::SavedSections.pop_back();
}

// INIClass_GetString_DisableAres
DEFINE_PATCH(0x528A10, 0x83, 0xEC, 0x0C, 0x33, 0xC0);
// INIClass_GetKeyName_DisableAres
DEFINE_PATCH(0x526CC0, 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x0C);
// INIClass__GetInt__Hack // pop edi, jmp + 6, nop
DEFINE_PATCH(0x5278C6, 0x5F, 0xEB, 0x06, 0x90);
// CCINIClass_ReadCCFile1_DisableAres
DEFINE_PATCH(0x474200, 0x8B, 0xF1, 0x8D, 0x54, 0x24, 0x0C)
// CCINIClass_ReadCCFile2_DisableAres
DEFINE_PATCH(0x474314, 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00)

DEFINE_HOOK(0x528A18, INIClass_GetString_SaveEntry, 0x6)
{
	INIInheritance::PushEntry(R, 0x18);
	return 0;
}

DEFINE_HOOK_AGAIN(0x528BC9, INIClass_GetString_FreeEntry, 0x5)
DEFINE_HOOK(0x528BBE, INIClass_GetString_FreeEntry, 0x5)
{
	INIInheritance::PopEntry();
	return 0;
}

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	return INIInheritance::ReadString(R, 0x528BB6);
}

DEFINE_HOOK(0x474230, CCINIClass_Load_Inheritance, 0x5)
{
	GET(CCINIClass*, ini, ESI);

	bool isSameFile = ini == INIInheritance::LastINIFile;
	if (!isSameFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::SavedIncludes.clear();
	}

	auto section = ini->GetSection("$Include");
	if (!section)
		return 0;

	for (auto node : section->EntryIndex)
	{
		if (!node.Data || !node.Data->Value || !*node.Data->Value)
			continue;
		auto filename = std::string(node.Data->Value);
		if (INIInheritance::SavedIncludes.contains(filename))
			continue;
		INIInheritance::SavedIncludes.insert(filename);
		auto file = GameCreate<CCFileClass>(node.Data->Value);
		if (file->Exists())
			INIInheritance::LastINIFile->ReadCCFile(file, false, false);
		GameDelete(file);
	}

	return 0;
}

DEFINE_HOOK(0x5276D0, INIClass_ReadInt_Overwrite, 0x5)
{
	int value = INIInheritance::ReadTemplate<int>(R);
	R->EAX(value);
	return 0x527838;
}

DEFINE_HOOK(0x5295F0, INIClass_ReadBool_Overwrite, 0x5)
{
	bool value = INIInheritance::ReadTemplate<bool>(R);
	R->EAX(value);
	return 0x5297A3;
}

DEFINE_HOOK(0x5283D0, INIClass_ReadDouble_Overwrite, 0x5)
{
	double value = INIInheritance::ReadTemplate<double>(R);
	_asm { fld value }
	return 0x52859F;
}

DEFINE_HOOK(0x529880, INIClass_ReadPoint2D_Overwrite, 0x5)
{
	auto value = INIInheritance::ReadTemplatePtr<Point2D>(R);
	R->EAX(value);
	return 0x52859F;
}

DEFINE_HOOK(0x529CA0, INIClass_ReadPoint3D_Overwrite, 0x5)
{
	auto value = INIInheritance::ReadTemplatePtr<CoordStruct>(R);
	R->EAX(value);
	return 0x529E63;
}

DEFINE_HOOK(0x527920, INIClass_ReadGUID_Overwrite, 0x5) // locomotor
{
	auto value = INIInheritance::ReadTemplatePtr<CLSID>(R);
	R->EAX(value);
	return 0x527B43;
}

// Fix issue with TilesInSet caused by incorrect vanilla INIs and the fixed parser returning correct default value (-1) instead of 0 for existing non-integer values
int __fastcall IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper(INIClass* pThis, void* _, const char* pSection, const char* pKey, int defaultValue)
{
	if (pThis->Exists(pSection, pKey))
		return pThis->ReadInteger(pSection, pKey, 0);

	return defaultValue;
}

DEFINE_JUMP(CALL, 0x545FD4, GET_OFFSET(IsometricTileTypeClass_ReadINI_TilesInSet_Wrapper));
