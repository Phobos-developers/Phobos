// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0x44, 0x4));

	R->ECX(R->ESI());
	TechnoExt::ExtMap.Find(pThis)->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, receiveDamageArgs.Attacker);
	pThis->Destroy();

	return 0x737F74;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

// Performance tweak, mark once instead of every frame.
// DEFINE_HOOK(0x423BC8, AnimClass_AI_CreateUnit_MarkOccupationBits, 0x6)
DEFINE_HOOK(0x4226F0, AnimClass_CTOR_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnitType)
		pThis->MarkAllOccupationBits(pThis->GetCell()->GetCoordsWithBridge());

	return 0; //return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_AI_CreateUnit_ActualEffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pType = pThis->Type;
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pType);

	if (auto const pCreateUnit = pTypeExt->CreateUnitType.get())
	{
		auto const pUnitType = pCreateUnit->Type;
		auto const pExt = AnimExt::ExtMap.Find(pThis);
		pThis->UnmarkAllOccupationBits(pThis->GetCell()->GetCoordsWithBridge());

		auto const facing = pCreateUnit->RandomFacing
			? static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pCreateUnit->Facing;

		auto const primaryFacing = pCreateUnit->InheritDeathFacings && pExt->FromDeathUnit ? pExt->DeathUnitFacing : facing;
		DirType* secondaryFacing = nullptr;

		if (pUnitType->WhatAmI() == AbstractType::UnitType && pUnitType->Turret && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pCreateUnit->InheritTurretFacings)
		{
			auto dir = pExt->DeathUnitTurretFacing.GetDir();
			secondaryFacing = &dir;
			Debug::Log("CreateUnit: Using stored turret facing %d from anim [%s]\n", pExt->DeathUnitTurretFacing.GetFacing<256>(), pType->get_ID());
		}

		TechnoTypeExt::CreateUnit(pCreateUnit, primaryFacing, secondaryFacing, pThis->Location, pThis->Owner, pExt->Invoker, pExt->InvokerHouse);
	}

	return (pType->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}
