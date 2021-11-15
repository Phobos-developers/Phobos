#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <CCINIClass.h>
#include <RulesClass.h>
#include <MixFileClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>


DEFINE_HOOK(0x524734, InfantryTypeClass_ReadINI, 0x6)
{
	if (!Phobos::Config::NoArtImageSwap)
	{
		GET(InfantryTypeClass*, infantryType, ESI);
		char tempBuffer[0x19];
		if (CCINIClass::INI_Art->ReadString(infantryType->ImageFile, "Image", NULL, tempBuffer, 0x19) != 0)
		{
			Debug::Log("[Phobos] Replacing image for %s with %s\n", infantryType->ImageFile, tempBuffer);
			char filename[260];
			_makepath(filename, 0, 0, tempBuffer, ".SHP");
			infantryType->Image = GameCreate<SHPReference>(filename);
		}
	}
	
	return 0;
}


DEFINE_HOOK(0x747B49, VehicleTypeClass_ReadINI, 0x6)
{
	if (!Phobos::Config::NoArtImageSwap)
	{
		GET(UnitTypeClass*, unitType, EDI);
		char tempBuffer[0x19];
		char savedBufffer[0x19];
		if (CCINIClass::INI_Art->ReadString(unitType->ImageFile, "Image", NULL, tempBuffer, 0x19) != 0)
		{
			Debug::Log("[Phobos] Replacing image for %s with %s\n", unitType->ImageFile, tempBuffer);
			if (unitType->Voxel)
			{
				strcpy(savedBufffer, unitType->ImageFile);
				strcpy(unitType->ImageFile, tempBuffer);
				unitType->LoadVoxel();
				strcpy(unitType->ImageFile, savedBufffer);
			}
			/*
			else
			{
				char filename[260];
				_makepath(filename, 0, 0, tempBuffer, ".SHP");
				unitType->Image = GameCreate<SHPReference>(filename);
			}
			*/
		}
	}

	return 0;
}

DEFINE_HOOK(0x41CD54, AircraftTypeClass_ReadINI, 0x6)
{
	if (!Phobos::Config::NoArtImageSwap)
	{
		GET(AircraftTypeClass*, aircraftType, ESI);
		char tempBuffer[0x19];
		char savedBufffer[0x19];
		if (CCINIClass::INI_Art->ReadString(aircraftType->ImageFile, "Image", NULL, tempBuffer, 0x19) != 0)
		{
			if (aircraftType->Voxel)
			{
				Debug::Log("[Phobos] Replacing image for %s with %s\n", aircraftType->ImageFile, tempBuffer);
				strcpy(savedBufffer, aircraftType->ImageFile);
				strcpy(aircraftType->ImageFile, tempBuffer);
				aircraftType->LoadVoxel();
				strcpy(aircraftType->ImageFile, savedBufffer);
			}
		}
	}

	return 0;
}
