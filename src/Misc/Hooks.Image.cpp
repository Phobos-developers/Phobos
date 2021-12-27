#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

#include <CCINIClass.h>
#include <RulesClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>

DEFINE_HOOK(0x524734, InfantryTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(InfantryTypeClass*, infantryType, ESI);
		char nameBuffer[0x19];

		if (CCINIClass::INI_Art->ReadString(infantryType->ImageFile, "Image", 0, nameBuffer, 0x19) != 0)
		{
			Debug::Log("[Phobos] Replacing image for %s with %s.\n", infantryType->ImageFile, nameBuffer);
			char filename[260];
			_makepath_s(filename, 0, 0, nameBuffer, ".SHP");
			infantryType->Image = GameCreate<SHPReference>(filename);
		}
	}
	
	return 0;
}

DEFINE_HOOK(0x747B49, VehicleTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(UnitTypeClass*, unitType, EDI);
		char nameBuffer[0x19];

		if (CCINIClass::INI_Art->ReadString(unitType->ImageFile, "Image", 0, nameBuffer, 0x19) != 0)
		{
			Debug::Log("[Phobos] Replacing image for %s with %s.\n", unitType->ImageFile, nameBuffer);
			if (unitType->Voxel)
			{
				char savedName[0x19];
				strcpy_s(savedName, unitType->ImageFile);
				strcpy_s(unitType->ImageFile, nameBuffer);
				unitType->LoadVoxel();
				strcpy_s(unitType->ImageFile, savedName);
			}
			else
			{
				char filename[260];
				_makepath_s(filename, 0, 0, nameBuffer, ".SHP");
				unitType->Image = GameCreate<SHPReference>(filename);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x41CD54, AircraftTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(AircraftTypeClass*, aircraftType, ESI);
		char nameBuffer[0x19];

		if (CCINIClass::INI_Art->ReadString(aircraftType->ImageFile, "Image", 0, nameBuffer, 0x19) != 0)
		{
			if (aircraftType->Voxel)
			{
				Debug::Log("[Phobos] Replacing image for %s with %s.\n", aircraftType->ImageFile, nameBuffer);
				char savedName[0x19];
				strcpy_s(savedName, aircraftType->ImageFile);
				strcpy_s(aircraftType->ImageFile, nameBuffer);
				aircraftType->LoadVoxel();
				strcpy_s(aircraftType->ImageFile, savedName);
			}
		}
	}

	return 0;
}
