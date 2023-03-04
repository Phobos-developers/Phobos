#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>

#include <vector>
#include <set>
#include <string>

namespace INIInheritance
{
	int ReadInt(REGISTERS* R, int address);
	int ReadString(REGISTERS* R, int address);
	void PushEntry(REGISTERS* R, int stackOffset);
	void PopEntry();

	CCINIClass* LastINIFile = nullptr;
	std::vector<char*> SavedEntries;
	std::vector<char*> SavedSections;
	std::set<std::string> SavedIncludes;
}

int INIInheritance::ReadInt(REGISTERS* R, int address)
{
	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBX);
	GET_STACK(int, defaultValue, STACK_OFFSET(stackOffset, 0xC));

	char* entryName = INIInheritance::SavedEntries.back();
	char* sectionName = INIInheritance::SavedSections.back();

	auto finalize = [R, address](int value)
	{
		R->Stack<int>(STACK_OFFSET(stackOffset, 0xC), value);
		return address;
	};

	// search for $Inherits entry
	char inheritSectionsString[0x100];
	if (ini->ReadString(sectionName, "$Inherits", NULL, inheritSectionsString, 0x100) == 0)
		return finalize(defaultValue);

	// for each section in csv, search for entry
	int buffer = MAXINT;
	char* state = NULL;
	char* split = strtok_s(inheritSectionsString, ",", &state);
	do
	{
		// if we found anything new (not default), we're done
		buffer = ini->ReadInteger(split, entryName, MAXINT);
		if (buffer != MAXINT)
			break;
	}
	while (split = strtok_s(NULL, ",", &state));

	return finalize(buffer != MAXINT ? buffer : defaultValue);
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
	}
	while (split = strtok_s(NULL, ",", &state));

	return finalize(buffer[0] ? buffer : defaultValue);
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

DEFINE_HOOK(0x528A18, INIClass_GetString_SaveEntry, 0x6)
{
	INIInheritance::PushEntry(R, 0x18);
	return 0;
}

DEFINE_HOOK(0x5276D7, INIClass_GetInt_SaveEntry, 0x6)
{
	INIInheritance::PushEntry(R, 0x14);
	return 0;
}

DEFINE_HOOK_AGAIN(0x528BC9, INIClass_GetString_FreeEntry, 0x5)
DEFINE_HOOK(0x528BBE, INIClass_GetString_FreeEntry, 0x5)
{
	INIInheritance::PopEntry();
	return 0;
}

DEFINE_HOOK_AGAIN(0x52782F, INIClass_GetInt_FreeEntry, 0x5)
DEFINE_HOOK_AGAIN(0x5278A9, INIClass_GetInt_FreeEntry, 0x7)
DEFINE_HOOK(0x527866, INIClass_GetInt_FreeEntry, 0x7)
{
	INIInheritance::PopEntry();
	return 0;
}

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	return INIInheritance::ReadString(R, 0x528BB6);
}

DEFINE_HOOK(0x5278CA, INIClass_GetInt_Inheritance_NoEntry, 0x5)
{
	int r = INIInheritance::ReadInt(R, 0);
	INIInheritance::PopEntry();
	return r;
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
