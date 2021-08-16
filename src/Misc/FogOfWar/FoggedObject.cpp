#include "FoggedObject.h"

#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <TacticalClass.h>
#include <Surface.h>
#include <SmudgeClass.h>
#include <TerrainClass.h>
#include <Drawing.h>
#include <TiberiumClass.h>
#include <FootClass.h>
#include <HouseClass.h>

#include "FogOfWar.h"

#include <Ext/Cell/Body.h>
#include <Ext/AnimType/Body.h>

FoggedObject::FoggedObject(ObjectClass* pObject)
{
	pObject->GetCoords(&this->Location);
	pObject->GetRenderDimensions(&this->Bound);
	this->Bound.X += TacticalClass::Instance->TacticalPos.X;
	this->Bound.Y += TacticalClass::Instance->TacticalPos.Y;
	this->CoveredRTTIType = pObject->WhatAmI();
	this->Visible = true;

	FogOfWar::FoggedObjects.insert(this);

	auto const pExt = CellExt::ExtMap.Find(pObject->GetCell());
	pExt->FoggedObjects.push_back(this);
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
		.Process(this->Visible)
		.Success();
}

bool FoggedObject::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->CoveredRTTIType)
		.Process(this->Location)
		.Process(this->Bound)
		.Process(this->Visible)
		.Success();
}

FoggedSmudge::FoggedSmudge(CellClass* pCell, int smudge, unsigned char smudgeData)
{
	pCell->GetCoords(&this->Location);
	this->Location.Z = pCell->Level * 104;
	Point2D position;
	TacticalClass::Instance->CoordsToClient(this->Location, &position);

	this->Bound.X = position.X - 30;
	this->Bound.Y = position.Y - 15;
	this->Bound.Width = 60;
	this->Bound.Height = 30;

	this->Bound.X += TacticalClass::Instance->TacticalPos.X;
	this->Bound.Y += TacticalClass::Instance->TacticalPos.Y;

	this->Smudge = smudge;
	this->SmudgeData = smudgeData;
	this->CoveredRTTIType = AbstractType::Smudge;
	this->Visible = true;

	FogOfWar::FoggedObjects.insert(this);

	auto const pExt = CellExt::ExtMap.Find(pCell);
	pExt->FoggedObjects.push_back(this);
}

