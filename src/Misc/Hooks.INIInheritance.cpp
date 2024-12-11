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
	const constexpr int inheritsCRC = -1871638965; // CRCEngine()("$Inherits", 9)

	int ReadString(CCINIClass* ini, int sectionCRC, int entryCRC, const char* defaultValue, char* buffer, int length, bool useCurrentSection, bool skipCheck);
	int ReadStringUseCRCActual(CCINIClass* ini, int sectionCRC, int entryCRC, const char* defaultValue, char* buffer, int length, bool useCurrentSection);

	template<typename T>
	T ReadTemplate(REGISTERS* R)
	{
		GET(CCINIClass*, ini, ECX);
		GET_STACK(char*, section, 0x4);
		GET_STACK(char*, entry, 0x8);
		GET_STACK(T, defaultValue, 0xC);
		INI_EX exINI(ini);
		T result;

		if (!detail::read<T>(result, exINI, section, entry))
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

		if (!detail::read<T>(*result, exINI, section, entry))
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

		if (!detail::read<CLSID>(*result, exINI, section, entry))
			*result = defaultValue;
		return result;
	}

	// passthrough instead of std::hash, because our keys are already unique CRCs
	struct Passthrough
	{
		std::size_t operator()(int const& x) const noexcept
		{
			return x;
		}
	};

	CCINIClass* LastINIFile = nullptr;
	std::set<std::string> SavedIncludes;
	std::unordered_map<int, std::string, Passthrough> Inherits;
}

int INIInheritance::ReadStringUseCRCActual(CCINIClass* ini, int sectionCRC, int entryCRC, const char* defaultValue, char* buffer, int length, bool useCurrentSection)
{
	INIClass::INISection* pSection;

	const auto finalize = [buffer, length](const char* result)
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

	if (useCurrentSection)
		pSection = ini->CurrentSection;
	else
		pSection = ini->SectionIndex.IsPresent(sectionCRC) ? ini->SectionIndex.Archive->Data : nullptr;

	if (!pSection)
		return finalize(defaultValue);

	const auto pEntry = pSection->EntryIndex.IsPresent(entryCRC) ? pSection->EntryIndex.Archive->Data : nullptr;
	if (!pEntry)
		return finalize(defaultValue);

	return finalize(pEntry->Value ? pEntry->Value : defaultValue);
}

int INIInheritance::ReadString(CCINIClass* ini, int sectionCRC, int entryCRC, const char* defaultValue,
	char* buffer, int length, bool useCurrentSection, bool skipCheck = false)
{
	const auto finalize = [buffer, length](const char* result)
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

	int resultLen = 0;

	if (!skipCheck)
	{
		// check if this section has the actual entry, if yes, then we're done
		resultLen = INIInheritance::ReadStringUseCRCActual(ini, sectionCRC, entryCRC, NULL, buffer, length, useCurrentSection);
		if (resultLen != 0)
			return buffer[0] ? resultLen : 0;
	}

	// if we were looking for $Inherits and failed, no recursion
	if (entryCRC == inheritsCRC)
		return 0;

	// read $Inherits entry only once per section
	const auto it = INIInheritance::Inherits.find(sectionCRC);
	char* inherits;
	if (it == INIInheritance::Inherits.end())
	{
		char stringBuffer[0x100];
		// if there's no saved $Inherits entry for this section, read now
		resultLen = INIInheritance::ReadStringUseCRCActual(ini, sectionCRC, inheritsCRC, NULL, stringBuffer, 0x100, useCurrentSection);
		INIInheritance::Inherits.emplace(sectionCRC, std::string(stringBuffer));
		// if we failed to find $Inherits, stop
		if (resultLen == 0)
			return finalize(defaultValue);
		inherits = stringBuffer;
	}
	else
	{
		// use the saved $Inherits entry
		if (it->second.empty())
			return finalize(defaultValue);
		// strdup because strtok edits the string
		inherits = _strdup(it->second.c_str());
	}

	// for each section in csv, search for entry
	char* state = NULL;
	char* split = strtok_s(inherits, ",", &state);
	do
	{
		const int splitsCRC = CRCEngine()(split, strlen(split));

		// if we've found anything, we're done
		resultLen = INIInheritance::ReadString(ini, splitsCRC, entryCRC, defaultValue, buffer, length, false);
		if (resultLen != 0)
			break;
		split = strtok_s(NULL, ",", &state);
	}
	while (split);
	free(inherits);

	return resultLen != 0 ? (buffer[0] ? resultLen : 0) : finalize(defaultValue);
}

