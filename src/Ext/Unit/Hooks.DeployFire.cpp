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
#include <ScenarioClass.h>

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
DEFINE_HOOK(0x4C7512, EventClass_Execute_StopUnitDeployFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);
	if (pUnit && pUnit->CurrentMission == Mission::Unload && pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer)
	{
		pUnit->SetTarget(nullptr);
		pThis->QueueMission(Mission::Guard, true);
	}

	return 0;
}

DEFINE_HOOK(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = abstract_cast<UnitClass*>(pThis);

	// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit)
	{
		auto const pType = pUnit->Type;

		if (pType->DeployFire && !pType->IsSimpleDeployer)
		{
			int weaponIndex = -1;
			auto const pWeapon = TechnoExt::GetDeployFireWeapon(pThis, weaponIndex);

			if (pWeapon && pWeapon->FireOnce)
			{
				const auto pExt = TechnoExt::ExtMap.Find(pThis);

				if (pExt->DeployFireTimer.HasTimeLeft())
					return DoNotExecute;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x73DCEF, UnitClass_Mission_Unload_DeployFire, 0x6)
{
	enum { SkipGameCode = 0x73DD3C };

	GET(UnitClass*, pThis, ESI);

	int weaponIndex = -1;
	auto const pWeapon = TechnoExt::GetDeployFireWeapon(pThis, weaponIndex);

	if (weaponIndex < 0 || !pWeapon)
	{
		pThis->QueueMission(Mission::Guard, true);
		return SkipGameCode;
	}

	const auto pCell = MapClass::Instance.GetCellAt(pThis->GetMapCoords());
	pThis->SetTarget(pCell);

	if (pThis->GetFireError(pCell, weaponIndex, true) == FireError::OK)
	{
		pThis->Fire(pThis->Target, weaponIndex);

		if (pWeapon->FireOnce)
		{
			pThis->SetTarget(nullptr);
			pThis->QueueMission(Mission::Guard, true);
			const auto pExt = TechnoExt::ExtMap.Find(pThis);
			const auto UnloadControl = &MissionControlClass::Array[(int)Mission::Unload];
			const int delay = static_cast<int>(UnloadControl->Rate * 900) + ScenarioClass::Instance->Random(0, 2);
			pExt->DeployFireTimer.Start(Math::min(pWeapon->ROF, delay));
		}
	}

	return SkipGameCode;
}
