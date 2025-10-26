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
#include "NewSWType.h"

std::vector<std::unique_ptr<NewSWType>> NewSWType::Array;

void NewSWType::Register(std::unique_ptr<NewSWType> pType)
{
	//Ares capture positive, so we use negative...
	pType->SetTypeIndex(-static_cast<int>(Array.size() + 2));
	Array.emplace_back(std::move(pType));
}

void NewSWType::Init()
{
	if (!Array.empty())
		return;
}

void NewSWType::Clear()
{
	Array.clear();
}

int NewSWType::GetNewSWTypeIdx(const char* TypeID)
{
	for (const auto& pNewSWType : Array)
	{
		if (!_strcmpi(pNewSWType->GetTypeID(), TypeID))
			return pNewSWType->GetTypeIndex();
	}

	return -1;
}

NewSWType* NewSWType::GetNthItem(int idx)
{
	return Array[-idx - 2].get();
}

int NewSWType::GetTypeIndex()
{
	return this->TypeIndex;
}

void NewSWType::SetTypeIndex(int idx)
{
	this->TypeIndex = idx;
}

bool NewSWType::LoadGlobals(PhobosStreamReader& stm)
{
	Init();

	for (const auto& pNewSWType : Array)
	{
		stm.RegisterChange(pNewSWType.get());
	}

	return stm.Success();
}

bool NewSWType::SaveGlobals(PhobosStreamWriter& stm)
{
	for (const auto& pNewSWType : Array)
	{
		stm.Save(pNewSWType.get());
	}

	return stm.Success();
}
