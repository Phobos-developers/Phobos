#pragma once
#include <functional>
#include "Constructs.h"

class TechnoClass;
class TechnoTypeClass;
class EBolt;
class FootClass;
class HouseClass;
class BuildingTypeClass;
class BuildingClass;
class HouseTypeClass;
class SuperClass;
class SuperWeaponTypeClass;
class AlphaShapeClass;

class AresTechnoExtData;
class AresTechnoTypeExtData;
class AresHouseExtData;
class AresSWTypeExtData;

class AresFunctions
{
public:
	static void InitAres3_0();
	static void InitAres3_0p1();
	static void InitNoAres();

	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static EBolt* (__stdcall* CreateAresEBolt)(WeaponTypeClass* pWeapon);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool PreventEscape);

	static bool(__thiscall* ReverseEngineer)(void* pAresHouseExt, TechnoTypeClass* pType);

	static bool(__thiscall* IsTargetConstraintsEligible)(void*, HouseClass*, bool);

	static void(__thiscall* UnitDeliveryStateMachine_Update)(void*);

	// WarheadTypeExt::ExtData
	static bool(__thiscall* ApplyPermaMC)(void*, HouseClass* pSourceHouse, AbstractClass* pTarget);

	static bool (*DetailsCurrentlyEnabled)();

	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;

	static PhobosMap<ObjectClass*, AlphaShapeClass*>* AlphaExtMap;

	// BuildingTypeExt::ExtData
	static void* (__thiscall* GetTunnel)(void*, HouseClass*);
	static void(__thiscall* AddPassengerFromTunnel)(void*, BuildingClass*, FootClass*);

	// VoxClass
	static int(__stdcall* FindEVAIndex)(const char* buffer);
private:
	static constexpr bool AresWasWrongAboutSpawnSurvivors = false;

	static void* _SWTypeExtMap;
	static AresSWTypeExtData* (__thiscall* _SWTypeExtMapFind)(void*, SuperWeaponTypeClass*);
};
