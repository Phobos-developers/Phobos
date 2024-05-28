#include "Body.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/EnumFunctions.h>

#include <Ext/Rules/Body.h>

void BuildingTypeExt::DrawPrimaryIcon(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
//PrimaryFactoryIndicator_Palette

	SHPStruct* pImage = RulesExt::Global()->PrimaryFactoryIndicator;
	Point2D position = { pLocation->X + pImage->Width + 10, pLocation->Y - pImage->Height - 10 };
	DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pImage, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	return;
}
