#pragma once
#include <functional>
#include "Constructs.h"
#include <StageClass.h>
#include <ArrayClasses.h>

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
class FactoryClass;

struct AresBuildType
{
	int ItemIndex{ -1 };
	AbstractType ItemType{ AbstractType::None };
	BYTE IsAlt{ 0 };
	BYTE Padding_09[3]{ 0, 0, 0 };
	FactoryClass* CurrentFactory{ nullptr };
	DWORD unknown_10{ 0 };
	StageClass Progress{};
	int FlashEndFrame{ 0 };

	bool operator == (const AresBuildType& rhs) const
	{
		return ItemIndex == rhs.ItemIndex && ItemType == rhs.ItemType;
	}

	bool operator != (const AresBuildType& rhs) const
	{
		return !(*this == rhs);
	}
};
static_assert(sizeof(AresBuildType) == 0x34);

class AresFunctions
{
public:
	static void InitAres3_0();
	static void InitAres3_0p1();
	static void InitNoAres();

	static DynamicVectorClass<AresBuildType>* TabCameos;

	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static EBolt* (__stdcall* CreateAresEBolt)(WeaponTypeClass* pWeapon);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool PreventEscape);

	static bool(__thiscall* IsTargetConstraintsEligible)(void*, HouseClass*, bool);

	static void(__thiscall* UnitDeliveryStateMachine_Update)(void*);

	static void(__thiscall* SetSpotlight)(void*, BuildingLightClass* pSpotlight);

	// WeaponTypeExt
	static bool(__thiscall* ApplyAbductor)(WeaponTypeClass**, TechnoClass* pFirer, FootClass* pTarget);
	
	// WarheadTypeExt
	static bool(__thiscall* ApplyPermaMC)(void*, HouseClass* pSourceHouse, AbstractClass* pTarget);

	static bool (*DetailsCurrentlyEnabled)();

	static void(*SendPDPlane)(HouseClass* pOwner, CellClass* pDestination, AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types, Iterator<int> Nums);

	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;

	static PhobosMap<ObjectClass*, AlphaShapeClass*>* AlphaExtMap;
	static PhobosMap<BombClass*, WeaponTypeClass**>* BombExtMap;

	// BuildingTypeExt
	static void* (__thiscall* GetTunnel)(void*, HouseClass*);
	static void(__thiscall* AddPassengerFromTunnel)(void*, BuildingClass*, FootClass*);

	// HouseExt
	static bool(__thiscall* ReverseEngineer)(void*, TechnoTypeClass* pType);
	static InfantryTypeClass* (__thiscall* GetCrew)(void*);

	// VoxClass
	static int(__stdcall* FindEVAIndex)(const char* buffer);
private:
	static constexpr bool AresWasWrongAboutSpawnSurvivors = false;

	static void* _SWTypeExtMap;
	static AresSWTypeExtData* (__thiscall* _SWTypeExtMapFind)(void*, SuperWeaponTypeClass*);
};
