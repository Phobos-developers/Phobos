#pragma once

// Mirror of the Antares interop ABI.
//
// Antares is an open-source reimplementation of Ares. It is a different compile, so
// none of the hardcoded RVAs or shadow structs in AresFunctions/AresAddressInit can
// ever apply to it -- and it is not Ares, so its behaviour must never be patched the
// way Ares' is. Instead it exports a versioned table of function pointers.
//
// SOURCE OF TRUTH: Antares/src/Interop/Api.h. This file must stay a byte-compatible
// mirror of it. The table is append-only on the Antares side; if a field is ever
// reordered there, everything below breaks silently, so check `size` before reading
// anything past the version block.
//
// Calling conventions are load-bearing. The table entries are __stdcall; the
// discovery export is __cdecl, which is also why its exported name is undecorated.

#include <windows.h>            // HRESULT
#include <GeneralStructures.h>  // CDTimerClass, an alias that cannot be forward-declared
#include <Utilities/Iterator.h> // passed by value, so it must be complete

#include <cstdint>

class AbstractClass;
class AircraftTypeClass;
class AlphaShapeClass;
class BuildingClass;
class BuildingTypeClass;
class CellClass;
class EBolt;
class FootClass;
class HouseClass;
class HouseTypeClass;
class InfantryTypeClass;
class ObjectClass;
class SuperWeaponTypeClass;
class TechnoClass;
class TechnoTypeClass;
class WarheadTypeClass;
class WeaponTypeClass;
struct VeterancyStruct;

enum class AntaresFactory : uint32_t
{
	WarFactory = 0,
	NavalYard = 1,
	Barracks = 2,
	AircraftFactory = 3,
	ConstructionYard = 4,
};

//! Subsystems Antares will stand down on request so we can own them instead.
enum class AntaresFeature : uint32_t
{
	EBolt = 0,
	AlphaImage = 1,
};

struct AntaresAPI_v1
{
	uint32_t size;
	uint32_t major;
	uint32_t minor;
	uint32_t patch;

	bool  (__stdcall* ConvertTypeTo)(TechnoClass* pThis, TechnoTypeClass* pToType);
	void  (__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool select, bool ignoreDefenses);
	bool  (__stdcall* ReverseEngineer)(HouseClass* pHouse, TechnoTypeClass* pVictimType);
	bool  (__stdcall* MeetsAITargetingConstraints)(SuperWeaponTypeClass* pType, HouseClass* pOwner, bool manual);
	bool  (__stdcall* IsSuperWeaponAvailable)(SuperWeaponTypeClass* pType, HouseClass* pHouse);
	bool  (__stdcall* ApplyPermaMindControl)(WarheadTypeClass* pWH, HouseClass* pOwner, AbstractClass* pTarget);
	bool  (__stdcall* DetailsCurrentlyEnabled)();
	EBolt* (__stdcall* CreateElectricBolt)(WeaponTypeClass* pWeapon);
	int   (__stdcall* FindEVAIndex)(const char* pID);
	bool  (__stdcall* CameoIsElite)(TechnoTypeClass* pType, HouseClass* pHouse);
	void  (__stdcall* SendParadropPlane)(HouseClass* pOwner, CellClass* pTarget,
		AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> types, Iterator<int> nums);
	void* (__stdcall* FindTunnel)(BuildingClass* pBuilding);
	void  (__stdcall* AddTunnelPassenger)(void* pTunnel, BuildingClass* pBuilding, FootClass* pPassenger);

	CDTimerClass* (__stdcall* GetDisableWeaponTimer)(TechnoClass* pThis);
	bool*         (__stdcall* GetDriverKilled)(TechnoClass* pThis);
	bool*         (__stdcall* GetInfiltrated)(HouseClass* pHouse, AntaresFactory factory);
	bool (__stdcall* IsPsionicsImmune)(TechnoTypeClass* pType, VeterancyStruct const* pVeterancy);
	bool (__stdcall* GetOperators)(TechnoTypeClass* pType, InfantryTypeClass* const** ppItems,
		int* pCount, bool* pAnyAllowed);
	bool (__stdcall* IsVeteranBuilding)(HouseTypeClass* pCountry, BuildingTypeClass* pType);
	AlphaShapeClass* (__stdcall* FindAlphaShape)(ObjectClass* pObject);

	bool (__stdcall* DisableFeature)(AntaresFeature feature);
	bool (__stdcall* IsFeatureDisabled)(AntaresFeature feature);
};

//! Exported by Antares, undecorated, __cdecl.
using GetAntaresAPIFunc = HRESULT(__cdecl*)(uint32_t wantMajor, AntaresAPI_v1** ppApi);

//! The major this build of Phobos was written against.
static constexpr uint32_t AntaresAPIMajor = 1;
