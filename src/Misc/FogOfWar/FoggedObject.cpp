#include "FoggedObject.h"

#include <TacticalClass.h>
#include <Surface.h>
#include <SmudgeClass.h>
#include <TerrainClass.h>
#include <Drawing.h>
#include <TiberiumClass.h>
#include <FootClass.h>
#include <HouseClass.h>

#include "FogOfWar.h"

FoggedObject::FoggedObject(AbstractType rtti, CoordStruct& location, RectangleStruct& bound)
	: CoveredRTTIType{ rtti }, Location{ location }, Bound{ bound }, Translucent{ true }
{
	auto const pCell = MapClass::Instance->TryGetCellAt(location);
	if (pCell)
	{
		FogOfWar::FoggedObjects.insert(this);

		auto const pExt = CellExt::ExtMap.Find(pCell);
		pExt->FoggedObjects.push_back(this);
	}
}

FoggedObject::FoggedObject(ObjectClass* pObject)
{
	auto const pCell = pObject->GetCell();
	if (pCell)
	{
		pCell->GetCoords(&this->Location);
		pObject->vt_entry_12C(&this->Bound); // __get_render_dimensions
		this->Bound.X += TacticalClass::Instance->VisibleArea.X;
		this->Bound.Y += TacticalClass::Instance->VisibleArea.Y;
		this->CoveredRTTIType = pObject->WhatAmI();
		this->Translucent = true;

		FogOfWar::FoggedObjects.insert(this);

		auto const pExt = CellExt::ExtMap.Find(pCell);
		pExt->FoggedObjects.push_back(this);
	}
}

FoggedObject::~FoggedObject()
{
	FogOfWar::FoggedObjects.erase(this);
}

int FoggedObject::GetType()
{
	return -1;
}

BuildingTypeClass* FoggedObject::GetBuildingType()
{
	return nullptr;
}

bool FoggedObject::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->CoveredRTTIType)
		.Process(this->Location)
		.Process(this->Bound)
		.Success();
}

bool FoggedObject::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->CoveredRTTIType)
		.Process(this->Location)
		.Process(this->Bound)
		.Success();
}

FoggedSmudge::FoggedSmudge(CoordStruct& location, RectangleStruct& bound, int smudge)
	: FoggedObject(AbstractType::Smudge, location, bound), Smudge{ smudge }
{
}

FoggedSmudge::FoggedSmudge(CellClass* pCell, int smudge, unsigned char smudgeData)
{
	pCell->GetCoords(&this->Location);
	
	Point2D position;
	TacticalClass::Instance->CoordsToClient(&this->Location, &position);
	
	this->Bound.X = position.X - 30;
	this->Bound.Y = position.Y - 15;
	this->Bound.Width = 60;
	this->Bound.Height = 30;

	this->Bound.X += TacticalClass::Instance->VisibleArea.X;
	this->Bound.Y += TacticalClass::Instance->VisibleArea.Y;

	this->Smudge = smudge;
	this->SmudgeData = smudgeData;
	this->CoveredRTTIType = AbstractType::Smudge;

	FogOfWar::FoggedObjects.insert(this);

	auto const pExt = CellExt::ExtMap.Find(pCell);
	pExt->FoggedObjects.push_back(this);
}

FoggedSmudge::~FoggedSmudge()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedSmudge::Draw(RectangleStruct & rect)
{
	auto const pSmudge = SmudgeTypeClass::Array->GetItem(this->Smudge);
	if (auto const pShapeData = pSmudge->GetImage())
	{
		auto const pCell = MapClass::Instance->TryGetCellAt(this->Location);
		if (!pCell->LightConvert)
			pCell->InitDrawer(0, 0x10000, 0, 1000, 1000, 1000);

		Point2D position
		{
			this->Bound.X - TacticalClass::Instance->VisibleArea.X - rect.X + Drawing::SurfaceDimensions_Hidden.X + 30,
			this->Bound.Y - TacticalClass::Instance->VisibleArea.Y - rect.Y + Drawing::SurfaceDimensions_Hidden.Y
		};

		auto nZAdjust = TacticalClass::Instance->AdjustForZ(this->Location.Z);

		DSurface::Temp->DrawSHP(pCell->LightConvert, pShapeData, 0, &position, &rect,
			BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered, 0, -nZAdjust, 2, pCell->Color1_Green, 0, 0, 0, 0, 0);
	}
}

int FoggedSmudge::GetType()
{
	return this->Smudge;
}

BuildingTypeClass* FoggedSmudge::GetBuildingType()
{
	return nullptr;
}

bool FoggedSmudge::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return FoggedObject::Load(Stm, RegisterForChange) && Stm
		.Process(this->Smudge)
		.Success();
}

bool FoggedSmudge::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Smudge)
		.Success();
}

FoggedTerrain::FoggedTerrain(CoordStruct& location, RectangleStruct& bound, int terrain)
	: FoggedObject(AbstractType::Terrain, location, bound), Terrain{ terrain }
{
}

