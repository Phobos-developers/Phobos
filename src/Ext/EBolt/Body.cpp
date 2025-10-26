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

#include <Ext/WeaponType/Body.h>

EBoltExt::ExtContainer EBoltExt::ExtMap;

EBolt* EBoltExt::CreateEBolt(WeaponTypeClass* pWeapon)
{
	const auto pBolt = GameCreate<EBolt>();
	const auto pBoltExt = EBoltExt::ExtMap.Find(pBolt);

	const int alternateIdx = pWeapon->IsAlternateColor ? 5 : 10;
	const int defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, alternateIdx);
	const int defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto& boltDisable = pWeaponExt->Bolt_Disable;
	const auto& boltColor = pWeaponExt->Bolt_Color;

	for (int idx = 0; idx < 3; ++idx)
	{
		if (boltDisable[idx])
			pBoltExt->Disable[idx] = true;
		else if (boltColor[idx].isset())
			pBoltExt->Color[idx] = boltColor[idx].Get();
		else
			pBoltExt->Color[idx] = Drawing::Int_To_RGB(idx < 2 ? defaultAlternate : defaultWhite);
	}

	pBoltExt->Arcs = pWeaponExt->Bolt_Arcs;
	pBolt->Lifetime = 1 << (std::clamp(pWeaponExt->Bolt_Duration.Get(), 1, 31) - 1);
	return pBolt;
}

// =============================
// load / save

template <typename T>
void EBoltExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Color)
		.Process(this->Disable)
		.Process(this->Arcs)
		.Process(this->BurstIndex)
		;
}

void EBoltExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<EBolt>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void EBoltExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<EBolt>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool EBoltExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool EBoltExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

void EBoltExt::ExtData::InvalidatePointer(void* ptr, bool removed)
{ }

// =============================
// container

EBoltExt::ExtContainer::ExtContainer() : Container("EBolt") { }
EBoltExt::ExtContainer::~ExtContainer() = default;

bool EBoltExt::ExtContainer::InvalidateExtDataIgnorable(void* const ptr) const
{
	return true;
}

// =============================
// container hooks

DEFINE_HOOK(0x4C1E42, EBolt_CTOR, 0x5)
{
	GET(EBolt*, pItem, EAX);

	EBoltExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pItem, ESI);

	EBoltExt::ExtMap.Remove(pItem);

	return 0;
}
