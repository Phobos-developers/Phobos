#include "AresFunctions.h"
#include "AresHelper.h"
#include "Patch.h"
#define NOTE_ARES_FUN(name,reladdr) AresFunctions::name = reinterpret_cast<decltype(AresFunctions::name)>(AresHelper::AresBaseAddress + reladdr)

decltype(AresFunctions::ConvertTypeTo) AresFunctions::ConvertTypeTo = nullptr;
decltype(AresFunctions::SpawnSurvivors) AresFunctions::SpawnSurvivors = nullptr;
decltype(AresFunctions::IsTargetConstraintsEligible) AresFunctions::IsTargetConstraintsEligible = nullptr;
decltype(AresFunctions::ApplyAcademy) AresFunctions::ApplyAcademy = nullptr;
std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> AresFunctions::SWTypeExtMap_Find;
std::function<AresHouseExtData* (HouseClass*)> AresFunctions::HouseExtMap_Find;

void* AresFunctions::_SWTypeExtMap = nullptr;
decltype(AresFunctions::_SWTypeExtMapFind) AresFunctions::_SWTypeExtMapFind = nullptr;
void* AresFunctions::_HouseExtMap = nullptr;
decltype(AresFunctions::_HouseExtMapFind) AresFunctions::_HouseExtMapFind = nullptr;

void Apply_Ares3_0_Patches();
void Apply_Ares3_0p1_Patches();

void AresFunctions::InitAres3_0()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x43650);
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4C0EB, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x48C69, { 0x30 });
	}
	else
	{
		NOTE_ARES_FUN(SpawnSurvivors, 0x464C0);
	}

	NOTE_ARES_FUN(IsTargetConstraintsEligible, 0x032110);

	NOTE_ARES_FUN(ApplyAcademy, 0x020750);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x57C70);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC1C54);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

	NOTE_ARES_FUN(_HouseExtMapFind, 0x57C70);
	NOTE_ARES_FUN(_HouseExtMap, 0xC1AA8);
	HouseExtMap_Find = [](HouseClass* houseClass) { return _HouseExtMapFind(_HouseExtMap, houseClass); };

#ifndef USING_MULTIFINITE_SYRINGE
	Apply_Ares3_0_Patches();
#endif
}

void AresFunctions::InitAres3_0p1()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x44130);
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4CD4B, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x498B9, { 0x30 });
	}
	else
	{
		NOTE_ARES_FUN(SpawnSurvivors, 0x47030);
	}

	NOTE_ARES_FUN(IsTargetConstraintsEligible, 0x032AF0);

	NOTE_ARES_FUN(ApplyAcademy, 0x0211D0);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x58900);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC2C50);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

	NOTE_ARES_FUN(_HouseExtMapFind, 0x589A0);
	NOTE_ARES_FUN(_HouseExtMap, 0xC2B08);
	HouseExtMap_Find = [](HouseClass* houseClass) { return _HouseExtMapFind(_HouseExtMap, houseClass); };

#ifndef USING_MULTIFINITE_SYRINGE
	Apply_Ares3_0p1_Patches();
#endif
}

#undef NOTE_ARES_FUN

