#include "FoggedObject.h"

#include <CellClass.h>
#include <BuildingClass.h>
#include <FootClass.h>
#include <OverlayClass.h>
#include <SmudgeClass.h>
#include <TerrainClass.h>
#include <AnimClass.h>

#include <TacticalClass.h>

#include <Helpers/Cast.h>
#include <Utilities/SavegameDef.h>

/*
* We don't use ": member{init}" like below for constructors to keep consistency except the base class init.
* FoggedObject::FoggedObject(ObjectTypeClass* pType, CellClass* pCell)
*	: Type { pType }, AttachedCell { pCell }
* - secsome
*/

#define RETURN_IF_FALSE(x) if(!x) return false;

#pragma region Instances

std::set<FoggedObject*> FoggedObject::Instances;
std::set<FoggedOverlay*> FoggedOverlay::Instances;
std::set<FoggedTerrain*> FoggedTerrain::Instances;
std::set<FoggedSmudge*> FoggedSmudge::Instances;
std::set<FoggedAnim*> FoggedAnim::Instances;
std::set<FoggedBuilding*> FoggedBuilding::Instances;

#pragma endregion

#pragma region FoggedObject

FoggedObject::FoggedObject(ObjectClass* pObject)
{
	this->Type = pObject->GetType();
	this->AttachedCell = pObject->GetCell();

	FoggedObject::Instances.insert(this);
}

FoggedObject::~FoggedObject()
{
	FoggedObject::Instances.erase(this);
}

bool FoggedObject::IsValid() const
{
	return this->Type && this->AttachedCell;
}

bool FoggedObject::DrawIt(RectangleStruct& Bounds) const
{
	return false;
}

bool FoggedObject::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Success();
}

bool FoggedObject::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Success();
}

#pragma endregion

#pragma region FoggedOverlay

FoggedOverlay::FoggedOverlay(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	FoggedOverlay::Instances.insert(this);
}

FoggedOverlay::~FoggedOverlay()
{
	FoggedOverlay::Instances.erase(this);
}

bool FoggedOverlay::DrawIt(RectangleStruct& Bounds) const
{
	// 4D19C4
	Point2D ptClient;
	RETURN_IF_FALSE(TacticalClass::Instance->CoordsToClient(this->AttachedCell->GetCoords(), &ptClient));
	ptClient.X -= 30;

	int nOldOverlay = this->AttachedCell->OverlayTypeIndex;
	unsigned char nOldOverlayData = this->AttachedCell->Powerup;
	
	this->AttachedCell->DrawOverlay(ptClient, Bounds);
	this->AttachedCell->DrawOverlayShadow(ptClient, Bounds);

	this->AttachedCell->OverlayTypeIndex= nOldOverlay;
	this->AttachedCell->Powerup = nOldOverlayData;

	return true;
}

#pragma endregion

#pragma region FoggedTerrain

FoggedTerrain::FoggedTerrain(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	FoggedTerrain::Instances.insert(this);
}

FoggedTerrain::~FoggedTerrain()
{
	FoggedTerrain::Instances.erase(this);
}

bool FoggedTerrain::DrawIt(RectangleStruct& Bounds) const
{
	auto const pType = static_cast<TerrainTypeClass*>(this->Type);
	auto const pShape = pType->GetImage();
	RETURN_IF_FALSE(pShape);

	CoordStruct Location = this->AttachedCell->GetCoords();
	Point2D ptClient;
	TacticalClass::Instance->CoordsToClient(Location, &ptClient);
	ptClient.X += DSurface::ViewBounds().X - Bounds.X;
	ptClient.X += DSurface::ViewBounds().Y - Bounds.Y;

	auto const nHeightOffset = -TacticalClass::Instance->AdjustForZ(Location.Z);
	if (!this->AttachedCell->LightConvert)
		this->AttachedCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);
	
	// Draw
	DSurface::Temp->DrawSHP(this->AttachedCell->LightConvert, pShape, 0, &ptClient, &Bounds,
		BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
		0, nHeightOffset - 4, ZGradientDescIndex::Vertical, this->AttachedCell->LightConvert->Color1.Green,
		0, nullptr, 0, 0, 0);
	// Draw shadow
	DSurface::Temp->DrawSHP(this->AttachedCell->LightConvert, pShape, pShape->Frames / 2, &ptClient, &Bounds,
		BlitterFlags::ZReadWrite | BlitterFlags::Darken | BlitterFlags::bf_400 | BlitterFlags::Centered,
		0, nHeightOffset - 2, ZGradientDescIndex::Flat, 1000, 0, nullptr, 0, 0, 0);

	return true;
}

