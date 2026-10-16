#include "FlyingStrings.h"
#include <Phobos.h>
#include <Phobos.CRT.h>
#include <BitFont.h>
#include <Utilities/EnumFunctions.h>
#include <New/Type/ResourceTypeClass.h>

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
	PhobosCRT::wstrCopy(item.Text, text, 0x40);
	Data.emplace_back(item);
}

void FlyingStrings::AddMoneyString(int amount, ObjectClass* pSource, HouseClass* pOwner,
	AffectedHouse displayToHouses, const CoordStruct& coords, Point2D pixelOffset)
{
	if (amount == 0 || MapClass::Instance.IsLocationShrouded(coords))
		return;

	if (displayToHouses != AffectedHouse::All && !EnumFunctions::CanTargetHouse(displayToHouses, pOwner, HouseClass::CurrentPlayer))
		return;

	if (pSource && pSource->VisualCharacter(false, nullptr) == VisualType::Hidden)
		return;

	const bool isPositive = amount > 0;
	const ColorStruct color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
	wchar_t moneyStr[0x20];
	swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));

	int width = 0, height = 0;
	if (BitFont::Instance)
	{
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);
	}

	FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
}

void FlyingStrings::AddResourceString(const ResourceTypeClass* pResource, int amount, ObjectClass* pSource, HouseClass* pOwner,
	AffectedHouse displayToHouses, const CoordStruct& coords, Point2D pixelOffset, const ColorStruct* pColorOverride)
{
	if (!pResource || amount == 0 || MapClass::Instance.IsLocationShrouded(coords))
		return;

	if (displayToHouses != AffectedHouse::All && !EnumFunctions::CanTargetHouse(displayToHouses, pOwner, HouseClass::CurrentPlayer))
		return;

	if (pSource && pSource->VisualCharacter(false, nullptr) == VisualType::Hidden)
		return;

	const bool isPositive = amount > 0;
	ColorStruct color;
	if (pColorOverride && *pColorOverride != ColorStruct { 0, 0, 0 })
		color = *pColorOverride;
	else
	{
		const ColorStruct resColor = pResource->Display_Color.Get();
		color = (resColor != ColorStruct { 0, 0, 0 }) ? resColor : (isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 });
	}

	wchar_t resStr[0x20];
	const wchar_t* label = pResource->Display_Label.Get();
	const bool useSpace = pResource->Display_Label_UseSpace.Get();
	if (label && *label)
	{
		if (pResource->Display_Label_InvertPosition.Get())
		{
			swprintf_s(resStr, useSpace ? L"%ls%d %ls" : L"%ls%d%ls", isPositive ? L"+" : L"-", std::abs(amount), label);
		}
		else
		{
			swprintf_s(resStr, useSpace ? L"%ls%ls %d" : L"%ls%ls%d", isPositive ? L"+" : L"-", label, std::abs(amount));
		}
	}
	else
	{
		swprintf_s(resStr, L"%ls%d", isPositive ? L"+" : L"-", std::abs(amount));
	}

	int width = 0, height = 0;
	if (BitFont::Instance)
	{
		BitFont::Instance->GetTextDimension(resStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);
	}

	FlyingStrings::Add(resStr, coords, color, pixelOffset);
}

void FlyingStrings::UpdateAll()
{
	if (Data.empty() || !TacticalClass::Instance || !DSurface::Temp)
		return;

	for (int i = static_cast<int>(Data.size()) - 1; i >= 0; --i)
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
