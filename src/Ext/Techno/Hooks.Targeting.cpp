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
#include "Body.h"

DEFINE_HOOK(0x6FA697, TechnoClass_Update_DontScanIfUnarmed, 0x6)
{
	enum { SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);
	return pThis->IsArmed() ? 0 : SkipTargeting;
}

DEFINE_HOOK(0x70982C, TechnoClass_TargetAndEstimateDamage_TargetingDelay, 0x8)
{
	enum { SkipGameCode = 0x70989C };

	GET(TechnoClass* const, pThis, ESI);
	GET(const int, frame, EAX);

	pThis->unknown_4FC = frame;
	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pThis->MegaMissionIsAttackMove())
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerAttackMoveTargetingDelay.Get(RulesExt::Global()->PlayerAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AIAttackMoveTargetingDelay.Get(RulesExt::Global()->AIAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}
	else if (pThis->CurrentMission == Mission::Area_Guard)
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExt::Global()->PlayerGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay))
			: pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExt::Global()->AIGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay));
	}
	else
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerNormalTargetingDelay.Get(RulesExt::Global()->PlayerNormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AINormalTargetingDelay.Get(RulesExt::Global()->AINormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}

	R->ECX(delay);
	R->EDX(frame);

	return SkipGameCode;
}
