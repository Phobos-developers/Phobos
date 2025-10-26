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

DEFINE_HOOK(0x5047D0, HouseClass_UpdateAngerNodes_SetForceEnemy, 0x6)
{
	GET(HouseClass*, pThis, EAX);
	enum { ReturnValue = 0x50483F };

	if (pThis)
	{
		const int forceIndex = HouseExt::ExtMap.Find(pThis)->GetForceEnemyIndex();

		if (forceIndex >= 0 || forceIndex == -2)
		{
			R->EDX(forceIndex == -2 ? -1 : forceIndex);
			return ReturnValue;
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x4F9BFC, HouseClass_ClearForceEnemy, 0xA)	// HouseClass_MakeAlly
DEFINE_HOOK(0x4FD772, HouseClass_ClearForceEnemy, 0xA)			// HouseClass_UpdateAI
{
	HouseClass* pThis = nullptr;

	if (R->Origin() == 0x4FD772)
		pThis = R->EBX<HouseClass*>();
	else
		pThis = R->ESI<HouseClass*>();

	if (pThis)
	{
		HouseExt::ExtMap.Find(pThis)->SetForceEnemyIndex(-1);
		pThis->UpdateAngerNodes(0, pThis);
		return R->Origin() + 0xA;
	}

	return 0;
}
