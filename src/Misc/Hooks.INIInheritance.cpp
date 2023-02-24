#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>

#include <vector>

namespace INIInheritance
{
	const char* Empty = "";
	std::vector<char*> SavedEntries;
	std::vector<char*> SavedSections;
	int ReadString(REGISTERS* R, int address);
}

int INIInheritance::ReadString(REGISTERS* R, int address)
{
	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
	//GET_STACK(char*, entryName, STACK_OFFSET(stackOffset, 0x8));
	//GET_STACK(const char*, sectionName, STACK_OFFSET(stackOffset, 0x4));

	char* entryName = INIInheritance::SavedEntries.back();
	char* sectionName = INIInheritance::SavedSections.back();

	auto finalize = [R, buffer, entryName, address](const char* value)
	{
		R->EDI(buffer);
		R->EAX(0);
		R->ECX(value);
		return address;
	};

	if (strncmp(entryName, "$Inherits", 10) == 0)
	{
		auto section = ini->GetSection(sectionName);
		if (!section)
			return finalize(defaultValue);
		for (auto entryNode : section->EntryIndex)
		{
			if (strncmp(entryNode.Data->Key, "$Inherits", 10) == 0)
			{
				return finalize(entryNode.Data->Value);
			}
		}
		return finalize(defaultValue);
	}

	// search for $Inherits entry
	char inheritSectionsString[0x100];
	if (ini->ReadString(sectionName, "$Inherits", INIInheritance::Empty, inheritSectionsString, 0x100) == 0)
		return finalize(defaultValue);

	// for each section in csv, search for entry
	char bufferStart = NULL;
	char* split = strtok(inheritSectionsString, ",");
	do
	{
		if (ini->ReadString(split, entryName, defaultValue, buffer, length) != 0)
			bufferStart = buffer[0];
		else
			buffer[0] = bufferStart;
	}
	while (split = strtok(NULL, ","));

	return finalize(buffer);
}

// INIClass_GetString_DisableAres
DEFINE_PATCH(0x528A10, 0x83, 0xEC, 0x0C, 0x33, 0xC0);
// INIClass_GetKeyName_DisableAres
DEFINE_PATCH(0x526CC0, 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x0C);

DEFINE_HOOK(0x528A18, INIClass_GetString_SaveEntry, 0x6)
{
	GET_STACK(char*, entryName, STACK_OFFSET(0x18, 0x8));
	GET_STACK(char*, sectionName, STACK_OFFSET(0x18, 0x4));
	INIInheritance::SavedEntries.push_back(_strdup(entryName));
	INIInheritance::SavedSections.push_back(_strdup(sectionName));
	return 0;
}

DEFINE_HOOK_AGAIN(0x528BC9, INIClass_GetString_FreeEntry, 0x5)
DEFINE_HOOK(0x528BBE, INIClass_GetString_FreeEntry, 0x5)
{
	char* entry = INIInheritance::SavedEntries.back();
	if (entry)
		free(entry);
	INIInheritance::SavedEntries.pop_back();

	char* section = INIInheritance::SavedSections.back();
	if (section)
		free(section);
	INIInheritance::SavedSections.pop_back();

	return 0;
}

// kind of useless in our case? if there's no section, there's no $Inherits entry either
/*
DEFINE_HOOK(0x528B80, INIClass_GetString_Inheritance_NoSection, 0x8)
{
	return INIInheritance::ReadString(R, 0x528B88);
}
*/

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	return INIInheritance::ReadString(R, 0x528BB6);
}

// piggyback on top of Ares version
/*
DEFINE_HOOK(0x525D23, INIClass_Load_Inherits, 0x5)
{
	LEA_STACK(char*, entry, STACK_OFFSET(0x478, -0x400));
	LEA_STACK(char*, section, STACK_OFFSET(0x478, -0x200));
	GET(char*, value, ESI);
	GET(CCINIClass*, ini, EBP);

	if (strncmp(entry, "$Inherits", 10) != 0)
		return 0;

	// for each name in csv, find and copy section
	char* valueCopy = _strdup(value);
	char* split = strtok(valueCopy, ",");
	do
	{
		auto copiedSection = ini->GetSection(split);
		if (!copiedSection)
			continue;
		for (auto entryNode: copiedSection->EntryIndex)
			ini->WriteString(section, entryNode.Data->Key, entryNode.Data->Value);
	}
	while (split = strtok(NULL, ","));
	free(valueCopy);

	return 0;
}
*/

// a parital attempt at reversing Ares
/*
DEFINE_HOOK(0x528A10, INIClass_GetString_Inheritance, 0x5)
{
	enum { End = 0x528BFA };
	const int stackOffset = 0x0;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, bufferSize, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
	GET_STACK(char*, entryName, STACK_OFFSET(stackOffset, 0x8));
	GET_STACK(const char*, sectionName, STACK_OFFSET(stackOffset, 0x4));

	if (!buffer || bufferSize < 2 || !sectionName || !entryName)
		return End;

	int len = INIInheritance::PhobosGetString(ini, sectionName, entryName, defaultValue, buffer);
	if(!len)
		return 0;

	// trim both sides
	while (*buffer && *buffer++ <= ' ');
	char* back = buffer + strlen(buffer);
	while (*--back && *back <= ' ');
	// set new size and null terminate the result
	bufferSize = std::min(back - buffer, bufferSize - 1);
	*(back + 1) = NULL;

	return End;
}
*/
