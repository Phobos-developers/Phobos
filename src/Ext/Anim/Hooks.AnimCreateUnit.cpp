// Anim-to--Unit
// Author: Otamaa, revisions by Starkku

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFSET(0x44, 0x4));

	R->ECX(R->ESI());
	TechnoExt::ExtMap.Find(pThis)->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
	pThis->Destroy();

	return 0x737F74;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

// Performance tweak, mark once instead of every frame.
// DEFINE_HOOK(0x423BC8, AnimClass_AI_CreateUnit_MarkOccupationBits, 0x6)
DEFINE_HOOK(0x4226F0, AnimClass_CTOR_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
	{
		auto location = pThis->GetCoords();

		if (auto pCell = pThis->GetCell())
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);

		pThis->MarkAllOccupationBits(location);
	}

	return 0; //return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_AI_CreateUnit_ActualEffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (auto const pUnitType = pTypeExt->CreateUnit.Get())
	{
		auto const pExt = AnimExt::ExtMap.Find(pThis);
		auto origLocation = pThis->Location;
		auto const pCell = pThis->GetCell();

		if (pCell)
			origLocation = pCell->GetCoordsWithBridge();
		else
			origLocation.Z = MapClass::Instance->GetCellFloorHeight(origLocation);

		pThis->UnmarkAllOccupationBits(origLocation);

		auto facing = pTypeExt->CreateUnit_RandomFacing
			? static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pTypeExt->CreateUnit_Facing;

		auto const primaryFacing = pTypeExt->CreateUnit_InheritDeathFacings && pExt->FromDeathUnit ? pExt->DeathUnitFacing : facing;
		DirType* secondaryFacing = nullptr;
		Mission* missionAI = nullptr;

		if (pUnitType->WhatAmI() == AbstractType::UnitType && pUnitType->Turret && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings)
		{
			auto dir = pExt->DeathUnitTurretFacing.GetDir();
			secondaryFacing = &dir;
			Debug::Log("CreateUnit: Using stored turret facing %d\n", pExt->DeathUnitTurretFacing.GetFacing<256>());
		}

		if (pTypeExt->CreateUnit_AIMission.isset())
			missionAI = &pTypeExt->CreateUnit_AIMission;

		TechnoTypeExt::CreateUnit(pUnitType, pThis->Location, primaryFacing, secondaryFacing, pThis->Owner, pExt->Invoker, pExt->InvokerHouse,
			pTypeExt->CreateUnit_SpawnAnim, pTypeExt->CreateUnit_SpawnHeight, pTypeExt->CreateUnit_AlwaysSpawnOnGround, pTypeExt->CreateUnit_ConsiderPathfinding,
			pTypeExt->CreateUnit_SpawnParachutedInAir, pTypeExt->CreateUnit_Mission, missionAI);
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}
