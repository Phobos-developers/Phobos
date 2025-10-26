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

ParticleSystemTypeExt::ExtContainer ParticleSystemTypeExt::ExtMap;

// =============================
// load / save

template <typename T>
void ParticleSystemTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AdjustTargetCoordsOnRotation)
		;
}

void ParticleSystemTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->AdjustTargetCoordsOnRotation.Read(exINI, pSection, "AdjustTargetCoordsOnRotation");
}

void ParticleSystemTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ParticleSystemTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleSystemTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ParticleSystemTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool ParticleSystemTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParticleSystemTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ParticleSystemTypeExt::ExtContainer::ExtContainer() : Container("ParticleSystemTypeClass") { }
ParticleSystemTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644215, ParticleSystemTypeClass_CTOR, 0x7)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);

	ParticleSystemTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x644986, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);

	ParticleSystemTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x644830, ParticleSystemTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6447E0, ParticleSystemTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64481F, ParticleSystemTypeClass_Load_Suffix, 0x6)
{
	ParticleSystemTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x644844, ParticleSystemTypeClass_Save_Suffix, 0x5)
{
	ParticleSystemTypeExt::ExtMap.SaveStatic();

	return 0;
}

//DEFINE_HOOK_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)// Section dont exist!
DEFINE_HOOK(0x644615, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
