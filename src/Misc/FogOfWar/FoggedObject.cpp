#include "FoggedObject.h"

#include <CellClass.h>
#include <BuildingClass.h>
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

#pragma region FoggedObject

FoggedObject::FoggedObject(ObjectClass* pObject)
{
	this->Type = pObject->GetType();
	this->AttachedCell = pObject->GetCell();
}

FoggedObject::~FoggedObject()
{

}

bool FoggedObject::IsValid() const
{
	return this->Type && this->AttachedCell;
}

bool FoggedObject::DrawIt() const
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

}

bool FoggedOverlay::DrawIt(RectangleStruct& const Bounds) const
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

}

bool FoggedTerrain::DrawIt() const
{
	return false;
}

#pragma endregion

#pragma region FoggedSmudge

FoggedSmudge::FoggedSmudge(ObjectClass* pObject)
	: FoggedObject(pObject)
{

}

bool FoggedSmudge::DrawIt() const
{
	return false;
}

#pragma endregion

#pragma region FoggedAnim

FoggedAnim::FoggedAnim(ObjectClass* pObject)
	: FoggedObject(pObject)
{
	auto const pAnim = abstract_cast<AnimClass*>(pObject);
	this->CurrentFrame = pAnim->Animation.Value;
}

bool FoggedAnim::DrawIt() const
{
	return false;
}

bool FoggedAnim::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Success();
}

bool FoggedAnim::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Type)
		.Process(this->AttachedCell)
		.Process(this->CurrentFrame)
		.Success();
}

#pragma endregion

#pragma region FoggedBuilding

// TO BE IMPLEMENTED

#pragma endregion