#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>
#include <Utilities/TemplateDef.h>

#include <CRC.h>
#include <CRT.h>

#include <vector>
#include <set>
#include <string>
#include <unordered_map>

namespace INIInheritance
{
	int ReadString(REGISTERS* R, int address);
	int ReadStringMagik(CCINIClass* ini, int sectionCRC, int entryCRC, char* defaultValue, char* buffer, int length);

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
	std::set<std::string> SavedIncludes;
	std::unordered_map<int, std::string> Inherits;
}

int INIInheritance::ReadStringMagik(CCINIClass* ini, int sectionCRC, int entryCRC, char* defaultValue, char* buffer, int length)
{
	INIClass::INISection* pSection;
	char* result;

	auto finalize = [buffer, length](char* result)
	{
		if (!result)
		{
			*buffer = NULL;
			return 0;
		}
		strncpy(buffer, result, length);
		buffer[length - 1] = NULL;
		CRT::strtrim(buffer);

		return (int)strlen(buffer);
	};

	if (!buffer || length < 2)
		return 0;

	// if we have the current section preloaded, great, if not, search for the section
	if (section == ini->CurrentSectionName)
	{
		pSection = ini->CurrentSection;
	}
	else
	{
		/*********** skipped section name crc **********/

		if (ini->SectionIndex.IndexCount == 0)
		{
			ini->CurrentSection = NULL;
			ini->CurrentSectionName = NULL;
			return finalize(defaultValue);
		}

		if (!ini->SectionIndex.Archive || ini->SectionIndex.Archive->ID != sectionCRC)
		{
			pSection = ini->SectionIndex.FetchIndex(sectionCRC);
			if (!pSection)
			{
				ini->CurrentSection = NULL;
				ini->CurrentSectionName = NULL;
				return finalize(defaultValue);
			}
			ini->SectionIndex.Archive = pSection;
		}

		if (ini->SectionIndex.IsPresent(sectionCRC))
		{
			pSection = ini->SectionIndex.Archive->Data;
		}
		else
		{
			// ???
			pSection = ...;
		}
		if (!pSection)
		{
			ini->CurrentSection = NULL;
			ini->CurrentSectionName = NULL;
			return finalize(defaultValue);
		}
		ini->CurrentSection = pSection;
		ini->CurrentSectionName = section;
	}

	if (!pSection)
		return finalize(defaultValue);

	/*********** skipped entry name crc **********/

	auto entryIndex = pSection->EntryIndex;
	if (!pSection->EntryIndex.IndexCount)
		return finalize(defaultValue);

	if (!entryIndex.Archive || entryIndex.Archive->ID != entryCRC)
	{
		auto v15 = entryIndex.FetchIndex(entryCRC);
		if (!v15)
			return finalize(defaultValue);
		entryIndex.Archive = v15;
	}
	if (entryIndex.IndexCount == 0)
	{
		// goto 30
	}
	if (entryIndex.Archive && entryIndex.Archive->ID == entryCRC)
	{
		// goto 18
	}
	auto v17 = entryIndex.FetchIndex(entryCRC);
	if (!v17)
	{
		// label 30
		v18 = ...;
		// goto 31
	}
	entryIndex.Archive = v17;
	// label 18
	v18 = entryIndex.Archive->Data;
	// label 31
	auto v24 = *v18;
	if (!v24)
		return finalize(defaultValue);

	result = v24->Value;

	return finalize(result ? result : defaultValue);
}

int INIInheritance::ReadString(REGISTERS* R, int address)
{
	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
	GET_STACK(int, entryCRC, STACK_OFFSET(stackOffset, 0x8));
	GET_STACK(int, sectionCRC, STACK_OFFSET(stackOffset, 0x4));

	auto finalize = [R, buffer, address](const char* value)
	{
		R->EDI(buffer);
		R->EAX(0);
		R->ECX(value);
		return address;
	};

	auto crc = CRCEngine();

	// if we were looking for $Inherits and failed, no recursion
	if (entryCRC == crc("$Inherits", 10))
		return finalize(defaultValue);

	// read $Inherits entry only once per section
	auto it = INIInheritance::Inherits.find(sectionCRC); // TODO check if you can pass hash directly?
	if (it == INIInheritance::Inherits.end())
	{
		// read $Inherits entry
		char stringBuffer[0x100];
		int retval = INIInheritance::ReadStringMagik(ini, sectionCRC, crc("$Inherits", 10), NULL, stringBuffer, 0x100);
		INIInheritance::Inherits.emplace(sectionCRC, std::string(stringBuffer));
		if (retval == 0)
			return finalize(defaultValue);
	}
	else
	{
		if (it->second[0] == NULL)
		{
			return finalize(defaultValue);
		}
	}

	// for each section in csv, search for entry
	char* state = NULL;
	char* inherits = _strdup(it->second.c_str());
	char* split = strtok_s(inherits, ",", &state);
	do
	{
		// if we found anything new (not default), we're done
		if (INIInheritance::ReadStringMagik(ini, crc(split, strlen(split)), entryCRC, NULL, buffer, length) != 0)
			break;
		split = strtok_s(NULL, ",", &state);
	}
	while (split);
	free(inherits);

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
		INIInheritance::Inherits.clear();
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
