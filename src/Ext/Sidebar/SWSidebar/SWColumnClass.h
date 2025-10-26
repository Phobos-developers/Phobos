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
#include <GadgetClass.h>

#include <vector>

class SWColumnClass : public GadgetClass
{
public:
	SWColumnClass() = default;
	SWColumnClass(int maxButtons, int x, int y, int width, int height);

	~SWColumnClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	bool AddButton(int superIdx);
	bool RemoveButton(int superIdx);
	void ClearButtons(bool remove = true);

	void SetHeight(int height);

	std::vector<SWButtonClass*> Buttons {};
	int MaxButtons { 0 };
};
