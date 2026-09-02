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

// Wrapper of values passed to Ares' SW target picker.
class AresSWTargetInfo
{
public:
	SuperClass* SW;
	HouseClass* Owner;
	void* Ext;  // Ares' SWTypeExt
	void* Unk1; // Unknown
	void* Unk2; // Unknown
};

// What Ares' SW target picker sets and returns.
class AresSWTargetResult
{
public:
	CellStruct TargetCell;
	bool WasSuccessful;
};

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

	static void(__thiscall* SetSpotlight)(void*, BuildingLightClass* pSpotlight);

	// WarheadTypeExt
	static bool(__thiscall* ApplyPermaMC)(void*, HouseClass* pSourceHouse, AbstractClass* pTarget);

	static bool (*DetailsCurrentlyEnabled)();

	static void(*SendPDPlane)(HouseClass* pOwner, CellClass* pDestination, AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types, Iterator<int> Nums);

	static AresSWTargetResult*(__stdcall* PickSuperWeaponTarget)(AresSWTargetResult*, AresSWTargetInfo*);

	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;

	static PhobosMap<ObjectClass*, AlphaShapeClass*>* AlphaExtMap;
	static PhobosMap<BombClass*, WeaponTypeClass**>* BombExtMap;

	// BuildingTypeExt
	static void* (__thiscall* GetTunnel)(void*, HouseClass*);
	static void(__thiscall* AddPassengerFromTunnel)(void*, BuildingClass*, FootClass*);

	// VoxClass
	static int(__stdcall* FindEVAIndex)(const char* buffer);
private:
	static constexpr bool AresWasWrongAboutSpawnSurvivors = false;

	static void* _SWTypeExtMap;
	static AresSWTypeExtData* (__thiscall* _SWTypeExtMapFind)(void*, SuperWeaponTypeClass*);
};
