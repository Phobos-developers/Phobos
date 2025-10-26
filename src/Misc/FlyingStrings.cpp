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
#include "FlyingStrings.h"
#include <Phobos.h>
#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>
#include <Utilities/EnumFunctions.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;

bool FlyingStrings::DrawAllowed(CoordStruct& nCoords)
{
	if (auto const pCell = MapClass::Instance.TryGetCellAt(nCoords))
		return !(pCell->IsFogged() || pCell->IsShrouded());

	return false;
}

void FlyingStrings::Add(const wchar_t* text, const CoordStruct& coords, ColorStruct color, Point2D pixelOffset)
{
	Item item {};
	item.Location = coords;
	item.PixelOffset = pixelOffset;
	item.CreationFrame = Unsorted::CurrentFrame;
	item.Color = Drawing::RGB_To_Int(color);
	PhobosCRT::wstrCopy(item.Text, text, 0x20);
	Data.emplace_back(item);
}

void FlyingStrings::AddMoneyString(int amount, HouseClass* owner, AffectedHouse displayToHouses, const CoordStruct& coords, Point2D pixelOffset)
{
	if (amount
		&& (displayToHouses == AffectedHouse::All
			|| owner && EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer)))
	{
		bool isPositive = amount > 0;
		ColorStruct color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
		wchar_t moneyStr[0x20];
		swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
	}
}

void FlyingStrings::UpdateAll()
{
	if (Data.empty())
		return;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

		auto [point, visible] = TacticalClass::Instance->CoordsToClient(dataItem.Location);

		point += dataItem.PixelOffset;

		RectangleStruct bound = DSurface::Temp->GetRect();
		bound.Height -= 32;

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration - 70)
		{
			point.Y -= (Unsorted::CurrentFrame - dataItem.CreationFrame);
			DSurface::Temp->DrawText(dataItem.Text, &bound, &point, dataItem.Color, 0, TextPrintType::NoShadow);
		}
		else
		{
			DSurface::Temp->DrawText(dataItem.Text, &bound, &point, dataItem.Color, 0, TextPrintType::NoShadow);
		}

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration || Unsorted::CurrentFrame < dataItem.CreationFrame)
			Data.erase(Data.begin() + i);
	}
}
