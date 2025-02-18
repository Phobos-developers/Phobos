#include "AresFunctions.h"
#include "AresHelper.h"
#include "Patch.h"
#define NOTE_ARES_FUN(name,reladdr) AresFunctions::name = reinterpret_cast<decltype(AresFunctions::name)>(AresHelper::AresBaseAddress + reladdr)

decltype(AresFunctions::ConvertTypeTo) AresFunctions::ConvertTypeTo = nullptr;
decltype(AresFunctions::SpawnSurvivors) AresFunctions::SpawnSurvivors = nullptr;
std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> AresFunctions::SWTypeExtMap_Find;

void* AresFunctions::_SWTypeExtMap = nullptr;
decltype(AresFunctions::_SWTypeExtMapFind) AresFunctions::_SWTypeExtMapFind = nullptr;

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
		NOTE_ARES_FUN(SpawnSurvivors, 0x464C0);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x57C70);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC1C54);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

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
		NOTE_ARES_FUN(SpawnSurvivors, 0x47030);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x58900);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC2C50);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

#ifndef USING_MULTIFINITE_SYRINGE
	Apply_Ares3_0p1_Patches();
#endif
}

#undef NOTE_ARES_FUN