FoggedSmudge::~FoggedSmudge()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedSmudge::Draw(RectangleStruct& rect)
{
	auto const pSmudge = SmudgeTypeClass::Array->GetItem(this->Smudge);
	if (auto& pImage = pSmudge->Image)
	{
		auto const pCell = MapClass::Instance->GetCellAt(this->Location);
		if (!pCell->LightConvert)
		{
			pCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
		}

		Point2D position {
			this->Bound.X - TacticalClass::Instance->TacticalPos.X - rect.X + DSurface::ViewBounds->X + 30,
			this->Bound.Y - TacticalClass::Instance->TacticalPos.Y - rect.Y + DSurface::ViewBounds->Y
		};

		int nZAdjust = -TacticalClass::Instance->AdjustForZ(this->Location.Z);

		DSurface::Temp->DrawSHP(pCell->LightConvert, pImage, 0, &position, &rect,
			BlitterFlags(0xE00u), 0, nZAdjust, ZGradient::Deg90, pCell->Color1_Green, 0, nullptr, 0, 0, 0);
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

FoggedTerrain::FoggedTerrain(ObjectClass* pObject, int terrain, int frameIdx)
	: FoggedObject(pObject), Terrain { terrain }, FrameIndex { frameIdx } {
	this->Visible = true;
}

FoggedTerrain::~FoggedTerrain()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedTerrain::Draw(RectangleStruct& rect)
{
	auto const pType = TerrainTypeClass::Array->GetItem(this->Terrain);

	if (auto& pImage = pType->Image)
	{
		auto const pCell = MapClass::Instance->GetCellAt(this->Location);

		Point2D position;
		TacticalClass::Instance->CoordsToClient(this->Location, &position);
		position.X += DSurface::ViewBounds->X - rect.X;
		position.Y += DSurface::ViewBounds->Y - rect.Y;

		int nHeightOffset = -TacticalClass::Instance->AdjustForZ(this->Location.Z);
		ConvertClass* pDrawer;
		int nIntensity;

		if (pType->SpawnsTiberium)
		{
			pDrawer = FileSystem::GRFTXT_TIBERIUM_PAL;
			nIntensity = pCell->Color1_Red;
			position.Y -= 16;
		}
		else
		{
			if (!pCell->LightConvert)
			{
				pCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
			}
			pDrawer = pCell->LightConvert;
			nIntensity = pCell->Color1_Green;
		}

		//if (*reinterpret_cast<BYTE*>(0x822CF1))
		DSurface::Temp->DrawSHP(pDrawer, pImage, this->FrameIndex + pImage->Frames / 2, &position, &rect,
			BlitterFlags(0x4601u), 0, nHeightOffset - 3, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

		DSurface::Temp->DrawSHP(pDrawer, pImage, this->FrameIndex, &position, &rect,
			BlitterFlags(pType->IsAnimated ? 0x2E00u : 0x4E00u), 0, nHeightOffset - 12, ZGradient::Deg90, nIntensity, 0, nullptr, 0, 0, 0);
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
		.Process(this->FrameIndex)
		.Success();
}

bool FoggedTerrain::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Terrain)
		.Process(this->FrameIndex)
		.Success();
}

FoggedOverlay::FoggedOverlay(CellClass* pCell, int overlay, unsigned char overlayData)
{
	pCell->GetCoords(&this->Location);
	RectangleStruct buffer;
	pCell->ShapeRect(&this->Bound);
	pCell->GetContainingRect(&buffer);

	FogOfWar::UnionRectangle(&this->Bound, &buffer);
	this->Bound.X += TacticalClass::Instance->TacticalPos.X - DSurface::ViewBounds->X;
	this->Bound.Y += TacticalClass::Instance->TacticalPos.Y - DSurface::ViewBounds->Y;

	this->CoveredRTTIType = AbstractType::Overlay;
	this->Overlay = overlay;
	this->OverlayData = overlayData;
	this->Visible = true;

	FogOfWar::FoggedObjects.insert(this);

	auto const pExt = CellExt::ExtMap.Find(pCell);
	pExt->FoggedObjects.push_back(this);
}

FoggedOverlay::~FoggedOverlay()
{
	this->FoggedObject::~FoggedObject();
}

void FoggedOverlay::Draw(RectangleStruct& rect)
{
	auto const pCell = MapClass::Instance->GetCellAt(this->Location);
	CoordStruct coords = { (((pCell->MapCoords.X << 8) + 128) / 256) << 8, (((pCell->MapCoords.Y << 8) + 128) / 256) << 8, 0 };

	Point2D position;
	TacticalClass::Instance->CoordsToClient(coords, &position);
	position.X -= 30;

	int oldOverlay = this->Overlay;
	unsigned char oldOverlayData = this->OverlayData;

	pCell->OverlayTypeIndex = this->Overlay;
	pCell->Powerup = this->OverlayData;
	pCell->DrawOverlay(position, rect);
	pCell->DrawOverlayShadow(position, rect);

	pCell->OverlayTypeIndex = oldOverlay;
	pCell->Powerup = oldOverlayData;
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

FoggedBuilding::FoggedBuilding(BuildingClass* pBuilding, bool bVisible)
{
	this->Location = pBuilding->Location;
	this->CoveredRTTIType = AbstractType::Building;
	this->Visible = bVisible;

	this->Owner = pBuilding->Owner;
	this->Type = pBuilding->Type;
	this->FrameIndex = pBuilding->GetCurrentFrame();
	this->FireStormWall = pBuilding->Type->LaserFence;

	for (int i = 0; i < 21; ++i)
	{
		auto& pAnim = pBuilding->Anims[i];
		if (!pAnim || pAnim->Invisible)
		{
			continue;
		}

		auto& pAnimType = pAnim->Type;
		if (!pAnimType || !pAnimType->ShouldFogRemove)
		{
			continue;
		}

		pAnim->IsFogged = true;
		this->BuildingAnims.emplace_back(FoggedBuildingAnimStruct {});
		auto& item = this->BuildingAnims.back();

		item.AnimType = pAnimType;
		item.FrameIndex = pAnimType->Start + pAnim->Animation.Value;
		item.Visible = true;
		item.ZAdjust = pAnim->ZAdjust;
		item.YSort = pAnim->YSortAdjust;

		pAnim->GetRenderDimensions(&item.OriginalBound);
		TacticalClass::Instance->RegisterDirtyArea(item.OriginalBound, false);
		item.Bound.X = item.OriginalBound.X + TacticalClass::Instance->TacticalPos.X;
		item.Bound.Y = item.OriginalBound.Y + TacticalClass::Instance->TacticalPos.Y;
		item.Bound.Width = item.OriginalBound.Width;
		item.Bound.Height = item.OriginalBound.Height;
	}

	pBuilding->Deselect();
	pBuilding->IsFogged = true;

	pBuilding->GetRenderDimensions(&this->OriginalBound);
	TacticalClass::Instance->RegisterDirtyArea(this->OriginalBound, false);
	this->Bound.X = this->OriginalBound.X + TacticalClass::Instance->TacticalPos.X;
	this->Bound.Y = this->OriginalBound.Y + TacticalClass::Instance->TacticalPos.Y;
	this->Bound.Width = this->OriginalBound.Width;
	this->Bound.Height = this->OriginalBound.Height;

	auto const pExt = CellExt::ExtMap.Find(pBuilding->GetCell());
	pExt->FoggedObjects.push_back(this);
	FogOfWar::FoggedObjects.insert(this);
}

FoggedBuilding::~FoggedBuilding()
{
	TacticalClass::Instance->RegisterDirtyArea(this->OriginalBound, false);
	auto const pCell = MapClass::Instance->GetCellAt(this->Location);
	if (auto pBld = pCell->GetBuilding())
	{
		pBld->IsFogged = false;
		for (int i = 0; i < 21; ++i)
		{
			auto& pAnim = pBld->Anims[i];
			if (!pAnim)
			{
				continue;
			}
			pAnim->IsFogged = false;
		}
	}
	if (this->BuildingAnims.size())
	{
		for (auto& Anim : this->BuildingAnims)
		{
			TacticalClass::Instance->RegisterDirtyArea(Anim.OriginalBound, false);
		}
	}
	this->FoggedObject::~FoggedObject();
}

void FoggedBuilding::Draw(RectangleStruct& rect)
{
	auto& pType = this->Type;
	auto& pSHP = pType->Image;
	if (!pSHP || pType->InvisibleInGame)
	{
		return;
	}

	Point2D Pos;
	CoordStruct tempLoc = this->Location - CoordStruct { 128, 128, 0 };
	TacticalClass::Instance->CoordsToClient(tempLoc, &Pos);

	Pos.X += DSurface::ViewBounds->X - rect.X;
	Pos.Y += DSurface::ViewBounds->Y - rect.Y;

	Point2D ZShapeMove = {
		((pType->GetFoundationWidth(), 0) << 8) - 256,
		((pType->GetFoundationHeight(false), 0) << 8) - 256
	};
	Point2D ZShapeMove_Adjusted;
	TacticalClass::Instance->AdjustForZShapeMove(&ZShapeMove_Adjusted, &ZShapeMove);

	Point2D OffsetZ = -ZShapeMove_Adjusted + Point2D {
		pType->ZShapePointMove.X + 144,
		pType->ZShapePointMove.Y + 172
	};

	int BaseZAdjust = -TacticalClass::Instance->AdjustForZ(this->Location.Z) - 2;
	int ZAdjust = BaseZAdjust;
	if (!this->FireStormWall)
	{
		BaseZAdjust += pType->NormalZAdjust;
	}

	Point2D tempPos = Point2D { this->Location.X, this->Location.Y };
	CellClass* pCell = MapClass::Instance->GetTargetCell(tempPos);
	if (!pCell->LightConvert)
	{
		pCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
	}
	auto Brightness = pType->ExtraLight + pCell->Color1_Red;

	ConvertClass* pConvert = pType->TerrainPalette ?
		pCell->LightConvert : ColorScheme::Array->GetItem(this->Owner->ColorSchemeIndex)->LightConvert;

	RectangleStruct Bounds = rect;
	int nMaxBoundHeight = Pos.Y + pSHP->Height / 2;
	if (Bounds.Height > nMaxBoundHeight)
	{
		Bounds.Height = nMaxBoundHeight;
	}

	if (Bounds.Height > 0)
	{
		if (this->FireStormWall)
		{
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex + pSHP->Frames / 2, &Pos, &Bounds,
				BlitterFlags(0x4E01u), 0, BaseZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex, &Pos, &Bounds,
				BlitterFlags(0x4E00u), 0, ZAdjust, ZGradient::Ground, Brightness, 0, nullptr, 0, 0, 0);
		}
		else
		{
			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex + pSHP->Frames / 2, &Pos, &Bounds,
				BlitterFlags(0x4E01u), 0, BaseZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

			DSurface::Temp->DrawSHP(pConvert, pSHP, this->FrameIndex, &Pos, &Bounds,
				BlitterFlags(0x4E00u), 0, ZAdjust, ZGradient::Deg90, Brightness, 0, FileSystem::BUILDINGZ_SHA, 0, OffsetZ.X, OffsetZ.Y);
		}
	}
}

void FoggedBuilding::DrawBuildingAnim(FoggedBuildingAnimStruct& Anim, RectangleStruct& rect)
{
	auto& pAnimType = Anim.AnimType;
	auto& pImage = pAnimType->Image;
	if (!pImage)
	{
		return;
	}
	auto& pBldType = this->Type;
	if (pBldType->InvisibleInGame)
	{
		return;
	}

	Point2D Pos;
	CoordStruct tempLoc = this->Location - CoordStruct { 128, 128, 0 };
	TacticalClass::Instance->CoordsToClient(tempLoc, &Pos);

	Pos.X += DSurface::ViewBounds->X - rect.X;
	Pos.Y += DSurface::ViewBounds->Y - rect.Y + pAnimType->YDrawOffset;

	int nFrameIdx = std::min(Anim.FrameIndex, int(pImage->Frames - 1));
	BlitterFlags eFlag = BlitterFlags(0x4E00u);

	if (pAnimType->Translucent)
	{
		eFlag |= BlitterFlags::TransLucent50;
	}
	else
	{
		switch (pAnimType->Translucency)
		{
		case 25:
			eFlag |= BlitterFlags::TransLucent25;
			break;
		case 50:
			eFlag |= BlitterFlags::TransLucent50;
			break;
		case 75:
			eFlag |= BlitterFlags::TransLucent75;
			break;
		}
	}

	Point2D tempPos = Point2D { this->Location.X, this->Location.Y };
	CellClass* pCell = MapClass::Instance->GetTargetCell(tempPos);
	if (!pCell->LightConvert)
	{
		pCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
	}
	auto Brightness = pBldType->ExtraLight + pCell->Color1_Red;

	int BaseZAdjust = -TacticalClass::Instance->AdjustForZ(this->Location.Z) - 2;
	int ZAdjust = Anim.ZAdjust * 2 + pAnimType->YDrawOffset - TacticalClass::Instance->AdjustForZ(this->Location.Z + Anim.YSort) - 2;
	if (pAnimType->Flat)
	{
		--ZAdjust;
	}

	ConvertClass* pConvert = pBldType->TerrainPalette ?
		pCell->LightConvert : ColorScheme::Array->GetItem(this->Owner->ColorSchemeIndex)->LightConvert;

	if (pAnimType->UseNormalLight)
	{
		Brightness = FOGGED_BRIGHTNESS;
	}
	if (!pAnimType->ShouldUseCellDrawer)
	{
		auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnimType);
		if (auto convert = pAnimTypeExt->Palette.GetConvert())
		{
			pConvert = convert;
		}
		else
		{
			pConvert = FileSystem::ANIM_PAL;
			Brightness = FOGGED_BRIGHTNESS;
		}
	}

	if (pAnimType->Shadow)
	{
		DSurface::Temp->DrawSHP(pConvert, pImage, nFrameIdx + pImage->Frames / 2, &Pos, &rect,
			BlitterFlags(0x4E01u), 0, BaseZAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
	DSurface::Temp->DrawSHP(pConvert, pImage, nFrameIdx, &Pos, &rect,
		eFlag, 0, ZAdjust, pAnimType->Flat ? ZGradient::Ground : ZGradient::Deg90, Brightness, 0, nullptr, 0, 0, 0);
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
		.Process(this->OriginalBound)
		.Process(this->FrameIndex)
		.Process(this->FireStormWall)
		.Process(this->Visible)
		.Success();
}

bool FoggedBuilding::Save(PhobosStreamWriter& Stm) const
{
	return FoggedObject::Save(Stm) && Stm
		.Process(this->Owner)
		.Process(this->Type)
		.Process(this->OriginalBound)
		.Process(this->FrameIndex)
		.Process(this->FireStormWall)
		.Process(this->Visible)
		.Success();
}

bool FoggedBuilding::FoggedBuildingAnimStruct::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool FoggedBuilding::FoggedBuildingAnimStruct::Save(PhobosStreamWriter& stm) const
{
	return const_cast<FoggedBuildingAnimStruct*>(this)->Serialize(stm);
}

template <typename T>
bool FoggedBuilding::FoggedBuildingAnimStruct::Serialize(T& stm)
{
	return stm
		.Process(this->AnimType)
		.Process(this->OriginalBound)
		.Process(this->Visible)
		.Process(this->Bound)
		.Process(this->FrameIndex)
		.Process(this->ZAdjust)
		.Process(this->YSort)
		.Success();
}
