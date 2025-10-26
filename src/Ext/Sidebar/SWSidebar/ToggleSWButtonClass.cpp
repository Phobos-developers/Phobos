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
#include "ToggleSWButtonClass.h"
#include "SWSidebarClass.h"
#include <GameOptionsClass.h>
#include <ScenarioClass.h>

#include <Ext/Side/Body.h>

ToggleSWButtonClass::ToggleSWButtonClass(int x, int y, int width, int height)
	: GadgetClass(x, y, width, height, (GadgetFlag::LeftPress | GadgetFlag::LeftRelease), false)
{
	SWSidebarClass::Instance.ToggleButton = this;
}

bool ToggleSWButtonClass::Draw(bool forced)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array.Items[ScenarioClass::Instance->PlayerSideIndex]);
	const auto pTogglePCX = SWSidebarClass::IsEnabled() ? pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : pSideExt->SuperWeaponSidebar_OffPCX.GetSurface();

	if (!pTogglePCX)
		return false;

	RectangleStruct destRect { this->X, this->Y, this->Width, this->Height };
	PCX::Instance.BlitToSurface(&destRect, DSurface::Composite, pTogglePCX);

	if (this->IsHovering)
	{
		const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor);
		DSurface::Composite->DrawRect(&destRect, tooltipColor);
	}

	return true;
}

void ToggleSWButtonClass::OnMouseEnter()
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return;

	this->IsHovering = true;
	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}

void ToggleSWButtonClass::OnMouseLeave()
{
	this->IsHovering = false;
	this->IsPressed = false;
	MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}

bool ToggleSWButtonClass::Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier)
{
	auto& columns = SWSidebarClass::Instance.Columns;

	if (columns.empty())
		return false;

	const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array.Items[ScenarioClass::Instance->PlayerSideIndex]);

	if (SWSidebarClass::IsEnabled() ? !pSideExt->SuperWeaponSidebar_OnPCX.GetSurface() : !pSideExt->SuperWeaponSidebar_OffPCX.GetSurface())
		return false;

	if (flags & GadgetFlag::LeftPress)
		this->IsPressed = true;

	if ((flags & GadgetFlag::LeftRelease) && this->IsPressed)
	{
		this->IsPressed = false;
		ToggleSWButtonClass::SwitchSidebar();
	}

	this->GadgetClass::Action(flags, pKey, KeyModifier::None);
	return true;
}

void ToggleSWButtonClass::UpdatePosition()
{
	Point2D position = Point2D::Empty;
	auto& columns = SWSidebarClass::Instance.Columns;

	if (!columns.empty())
	{
		const auto backColumn = columns.back();
		position.X = SWSidebarClass::Instance.IsEnabled() ? backColumn->X + backColumn->Width : 0;
		position.Y = backColumn->Y + (backColumn->Height - this->Height) / 2;
	}
	else
	{
		position.X = 0;
		position.Y = (GameOptionsClass::Instance.ScreenHeight - this->Height) / 2;
	}

	this->SetPosition(position.X, position.Y);
}

bool ToggleSWButtonClass::SwitchSidebar()
{
	VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, 0x2000, 1.0);
	ScenarioExt::Global()->SWSidebar_Enable = !ScenarioExt::Global()->SWSidebar_Enable;

	const bool disabled = !SWSidebarClass::IsEnabled();
	const auto& columns = SWSidebarClass::Instance.Columns;

	if (!columns.empty())
	{
		for (const auto& pColumn : columns)
		{
			pColumn->Disabled = disabled;
			const auto& buttons = pColumn->Buttons;

			if (!buttons.empty())
			{
				for (const auto& pButton : buttons)
					pButton->Disabled = disabled;
			}
		}
	}

	if (const auto pToggle = SWSidebarClass::Instance.ToggleButton)
		pToggle->UpdatePosition();

	return !disabled;
}
