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

FoggedObject::FoggedObject()
{
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
	this->OverlayData = this->AttachedCell->Powerup;

	FoggedOverlay::Instances.insert(this);
}

FoggedOverlay::FoggedOverlay(CellClass* pCell)
	: FoggedObject()
{
	this->Type = OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex);
	this->AttachedCell = pCell;
	this->OverlayData = pCell->Powerup;

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
	if (!TacticalClass::Instance->CoordsToClient(this->AttachedCell->GetCoords(), &ptClient))
		return false;
	ptClient.X -= 30;

	auto const pType = static_cast<OverlayTypeClass*>(this->Type);

	int nOldOverlay = this->AttachedCell->OverlayTypeIndex;
	unsigned char nOldOverlayData = this->AttachedCell->Powerup;
	
	this->AttachedCell->OverlayTypeIndex = pType->ArrayIndex;
	this->AttachedCell->Powerup = this->OverlayData;

	this->AttachedCell->DrawOverlay(ptClient, Bounds);
	this->AttachedCell->DrawOverlayShadow(ptClient, Bounds);

	this->AttachedCell->OverlayTypeIndex= nOldOverlay;
	this->AttachedCell->Powerup = nOldOverlayData;

	return true;
}

bool FoggedOverlay::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->OverlayData)
		.Success();
}

bool FoggedOverlay::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->OverlayData)
		.Success();
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
	if (!pShape)
		return false;

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

FoggedSmudge::FoggedSmudge(CellClass* pCell)
	: FoggedObject()
{
	this->Type = SmudgeTypeClass::Array->GetItem(pCell->SmudgeTypeIndex);
	this->AttachedCell = pCell;
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
	if (!pShape)
		return false;

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
	if (!pShape)
		return false;
	
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

FoggedBuilding::FoggedBuilding(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	auto const pBld = abstract_cast<BuildingClass*>(pObject);
	
	pBld->GetRenderDimensions(&this->Bound);
	this->Bound.X += TacticalClass::Instance->TacticalPos.X;
	this->Bound.Y += TacticalClass::Instance->TacticalPos.Y;
	
	TacticalClass::Instance->CoordsToClient(pBld->GetCoords(), &this->Position);
	this->Position.X += DSurface::ViewBounds().X - Bound.X;
	this->Position.Y += DSurface::ViewBounds().Y - Bound.Y;

	for (auto const pAnim : pBld->Anims)
		if (pAnim && !pAnim->IsVisible)
			this->Animations.push_back(GameCreate<FoggedAnim>(pAnim));

	auto const pBldType = static_cast<BuildingTypeClass*>(pBld->GetTechnoType());
	if (!pBldType->InvisibleInGame)
	{
		
	}

	FoggedBuilding::Instances.erase(this);
}

FoggedBuilding::~FoggedBuilding()
{
	FoggedBuilding::Instances.erase(this);
}

bool FoggedBuilding::DrawIt(RectangleStruct& Bounds) const
{
	return false;
}

bool FoggedBuilding::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->Position)
		.Process(this->Bound)
		.Process(this->Animations)
		.Success();
}

bool FoggedBuilding::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->Position)
		.Process(this->Bound)
		.Process(this->Animations)
		.Success();
}

/*
* Drawing
* 
*	// Following things are from BuildingClass::DrawAgain 
	ColorStruct SpecialColor;
	if (pBld->Airstrike)
		SpecialColor = RulesClass::Instance()->ColorAdd[RulesClass::Instance()->LaserTargetColor];
	else if (pBld->IsIronCurtained())
		SpecialColor = RulesClass::Instance()->ColorAdd[RulesClass::Instance()->IronCurtainColor];
	else if (pBld->ForceShielded)
		SpecialColor = RulesClass::Instance->ColorAdd[RulesClass::Instance()->ForceShieldColor]; 

	// I'm not quite sure what happened to SpecialColor while drawing, 
	// so we don't take them into consider for now - secsome
	
	// Don't take doors into consideration for now for it is too difficult to impl - secsome
	
	Point2D ptZShapeMove { 198,446 };
	bool bIsRoof = false;
	
	auto const eCurrentMission = pBld->GetCurrentMission();
	if (eCurrentMission == Mission::Unload)
		if (auto const pLink = pBld->RadioLinks[0])
			if (pLink->GetTechnoType()->JumpJet || pLink->GetTechnoType()->BalloonHover)
				bIsRoof = true;

	int nNormalZAdjust = pBldType->NormalZAdjust;
	SHPStruct* pSHP = nullptr;
	if (pBld->GetCurrentMission() == Mission::Unload)
		if (bIsRoof && pBldType->RoofDeployingAnim)
		{
			pSHP = pBldType->RoofDeployingAnim;
			nNormalZAdjust = -40;
		}
		else
		{
			pSHP = pBldType->DeployingAnim;
			nNormalZAdjust = -20;
		}

	if (eCurrentMission != Mission::Construction && eCurrentMission != Mission::Selling)
		ptZShapeMove += pBldType->ZShapePointMove;

	Point2D ptClient
	{
		(pBldType->GetFoundationHeight(false) << 8) - 256,
		(pBldType->GetFoundationWidth() << 8) - 256
	};
	
	Point2D ptAdjusted;
	TacticalClass::Instance()->AdjustForZShapeMove(&ptAdjusted, &ptClient);
	
	int nFrame = pBld->GetShapeNumber();
	if (nFrame >= pSHP->Frames / 2)
		nFrame = pSHP->Frames / 2;

	ptZShapeMove -= ptAdjusted;

	auto const nAdjustForHeight = TacticalClass::Instance()->AdjustForZ(pBld->Location.Z);
	auto const nBrightness = pBldType->ExtraLight + this->AttachedCell->Color1.R;

	if (Bounds.Height > 0) // pBounds->Height > 0
		pBld->DrawObject(pSHP, nFrame, &Position, &Bounds, 0, 0x100,
			nNormalZAdjust - nAdjustForHeight,
			ZGradientDescIndex::Vertical, 1, nBrightness, 0,
			pBldType->GetFoundationWidth() >= 8 ? nullptr : (SHPStruct*)0x89DDBC, 0, ptZShapeMove.X, ptZShapeMove.Y, 0);
	if (pBldType->BibShape && pBld->BState)
		pBld->DrawObject(pBldType->BibShape, pBld->GetShapeNumber(), &Position, &Bounds, 0, 0x100,
			-1 - nAdjustForHeight, ZGradientDescIndex::Flat, 1,
			nBrightness, 0, 0, 0, 0, 0, 0);
	
	if (eCurrentMission == Mission::Unload)
		pBld->DrawObject(bIsRoof ? pBldType->UnderRoofDoorAnim : pBldType->UnderDoorAnim, 
			pBld->Health / pBldType->Strength <= RulesClass::Instance()->ConditionYellow ? 1 : 0, &Position, &Bounds,
			0, 0x100, -nAdjustForHeight, ZGradientDescIndex::Flat, 1, nBrightness, 0, 0, 0, 0, 0, 0);
*/

#pragma endregion