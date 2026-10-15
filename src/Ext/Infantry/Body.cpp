#include "Body.h"

#include <Ext/Cell/Body.h>
#include <Ext/InfantryType/Body.h>

InfantryExt::ExtContainer InfantryExt::ExtMap;

// Returns hardcoded prone/deployed FLH overrides for infantry, if set.
CoordStruct InfantryExt::GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	auto const pTypeExt = InfantryTypeExt::Fetch(pThis->Type);
	Nullable<CoordStruct> pickedFLH;

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
			pickedFLH = pTypeExt->DeployedPrimaryFireFLH;
		else if (weaponIndex == 1)
			pickedFLH = pTypeExt->DeployedSecondaryFireFLH;
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
				pickedFLH = pTypeExt->PronePrimaryFireFLH;
			else if (weaponIndex == 1)
				pickedFLH = pTypeExt->ProneSecondaryFireFLH;
		}
	}

	if (pickedFLH.isset())
	{
		FLH = pickedFLH.Get();
		FLHFound = true;
	}

	return FLH;
}

// A conversion into something that is not an infantry has to happen on the centre of the
// infantry's cell, but an infantry stands on a sub-position of it. Starts the walk there and
// remembers the conversion; returns true while the caller has to wait for it to finish
// (UpdatePendingConvert does that once the infantry has arrived).
//
// MoveTo cannot do the walk. The walk locomotor's AI (0x75AEC0) asks the pathfinder for a
// route first (FootClass::UpdatePathfinding, 0x4D3920) and a destination inside the infantry's
// own cell yields none, so it falls through to a distance test (0x75B046). A sub-position ->
// centre hop is only ~90 leptons, well under the "already arrived" threshold at
// Rules+0x1718, so the AI calls foot->vt[0x480](0, 1) and stops: the order is silently
// dropped.
//
// ILocomotion::Force_Immediate_Destination is the generic route to that routine, and it is a
// plain ILocomotion method rather than a WalkLocomotionClass speciality: it sits at vtable
// slot 0x78 for every locomotor (identified by the slot's stack-cleanup arity matching
// `__stdcall (CoordStruct)`, i.e. RET 0x10). The other ten locomotors share a no-op stub
// (0x55AC00, a bare RET 0x10), while the walk locomotor overrides it at 0x75AE30 with a call
// to FUN_0075C240 - the same "resolve this cell's sub-position and set HeadToCoord" routine
// the AI reaches at 0x75BC13.
//
// Note that YRpp annotates this method with "Don't use this function." (Interfaces.h). Its
// behaviour here is exactly what is needed and it does not set IsMoving, relying on the AI
// picking HeadToCoord up on the next frame, so it is worth in-game verification rather than
// assuming.
bool InfantryExt::PrepareConvertToType(InfantryClass* pThis, TechnoTypeClass* pToType)
{
	if (!pThis->Locomotor || !pToType)
		return false;

	if (!TechnoExt::CanConvertToType(pThis, pToType))
		return false;

	// Infantry to infantry keeps its sub-position: the new infantry simply takes over.
	if (pToType->WhatAmI() == AbstractType::InfantryType)
		return TechnoExt::ConvertToType(pThis, pToType);

	auto const pCell = MapClass::Instance.TryGetCellAt(pThis->GetCoords());

	if (!pCell)
		return false;

	if (pCell->OccupationFlags & 0x01)
		return false;

	auto const pExt = InfantryExt::Fetch(pThis);

	// Already walking there for a conversion that was ordered before this one; keep it, so the
	// cell's flag is only raised once for one walk.
	if (pExt->PendingConvertType)
		return true;

	pExt->PendingConvertType = pToType;
	pThis->PlayAnim(Sequence::Ready, true);
	auto const center = pCell->GetCenterCoords();
	auto const pCellExt = CellExt::Fetch(pCell);

	// CellClass::FindInfantrySubposition only yields the centre while InfantryCenterAllowed is
	// raised (see the hook in Ext/Cell/Hooks.cpp). It has to stay raised for the whole walk:
	// the AI re-enters FUN_0075C240 every frame (0x75BE18, with the Empty sentinel, so the
	// routine re-resolves the foot's current position) and would otherwise overwrite
	// HeadToCoord with a corner again. ClearPendingConvertToTypeAtCenter lowers it again.

	pCellExt->InfantryCenterAllowed++;
	pThis->Locomotor->Force_Immediate_Destination(center);
	pCellExt->InfantryCenterAllowed--;

	return true;
}

// Finishes a conversion that was deferred until the infantry reached the centre of its cell.
// The spot is checked again on arrival: the world may have changed while walking, and a
// conversion that cannot be done there fails instead of stacking two objects on one cell.
void InfantryExt::UpdatePendingConvert(InfantryClass* pThis)
{
	auto const pThisExt = InfantryExt::Fetch(pThis);
	auto const pToType = pThisExt->PendingConvertType;

	if (!pToType)
		return;

	// Still on the way. Force_Immediate_Destination does not set IsMoving, the locomotor does
	// that itself when it picks HeadToCoord up, so this only waits for an actual walk.
	if (pThis->Locomotor && pThis->Locomotor->Is_Moving())
		return;

	const bool canConvert = InfantryExt::CanConvertAtCellCenter(pThis)
		&& TechnoExt::CanConvertToType(pThis, pToType);		

	pThisExt->PendingConvertType = nullptr;

	if (canConvert)
	{
		TechnoExt::ConvertToType(pThis, pToType);
	}
	else if (pThis->Owner->IsCurrentPlayer() && (pThisExt->HasDeployConverted || pThisExt->HasUndeployConverted))
	{
		VoxClass::PlayIndex(VoxClass::FindIndex(GameStrings::EVA_CannotDeployHere));
	}
}

// Nobody else may stand on the spot the converted object needs.
bool InfantryExt::CanConvertAtCellCenter(InfantryClass* pThis)
{
	auto const pCell = MapClass::Instance.TryGetCellAt(pThis->GetCoords());

	if (!pCell)
		return false;

	for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
	{
		if (pObject != pThis && (pObject->AbstractFlags & AbstractFlags::Techno))
			return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void InfantryExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SkipTargetChangeResetSequence)
		.Process(this->HasDeployConverted)
		.Process(this->HasUndeployConverted)
		;
}

void InfantryExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") { }
InfantryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x517A60, InfantryClass_CTOR, 0xE)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK(0x517F81, InfantryClass_DTOR, 0x8)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Remove(pItem);

	return 0;
}
