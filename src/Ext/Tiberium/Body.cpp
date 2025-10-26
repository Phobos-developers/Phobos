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

TiberiumExt::ExtContainer TiberiumExt::ExtMap;

// =============================
// load / save

template <typename T>
void TiberiumExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->MinimapColor)
		;
}

void TiberiumExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->MinimapColor.Read(exINI, pSection, "MinimapColor");
}

void TiberiumExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TiberiumClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TiberiumExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TiberiumClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TiberiumExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TiberiumExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TiberiumExt::ExtContainer::ExtContainer() : Container("TiberiumClass") { }
TiberiumExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x721876, TiberiumClass_CTOR, 0x5)
{
	GET(TiberiumClass*, pItem, ESI);

	TiberiumExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x721888, TiberiumClass_DTOR, 0x6)
{
	GET(TiberiumClass*, pItem, ECX);

	TiberiumExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x721E80, TiberiumClass_SaveLoad_Prefix, 0x7)
DEFINE_HOOK(0x7220D0, TiberiumClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TiberiumClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TiberiumExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x72208C, TiberiumClass_Load_Suffix, 0x7)
{
	TiberiumExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x72212C, TiberiumClass_Save_Suffix, 0x5)
{
	TiberiumExt::ExtMap.SaveStatic();

	return 0;
}

//DEFINE_HOOK_AGAIN(0x721CE9, TiberiumClass_LoadFromINI, 0xA)// Section dont exist!
DEFINE_HOOK_AGAIN(0x721CDC, TiberiumClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x721C7B, TiberiumClass_LoadFromINI, 0xA)
{
	GET(TiberiumClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0xC4, 0x4));

	TiberiumExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
