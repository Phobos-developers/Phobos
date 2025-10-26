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
#pragma once
#include <functional>
#include "Constructs.h"

class TechnoClass;
class TechnoTypeClass;
class EBolt;
class FootClass;
class HouseClass;
class BuildingTypeClass;
class BuildingClass;
class HouseTypeClass;
class SuperClass;
class SuperWeaponTypeClass;
class AlphaShapeClass;

class AresTechnoExtData;
class AresTechnoTypeExtData;
class AresHouseExtData;
class AresSWTypeExtData;

class AresFunctions
{
public:
	static void InitAres3_0();
	static void InitAres3_0p1();
	static void InitNoAres();

	// TechnoExt
	static bool(__stdcall* ConvertTypeTo)(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static EBolt* (__stdcall* CreateAresEBolt)(WeaponTypeClass* pWeapon);

	static void(__stdcall* SpawnSurvivors)(FootClass* pThis, TechnoClass* pKiller, bool Select, bool IgnoreDefenses);

	static bool(__thiscall* ReverseEngineer)(void* pAresHouseExt, TechnoTypeClass* pType);

	static bool(__thiscall* IsTargetConstraintsEligible)(void*, HouseClass*, bool);

	static std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> SWTypeExtMap_Find;

	static PhobosMap<ObjectClass*, AlphaShapeClass*>* AlphaExtMap;

	// BuildingTypeExt::ExtData
	static void* (__thiscall* GetTunnel)(void*, HouseClass*);
	static void(__thiscall* AddPassengerFromTunnel)(void*, BuildingClass*, FootClass*);
private:

	static constexpr bool _maybe = false;

	static constexpr bool AresWasWrongAboutSpawnSurvivors = _maybe;

	static void* _SWTypeExtMap;
	static AresSWTypeExtData* (__thiscall* _SWTypeExtMapFind)(void*, SuperWeaponTypeClass*);
};