#pragma endregion

#pragma region FoggedSmudge

FoggedSmudge::FoggedSmudge(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	this->CurrentFrame = 0; // Dunno how to init it, set it to zero currently

	FoggedSmudge::Instances.insert(this);
}

FoggedSmudge::~FoggedSmudge()
{
	FoggedSmudge::Instances.erase(this);
}

bool FoggedSmudge::DrawIt(RectangleStruct& Bounds) const
{
	auto const pType = static_cast<TerrainTypeClass*>(this->Type);
	auto const pShape = pType->GetImage();
	RETURN_IF_FALSE(pShape);

	if (!this->AttachedCell->LightConvert)
		this->AttachedCell->InitLightConvert(0, 0x10000, 0, 1000, 1000, 1000);

	Point2D ptClient;
	TacticalClass::Instance->CoordsToClient(this->AttachedCell->GetCoords(), &ptClient);

	Point2D ptPos
	{
		ptClient.X - TacticalClass::Instance->TacticalPos.X + DSurface::ViewBounds().X - Bounds.X ,
		ptClient.Y - TacticalClass::Instance->TacticalPos.Y + DSurface::ViewBounds().Y - Bounds.Y - 15
	};

	auto const nHeightOffset = -TacticalClass::Instance->AdjustForZ(this->AttachedCell->GetCoords().Z);

	DSurface::Temp->DrawSHP(this->AttachedCell->LightConvert, pShape, this->CurrentFrame, &ptPos, &Bounds,
		BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered, 0, nHeightOffset,
		ZGradientDescIndex::Vertical, this->AttachedCell->LightConvert->Color1.Green,
		0, nullptr, 0, 0, 0);

	return true;
}

bool FoggedSmudge::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Success();
}

bool FoggedSmudge::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Success();
}

#pragma endregion

#pragma region FoggedAnim

FoggedAnim::FoggedAnim(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	auto const pAnim = abstract_cast<AnimClass*>(pObject);
	this->CurrentFrame = pAnim->Animation.Value;
	this->Location = pAnim->Location;

	FoggedAnim::Instances.erase(this);
}

FoggedAnim::~FoggedAnim()
{
	FoggedAnim::Instances.erase(this);
}

bool FoggedAnim::DrawIt(RectangleStruct& Bounds) const
{
	auto const pType = static_cast<AnimTypeClass*>(this->Type);
	auto const pShape = pType->GetImage();
	RETURN_IF_FALSE(pShape);
	
	Point2D ptClient;
	TacticalClass::Instance->CoordsToClient(this->Location, &ptClient);
	
	auto const nHeightOffset = -TacticalClass::Instance->AdjustForZ(this->AttachedCell->GetCoords().Z); // not sure
	auto const eZGradientDescIndex = pType->Flat ? ZGradientDescIndex::Flat : ZGradientDescIndex::Vertical;
	auto const pDrawer = pType->ShouldUseCellDrawer ? this->AttachedCell->LightConvert : FileSystem::ANIM_PAL();
	// Load CustomPalette if exists

	auto const nIntensity = pType->UseNormalLight ? 1000 : this->AttachedCell->LightConvert->Color1.Red;

	DSurface::Temp->DrawSHP(pDrawer, pShape, this->CurrentFrame, &ptClient, &Bounds,
		BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered,
		0, nHeightOffset, eZGradientDescIndex, nIntensity, 0, nullptr, 0, 0, 0);

	if (pType->Shadow)
		DSurface::Temp->DrawSHP(pDrawer, pShape, this->CurrentFrame + pShape->Frames / 2, &ptClient, &Bounds,
			BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered | BlitterFlags::Darken,
			0, nHeightOffset, ZGradientDescIndex::Vertical, 1000, 0, nullptr, 0, 0, 0);

	return true;
}

bool FoggedAnim::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Process(this->Location)
		.Success();
}

bool FoggedAnim::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Process(this->Location)
		.Success();
}

#pragma endregion

#pragma region FoggedBuilding

// TO BE IMPLEMENTED

#pragma endregion