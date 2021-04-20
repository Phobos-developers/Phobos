#include "FoggedObject.h"

#include <TacticalClass.h>
#include <Surface.h>
#include <SmudgeClass.h>
#include <TerrainClass.h>
#include <Drawing.h>
#include <TiberiumClass.h>
#include <FootClass.h>

FoggedObject::~FoggedObject() = default;

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
		.Process(this->Location)
		.Process(this->Bound)
		.Success();
}

bool FoggedObject::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Location)
		.Process(this->Bound)
		.Success();
}

FoggedSmudge::~FoggedSmudge() = default;

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
			this->Bound.X - TacticalClass::Instance->TacticalPos0.X - rect.X + Drawing::SurfaceDimensions_Hidden.X + 30,
			this->Bound.Y - TacticalClass::Instance->TacticalPos0.Y - rect.Y + Drawing::SurfaceDimensions_Hidden.Y
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

FoggedTerrain::~FoggedTerrain() = default;

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

FoggedOverlay::~FoggedOverlay() = default;

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

FoggedBuilding::~FoggedBuilding()
{
}

void FoggedBuilding::Draw(RectangleStruct& rect)
{
}

BuildingTypeClass* FoggedBuilding::GetBuildingType()
{
	return this->Type;
}

bool FoggedBuilding::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return false;
}

bool FoggedBuilding::Save(PhobosStreamWriter& Stm) const
{
	return false;
}
