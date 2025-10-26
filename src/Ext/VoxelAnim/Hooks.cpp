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

#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Additional, 0xC)
{
	GET(VoxelAnimClass* const, pThis, EBX);

	//auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	const auto pThisExt = VoxelAnimExt::ExtMap.Find(pThis);

	if (!pThisExt->LaserTrails.empty())
	{
		const CoordStruct location = pThis->GetCoords();

		for (const auto& pTrail : pThisExt->LaserTrails)
		{
			if (!pTrail->LastLocation.isset())
				pTrail->LastLocation = location;

			pTrail->Visible = pThis->IsVisible;
			pTrail->Update(location);
		}
	}

	return 0;
}

DEFINE_HOOK(0x74A027, VoxelAnimClass_AI_Expired, 0x6)
{
	enum { SkipGameCode = 0x74A22A };

	GET(VoxelAnimClass* const, pThis, EBX);
	GET(const int, flag, EAX);

	const bool heightFlag = flag & 0xFF;
	auto const pType = pThis->Type;
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pType);
	auto const splashAnims = pTypeExt->SplashAnims.GetElements(RulesClass::Instance->SplashList);

	AnimExt::HandleDebrisImpact(pType->ExpireAnim, pTypeExt->WakeAnim, splashAnims, pThis->OwnerHouse, pType->Warhead, pType->Damage,
		pThis->GetCell(), pThis->Location, heightFlag, pType->IsMeteor, pTypeExt->Warhead_Detonate, pTypeExt->ExplodeOnWater, pTypeExt->SplashAnims_PickRandom);

	return SkipGameCode;
}

DEFINE_HOOK(0x74A70E, VoxelAnimClass_AI_Trailer, 0x6)
{
	enum { SkipGameCode = 0x74A7AB };

	GET(VoxelAnimClass* const, pThis, EBX);

	const auto pType = pThis->Type;

	if (const auto pAnimType = pType->TrailerAnim)
	{
		const auto pExt = VoxelAnimExt::ExtMap.Find(pThis);

		if (pExt->TrailerSpawnTimer.Expired())
		{
			pExt->TrailerSpawnTimer.Start(VoxelAnimTypeExt::ExtMap.Find(pType)->Trailer_SpawnDelay.Get());
			auto const pTrailerAnim = GameCreate<AnimClass>(pAnimType, pThis->Location, 1, 1);
			AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->OwnerHouse, nullptr, false, true);
		}
	}

	return SkipGameCode;
}
