#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <CCINIClass.h>
#include <RulesClass.h>
#include <MixFileClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>


DEFINE_HOOK(0x524734, InfantryTypeClass_ReadINI, 0x6)
{
	GET(InfantryTypeClass*, infantryType, ESI);
	char tempBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(infantryType->ImageFile, "Image", NULL, tempBuffer, 0x19) != 0)
	{
		Debug::Log("[Phobos] Replacing image for %s with %s\n", infantryType->ImageFile, tempBuffer);
		char filename[260];
		_makepath(filename, 0, 0, tempBuffer, ".SHP");
		infantryType->Image = (SHPStruct*)MixFileClass::Retrieve(filename, 0);
	}	
	
	return 0;
}

/*
DEFINE_HOOK(0x747B49, VehicleTypeClass_ReadINI, 0x6)
{
	GET(UnitTypeClass*, unitType, EDI);
	char tempBuffer[0x19];
	if (CCINIClass::INI_Art->ReadString(unitType->ImageFile, "Image", NULL, tempBuffer, 0x19) != 0)
	{
		char filename[260];
		if (!unitType->Voxel)
		{
			_makepath(filename, 0, 0, tempBuffer, ".SHP");
			unitType->Image = (SHPStruct*)MixFileClass::Retrieve(filename, 0);
		}
		else
		{
			_makepath(filename, 0, 0, tempBuffer, ".VXL");
			unitType->Image = (SHPStruct*)MixFileClass::Retrieve(filename, 0);
		}
	}

	return 0;
}
*/
