#pragma once
#include <functional>
class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;
class BuildingClass;
class HouseTypeClass;
class SuperClass;
class SuperWeaponTypeClass;

class AresTechnoExtData;
class AresTechnoTypeExtData;
class AresHouseExtData;
class AresSWTypeExtData;

class AresFunctions
{
public:
	static void InitAres3_0();
	static void InitAres3_0p1();
	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool IgnoreDefenses);
	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;
private:
	static constexpr bool _maybe = false;

	static constexpr bool AresWasWrongAboutSpawnSurvivors = _maybe;

	static void* _SWTypeExtMap;
	static AresSWTypeExtData* (__thiscall* _SWTypeExtMapFind)(void*, SuperWeaponTypeClass*);
};