// INIClass__GetInt__Hack // pop edi, jmp + 6, nop
DEFINE_PATCH(0x5278C6, 0x5F, 0xEB, 0x06, 0x90);

DEFINE_HOOK(0x528BAC, INIClass_GetString_Inheritance_NoEntry, 0xA)
{
	const int stackOffset = 0x1C;
	GET(CCINIClass*, ini, EBP);
	GET_STACK(int, length, STACK_OFFSET(stackOffset, 0x14));
	GET_STACK(char*, buffer, STACK_OFFSET(stackOffset, 0x10));
	GET_STACK(const char*, defaultValue, STACK_OFFSET(stackOffset, 0xC));
	GET_STACK(int, entryCRC, STACK_OFFSET(stackOffset, 0x8));
	GET_STACK(int, sectionCRC, STACK_OFFSET(stackOffset, 0x4));

	// if we're in a different CCINIClass now, clear old data
	if (ini != INIInheritance::LastINIFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::Inherits.clear();
	}

	INIInheritance::ReadString(ini, sectionCRC, entryCRC, defaultValue, buffer, length, true, true);

	R->EDI(buffer);
	R->EAX(0);
	R->ECX(buffer[0] ? buffer : defaultValue);

	return 0x528BB6;
}

DEFINE_HOOK(0x474230, CCINIClass_Load_Inheritance, 0x5)
{
	GET(CCINIClass*, ini, ESI);

	// if we're in a different CCINIClass now, clear old data
	if (ini != INIInheritance::LastINIFile)
	{
		INIInheritance::LastINIFile = ini;
		INIInheritance::SavedIncludes.clear();
	}

	const auto section = ini->GetSection("$Include");
	if (!section)
		return 0;

	// include path in the list
	for (const auto& node : section->EntryIndex)
	{
		if (!node.Data || !node.Data->Value || !*node.Data->Value)
			continue;

		// only include each file once
		const auto filename = std::string(node.Data->Value);
		if (INIInheritance::SavedIncludes.contains(filename))
			continue;
		INIInheritance::SavedIncludes.insert(filename);

		// merge included file into the current CCINIClass
		CCFileClass file (node.Data->Value);
		if (file.Exists())
			INIInheritance::LastINIFile->ReadCCFile(&file, false, false);
	}

	return 0;
}

DEFINE_HOOK(0x5276D0, INIClass_ReadInt_Overwrite, 0x5)
{
	const int value = INIInheritance::ReadTemplate<int>(R);
	R->EAX(value);
	return 0x527838;
}

DEFINE_HOOK(0x5295F0, INIClass_ReadBool_Overwrite, 0x5)
{
	const bool value = INIInheritance::ReadTemplate<bool>(R);
	R->EAX(value);
	return 0x5297A3;
}

DEFINE_HOOK(0x5283D0, INIClass_ReadDouble_Overwrite, 0x5)
{
	const double value = INIInheritance::ReadTemplate<double>(R);
	_asm { fld value }
	return 0x52859F;
}

DEFINE_HOOK(0x529880, INIClass_ReadPoint2D_Overwrite, 0x5)
{
	const auto value = INIInheritance::ReadTemplatePtr<Point2D>(R);
	R->EAX(value);
	return 0x52859F;
}

DEFINE_HOOK(0x529CA0, INIClass_ReadPoint3D_Overwrite, 0x5)
{
	const auto value = INIInheritance::ReadTemplatePtr<CoordStruct>(R);
	R->EAX(value);
	return 0x529E63;
}

DEFINE_HOOK(0x527920, INIClass_ReadGUID_Overwrite, 0x5) // locomotor
{
	const auto value = INIInheritance::ReadTemplatePtr<CLSID>(R);
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
