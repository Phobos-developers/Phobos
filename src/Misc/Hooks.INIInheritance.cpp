#include <Utilities/Debug.h>
#include <Helpers/Macro.h>
#include <CCINIClass.h>

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
