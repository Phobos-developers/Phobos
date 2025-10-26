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

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class InsigniaTypeClass final : public Enumerable<InsigniaTypeClass>
{
public:
	Promotable<SHPStruct*> Insignia;
	Promotable<int> InsigniaFrame;

	InsigniaTypeClass(const char* const pTitle) : Enumerable<InsigniaTypeClass>(pTitle)
		, Insignia { }
		, InsigniaFrame { -1 }
	{ }

	void LoadFromINI(CCINIClass* pINI);
	// No need to save and load as it's only for parsing
};
