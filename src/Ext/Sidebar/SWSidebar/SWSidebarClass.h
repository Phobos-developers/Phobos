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
#include "SWButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"
#include <Ext/Scenario/Body.h>
#include <CommandClass.h>

class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void InitClear();
	void InitIO();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();
	static void RecheckCameo();

	static SWSidebarClass Instance;

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	SWButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };
	bool DisableEntry { false };

	static CommandClass* Commands[10];
};
