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

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>

CellExt::ExtContainer CellExt::ExtMap;

// =============================
// load / save

template <typename T>
void CellExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->RadSites)
		.Process(this->RadLevels)
		.Process(this->InfantryCount)
		;
}

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CellClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CellClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void CellExt::ExtData::InvalidatePointer(void* ptr, bool removed)
{ }

bool CellExt::RadLevel::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool CellExt::RadLevel::Save(PhobosStreamWriter& stm) const
{
	return const_cast<CellExt::RadLevel*>(this)->Serialize(stm);
}

template <typename T>
bool CellExt::RadLevel::Serialize(T& stm)
{
	return stm
		.Process(this->Rad)
		.Process(this->Level)
		.Success();
}

// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") { }
CellExt::ExtContainer::~ExtContainer() = default;

bool CellExt::ExtContainer::InvalidateExtDataIgnorable(void* const ptr) const
{
	return true;
}

// =============================
// container hooks

DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);

	CellExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);

	CellExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	CellExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 5)
{
	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
{
	CellExt::ExtMap.SaveStatic();
	return 0;
}
