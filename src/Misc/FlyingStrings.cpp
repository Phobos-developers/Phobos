#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;
int FlyingStrings::CurrentCycleWidth = -FlyingStrings::MaxCycleWidth;

bool FlyingStrings::DrawAllowed(CoordStruct &nCoords)
{
	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords))
	{
		if (pCell->IsFogged() || pCell->IsShrouded())
			return false;

		return true;
	}

	return false;
}

void FlyingStrings::Add(const wchar_t *text, CoordStruct coords, ColorStruct color, bool isCyclic)
{
	Item item{};
	item.Location = coords;

	item.CreationFrame = Unsorted::CurrentFrame;
	item.Color = Drawing::RGB2DWORD(color);
	PhobosCRT::wstrCopy(item.Text, text, 0x20);

	if (isCyclic)
	{
		if (CurrentCycleWidth >= MaxCycleWidth)
			CurrentCycleWidth = -MaxCycleWidth;

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(item.Text, &width, &height, MaxCycleWidth);
		item.XOffset = CurrentCycleWidth;
		CurrentCycleWidth += (width + 2);
	}

	Data.push_back(item);
}

void FlyingStrings::UpdateAll()
{
	if (Data.empty())
		return;

	for (auto const &dataItem : Data)
	{
		Point2D point;

		TacticalClass::Instance->CoordsToClient(dataItem.Location, &point);

		if (dataItem.XOffset != 0)
			point.X += dataItem.XOffset;

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration - 70)
		{
			point.Y -= (Unsorted::CurrentFrame - dataItem.CreationFrame);
			DSurface::Temp->DrawText(dataItem.Text, point.X, point.Y, dataItem.Color);
		}
		else
		{
			DSurface::Temp->DrawText(dataItem.Text, point.X, point.Y, dataItem.Color);
		}
	}

	auto const it = std::remove_if(Data.begin(), Data.end(), [](Item nItem) {
		return Unsorted::CurrentFrame > nItem.CreationFrame + Duration || Unsorted::CurrentFrame < nItem.CreationFrame;
		});

	if (it != Data.end())
		Data.erase(it);
}
