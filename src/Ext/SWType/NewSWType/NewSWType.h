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
#include <Ext/SWType/Body.h>
#include <SuperClass.h>
#include <HouseClass.h>

class NewSWType
{
public:

	static void Init();
	static void Clear();
	static int GetNewSWTypeIdx(const char* TypeID);
	static NewSWType* GetNthItem(int idx);

	virtual ~NewSWType() = default;

	virtual int GetTypeIndex() final;

	// selectable override

	virtual void Initialize(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) { }

	// must be override

	virtual const char* GetTypeID() = 0;
	virtual bool Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer) = 0;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

protected:
	virtual void SetTypeIndex(int idx) final;

private:
	static std::vector<std::unique_ptr<NewSWType>> Array;
	static void Register(std::unique_ptr<NewSWType> pType);

	int TypeIndex = -1;
};
