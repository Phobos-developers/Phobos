#pragma once
class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

class AresFunctions
{
public:
	static void InitAres3_0();
	static void InitAres3_0p1();
	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool IgnoreDefenses);
private:
	static constexpr bool _maybe = false;

	static constexpr bool AresWasWrongAboutSpawnSurvivors = _maybe;
	static constexpr bool AresWasWrongAboutAduction = true;
};
