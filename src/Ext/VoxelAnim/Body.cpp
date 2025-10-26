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
#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Utilities/Macro.h>

VoxelAnimExt::ExtContainer VoxelAnimExt::ExtMap;

void VoxelAnimExt::InitializeLaserTrails(VoxelAnimClass* pThis)
{
	const auto pThisExt = VoxelAnimExt::ExtMap.Find(pThis);

	if (pThisExt->LaserTrails.size())
		return;

	const auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find(pThis->Type);
	const auto pOwner = pThis->OwnerHouse;
	pThisExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		pThisExt->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[idxTrail].get(), pOwner));
}

void VoxelAnimExt::ExtData::Initialize() { }

// =============================
// load / save
template <typename T>
void VoxelAnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LaserTrails)
		.Process(this->TrailerSpawnTimer)
		;
}

void VoxelAnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<VoxelAnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void VoxelAnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<VoxelAnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool VoxelAnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool VoxelAnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

VoxelAnimExt::ExtContainer::ExtContainer() : Container("VoxelAnimClass") { }
VoxelAnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x749951, VoxelAnimClass_CTOR, 0xC)
DEFINE_HOOK(0x74942E, VoxelAnimClass_CTOR, 0xC)
{
	GET(VoxelAnimClass*, pItem, ESI);

	VoxelAnimExt::ExtMap.TryAllocate(pItem);
	VoxelAnimExt::InitializeLaserTrails(pItem);

	return 0;
}

DEFINE_HOOK(0x7499F1, VoxelAnimClass_DTOR, 0x5)
{
	GET(VoxelAnimClass*, pItem, ECX);

	VoxelAnimExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x74A970, VoxelAnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x74AA10, VoxelAnimClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(VoxelAnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x74A9FB, VoxelAnimClass_Load_Suffix, 0x7)
{
	VoxelAnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x74AA24, VoxelAnimClass_Save_Suffix, 0x5)
{
	VoxelAnimExt::ExtMap.SaveStatic();
	return 0;
}