FoggedTerrain::FoggedTerrain(ObjectClass* pObject, int terrain)
	: FoggedObject(pObject), Terrain{ terrain }
{
}

FoggedTerrain::~FoggedTerrain()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedTerrain::Draw(RectangleStruct & rect)
{
	auto const pTerrain = TerrainTypeClass::Array->GetItem(this->Terrain);
	if (auto const pShapeData = pTerrain->GetImage())
	{
		auto const pCell = MapClass::Instance->TryGetCellAt(this->Location);
		
		Point2D position;
		TacticalClass::Instance->CoordsToClient(&this->Location, &position);
		position.X += Drawing::SurfaceDimensions_Hidden.X - rect.X;
		position.X += Drawing::SurfaceDimensions_Hidden.Y - rect.Y;

		auto nHeightOffset = -TacticalClass::Instance->AdjustForZ(this->Location.Z);
		ConvertClass* pDrawer;
		int nIntensity;

		if (pTerrain->SpawnsTiberium)
		{
			pDrawer = FileSystem::GRFTXT_TIBERIUM_PAL;
			nIntensity = pCell->Color1_Red;
			position.Y -= 16;
		}
		else
		{
			if (!pCell->LightConvert)
				pCell->InitDrawer(0, 0x10000, 0, 1000, 1000, 1000);
			pDrawer = pCell->LightConvert;
			nIntensity = pCell->Color1_Green;
		}

		// use BF_FLAT(2000)|BF_ALPHA|BF_400|BF_CENTER if animated 
		// or just BF_USE_ZBUFFER|BF_ALPHA|BF_400|BF_CENTER
		DSurface::Temp->DrawSHP(pDrawer, pShapeData, 0, &position, &rect,
			static_cast<BlitterFlags>(pTerrain->IsAnimated ? 0x2E00 : 0x4E00), 0, nHeightOffset - 12, 2, nIntensity, 0, 0, 0, 0, 0);

		if (!pCell->LightConvert)
			pCell->InitDrawer(0, 0x10000, 0, 1000, 1000, 1000);
		pDrawer = pCell->LightConvert;

		// BF_USE_ZBUFFER|BF_ALPHA|BF_400|BF_CENTER|BF_DARKEN
		DSurface::Temp->DrawSHP(pDrawer, pShapeData, pShapeData->Frames / 2, &position, &rect,
			(BlitterFlags)0x4E01, 0, nHeightOffset - 3, 0, 1000, 0, 0, 0, 0, 0);
	}
}

int FoggedTerrain::GetType()
{
	return this->Terrain;
}

BuildingTypeClass* FoggedTerrain::GetBuildingType()
{
	return nullptr;
}

bool FoggedTerrain::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return FoggedObject::Load(Stm, RegisterForChange) && Stm
		.Process(this->Terrain)
		.Success();
}

bool FoggedTerrain::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Terrain)
		.Success();
}

FoggedOverlay::FoggedOverlay(CoordStruct& location, RectangleStruct& bound, int overlay, unsigned char overlayData)
	: FoggedObject(AbstractType::Overlay, location, bound), Overlay{ overlay }, OverlayData{ overlayData }
{
}

FoggedOverlay::FoggedOverlay(CellClass* pCell, int overlay, unsigned char overlayData)
{
	pCell->GetCoords(&this->Location);
	RectangleStruct buffer;
	pCell->ShapeRect(&this->Bound);
	pCell->GetContainingRect(&buffer);

	FogOfWar::UnionRectangle(&this->Bound, &buffer);
    this->Bound.X += TacticalClass::Instance->VisibleArea.X - Drawing::SurfaceDimensions_Hidden.X;
	this->Bound.Y += TacticalClass::Instance->VisibleArea.Y - Drawing::SurfaceDimensions_Hidden.Y;

	this->CoveredRTTIType = AbstractType::Overlay;
	this->Overlay = overlay;
	this->OverlayData = overlayData;

	FogOfWar::FoggedObjects.insert(this);

	auto const pExt = CellExt::ExtMap.Find(pCell);
	pExt->FoggedObjects.push_back(this);
}

FoggedOverlay::~FoggedOverlay()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedOverlay::Draw(RectangleStruct & rect)
{
	auto const pCell = MapClass::Instance->TryGetCellAt(this->Location);

	Point2D position;
	TacticalClass::Instance->CoordsToClient(&this->Location, &position);
	position.X -= 30;

	int oldOverlay = this->Overlay;
	unsigned char oldOverlayData = this->OverlayData;

	pCell->OverlayTypeIndex = this->Overlay;
	pCell->OverlayData = this->OverlayData;
	pCell->DrawOverlay(&position, &rect);
	pCell->DrawOverlayShadow(&position, &rect);

	pCell->OverlayTypeIndex = oldOverlay;
	pCell->OverlayData = oldOverlayData;
}

int FoggedOverlay::GetType()
{
	return this->Overlay;
}

BuildingTypeClass* FoggedOverlay::GetBuildingType()
{
	return nullptr;
}

