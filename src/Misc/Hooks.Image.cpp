#include <Helpers/Macro.h>
#include <CCINIClass.h>
#include <RulesClass.h>
#include <InfantryTypeClass.h>

/*
DEFINE_HOOK(0x52CA02, Init_Game, 0x6)
{
	
	GET(CCINIClass*, pINI, EAX);

	RulesClass::Instance->Read_InfantryTypes(pINI);

	for (int i = 0; i < InfantryTypeClass::Array->Count; ++i)
	{
		InfantryTypeClass* infantryType = InfantryTypeClass::Array->Items[i];
		infantryType->LoadFromINI(pINI);
		CCINIClass::INI_Art->ReadString(infantryType->ID, "Image", infantryType->ImageFile, infantryType->ImageFile, 0x19);
		_snprintf_s(infantryType->ImageFile, sizeof("COW"), "COW");
		if (infantryType->ID == 0)
		{
			return 1;
		}
	}

	return 0;
}
*/

DEFINE_HOOK(0x5F9629, ObjectTypeClass_ReadINI, 0x5)
{
	
	GET(ObjectTypeClass*, objectType, EBX);
	if (objectType->WhatAmI() == AbstractType::Infantry)
	{
		CCINIClass::INI_Art->ReadString(objectType->ID, "Image", objectType->ImageFile, objectType->ImageFile, 0x19);
		if (objectType->ID == 0)
		{
			return 1;
		}
	}
	
	return 0;
}
