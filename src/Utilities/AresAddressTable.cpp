#pragma once
#include "AresFunctions.h"
#include "AresHelper.h"
#include "Patch.h"
#define NOTE_ARES_FUN(name,reladdr) AresFunctions::name = reinterpret_cast<decltype(AresFunctions::name)>(AresHelper::AresBaseAddress + reladdr)

decltype(AresFunctions::ConvertTypeTo) AresFunctions::ConvertTypeTo = nullptr;
decltype(AresFunctions::SpawnSurvivors) AresFunctions::SpawnSurvivors = nullptr;

void AresFunctions::InitAres3_0()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x043650);
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4C0EB, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x48C69, { 0x30 });
	}
	else
		NOTE_ARES_FUN(SpawnSurvivors, 0x0464C0);
}

void AresFunctions::InitAres3_0p1()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x044130);
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4CD4B, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x498B9, { 0x30 });
	}
	else
		NOTE_ARES_FUN(SpawnSurvivors, 0x047030);
}

#undef NOTE_ARES_FUN

