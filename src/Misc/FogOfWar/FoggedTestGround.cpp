//#include <Surface.h>
//#include <WWMouseClass.h>
//#include <CellClass.h>
//#include <MapClass.h>
//#include <TacticalClass.h>
//#include <CellClass.h>
//#include <BuildingClass.h>
//#include <FootClass.h>
//#include <OverlayClass.h>
//#include <SmudgeClass.h>
//#include <TerrainClass.h>
//#include <AnimClass.h>
//#include <OverlayTypeClass.h>
//
//#include "FoggedObject.h"
//
//void DrawTest(TerrainTypeClass* pType, CellClass* pCell)
//{
//	auto const pShape = pType->GetImage();
//	if (!pShape) return;
//	CoordStruct Location = pCell->GetCoords();
//	Point2D ptClient;
//	TacticalClass::Instance->CoordsToClient(Location, &ptClient);
//	// ptClient.X += DSurface::ViewBounds().X;
//	// ptClient.X += DSurface::ViewBounds().Y;
//
//	auto const nHeightOffset = -TacticalClass::Instance->AdjustForZ(Location.Z);
//	if (!pCell->LightConvert)
//		pCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
//
//	RectangleStruct Buffer;
//
//	// Draw
//	DSurface::Temp->DrawSHP(pCell->LightConvert, pShape, 0, &ptClient, DSurface::Temp->GetRect(&Buffer),
//		BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
//		0, nHeightOffset - 4, ZGradientDescIndex::Vertical, pCell->LightConvert->Color1.Green,
//		0, nullptr, 0, 0, 0);
//	// Draw shadow
//	DSurface::Temp->DrawSHP(pCell->LightConvert, pShape, pShape->Frames / 2, &ptClient, DSurface::Temp->GetRect(&Buffer),
//		BlitterFlags::ZReadWrite | BlitterFlags::Darken | BlitterFlags::bf_400 | BlitterFlags::Centered,
//		0, nHeightOffset - 2, ZGradientDescIndex::Flat, 1000, 0, nullptr, 0, 0, 0);
//}
//
//DEFINE_HOOK(4F4583, DrawGScreenPic, 6)
//{
//	Point2D ptMouse;
//	WWMouseClass::Instance->GetCoords(&ptMouse);
//	CoordStruct coord = TacticalClass::Instance->ClientToCoords(ptMouse);
//	CellClass* pCell = MapClass::Instance->TryGetCellAt(coord);
//
//	if (pCell)
//		DrawTest(TerrainTypeClass::Array->GetItem(14), pCell); // TREE01
//
//	return 0;
//}