bool FoggedOverlay::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return FoggedObject::Load(Stm, RegisterForChange) && Stm
		.Process(this->Overlay)
		.Process(this->OverlayData)
		.Success();
}

bool FoggedOverlay::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Overlay)
		.Process(this->OverlayData)
		.Success();
}

FoggedBuilding::FoggedBuilding(CoordStruct& location, RectangleStruct& bound, BuildingClass* pBuilding, bool bTranslucent)
	: FoggedObject(AbstractType::Building, location, bound)
{
	this->Owner = pBuilding->Owner;
	this->Type = pBuilding->Type;
	this->FrameIndex = pBuilding->CurrentFrame();
	this->FireStormWall = pBuilding->Type->LaserFence;
	this->Translucent = bTranslucent;

	// MORE!
}

FoggedBuilding::FoggedBuilding(BuildingClass* pObject, bool bTranslucent)
	: FoggedObject(pObject)
{
	this->Owner = pObject->Owner;
	this->Type = pObject->Type;
	this->FrameIndex = pObject->CurrentFrame();
	this->FireStormWall = pObject->Type->LaserFence;
	this->Translucent = bTranslucent;
}

FoggedBuilding::~FoggedBuilding()
{
	this->FoggedObject::~FoggedObject();
}

// By AutoGavy
void FoggedBuilding::Draw(RectangleStruct & rect)
{
	auto pType = this->Type;
	auto pSHP = pType->GetImage();
	if (!pSHP || pType->InvisibleInGame)
		return;

	Point2D vPos;
	CoordStruct tempLoc = this->Location - CoordStruct{ 128, 128, 0 };
	TacticalClass::Instance->CoordsToClient(&tempLoc, &vPos);

	vPos.X += Drawing::SurfaceDimensions_Hidden.X - this->Bound.X;
	vPos.Y += Drawing::SurfaceDimensions_Hidden.Y - this->Bound.Y;

	Point2D vZShapeMove = {
		((pType->GetFoundationWidth(), 0) << 8) - 256,
		((pType->GetFoundationHeight(false), 0) << 8) - 256
	};
	Point2D buffer = vZShapeMove;
	TacticalClass::Instance->AdjustForZShapeMove(&vZShapeMove, &buffer);

	Point2D vOffsetZ = Point2D{
		pType->ZShapePointMove.X + 144,
		pType->ZShapePointMove.Y + 172
	} - vZShapeMove;
	int nZAdjust = TacticalClass::Instance->AdjustForZ(this->Location.Z);

	Point2D tempPos = Point2D{ this->Location.X, this->Location.Y };
	CellClass* const pCell = MapClass::Instance->GetTargetCell(&tempPos);
	if (!pCell->LightConvert)
		pCell->InitDrawer(0, 0x10000, 0, 1000, 1000, 1000);

	ConvertClass* const pConvert = pType->TerrainPalette ?
		pCell->LightConvert : (*ColorScheme::Array)[this->Owner->ColorSchemeIndex]->LightConvert;

	RectangleStruct vBounds = this->Bound;
	if (vBounds.Width > vPos.Y)
		vBounds.Width = vPos.Y;

	BlitterFlags eFlag = static_cast<BlitterFlags>(0x4E00u);
	if (vBounds.Width > 0) {
		if (this->FireStormWall) {
			// draw shadow
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex + pSHP->Frames / 2, &vPos, &vBounds,
				eFlag | BlitterFlags::Darken, 0, -nZAdjust, 0, 1000, 0, nullptr, 0, 0, 0);
			// draw main
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex, &vPos, &vBounds,
				eFlag, 0, -nZAdjust, 0, DWORD(pType->ExtraLight + pCell->Color1_Red), 0, nullptr, 0, 0, 0);
		}
		else {
			// draw shadow
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex + pSHP->Frames / 2, &vPos, &vBounds,
				eFlag | BlitterFlags::Darken, 0, -nZAdjust, 0, 1000, 0, nullptr, 0, 0, 0);
			// draw main
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex, &vPos, &vBounds,
				eFlag, 0, nZAdjust + pType->NormalZAdjust - 2, 0, DWORD(pType->ExtraLight + pCell->Color1_Red), 0, nullptr, 0, vOffsetZ.X, vOffsetZ.Y);
		}
	}
}

int FoggedBuilding::GetType()
{
	return -1;
}

BuildingTypeClass* FoggedBuilding::GetBuildingType()
{
	return this->Type;
}

bool FoggedBuilding::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return FoggedObject::Load(Stm, RegisterForChange) && Stm
		.Process(this->Owner)
		.Process(this->Type)
		.Process(this->FrameIndex)
		.Process(this->FireStormWall)
		.Process(this->Translucent)
		.Success();
}

bool FoggedBuilding::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Owner)
		.Process(this->Type)
		.Process(this->FrameIndex)
		.Process(this->FireStormWall)
		.Process(this->Translucent)
		.Success();
}
