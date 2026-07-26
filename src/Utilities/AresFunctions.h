#pragma once
#include <functional>
#include "Constructs.h"
#include "AntaresAPI.h"   // AntaresFactory, and the types the accessors traffic in

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

	//! Fill the shared entries below from Antares' interop table rather than from
	//! Ares RVAs. Anything that only exists to support a patch into Ares' own code
	//! stays null: Antares is never patched, it hands subsystems over instead.
	static void InitAntares();

	static void InitNoAres();

	// ---------------------------------------------------------------------------
	// Extension data, reached through accessors rather than by offset.
	//
	// Both backends fill these. For Ares they are thunks over the layouts we know;
	// for Antares they come straight from its table. Call sites go through them so
	// no shadow struct has to exist outside AresAddressInit.cpp.
	static AlphaShapeClass* (__stdcall* FindAlphaShape)(ObjectClass* pObject);
	static CDTimerClass* (__stdcall* GetDisableWeaponTimer)(TechnoClass* pThis);
	static bool* (__stdcall* GetDriverKilled)(TechnoClass* pThis);
	static bool (__stdcall* IsPsionicsImmune)(TechnoTypeClass* pType, VeterancyStruct const* pVeterancy);
	static bool (__stdcall* IsVeteranBuilding)(HouseTypeClass* pCountry, BuildingTypeClass* pType);
	static bool* (__stdcall* GetInfiltrated)(HouseClass* pHouse, AntaresFactory factory);
	static bool (__stdcall* GetOperators)(TechnoTypeClass* pType, InfantryTypeClass* const** ppItems,
		int* pCount, bool* pAnyAllowed);
	//! The tunnel network a building belongs to, or null. Opaque -- pass it back
	//! to AddPassengerFromTunnel and nothing else.
	static void* (__stdcall* FindTunnel)(BuildingClass* pBuilding);
	// ---------------------------------------------------------------------------

	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static EBolt* (__stdcall* CreateAresEBolt)(WeaponTypeClass* pWeapon);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool PreventEscape);

	static bool(__thiscall* ReverseEngineer)(void* pAresHouseExt, TechnoTypeClass* pType);

	static bool(__thiscall* IsTargetConstraintsEligible)(void*, HouseClass*, bool);

	static void(__thiscall* UnitDeliveryStateMachine_Update)(void*);

	// WarheadTypeExt
	static bool(__thiscall* ApplyPermaMC)(void*, HouseClass* pSourceHouse, AbstractClass* pTarget);

	static bool (*DetailsCurrentlyEnabled)();

	static void(*SendPDPlane)(HouseClass* pOwner, CellClass* pDestination, AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types, Iterator<int> Nums);

	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;

	static PhobosMap<ObjectClass*, AlphaShapeClass*>* AlphaExtMap;

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
